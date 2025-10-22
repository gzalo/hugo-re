#!/usr/bin/env python3
"""
Audio Server for Hugo Re-implementation
Listens for JSON UDP packages to play and stop audio files.
"""

import argparse
import json
import os
import socket
import threading
import time
from typing import Dict, Optional
import uuid

import sounddevice as sd
import soundfile as sf
import numpy as np


class AudioInstance:
    """Represents a playing audio instance."""
    
    def __init__(self, instance_id: str, data: np.ndarray, samplerate: int, 
                 loops: int, volume: float):
        self.instance_id = instance_id
        self.data = data
        self.samplerate = samplerate
        self.loops = loops
        self.volume = volume
        self.current_loop = 0
        self.current_frame = 0
        self.is_playing = True
        self.fade_duration = 0.0
        self.fade_start_time = None
        self.original_volume = volume
        
    def get_next_frames(self, num_frames: int) -> Optional[np.ndarray]:
        """Get the next frames to play."""
        if not self.is_playing:
            return None
            
        # Handle fade out
        if self.fade_start_time is not None:
            elapsed = time.time() - self.fade_start_time
            if elapsed >= self.fade_duration:
                self.is_playing = False
                return None
            fade_factor = 1.0 - (elapsed / self.fade_duration)
            current_volume = self.original_volume * fade_factor
        else:
            current_volume = self.volume
        
        frames_remaining = len(self.data) - self.current_frame
        frames_to_read = min(num_frames, frames_remaining)
        
        if frames_to_read == 0:
            # End of current loop
            if self.loops == -1 or self.current_loop < self.loops:
                self.current_frame = 0
                self.current_loop += 1
                return self.get_next_frames(num_frames)
            else:
                self.is_playing = False
                return None
        
        # Get frames and apply volume
        frames = self.data[self.current_frame:self.current_frame + frames_to_read]
        self.current_frame += frames_to_read
        
        # Apply volume
        frames = frames * current_volume
        
        # If we need more frames to fill the buffer, pad with zeros
        if frames_to_read < num_frames:
            padding = np.zeros((num_frames - frames_to_read, frames.shape[1] if len(frames.shape) > 1 else 1))
            if len(frames.shape) == 1:
                frames = frames.reshape(-1, 1)
            frames = np.vstack([frames, padding])
        
        return frames
    
    def stop(self, fade_duration: float = 0.0):
        """Stop the instance with optional fade."""
        if fade_duration > 0:
            self.fade_duration = fade_duration
            self.fade_start_time = time.time()
        else:
            self.is_playing = False


