# Audio Server

A Python-based audio server that listens for JSON UDP packages to control audio playback.

## Features

- Play audio files with looping support
- Stop audio with optional fade-out
- Multiple simultaneous audio instances
- Real-time mixing
- UDP-based JSON API

## Installation

```bash
pip install -r requirements.txt
```

## Usage

```bash
python audio_server.py --host 0.0.0.0 --port 9001 --assets ./resources/ --device "audio_device_name"
```

### Command Line Arguments

- `--host`: Host to bind to (default: 0.0.0.0)
- `--port`: Port to listen on (default: 9001)
- `--assets`: Path to assets directory (required)
- `--device`: Audio device name (optional)
- `--list-devices`: List available audio devices and exit

## API

### PLAY Command

Start playing an audio file and receive an instance ID.

**Request:**
```json
{
  "cmd": "PLAY",
  "resource": "ForestData/sfx/atmos-lp.wav",
  "loops": -1,
  "volume": 1.0
}
```

**Parameters:**
- `cmd`: Command type (must be "PLAY")
- `resource`: Path to audio file relative to assets directory
- `loops`: Number of times to loop (-1 for infinite, 0 for play once)
- `volume`: Volume level (0.0 to 1.0)

**Response:**
```json
{
  "instance_id": "uuid-string"
}
```

### STOP Command

Stop a playing audio instance.

**Request:**
```json
{
  "cmd": "STOP",
  "instance_id": "uuid-string",
  "duration": 0.5
}
```

**Parameters:**
- `cmd`: Command type (must be "STOP")
- `instance_id`: The instance ID returned from PLAY command
- `duration`: Optional fade-out duration in seconds (default: 0.0)

**Response:**
```json
{
  "success": true,
  "instance_id": "uuid-string"
}
```

## Example Client

```python
import socket
import json

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Play a sound
play_cmd = {
    "cmd": "PLAY",
    "resource": "ForestData/sfx/atmos-lp.wav",
    "loops": -1,
    "volume": 1.0
}
sock.sendto(json.dumps(play_cmd).encode(), ("localhost", 9001))
response, _ = sock.recvfrom(4096)
result = json.loads(response.decode())
instance_id = result["instance_id"]

# Stop the sound with fade
stop_cmd = {
    "cmd": "STOP",
    "instance_id": instance_id,
    "duration": 1.0
}
sock.sendto(json.dumps(stop_cmd).encode(), ("localhost", 9001))
response, _ = sock.recvfrom(4096)
```
