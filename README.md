# AllPassPhase

AllPassPhase is a VST3 audio effect by `enummusic` that creates phase dispersion by running audio through cascaded all-pass filters. It has no custom GUI in this migration step; the host/JUCE generic editor exposes the parameters.

## Parameters

- Frequency: filter frequency in Hz. The host-facing range is 20 Hz to 20 kHz and keeps the original VST2 exponential knob curve.
- Q: filter resonance. The processing path clamps the real value to a minimum of `0.005` to avoid unstable low values.
- Intensity: number of cascaded all-pass filters, from 0 to 50. A value of 0 bypasses the effect.
- Mix: dry/wet mix. A value of 0 bypasses the effect.

## Build Requirements

- CMake 3.22 or newer
- A C++17 compiler
- Git, used by CMake `FetchContent` to download JUCE
- Platform plugin tooling:
  - Windows: Visual Studio 2022 Build Tools or newer
  - macOS: Xcode command line tools
  - Linux: a supported compiler plus the normal JUCE Linux development packages for audio/plugin builds

JUCE 8 is fetched automatically by CMake. This repository no longer depends on Steinberg's VST2 SDK.

## Configure And Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AllPassPhase_VST3
```

The VST3 bundle is generated under the CMake build tree. The exact path depends on the generator and configuration, commonly:

```text
build/AllPassPhase_artefacts/Release/VST3/AllPassPhase.vst3
```

On macOS or Linux, use your normal CMake generator:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target AllPassPhase_VST3
```

## Optional Quality Tools

Formatting is available when `clang-format` is installed:

```powershell
cmake --build build --target format
```

`clang-tidy` is opt-in so a basic build still works on machines without it:

```powershell
cmake -S . -B build -DALLPASSPHASE_ENABLE_CLANG_TIDY=ON
cmake --build build --config Release --target AllPassPhase_VST3
```

## Migration Notes

- The VST2 entry point and `AudioEffectX` processor were removed.
- `AllPassFilter`, `LRCrossoverFilter`, and `HardClip` are preserved as the existing pure DSP files.
- `AllPassFilter` remains the processing filter used by the plugin. `LRCrossoverFilter` and `HardClip` are still compiled but not used in the main processing path, matching the old repository behavior.
- Stereo remains the main layout. Mono is supported by using the left-channel filter bank predictably.
- Parameter state is saved and restored through `juce::AudioProcessorValueTreeState`.
- The old VST2 program bank of 16 named programs was not recreated. VST3 hosts should use normal plugin state/presets.
- The VST3 plugin/manufacturer codes are new JUCE identifiers because the old lowercase VST2 unique ID was not directly suitable for the JUCE CMake plugin code requirement.

## Validation

After building, load the generated VST3 in a host and check:

- stereo audio passes correctly;
- Frequency, Q, Intensity, and Mix appear in the host and can be automated;
- state is restored after saving/reopening a project;
- low-frequency automation does not produce obvious instability.
