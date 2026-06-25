# allpassphase+

allpassphase+ is a `vst3` audio effect based on enummusic's [`allpassphase`](https://github.com/enummusic/allpassphase) plugin.
it creates phase dispersion by running the incoming signal through cascaded all-pass filters, which can soften transients, add a laser-like sweep, or give bass sounds a less phase-coherent character.

this fork keeps the original idea and core all-pass filter behavior, but rebuilds the plugin around a modern `juce 8`/`cmake` project and extends the cascade depth for heavier sound design.

## changes from the original

the original `enummusic/allpassphase` repository is a `vst2` plugin built directly on steinberg's old `AudioEffectX` sample-project workflow. this repository is a `juce 8` plugin that builds only `vst3` (for now).

main differences:

- the plugin wrapper was rewritten from `vst2` `AudioEffectX` code to a `juce` `AudioProcessor`.
- the build was replaced with a standalone `CMakeLists.txt` project.
- `juce` is fetched automatically with `cmake` `FetchContent`.
- the project no longer requires the steinberg `vst2` sdk or copying files into the old `aDelay` sample folder.
- the product is now named `AllPassPhase+`.
- the vendor identity is now `Kwwala`.
- the plugin format is now `vst3` only.
- a small native editor was added with sliders attached to plugin parameters.
- parameters are managed with `juce::AudioProcessorValueTreeState`.
- parameter state is saved and restored through `apvts` xml state.
- the maximum intensity was raised from 50 to 100 cascaded all-pass filters per channel.
- the processor uses the host sample rate when setting up filters instead of assuming `44100 hz`.
- temporary audio buffers are prepared outside the realtime processing path instead of being allocated inside every block.
- mono and stereo layouts are handled through `juce` bus layout checks.
- `clang-format`, optional `clang-tidy`, and a windows `vst3` build workflow were added.

the original pure dsp files are intentionally still recognizable:

- `AllPassFilter` remains the filter used by the processing path.
- `LRCrossoverFilter` and `HardClip` are still present, but they are not part of the main audible processing path.
- the exponential frequency mapping from the original knob behavior is preserved internally.
- `Q` is still clamped to a minimum true value of `0.005` to avoid unstable low values.
- the low-frequency state reset safeguard is preserved for fast modulation.
- silence detection is still used to reduce idle processing.

## parameters

- `Frequency`: filter frequency in `hz`. the host-facing range is approximately `20 hz` to `20 khz`, using the
  original exponential knob curve internally.
- `Q`: filter resonance. values are clamped to a minimum true value of `0.005` in the processing path.
- `Intensity`: number of cascaded all-pass filters, from `0` to `100`. `0` bypasses the audible effect.
- `Mix`: dry/wet blend. `0` bypasses the audible effect.

## build requirements

- `cmake` `3.22` or newer
- a `c++17` compiler
- `git`, used by `cmake` `FetchContent` to download `juce`
- platform plugin tooling:
  - windows: `visual studio 2022` build tools or newer
  - macos: `xcode` command line tools
  - linux: a supported compiler plus the usual `juce` linux development packages for audio/plugin builds

## configure and build

on windows:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AllPassPhase_VST3
```

the generated `vst3` bundle is placed under the `cmake` build tree. with the visual studio generator, the path is
usually:

```text
build/AllPassPhase_artefacts/Release/VST3/AllPassPhase+.vst3
```

on macos or linux, use a normal single-config `cmake` generator:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target AllPassPhase_VST3
```

## quality tools

formatting is available when `clang-format` is installed:

```powershell
cmake --build build --target format
```

`clang-tidy` is opt-in:

```powershell
cmake -S . -B build -DALLPASSPHASE_ENABLE_CLANG_TIDY=ON
cmake --build build --config Release --target AllPassPhase_VST3
```

## validation

after building, load the generated `vst3` in a host and check:

- stereo audio passes correctly.
- mono audio passes correctly.
- `Frequency`, `Q`, `Intensity`, and `Mix` appear in the host and can be automated.
- `Intensity` can move between `0` and `100` without crashes or memory churn.
- `Mix` at `0` and `Intensity` at `0` bypass the audible effect.
- state is restored after saving and reopening a project.
- low-frequency automation does not produce obvious instability.
