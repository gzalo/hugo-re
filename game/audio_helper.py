"""Helper module for audio playback that routes to UDP audio servers."""

from udp_audio_client import AudioManager


class AudioHelper:
    """Helper class to replace pygame.mixer.Sound calls with UDP-based calls."""
    
    @staticmethod
    def play(sound, country: str = None, loops: int = 0, maxtime: int = 0, fade_ms: int = 0):
        """
        Play a sound via UDP.
        
        Args:
            sound: pygame.mixer.Sound object
            country: Country code to determine UDP port
            loops: Number of loops (0 = play once, -1 = loop forever)
            maxtime: Not implemented
            fade_ms: Not implemented
        """
        if country:
            AudioManager().play(sound, country, loops=loops)
        else:
            # Fallback to pygame if no country specified
            sound.play(loops=loops, maxtime=maxtime, fade_ms=fade_ms)
    
    @staticmethod
    def stop(sound, country: str = None):
        """
        Stop a sound via UDP.
        
        Args:
            sound: pygame.mixer.Sound object
            country: Country code to determine UDP port
        """
        if country:
            AudioManager().stop(sound, country)
        else:
            # Fallback to pygame if no country specified
            sound.stop()
    
    @staticmethod
    def fadeout(sound, country: str = None, time: int = 0):
        """
        Fade out a sound via UDP.
        
        Args:
            sound: pygame.mixer.Sound object
            country: Country code to determine UDP port
            time: Fade duration in milliseconds
        """
        if country:
            AudioManager().fadeout(sound, country, duration=time)
        else:
            # Fallback to pygame if no country specified
            sound.fadeout(time)
