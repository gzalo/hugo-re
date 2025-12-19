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
