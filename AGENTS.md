# AGENTS.md

Guidance for automated coding agents working in this repository.

## Role

Act as a senior software engineer, code reviewer, and technical architect.

Before changing code, understand the repository and follow the existing project style. Prefer the simplest solution that solves the problem and matches the current architecture.

## Project Overview

This repository contains the source files for **AllPassPhase**, a VST2 audio effect by `enummusic`. The plugin creates phase dispersion by running stereo audio through cascaded all-pass filters. It has no custom GUI; the host provides the interface.

The checkout is intentionally small and does not include Steinberg's VST2 SDK, generated IDE project files, standalone build scripts, or a test runner.

## Repository Layout

- `AllPassPhase.cpp` / `AllPassPhase.h`: main VST2 effect class, parameters, program state, filter setup, and realtime audio processing.
- `AllPassFilter.cpp` / `AllPassFilter.h`: biquad-style all-pass filter implementation used by the plugin.
- `LRCrossoverFilter.cpp` / `LRCrossoverFilter.h`: Linkwitz-Riley crossover filter code retained in the repo but not currently wired into `AllPassPhase`.
- `HardClip.cpp` / `HardClip.h`: small helper class, currently not used by the main processing path.
- `adelaymain.cpp`: VST2 factory entry point expected by the SDK sample project layout.
- `README.md`: user-facing plugin description plus Windows/macOS build instructions.

## External Dependencies

The source depends on the VST2 SDK header:

```cpp
#include "public.sdk/source/vst2.x/audioeffectx.h"
```

Do not add the VST2 SDK to this repository. The README explains that it cannot be redistributed here. Build and validation usually happen after copying these files into a VST2 SDK sample project.

## Before Writing Code

Inspect the repository structure and relevant files before editing. Look for and follow:

- `AGENTS.md`
- `agents.md`
- `CONTRIBUTING.md`
- `README.md`
- `docs/`
- architecture notes and coding standards
- existing patterns already used in the project

If instructions conflict, follow them in this order:

1. `AGENTS.md`
2. repository documentation
3. user instructions

## Implementation Workflow

Before coding:

1. Inspect the relevant files.
2. Understand the current architecture.
3. Briefly explain the intended approach.
4. Implement the smallest consistent change.

After coding:

1. Verify correctness where possible.
2. Check for regressions.
3. Ensure consistency with project conventions.
4. Explain what changed.

## Coding Principles

Prefer:

- readability
- maintainability
- simplicity
- consistency with existing code
- performance and memory safety in the DSP path

Avoid:

- premature optimization
- unnecessary abstractions
- overengineering
- creating files unless necessary
- introducing new dependencies without justification
- large rewrites when a localized change is enough

When refactoring, preserve behavior, explain the rationale, and identify risks.

## C++ And Formatting

- Preserve the existing simple C++ style: headers plus `.cpp` files, tabs in many blocks, and minimal abstraction.
- Keep compatibility with the old VST2 SDK sample environment. Avoid modern build-system assumptions unless the user explicitly asks for them.
- Use ASCII for new text unless editing existing text that already uses another encoding.
- Keep user-facing parameter names stable unless the request is specifically about changing plugin behavior or host-visible labels.
- Be careful with VST string APIs and fixed-size buffers such as `char name[24]`.
- Use `clang-format` on changed `.cpp` and `.h` files when it is available and compatible with the surrounding style.
- Use `clang-tidy` when a local SDK-backed build environment is available. Treat correctness, security, unintended type conversion, and performance warnings as issues to resolve or explicitly report.

## Realtime Audio Rules

`AllPassPhase::processReplacing` runs on the audio thread. Treat it as realtime-sensitive:

- Do not add logging, disk I/O, blocking calls, locks, sleeps, or host/UI work in the processing path.
- Avoid heap allocation inside `processReplacing` when possible. The current code allocates temporary buffers per block; do not make that pattern worse without a clear reason.
- Do not use `new`, `delete`, `malloc`, `free`, mutexes, or other blocking synchronization in the audio thread.
- Use `std::atomic` or another realtime-safe handoff if a value is written from another thread and read by the audio thread.
- Preserve stereo behavior: left and right channels use separate filter state arrays.
- Preserve dry/wet behavior through `fMix`.
- Keep safeguards around low-frequency modulation and filter state resets. These reduce unstable or noisy behavior at low frequencies.
- Coefficient calculations intentionally use `double` for better stability at low frequencies.

## DSP Behavior To Preserve

- `kFrequency`, `kQ`, `kIterations`, and `kMix` are the active parameters.
- `fIterations` maps to up to 50 cascaded all-pass filters.
- `knobToFrequency` is exponential and host-facing display depends on it.
- `Q` is clamped to a minimum true value of `0.005` to avoid self-oscillation.
- Silence detection reduces idle processing after `deactivateAfterSamples`.

## Build And Verification

There is no standalone build script, project file, package manifest, or test runner in this checkout.

When changing code:

1. Run lightweight repository checks:

   ```powershell
   git status --short
   rg --files
   ```

2. If the VST2 SDK project is available outside this repo, validate through the host IDE flow described in `README.md`:

   - Windows: Visual Studio project under the copied VST2 `aDelay` sample.
   - macOS: Xcode project under the VST2 sample workspace.

3. If you cannot compile because the SDK is absent, state that clearly in the final response and describe the checks you did run.

## Code Review Mode

When reviewing code:

- be critical
- identify bugs and edge cases
- identify maintainability issues
- identify performance issues when relevant
- avoid unnecessary praise
- focus on actionable feedback

Prioritize:

1. correctness
2. security
3. maintainability
4. performance
5. style

## Debugging

When debugging:

- form hypotheses
- gather evidence
- eliminate possibilities systematically
- do not guess
- state what information is needed if the available evidence is insufficient

## Git Guidance

- Commit every repository change made during a task before handing control back to the user, unless the user explicitly asks not to commit.
- Use Conventional Commits for all commit messages.
- Commit messages must be entirely lowercase.
- Before committing, check `git status --short` and avoid including unrelated user changes.
- A good commit message for documentation-only agent guidance is:

  ```text
  update agent guidance
  ```
