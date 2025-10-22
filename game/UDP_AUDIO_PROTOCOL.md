# UDP Audio Protocol

This document describes the UDP-based audio protocol used to send audio playback commands to external audio servers.

## Overview

The game sends JSON messages over UDP to control audio playback on external servers. Each country (player position) has its own UDP port:

- **ar** (Argentina): Port 9001
- **cl** (Chile): Port 9002
- **dn** (Denmark): Port 9003
- **fr** (France): Port 9004

## Message Format

All messages are JSON objects sent as UTF-8 encoded strings.

### PLAY Command

Starts playback of an audio resource.

**Request:**
```json
{
  "cmd": "PLAY",
  "resource": "ForestData/sfx/atmos-lp.wav",
  "loops": -1,
  "volume": 1.0
}
```

**Fields:**
- `cmd`: Command type (always "PLAY")
- `resource`: Path to the audio resource relative to the data directory
- `loops`: Number of times to loop (-1 for infinite loop, 0 for play once)
- `volume`: Volume level from 0.0 to 1.0

**Response:**
```json
{
  "instance_id": 42
}
```

**Fields:**
- `instance_id`: Unique identifier for this audio instance, used for stopping

### STOP Command

Stops playback of a specific audio instance.

**Request:**
```json
{
  "cmd": "STOP",
  "instance_id": 42,
  "duration": 1000
}
```

**Fields:**
- `cmd`: Command type (always "STOP")
- `instance_id`: The instance ID returned from the PLAY command
- `duration`: Fade-out duration in milliseconds (0 for immediate stop)

**Response:**
No response expected.

## Implementation Notes

### Audio Manager
The `AudioManager` singleton class manages UDP clients for each country:
- Maps country codes to ports
- Maintains a registry of sound objects and their resource paths
- Creates and manages UDP client instances per country
- Tracks instance IDs for each sound object

### Audio Helper
The `AudioHelper` module provides a drop-in replacement for pygame.mixer.Sound calls:
- `AudioHelper.play(sound, country, loops=0)`: Play a sound
- `AudioHelper.stop(sound, country)`: Stop a sound
- `AudioHelper.fadeout(sound, country, time)`: Fade out a sound

### Usage Example

```python
from audio_helper import AudioHelper
from forest.forest_resources import ForestResources

# Play background atmosphere in a loop for Argentina
AudioHelper.play(ForestResources.sfx_bg_atmosphere, "ar", loops=-1)

# Stop the background atmosphere with 1 second fade
AudioHelper.fadeout(ForestResources.sfx_bg_atmosphere, "ar", 1000)
```

## Server Implementation

External audio servers should:
1. Listen on UDP ports 9001-9004
2. Parse incoming JSON messages
3. Handle PLAY commands by:
   - Loading and playing the specified audio resource
   - Generating a unique instance ID
   - Sending back the instance ID in JSON format
4. Handle STOP commands by:
   - Finding the audio instance by ID
   - Applying fade-out if duration > 0
   - Stopping the audio playback

## Error Handling

The UDP client includes a 1-second timeout for responses to PLAY commands. If no response is received:
- The function returns `None`
- Audio playback continues normally (server may have received the command)

Network errors are logged but do not crash the application.
