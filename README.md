# gta4-reversed

[![Build ASI (x86)](https://github.com/gemini25873-dot/gta4-reversed/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/gemini25873-dot/gta4-reversed/actions/workflows/build.yml)

An open-source C++20 reverse engineering and compatibility project targeting **Grand Theft Auto IV (`1.0.7.0` x86)**.

The project builds into `gta4_reversed.asi` and hooks reversed RAGE engine and game classes at runtime, allowing incremental replacement and verification inside a real copy of `GTAIV.exe` without modifying original game files.

> **Disclaimer:** This repository contains **no copyrighted game assets, binaries, or proprietary data**. It is strictly a research and modding SDK effort and requires a legitimate copy of Grand Theft Auto IV (`1.0.7.0`) to run.

---

## Current Progress

- **Target Binary:** `GTAIV.exe` `1.0.7.0` (x86, `43,740` functions)
- **Hooked Functions:** `226` implemented (`222` active at runtime; `~0.52%`)
- **Pure C++ Modules (`0` `Core::Hook::Call` fallbacks):**
  - `Core`: `Matrix.cpp`, `Pool.cpp`, `Vector.cpp`
  - `Game`: `LoadingScreens.cpp`
  - `Rage`: `phBound.cpp`, `phBoundComposite.cpp`, `phInst.cpp`, `phArticulatedCollider.cpp`, `phConstraint.cpp`, `phLevelNew.cpp`

### Module Breakdown

| Module | File | Hooked Functions | Remaining `Hook::Call` | Notes |
| --- | --- | ---: | ---: | --- |
| Core | `source/Core/Matrix.cpp` | 1 | 0 | Pure C++ |
| Core | `source/Core/Pool.cpp` | 5 | 0 | Pure C++ |
| Core | `source/Core/Vector.cpp` | 2 | 0 | Pure C++ |
| Game | `source/Game/Cam.cpp` | 6 | 8 | |
| Game | `source/Game/Cheat.cpp` | 40 | 22 | |
| Game | `source/Game/Clock.cpp` | 10 | 2 | |
| Game | `source/Game/DynamicEntity.cpp` | 7 | 11 | |
| Game | `source/Game/Entity.cpp` | 8 | 6 | |
| Game | `source/Game/General.cpp` | 4 | 2 | |
| Game | `source/Game/LoadingScreens.cpp` | 1 | 0 | Pure C++ (intro skip enabled) |
| Game | `source/Game/Object.cpp` | 4 | 10 | |
| Game | `source/Game/Ped.cpp` | 8 | 21 | |
| Game | `source/Game/Physical.cpp` | 9 | 8 | |
| Game | `source/Game/Stats.cpp` | 19 | 8 | |
| Game | `source/Game/Timer.cpp` | 10 | 15 | |
| Game | `source/Game/Vehicle.cpp` | 4 | 13 | |
| Game | `source/Game/WeaponInfo.cpp` | 13 | 15 | |
| Game | `source/Game/Weather.cpp` | 5 | 9 | |
| Game | `source/Game/World.cpp` | 4 | 3 | `CPhysicsWorld::InjectHooks()` currently disabled in `CWorld::InjectHooks()` |
| Rage | `source/Rage/phArchetype.cpp` | 15 | 6 | |
| Rage | `source/Rage/phArticulatedCollider.cpp` | 4 | 0 | Pure C++ |
| Rage | `source/Rage/phBound.cpp` | 4 | 0 | Pure C++ |
| Rage | `source/Rage/phBoundBVH.cpp` | 8 | 5 | |
| Rage | `source/Rage/phBoundComposite.cpp` | 10 | 0 | Pure C++ |
| Rage | `source/Rage/phCollider.cpp` | 8 | 2 | |
| Rage | `source/Rage/phConstraint.cpp` | 5 | 0 | Pure C++ |
| Rage | `source/Rage/phInst.cpp` | 8 | 0 | Pure C++ |
| Rage | `source/Rage/phLevelNew.cpp` | 3 | 0 | Pure C++ |
| Rage | `source/Rage/phSimulator.cpp` | 1 | 1 | |

---

## Compatibility Features (`dllmain.cpp`)

- **Complete Edition Compatibility on `1.0.7.0`** *(for downgraded Complete Edition installs)*:
  - Multi-WTD DLC radio HUD logo fix
  - Stubbing missing Complete Edition script natives (e.g., `0x2C89A1F9` / `SCRT70`)

---

## Repository Structure

```text
source/
├── Core/        # Hook engine, logging, math (Vector/Matrix), Pool helpers
├── Game/        # GTA IV game classes (CEntity, CPed, CVehicle, CCam, CWeaponInfo, CWorld, ...)
├── Rage/        # RAGE physics engine (phBound*, phArchetype, phInst, phCollider*, phLevelNew, ...)
└── dllmain.cpp  # ASI entry point, VEH crash logger, compatibility & hook registration
```

---

## Building

### Requirements
- Windows (x86 target)
- MSVC x86 C++20 toolchain (Visual Studio / Build Tools or Portable MSVC)
- CMake `3.20+` and Ninja (or Visual Studio generator)
- No external third-party libraries required

### Build Steps

You can run `build.bat` directly, or build manually from an **x86 Native Tools Command Prompt**:

```bat
cmake -S . -B build/Release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/Release
```

The compiled output is written to:
- `bin/Release/gta4_reversed.asi`
- `bin/Release/gta4_reversed.pdb`

### Running in Game
1. Make sure you have a working, legitimate GTA IV `1.0.7.0` installation with an ASI loader (e.g., Ultimate ASI Loader). If you are running a downgraded Steam / Complete Edition installation with **ZolikaPatch**, keep only `LaunchWithPlayGTAIV=1` and `SkipLauncher=1` enabled in `ZolikaPatch.ini` (disable other optional patches to avoid hook conflicts with `gta4_reversed.asi`).
2. Copy `bin/Release/gta4_reversed.asi` (and optionally `.pdb` for crash stack traces) into your GTA IV root folder.
3. Launch the game (`PlayGTAIV.exe` / `GTAIV.exe`) and check `gta4_reversed.log` for initialization and any VEH diagnostics.

---

## Contributing

Contributions are very welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for coding conventions, hooking rules, struct verification guidelines, and good starting areas.