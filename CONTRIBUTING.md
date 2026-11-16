# Contributing to gta4-reversed

Thank you for helping reverse engineer and reimplement GTA IV (`1.0.7.0`)!

## Target & Environment
- **Binary:** `GTAIV.exe` **v1.0.7.0** (32-bit x86).
- **Language:** C++20 (`/W4`, no external dependencies).
- **Addresses:** Always document and hook using `1.0.7.0` virtual addresses (rebased dynamically at runtime via `Core::Hook`).

---

## Coding & Hooking Conventions

1. **Class & Hook Structure**
   - Keep each class in its matching `source/Game/` or `source/Rage/` header/source pair.
   - Expose a `static void InjectHooks();` method and register it in `InitPlugin()` inside `source/dllmain.cpp`.
   - Use `Core::Hook::Install(...)` for detouring functions.

2. **Verify Struct Layouts**
   - Always verify field offsets and class sizes locally against disassembly/RTTI and add `static_assert(sizeof(ClassName) == 0x...)` wherever the full object size is known.
   - Preserve exact calling conventions (`__thiscall`, `__stdcall`, `__cdecl`, `__fastcall`). Watch out for MSVC LTCG helpers that pass arguments in non-standard registers (`__usercall`)—these must be rewritten cleanly in C++ rather than hooked directly.

3. **Eliminate `Core::Hook::Call` / `CallMethod` Over Time**
   - Calling back into original game functions via `Core::Hook::Call` is fine while bootstrapping a class, but the goal is pure C++ implementations with `0` `Hook::Call` dependencies.
   - If a helper is small or already understood, implement it in C++ directly instead of adding another `Call` wrapper.

4. **Test Incrementally**
   - Build in Release (`x86`) and test in-game with a legitimate `GTAIV.exe 1.0.7.0` installation.
   - Verify both normal gameplay (spawning, driving, physics collisions, weapon firing, camera movement) and `gta4_reversed.log` for any VEH exceptions.
   - If a new hook introduces a regression, isolate the class/function before committing.

---

## Good Places to Start

- **Pure C++ Cleanup:** Pick a file in `source/Game/` or `source/Rage/` that still has `Core::Hook::Call` / `CallMethod` calls (see the table in `README.md`) and replace them with verified C++ implementations.
- **Expand Existing Subsystems:**
  - `CPhysicsWorld` (`source/Game/World.cpp`)
  - `source/Rage/` physics classes (`phBoundBVH`, `phCollider`, `phSimulator`).
  - `source/Game/` core entities (`CEntity`, `CDynamicEntity`, `CPhysical`, `CPed`, `CVehicle`, `CWeaponInfo`, `CCam`).
- **Documentation:** Document verified field offsets, calling conventions, and virtual addresses in code comments and PR descriptions (please do not commit raw disassembly/decompiler dumps or copyrighted binaries).