class AudioServer:
    """Audio server that handles UDP JSON commands."""
    
    def __init__(self, host: str, port: int, assets_path: str, device: Optional[str] = None):
        self.host = host
        self.port = port
        self.assets_path = os.path.abspath(assets_path)
        self.device = device
        self.instances: Dict[str, AudioInstance] = {}
        self.running = False
        self.lock = threading.Lock()
        self.stream = None
        
    def audio_callback(self, outdata: np.ndarray, frames: int, time_info, status):
        """Callback for sounddevice stream."""
        if status:
            print(f"Audio callback status: {status}")
        
        # Mix all playing instances
        outdata.fill(0)
        
        with self.lock:
            instances_to_remove = []
            
            for instance_id, instance in self.instances.items():
                frames_data = instance.get_next_frames(frames)
                
                if frames_data is None:
                    instances_to_remove.append(instance_id)
                else:
                    # Ensure correct shape for mixing
                    if len(frames_data.shape) == 1:
                        frames_data = frames_data.reshape(-1, 1)
                    
                    # Mix into output (clip to prevent overflow)
                    if frames_data.shape[1] == outdata.shape[1]:
                        outdata[:] += frames_data
                    elif frames_data.shape[1] == 1 and outdata.shape[1] == 2:
                        # Mono to stereo
                        outdata[:] += np.hstack([frames_data, frames_data])
                    elif frames_data.shape[1] == 2 and outdata.shape[1] == 1:
                        # Stereo to mono
                        outdata[:] += frames_data.mean(axis=1).reshape(-1, 1)
            
            # Remove finished instances
            for instance_id in instances_to_remove:
                del self.instances[instance_id]
        
        # Clip output to prevent distortion
        np.clip(outdata, -1.0, 1.0, out=outdata)
    
    def handle_play_command(self, cmd_data: dict) -> dict:
        """Handle PLAY command."""
        try:
            resource = cmd_data.get("resource")
            loops = cmd_data.get("loops", 0)
            volume = cmd_data.get("volume", 1.0)
            
            if not resource:
                return {"error": "Missing 'resource' field"}
            
            # Build full path to audio file
            audio_path = os.path.join(self.assets_path, resource)
            
            if not os.path.exists(audio_path):
                return {"error": f"Audio file not found: {audio_path}"}
            
            # Load audio file
            data, samplerate = sf.read(audio_path, dtype='float32')
            
            # Generate unique instance ID
            instance_id = str(uuid.uuid4())
            
            # Create audio instance
            instance = AudioInstance(instance_id, data, samplerate, loops, volume)
            
            with self.lock:
                self.instances[instance_id] = instance
            
            return {"instance_id": instance_id}
            
        except Exception as e:
            return {"error": f"Failed to play audio: {str(e)}"}
    
    def handle_stop_command(self, cmd_data: dict) -> dict:
        """Handle STOP command."""
        try:
            instance_id = cmd_data.get("instance_id")
            fade_duration = cmd_data.get("duration", 0.0)
            
            if not instance_id:
                return {"error": "Missing 'instance_id' field"}
            
            with self.lock:
                if instance_id not in self.instances:
                    return {"error": f"Instance not found: {instance_id}"}
                
                self.instances[instance_id].stop(fade_duration)
            
            return {"success": True, "instance_id": instance_id}
            
        except Exception as e:
            return {"error": f"Failed to stop audio: {str(e)}"}
    
    def handle_command(self, data: bytes, addr: tuple):
        """Handle incoming UDP command."""
        try:
            # Parse JSON
            cmd = json.loads(data.decode('utf-8'))
            cmd_type = cmd.get("cmd")
            
            if cmd_type == "PLAY":
                response = self.handle_play_command(cmd)
            elif cmd_type == "STOP":
                response = self.handle_stop_command(cmd)
            else:
                response = {"error": f"Unknown command: {cmd_type}"}
            
            # Send response back to client
            response_data = json.dumps(response).encode('utf-8')
            self.sock.sendto(response_data, addr)
            
        except json.JSONDecodeError as e:
            error_response = json.dumps({"error": f"Invalid JSON: {str(e)}"}).encode('utf-8')
            self.sock.sendto(error_response, addr)
        except Exception as e:
            error_response = json.dumps({"error": f"Server error: {str(e)}"}).encode('utf-8')
            self.sock.sendto(error_response, addr)
    
    def start(self):
        """Start the audio server."""
        print(f"Starting Audio Server on {self.host}:{self.port}")
        print(f"Assets path: {self.assets_path}")
        if self.device:
            print(f"Audio device: {self.device}")
        
        # Create UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        
        # Start audio stream
        self.stream = sd.OutputStream(
            callback=self.audio_callback,
            channels=2,
            samplerate=44100,
            device=self.device,
            blocksize=1024
        )
        self.stream.start()
        
        print("Audio Server ready and listening for commands...")
        
        self.running = True
        
        try:
            while self.running:
                data, addr = self.sock.recvfrom(4096)
                self.handle_command(data, addr)
        except KeyboardInterrupt:
            print("\nShutting down server...")
        finally:
            self.stop()
    
    def stop(self):
        """Stop the audio server."""
        self.running = False
        if self.stream:
            self.stream.stop()
            self.stream.close()
        self.sock.close()
        print("Audio Server stopped.")


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Audio Server for Hugo Re-implementation"
    )
    parser.add_argument(
        "--host",
        default="0.0.0.0",
        help="Host to bind to (default: 0.0.0.0)"
    )
    parser.add_argument(
        "--port",
        type=int,
        default=9001,
        help="Port to listen on (default: 9001)"
    )
    parser.add_argument(
        "--assets",
        required=True,
        help="Path to assets directory"
    )
    parser.add_argument(
        "--device",
        default=None,
        help="Audio device name (optional)"
    )
    parser.add_argument(
        "--list-devices",
        action="store_true",
        help="List available audio devices and exit"
    )
    
    args = parser.parse_args()
    
    if args.list_devices:
        print("Available audio devices:")
        print(sd.query_devices())
        return
    
    # Validate assets path
    if not os.path.exists(args.assets):
        print(f"Error: Assets path does not exist: {args.assets}")
        return
    
    # Create and start server
    server = AudioServer(args.host, args.port, args.assets, args.device)
    server.start()


if __name__ == "__main__":
    main()
