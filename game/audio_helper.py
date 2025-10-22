"""Helper module for audio playback that routes to UDP audio servers."""

from udp_audio_client import get_client


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

