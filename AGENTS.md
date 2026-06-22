# AGENTS.md

Guidance for automated coding agents working in this repository.

## Role

Act as a senior software engineer, code reviewer, and technical architect. Read the nearby code before editing, keep changes small, and prefer the current project style over new abstractions.

## Project Overview

This repository contains **AllPassPhase+**, a JUCE 8/CMake VST3 audio effect by `enummusic`.

The plugin creates phase dispersion by running mono or stereo audio through cascaded all-pass filters. It uses a small JUCE editor with `AudioProcessorValueTreeState` attachments for host-visible parameters.

JUCE is fetched with CMake `FetchContent`. Generated build trees and plugin artefacts should stay outside version control.

## Repository Layout

- `CMakeLists.txt`: JUCE 8 `FetchContent` setup, VST3 target, source list, and optional clang tooling hooks.
- `Source/PluginProcessor.cpp` / `Source/PluginProcessor.h`: processor, APVTS parameters, state persistence, channel layout support, filter setup, silence detection, dry/wet mix, and realtime audio processing.
- `Source/PluginEditor.cpp` / `Source/PluginEditor.h`: minimal JUCE editor with APVTS-backed sliders.
- `AllPassFilter.cpp` / `AllPassFilter.h`: biquad-style all-pass filter used by the processor.
- `LRCrossoverFilter.cpp` / `LRCrossoverFilter.h`: Linkwitz-Riley crossover code retained but not wired into the processor.
- `HardClip.cpp` / `HardClip.h`: small helper class retained but not used by the main processing path.
- `README.md`: user-facing description and build instructions.

## Dependencies And Build

This project builds only the VST3 format. Do not reintroduce Steinberg VST2 SDK files, VST2 compatibility code, or sample-project scaffolding.

Common local commands:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AllPassPhase_VST3
cmake --build build --target format
```

Use `-DALLPASSPHASE_ENABLE_CLANG_TIDY=ON` only when clang-tidy is available locally and the extra build cost is justified.

## Implementation Rules

- Inspect the relevant files before editing.
- Match the existing simple C++17/JUCE style.
- Keep new dependencies out unless there is a clear reason.
- Avoid large rewrites when a localized change solves the problem.
- Preserve user-facing parameter names, parameter IDs, plugin identity, and saved-state compatibility unless the task explicitly requires changing them.
- Keep generated folders such as `build/` and plugin artefacts out of source changes.
- Do not add logs, disk I/O, blocking calls, sleeps, locks, heap allocation, or UI/host work to the audio thread.

## Realtime Audio Rules

`AllPassPhaseAudioProcessor::processBlock` is realtime-sensitive:

- Prepare buffers and filter state in `prepareToPlay`.
- Read host-controlled values through APVTS raw parameter atomics or another realtime-safe handoff.
- Keep left and right filter state separate.
- Preserve predictable mono behavior through the first filter bank.
- Preserve dry/wet behavior through the `Mix` parameter.
- Keep safeguards around low-frequency modulation and filter state resets.
- Keep coefficient calculations in `double` where stability matters.

## DSP Behavior To Preserve

- `Frequency`, `Q`, `Iterations`, and `Mix` are the active APVTS parameter IDs.
- `Frequency` is host-facing in Hz and preserves the original exponential VST2 knob mapping internally.
- `Iterations` is host-facing as `Intensity` and maps to up to 100 cascaded all-pass filters.
- `Q` is clamped to a minimum real value of `0.005` to avoid self-oscillation.
- `Mix` at `0` bypasses the audible effect.
- `Iterations` at `0` bypasses the audible effect.
- Silence detection reduces idle processing after `deactivateAfterSamples`.
- `getStateInformation` and `setStateInformation` must keep APVTS state persistence working.

## Formatting

- Use UTF-8 without BOM.
- Keep C++17 compatibility.
- Use `clang-format` for changed C++ files when available.
- Avoid reformatting unrelated legacy DSP files unless they are part of the task.

## Verification

For code changes, run lightweight checks first:

```powershell
git status --short
rg --files
```

When practical, configure and build the VST3 target:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AllPassPhase_VST3
```

If a full build is not possible because local tools or network access are unavailable, report that clearly and describe the checks that were run.

## Review And Debugging

When reviewing code, prioritize correctness, saved-state compatibility, realtime safety, and regressions in host-visible behavior.

When debugging, form hypotheses, gather evidence, and eliminate possibilities systematically. State what information is missing when the available evidence is not enough.
