import json
import socket
import threading
from typing import Optional, Dict
import pygame


class UDPAudioClient:
    """Client for sending audio commands via UDP to external audio server."""
    
    def __init__(self, port: int):
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.lock = threading.Lock()
        self.instance_map: Dict[object, int] = {}  # Maps Sound objects to instance IDs
    
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
    
    def play(self, sound_obj: object, resource: str, loops: int = 0, volume: float = 1.0) -> Optional[int]:
        """
        Send PLAY command to audio server.
        
        Args:
            sound_obj: The pygame.mixer.Sound object (used as key)
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
            self.instance_map[sound_obj] = instance_id
            return instance_id
        return None
    
    def stop(self, sound_obj: object, fade_duration: int = 0):
        """
        Send STOP command to audio server.
        
        Args:
            sound_obj: The pygame.mixer.Sound object (used as key)
            fade_duration: Fade out duration in milliseconds
        """
        instance_id = self.instance_map.get(sound_obj)
        if instance_id is not None:
            command = {
                "cmd": "STOP",
                "instance_id": instance_id,
                "duration": fade_duration
            }
            self.send_command(command)
            del self.instance_map[sound_obj]
    
    def close(self):
        """Close the UDP socket."""
        self.sock.close()


class AudioManager:
    """Global audio manager that routes audio commands to appropriate UDP ports."""
    
    _instance = None
    
    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(AudioManager, cls).__new__(cls)
            cls._instance.initialized = False
        return cls._instance
    
    def __init__(self):
        if not self.initialized:
            self.clients: Dict[str, UDPAudioClient] = {}
            self.sound_to_path: Dict[object, str] = {}
            self.initialized = True
    
    def get_port_for_country(self, country: str) -> int:
        """Map country code to UDP port (9001-9004)."""
        country_to_port = {
            "ar": 9001,
            "cl": 9002,
            "dn": 9003,
            "fr": 9004
        }
        return country_to_port.get(country, 9001)
    
    def get_client(self, country: str) -> UDPAudioClient:
        """Get or create UDP client for a country."""
        if country not in self.clients:
            port = self.get_port_for_country(country)
            self.clients[country] = UDPAudioClient(port)
        return self.clients[country]
    
    def register_sound(self, sound_obj: object, resource_path: str):
        """Register a sound object with its resource path."""
        self.sound_to_path[sound_obj] = resource_path
    
    def play(self, sound_obj: object, country: str, loops: int = 0):
        """Play a sound via UDP for the specified country."""
        resource_path = self.sound_to_path.get(sound_obj)
        if resource_path:
            client = self.get_client(country)
            client.play(sound_obj, resource_path, loops=loops, volume=1.0)
    
    def stop(self, sound_obj: object, country: str):
        """Stop a sound via UDP for the specified country."""
        client = self.get_client(country)
        client.stop(sound_obj, fade_duration=0)
    
    def fadeout(self, sound_obj: object, country: str, duration: int):
        """Fade out a sound via UDP for the specified country."""
        client = self.get_client(country)
        client.stop(sound_obj, fade_duration=duration)
    
    def close_all(self):
        """Close all UDP clients."""
        for client in self.clients.values():
            client.close()
        self.clients.clear()
