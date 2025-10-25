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
from typing import Dict, Optional, List
import uuid

import sounddevice as sd
import soundfile as sf
import numpy as np
from scipy import signal
from functools import partial

# Map ports -> sink names in the same order as the ports given
DEFAULT_SINKS = [
    "virtual_sink_0",
    "virtual_sink_1",
    "virtual_sink_2",
    "virtual_sink_3",
]


class AudioInstance:
    """Represents a playing audio instance."""

    def __init__(self, instance_id: str, data: np.ndarray, samplerate: int,
                 loops: int, volume: float, target_samplerate: int = 44100):
        self.instance_id = instance_id
        self.original_samplerate = samplerate
        self.target_samplerate = target_samplerate

        # Resample if sample rates don't match
        if samplerate != target_samplerate:
            data = self._resample_audio(data, samplerate, target_samplerate)

        # Ensure shape is (N, channels). We'll use mono, so shape (N, 1)
        if len(data.shape) == 1:
            data = data.reshape(-1, 1)
        elif data.shape[1] > 1:
            # Keep original multi-channel for generality; server will downmix as needed
            pass

        self.data = data.astype(np.float32)
        self.samplerate = target_samplerate
        self.loops = loops
        self.volume = volume
        self.current_loop = 0
        self.current_frame = 0
        self.is_playing = True
        self.fade_duration = 0.0
        self.fade_start_time = None
        self.original_volume = volume

    def _resample_audio(self, data: np.ndarray, orig_sr: int, target_sr: int) -> np.ndarray:
        """Resample audio to target sample rate."""
        if orig_sr == target_sr:
            return data

        ratio = target_sr / orig_sr
        if len(data.shape) == 1:
            num_samples = int(len(data) * ratio)
            resampled = signal.resample(data, num_samples)
        else:
            num_samples = int(data.shape[0] * ratio)
            resampled = signal.resample(data, num_samples, axis=0)

        return resampled.astype(np.float32)

    def get_next_frames(self, num_frames: int) -> Optional[np.ndarray]:
        """Get the next frames to play (returns shape (num_frames, channels))."""
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

        frames = self.data[self.current_frame:self.current_frame + frames_to_read]
        self.current_frame += frames_to_read

        # Apply volume
        frames = frames * current_volume

        # If we need more frames to fill the buffer, pad with zeros
        if frames_to_read < num_frames:
            channels = frames.shape[1] if frames.ndim > 1 else 1
            padding = np.zeros((num_frames - frames_to_read, channels), dtype=np.float32)
            if frames.ndim == 1:
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

    OUTPUT_SAMPLERATE = 44100  # Output sample rate for audio stream
    BLOCKSIZE = 1024

    def __init__(self, host: str, ports: List[int], assets_path: str,
                 sinks: Optional[List[str]] = None, device_names: Optional[List[str]] = None):
        self.host = host
        self.ports = ports if isinstance(ports, list) else [ports]
        self.assets_path = os.path.abspath(assets_path)
        self.running = False
        self.lock = threading.Lock()

        # Sinks is a list of sink names corresponding to the ports order
        self.sinks = sinks if sinks is not None else DEFAULT_SINKS[:len(self.ports)]

        # Streams and instances per sink
        self.streams: Dict[int, sd.OutputStream] = {}
        self.instances_per_sink: Dict[int, Dict[str, AudioInstance]] = {i: {} for i in range(len(self.sinks))}

        self.listener_threads: List[threading.Thread] = []
        self.sockets: List[socket.socket] = []

    # --------------------
    # Device / sink helpers
    # --------------------
    def _find_portaudio_device_for_sink(self, sink_name_substr: str) -> Optional[int]:
        """
        Find a PortAudio device index whose name contains sink_name_substr.
        If none found, return None (caller may fall back to default).
        """
        try:
            devices = sd.query_devices()
        except Exception:
            return None

        for idx, dev in enumerate(devices):
            # dev is a dict-like object with 'name'
            name = dev.get('name', '') if isinstance(dev, dict) else str(dev)
            if sink_name_substr in name:
                return idx
        # sometimes the sink might appear with additional text - try partial match
        for idx, dev in enumerate(devices):
            name = dev.get('name', '') if isinstance(dev, dict) else str(dev)
            if sink_name_substr.split('_')[-1] in name:
                return idx
        return None

    # --------------------
    # Audio mixing per sink
    # --------------------
    def audio_callback(self, sink_index: int, outdata: np.ndarray, frames: int, time_info, status):
        """Callback for each OutputStream (mono)."""
        if status:
            print(f"[sink {sink_index}] Audio callback status: {status}")

        # Fill with silence
        outdata.fill(0.0)

        with self.lock:
            instances_to_remove = []
            instances = self.instances_per_sink.get(sink_index, {})

            for instance_id, instance in list(instances.items()):
                frames_data = instance.get_next_frames(frames)
                if frames_data is None:
                    instances_to_remove.append(instance_id)
                    continue

                # Ensure frames_data shape is (frames, channels)
                if frames_data.ndim == 1:
                    frames_data = frames_data.reshape(-1, 1)

                # Downmix to mono if necessary by averaging channels
                if frames_data.shape[1] > 1:
                    mono = frames_data.mean(axis=1).reshape(-1, 1)
                else:
                    mono = frames_data

                # Mix into outdata (both are shape (frames, 1))
                try:
                    outdata[:, 0] += mono[:, 0]
                except Exception as e:
                    print(f"[sink {sink_index}] Mixing error: {e}")

            # Remove finished instances
            for iid in instances_to_remove:
                del instances[iid]

        # Clip to avoid overflow
        np.clip(outdata, -1.0, 1.0, out=outdata)

    # --------------------
    # Command handlers
    # --------------------
    def handle_play_command(self, cmd_data: dict, sink_index: int) -> dict:
        """Handle PLAY command for a specific sink_index."""
        try:
            resource = cmd_data.get("resource")
            loops = int(cmd_data.get("loops", 0))
            volume = float(cmd_data.get("volume", 1.0))

            if not resource:
                return {"error": "Missing 'resource' field"}

            audio_path = os.path.join(self.assets_path, resource)
            if not os.path.exists(audio_path):
                return {"error": f"Audio file not found: {audio_path}"}

            data, samplerate = sf.read(audio_path, dtype='float32')
            # Downmix to mono immediately
            if data.ndim == 1:
                mono = data.reshape(-1, 1)
            else:
                mono = data.mean(axis=1).reshape(-1, 1)

            instance_id = str(uuid.uuid4())
            instance = AudioInstance(instance_id, mono, samplerate, loops, volume,
                                     target_samplerate=self.OUTPUT_SAMPLERATE)

            with self.lock:
                self.instances_per_sink[sink_index][instance_id] = instance

            return {"instance_id": instance_id}

        except Exception as e:
            return {"error": f"Failed to play audio: {str(e)}"}

    def handle_stop_command(self, cmd_data: dict) -> dict:
        """Handle STOP command. Searches all sinks for the instance_id."""
        try:
            instance_id = cmd_data.get("instance_id")
            fade_duration = float(cmd_data.get("duration", 0.0))

            if not instance_id:
                return {"error": "Missing 'instance_id' field"}

            with self.lock:
                found = False
                for sink_index, instances in self.instances_per_sink.items():
                    if instance_id in instances:
                        instances[instance_id].stop(fade_duration)
                        found = True
                        break

            if not found:
                return {"error": f"Instance not found: {instance_id}"}

            return {"success": True, "instance_id": instance_id}

        except Exception as e:
            return {"error": f"Failed to stop audio: {str(e)}"}

    def handle_command(self, data: bytes, addr: tuple, sock: socket.socket, port: int):
        """Handle incoming UDP command; port tells which sink to use."""
        try:
            cmd = json.loads(data.decode('utf-8'))
            cmd_type = cmd.get("cmd")
            # Determine sink index for this port
            try:
                sink_index = self.ports.index(port)
            except ValueError:
                # port not found in configured ports
                response = {"error": f"Port {port} not configured"}
                sock.sendto(json.dumps(response).encode('utf-8'), addr)
                return

            if cmd_type == "PLAY":
                response = self.handle_play_command(cmd, sink_index)
            elif cmd_type == "STOP":
                response = self.handle_stop_command(cmd)
            else:
                response = {"error": f"Unknown command: {cmd_type}"}

            sock.sendto(json.dumps(response).encode('utf-8'), addr)

        except json.JSONDecodeError as e:
            error_response = {"error": f"Invalid JSON: {str(e)}"}
            sock.sendto(json.dumps(error_response).encode('utf-8'), addr)
        except Exception as e:
            error_response = {"error": f"Server error: {str(e)}"}
            sock.sendto(json.dumps(error_response).encode('utf-8'), addr)

    # --------------------
    # Networking / listeners
    # --------------------
    def listen_on_port(self, port: int):
        """Listen for commands on a specific port and pass the port to handler."""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(1.0)
        sock.bind((self.host, port))
        self.sockets.append(sock)
        print(f"Listening on {self.host}:{port}")

        try:
            while self.running:
                try:
                    data, addr = sock.recvfrom(4096)
                    # Pass the port so handler knows which sink index to target
                    self.handle_command(data, addr, sock, port)
                except socket.timeout:
                    continue
        except Exception as e:
            print(f"Error on port {port}: {e}")
        finally:
            sock.close()
            print(f"Closed listener on port {port}")

    # --------------------
    # Startup / Shutdown
    # --------------------
    def start_streams(self):
        """Create one output stream per sink (mono)."""
        for i, sink_name in enumerate(self.sinks):
            # Find a portaudio device index for this sink name. If not found, use default (None).
            dev_idx = self._find_portaudio_device_for_sink(sink_name)
            print("IDX " + str(dev_idx))
            if dev_idx is None:
                print(f"[warn] No PortAudio device name matched '{sink_name}'. Using default device for stream {i}. "
                      "Make sure PulseAudio/PortAudio exposes the sink as a device name.")
            else:
                print(f"[info] Stream {i} will use PortAudio device index {dev_idx} matching '{sink_name}'")

            # Build callback that binds sink_index
            callback = partial(self.audio_callback, i)

            try:
                stream = sd.OutputStream(
                    callback=callback,
                    channels=1,
                    samplerate=self.OUTPUT_SAMPLERATE,
                    device=dev_idx,
                    blocksize=self.BLOCKSIZE,
                )
                stream.start()
                self.streams[i] = stream
                print(f"Started output stream for sink index {i} (sink '{sink_name}')")
            except Exception as e:
                print(f"[error] Could not start stream for sink {i} ('{sink_name}'): {e}")
                # Clean up any started streams
                for s in self.streams.values():
                    try:
                        s.stop()
                        s.close()
                    except Exception:
                        pass
                raise

    def start(self):
        """Start the audio server."""
        print(f"Starting Audio Server on {self.host} ports {self.ports}")
        print(f"Assets path: {self.assets_path}")
        print(f"Sinks mapping (ports -> sinks):")
        for p, s in zip(self.ports, self.sinks):
            print(f"  {p} -> {s}")

        # Try to start one output stream per sink
        try:
            self.start_streams()
            print("All audio output streams started successfully")
        except Exception as e:
            print(f"Warning: Could not start all audio output streams: {e}")
            print("Exiting — streams are required for real audio output.")
            return

        self.running = True

        # Start listener threads for each port
        for port in self.ports:
            thread = threading.Thread(target=self.listen_on_port, args=(port,), daemon=True)
            thread.start()
            self.listener_threads.append(thread)

        print("Audio Server ready and listening for commands...")

        try:
            while self.running:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nShutting down server...")
        finally:
            self.stop()

    def stop(self):
        """Stop the audio server."""
        self.running = False

        # Wait for listener threads to finish
        for thread in self.listener_threads:
            thread.join(timeout=1.0)

        # Close streams
        for i, stream in self.streams.items():
            try:
                stream.stop()
                stream.close()
                print(f"Closed stream {i}")
            except Exception as e:
                print(f"Warning during stream cleanup for {i}: {e}")

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
        "--ports",
        type=str,
        default="9001,9002,9003,9004",
        help="Comma-separated list of ports to listen on (default: 9001,9002,9003,9004)"
    )
    parser.add_argument(
        "--assets",
        help="Path to assets directory",
        required=True
    )
    parser.add_argument(
        "--sinks",
        type=str,
        default=",".join(DEFAULT_SINKS),
        help="Comma-separated list of PulseAudio sink names in the same order as ports"
    )
    parser.add_argument(
        "--list-devices",
        action="store_true",
        help="List available audio devices and exit"
    )

    args = parser.parse_args()

    if args.list_devices:
        print("Available PortAudio devices (sd.query_devices()):")
        print(sd.query_devices())
        return

    # Parse ports
    try:
        ports = [int(p.strip()) for p in args.ports.split(',')]
    except ValueError:
        print(f"Error: Invalid ports format: {args.ports}")
        return

    sinks = [s.strip() for s in args.sinks.split(',')]
    if len(sinks) < len(ports):
        print("Error: You must provide at least as many sink names as ports")
        return

    # Validate assets path
    if not os.path.exists(args.assets):
        print(f"Error: Assets path does not exist: {args.assets}")
        return

    server = AudioServer(args.host, ports, args.assets, sinks=sinks)
    server.start()


if __name__ == "__main__":
    main()
