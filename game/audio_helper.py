"""Helper module for audio playback that routes to UDP audio servers."""

import json
import socket
import threading
from typing import Optional, Dict


class UDPAudioClient:
    """Client for sending audio commands via UDP to external audio server."""
    
    def __init__(self, port: int):
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.lock = threading.Lock()
        self.instance_map: Dict[str, int] = {}  # Maps resource strings to instance IDs
    
    def send_command(self, command: dict) -> Optional[dict]:
        """Send a command to the audio server and receive response."""
        try:
            with self.lock:
                message = json.dumps(command).encode('utf-8')
                self.sock.sendto(message, ('127.0.0.1', self.port))
                
                # Set timeout for receiving response
                self.sock.settimeout(1.0)
                try:
                    data, _ = self.sock.recvfrom(4096)
                    response = json.loads(data.decode('utf-8'))
                    return response
                except socket.timeout:
                    # No response received, return None
                    return None
        except Exception as e:
            print(f"Error sending UDP command: {e}")
            return None
    
    def play(self, resource: str, loops: int = 0, volume: float = 1.0) -> Optional[int]:
        """
        Send PLAY command to audio server.
        
        Args:
            resource: Path to audio resource
            loops: Number of loops (-1 for infinite)
            volume: Volume level (0.0 to 1.0)
        
        Returns:
            Instance ID if successful, None otherwise
        """
        command = {
            "cmd": "PLAY",
            "resource": resource,
            "loops": loops,
            "volume": volume
        }
        response = self.send_command(command)
        if response and "instance_id" in response:
            instance_id = response["instance_id"]
            self.instance_map[resource] = instance_id
            return instance_id
        return None
    
    def stop(self, resource: str, fade_duration: int = 0):
        """
        Send STOP command to audio server.
        
        Args:
            resource: Path to audio resource
            fade_duration: Fade out duration in milliseconds
        """
        instance_id = self.instance_map.get(resource)
        if instance_id is not None:
            command = {
                "cmd": "STOP",
                "instance_id": instance_id,
                "duration": fade_duration
            }
            self.send_command(command)
            del self.instance_map[resource]
    
    def close(self):
        """Close the UDP socket."""
        self.sock.close()


# Global clients dictionary for each port
_clients: Dict[int, UDPAudioClient] = {}


def get_client(port: int) -> UDPAudioClient:
    """Get or create UDP client for a port."""
    if port not in _clients:
        _clients[port] = UDPAudioClient(port)
    return _clients[port]


class AudioHelper:
    """Helper class to replace pygame.mixer.Sound calls with UDP-based calls."""
    
    @staticmethod
    def play(resource: str, port: int, loops: int = 0):
        """
        Play a sound via UDP.
        
        Args:
            resource: Resource path string
            port: UDP port number for the audio server
            loops: Number of loops (0 = play once, -1 = loop forever)
        """
        client = get_client(port)
        client.play(resource, loops=loops, volume=1.0)
    
    @staticmethod
    def stop(resource: str, port: int):
        """
        Stop a sound via UDP.
        
        Args:
            resource: Resource path string
            port: UDP port number for the audio server
        """
        client = get_client(port)
        client.stop(resource, fade_duration=0)
    
    @staticmethod
    def fadeout(resource: str, port: int, time: int = 0):
        """
        Fade out a sound via UDP.
        
        Args:
            resource: Resource path string
            port: UDP port number for the audio server
            time: Fade duration in milliseconds
        """
        client = get_client(port)
        client.stop(resource, fade_duration=time)

