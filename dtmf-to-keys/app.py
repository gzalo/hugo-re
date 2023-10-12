import pyaudio
import numpy as np
import keyboard  # Import the keyboard library for simulating keypresses

# Constants for DTMF frequencies
DTMF_FREQUENCIES = {
    '1': (697, 1209),
    '2': (697, 1336),
    '3': (697, 1477),
    'A': (697, 1633),
    '4': (770, 1209),
    '5': (770, 1336),
    '6': (770, 1477),
    'B': (770, 1633),
    '7': (852, 1209),
    '8': (852, 1336),
    '9': (852, 1477),
    'C': (852, 1633),
    '*': (941, 1209),
    '0': (941, 1336),
    '#': (941, 1477),
    'D': (941, 1633)
}

MAPPING = {
    '1': 79,
    '2': 80,
    '3': 81,
    '4': 75,
    '5': 76,
    '6': 77,
    '7': 71,
    '8': 72,
    '9': 73,
    '0': 82,
}

# Function to detect DTMF tone
def detect_dtmf(sample_rate, audio_data):
    # Hamming windowing
    audio_data = audio_data * np.hamming(len(audio_data))

    power_spectrum = np.abs(np.fft.fft(audio_data)) ** 2

    freqs = np.fft.fftfreq(len(audio_data), 1/sample_rate)

    dtmf_energies = {}
    max_energy = 0
    for key, value in DTMF_FREQUENCIES.items():
        freq1, freq2 = value
        bin1 = np.argmin(np.abs(freqs-freq1))
        bin2 = np.argmin(np.abs(freqs-freq2))
        energy = power_spectrum[bin1] + power_spectrum[bin2]
        max_energy = max(max_energy, energy)
        dtmf_energies[key] = energy

    if max_energy < 8000:
        return None
        
    tone = max(dtmf_energies, key = dtmf_energies.get)
    return tone

# Main function to record and detect DTMF tones
def main():
    p = pyaudio.PyAudio()

    # Open microphone stream
    stream = p.open(format=pyaudio.paFloat32, channels=1, rate=44100, input=True, frames_per_buffer=1024)
    volume_threshold = 0.4  # Adjust this value as needed
    old_code = None

    while True:
        audio_data = np.frombuffer(stream.read(1024), dtype=np.float32)
        volume = np.max(np.abs(audio_data))

        dtmf_code = None
        if volume > volume_threshold:
            dtmf_code = detect_dtmf(44100, audio_data)

        if dtmf_code != old_code:
            if old_code == None:
                print("DOWN " + dtmf_code)
                mapping = MAPPING.get(dtmf_code)
                if mapping != None:
                    keyboard.press(mapping)
            elif dtmf_code == None:
                print("UP " + old_code)
                mapping = MAPPING.get(old_code)
                if mapping != None:
                    keyboard.release(mapping)
            else:
                print("UP " + old_code)
                mapping = MAPPING.get(old_code)
                if mapping != None:
                    keyboard.release(mapping)
                print("DOWN " + dtmf_code)
                mapping = MAPPING.get(dtmf_code)
                if mapping != None:
                    keyboard.press(mapping)

        old_code = dtmf_code

    stream.stop_stream()
    stream.close()
    p.terminate()

if __name__ == "__main__":
    main()