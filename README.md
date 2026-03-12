# WaveFunc

`WaveFunc` is a tiny C++ console program that writes a 16-bit PCM `.wav` file containing a generated sine wave.

## What it does

- generates a 44-byte PCM WAV header in code
- produces a 440 Hz sine wave (`A4`)
- writes `test.wav` with these defaults:
  - duration: `5000 ms`
  - sample rate: `44100 Hz`
  - channels: `1` (`mono`)
  - bit depth: `16-bit PCM`

## Recent improvements

- fixed `BlockAlign` calculation to follow the WAV PCM spec
- replaced Windows-only `stdafx.h` usage and updated file opening logic so the sample builds cleanly with the validated Windows LLVM toolchain while still using standard file I/O outside MSVC
- switched header fields to fixed-width integer types for predictable WAV layout
- replaced manual heap allocation with `std::vector`
- added basic file write error handling
- kept the program intentionally small and focused on 16-bit mono output

## Build

The commands below were validated in this review environment with LLVM `clang++ 22.1.0`.

```bash
"/c/Program Files/LLVM/bin/clang++.exe" -std=c++17 -Wall -Wextra -pedantic WaveExec.cpp -o WaveExec.exe
```

## Run

```bash
./WaveExec.exe
```

Running the executable creates `test.wav` in the current directory.

## Verify the generated WAV file

You can check the generated file with Python's standard library.

```bash
python - <<'PY'
import pathlib
import wave

path = pathlib.Path("test.wav")
assert path.exists(), "test.wav was not created"

with wave.open(str(path), "rb") as wav_file:
    assert wav_file.getnchannels() == 1
    assert wav_file.getsampwidth() == 2
    assert wav_file.getframerate() == 44100
    assert wav_file.getnframes() == 220500

print("WAV verification passed")
PY
```

## Notes

- this sample intentionally targets `16-bit PCM` output only
- the program is small enough to be used as a learning example for WAV header structure and PCM sample generation
- if you want to extend it to stereo or other sample sizes, update both the header generation and sample packing logic together
- the build and runtime verification in this review were performed on Windows; broader platform validation was not part of this change
