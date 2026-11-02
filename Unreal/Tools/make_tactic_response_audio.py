"""Deterministic original short battle stings. No recordings or external samples."""
import math
from pathlib import Path
import random
import struct
import wave

SOUNDS = {
    "S_TacticConnectV1": (0.76, (196.0, 293.665)),
    "S_TacticCombo2V1": (0.84, (196.0, 293.665, 391.995)),
    "S_TacticCombo3V1": (0.94, (196.0, 293.665, 391.995, 587.33)),
    "S_TacticSupportV1": (0.94, (220.0, 329.628, 440.0)),
    "S_TacticStopV1": (0.55, (146.832, 130.813)),
}

def generate(root):
    root = Path(root)
    root.mkdir(parents=True, exist_ok=True)
    rate = 22050
    for index, (name, (duration, notes)) in enumerate(SOUNDS.items()):
        path = root / (name + ".wav")
        if path.exists():
            continue  # Preserve authored replacements; version names when changing synthesis.
        rng = random.Random(8510 + index)
        support, stopped = "Support" in name, "Stop" in name
        values = []
        for i in range(round(duration * rate)):
            t = i / rate
            drum = math.sin(2 * math.pi * (80 * t - 16 * t * t))
            value = (0.10 if support else 0.16 if stopped else 0.36) * math.exp(-t * 16) * drum
            value += (0.015 if support else 0.06) * math.exp(-t * 26) * rng.uniform(-1, 1)
            for n, frequency in enumerate(notes):
                age = t - n * (0.085 if not stopped else 0.12)
                if age < 0:
                    continue
                # Soft plucked strings/bells with a fifth; no harsh square waves.
                env = (1 - math.exp(-age * 180)) * math.exp(-age * (5 if support else 7))
                tone = math.sin(2 * math.pi * frequency * age)
                tone += .28 * math.sin(2 * math.pi * frequency * 2 * age) * math.exp(-age * 8)
                tone += .10 * math.sin(2 * math.pi * frequency * 3 * age) * math.exp(-age * 15)
                value += (0.12 if stopped else 0.18) * env * tone
            value *= min(t * 400, 1) * min((duration - t) * 65, 1)
            values.append(value)
        peak = max(abs(v) for v in values)
        gain = min(1.0, (0.30 if stopped else 0.72) / peak)
        samples = [round(v * gain * 32767) for v in values]
        with wave.open(str(path), "wb") as out:
            out.setnchannels(1)
            out.setsampwidth(2)
            out.setframerate(rate)
            out.writeframes(struct.pack("<" + "h" * len(samples), *samples))

if __name__ == "__main__":
    generate(Path(__file__).resolve().parents[1] / "ArtSource" / "Audio")
