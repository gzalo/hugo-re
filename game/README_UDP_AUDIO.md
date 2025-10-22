# UDP Audio Implementation

This implementation converts all pygame mixer audio calls to UDP messages sent to external audio servers.

## Quick Start

### For Game Developers

The audio system is transparent - no changes needed in game logic. Audio calls automatically route to UDP servers based on the player's country:

```python
# Audio is played via UDP for the current player's country
AudioHelper.play(ForestResources.sfx_bg_atmosphere, self.context.country, loops=-1)

# Audio is stopped via UDP
AudioHelper.stop(ForestResources.sfx_bg_atmosphere, self.context.country)

# Audio fades out via UDP
AudioHelper.fadeout(CaveResources.bg_music, self.context.country, 1000)
```

### For Audio Server Developers

Implement a UDP server listening on ports 9001-9004 that handles these JSON messages:

1. **PLAY Command** - Start audio playback
   - Receive: `{"cmd": "PLAY", "resource": "path/to/audio.wav", "loops": -1, "volume": 1.0}`
   - Respond: `{"instance_id": 42}`

2. **STOP Command** - Stop audio playback
   - Receive: `{"cmd": "STOP", "instance_id": 42, "duration": 1000}`
   - No response needed

## Architecture

### Components

1. **UDPAudioClient** (`udp_audio_client.py`)
   - Handles UDP socket communication
   - Serializes/deserializes JSON messages
   - Manages instance ID tracking

2. **AudioManager** (`udp_audio_client.py`)
   - Singleton managing all UDP clients
   - Maps countries to ports
   - Tracks sound-to-resource-path mappings

3. **AudioHelper** (`audio_helper.py`)
   - Drop-in replacement for pygame.mixer.Sound calls
   - Routes audio commands through AudioManager
   - Provides backward compatibility

### Port Assignment

| Country | Port |
|---------|------|
| Argentina (ar) | 9001 |
| Chile (cl) | 9002 |
| Denmark (dn) | 9003 |
| France (fr) | 9004 |

## Message Protocol

See [UDP_AUDIO_PROTOCOL.md](UDP_AUDIO_PROTOCOL.md) for complete protocol documentation.

### PLAY Message Example

```json
{
  "cmd": "PLAY",
  "resource": "ForestData/sfx/atmos-lp.wav",
  "loops": -1,
  "volume": 1.0
}
```

**Response:**
```json
{
  "instance_id": 42
}
```

### STOP Message Example

```json
{
  "cmd": "STOP",
  "instance_id": 42,
  "duration": 1000
}
```

## Implementation Details

### Sound Registration

All sounds are registered with the AudioManager when loaded:

```python
# In resource.py
sound = pygame.mixer.Sound(Resource.DATA_DIR + "/" + resource_path)
AudioManager().register_sound(sound, resource_path)
return sound
```

### Country Context

Every game state has access to the country through `self.context.country`:

```python
class MyState(State):
    def on_enter(self):
        # Country available in all states
        AudioHelper.play(some_sound, self.context.country)
```

### Thread Safety

The UDPAudioClient uses thread-safe locks for socket operations:

```python
with self.lock:
    self.sock.sendto(message, ('127.0.0.1', self.port))
```

## Testing

Run the test suite:

```bash
python3 /tmp/test_udp_audio.py
```

View usage examples:

```bash
python3 /tmp/usage_example.py
```

## Migration Guide

### Before
```python
import pygame
pygame.mixer.Sound.play(ForestResources.sfx_bg_atmosphere, loops=-1)
pygame.mixer.Sound.stop(ForestResources.sfx_bg_atmosphere)
pygame.mixer.Sound.fadeout(CaveResources.bg_music, 1000)
```

### After
```python
from audio_helper import AudioHelper
AudioHelper.play(ForestResources.sfx_bg_atmosphere, self.context.country, loops=-1)
AudioHelper.stop(ForestResources.sfx_bg_atmosphere, self.context.country)
AudioHelper.fadeout(CaveResources.bg_music, self.context.country, 1000)
```

## Limitations

1. **Volume Control**: Not yet implemented in protocol
2. **maxtime Parameter**: Not supported
3. **fade_ms on Play**: Not supported
4. **Server Response Timeout**: 1 second (configurable)

## Future Enhancements

- [ ] Dynamic volume control
- [ ] Audio streaming for large files
- [ ] Connection pooling
- [ ] Retry logic for failed commands
- [ ] Metrics and monitoring
- [ ] Multiple audio backend support

## Files Modified

- 32 files total (31 game files + 1 resource file)
- 12 cave game state files
- 18 forest game state files
- 1 core resource loader file
- 3 new support files

## Troubleshooting

### Audio Not Playing

1. Check if UDP server is running on correct port
2. Verify server is listening on 127.0.0.1
3. Check firewall settings
4. Enable debug logging in AudioManager

### Wrong Port

Verify country mapping in `AudioManager.get_port_for_country()`:
```python
country_to_port = {
    "ar": 9001,
    "cl": 9002,
    "dn": 9003,
    "fr": 9004
}
```

### Instance ID Not Tracked

Ensure sound is registered in AudioManager:
```python
AudioManager().register_sound(sound, resource_path)
```

## Performance Considerations

- UDP is fire-and-forget, minimal latency
- JSON serialization is fast for small messages
- Socket timeout is 1 second for responses
- Thread-safe but non-blocking operations

## Security Considerations

- Localhost-only communication (127.0.0.1)
- No authentication required (local trust)
- JSON schema validation recommended on server
- Resource path validation needed on server

## Contributing

When adding new audio:

1. Load sound through `Resource.load_sfx()` or `Resource.load_speak()`
2. Use `AudioHelper` for all playback
3. Pass `self.context.country` for routing
4. Test with all 4 countries/ports

## License

Same as parent project.
