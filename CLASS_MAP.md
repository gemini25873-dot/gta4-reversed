# GTA IV (`1.0.7.0` x86) Class & Function Map

This document maps the real binary scope of `GTAIV.exe` (`1.0.7.0` x86) against the currently implemented modules in `gta4-reversed`. It contains **only metadata** (class names, RTTI/vtable addresses, virtual slot counts, and hook counts — no decompiled code).

---

## 1. Binary Overview (`GTAIV.exe 1.0.7.0`)

- **Total Functions:** `43,740`
- **RTTI Type Descriptors (`.?AV` / `.?AU`):** `4,019`
  - `rage::` (RAGE engine: physics, audio, graphics, streaming, net): `2,773`
  - `C*` (Game classes, excluding `CTask*`): `685`
  - `CTask*` (Ped/Vehicle AI task classes): `331`
  - `aud*` (Top-level audio classes): `27`
  - Other (`ART`, `NMutils`, callbacks, templates): `203`
- **Mapped Vtables:** `3,414` (`44,608` total virtual function slots across all vtables)
- **Currently Implemented Hooks:** `538` (`535` in `Core/`/`Game/`/`Rage/` + `3` compatibility hooks in `dllmain.cpp`)

> **Note:** Static/utility classes without virtual functions (`CTimer`, `CClock`, `CWeather`, `CWorld`, `CGeneral`, `CPickups`, `CGame`, `CFileMgr`, `CStreaming`, etc.) do not have RTTI entries or vtables; their total function count must be tracked via call-graph/namespace clustering in IDA.

---

## 2. Implemented Modules vs. Vtable & Dependency Status

| Module | File | Hooked | Unique `Hook::Call` | Call Sites | IDA Vtable | Vtable Slots | Status |
| --- | --- | ---: | ---: | ---: | --- | ---: | --- |
| Core | `source/Core/Matrix.cpp` | 1 | 0 | 0 | — | — | Pure C++ |
| Core | `source/Core/Pool.cpp` | 9 | 0 | 0 | — | — | Pure C++ |
| Core | `source/Core/Vector.cpp` | 2 | 0 | 0 | — | — | Pure C++ |
| Game | `source/Game/LoadingScreens.cpp` | 1 | 0 | 0 | — | — | Pure C++ |
| Game | `source/Game/Pad.cpp` | 14 | 0 | 0 | — | — | Pure C++ |
| Game | `source/Game/DummyObject.cpp` | 8 | 1 | 1 | `0xDA5994` | 51 | Leaf candidate |
| Game | `source/Game/ModelInfo.cpp` | 11 | 1 | 1 | `0xD7CD04` (Base) | 15 | Leaf candidate |
| Game | `source/Game/General.cpp` | 4 | 0 | 0 | — | — | Pure C++ |
| Game | `source/Game/IplStore.cpp` | 15 | 0 | 0 | — | — | Pure C++ |
| Game | `source/Game/Clock.cpp` | 10 | 1 | 2 | — | — | Leaf candidate |
| Game | `source/Game/FileMgr.cpp` | 14 | 2 | 3 | — | — | Leaf candidate |
| Game | `source/Game/Building.cpp` | 5 | 3 | 3 | `0xDA3A3C` | 51 | Partial |
| Game | `source/Game/Camera.cpp` | 15 | 3 | 6 | — | — | Partial |
| Game | `source/Game/TxdStore.cpp` | 23 | 3 | 3 | — | — | Partial |
| Game | `source/Game/World.cpp` | 4 | 3 | 3 | — | — | Partial |
| Game | `source/Game/Zones.cpp` | 8 | 4 | 10 | — | — | Partial |
| Game | `source/Game/Entity.cpp` | 8 | 4 | 4 | `0xD83BAC` | 51 | Partial |
| Game | `source/Game/Explosion.cpp` | 10 | 4 | 6 | — | — | Partial |
| Game | `source/Game/Garages.cpp` | 13 | 5 | 5 | — | — | Partial |
| Game | `source/Game/Physical.cpp` | 9 | 5 | 8 | `0xD89F2C` | 72 | Partial |
| Game | `source/Game/Stats.cpp` | 19 | 6 | 8 | — | — | Partial |
| Game | `source/Game/Wanted.cpp` | 10 | 6 | 8 | — | — | Partial |
| Game | `source/Game/Cam.cpp` | 7 | 7 | 9 | `0xD80464` | 11 | Partial |
| Game | `source/Game/Streaming.cpp` | 19 | 7 | 7 | — | — | Partial |
| Game | `source/Game/WeaponInfo.cpp` | 13 | 7 | 15 | — | — | Partial |
| Game | `source/Game/Weather.cpp` | 5 | 7 | 9 | — | — | Partial |
| Game | `source/Game/Object.cpp` | 4 | 8 | 10 | `0xD6D47C` | 72 | Partial |
| Game | `source/Game/PlayerInfo.cpp` | 18 | 8 | 13 | — | — | Partial |
| Game | `source/Game/DynamicEntity.cpp` | 7 | 10 | 11 | `0xD82E14` | 55 | Partial |
| Game | `source/Game/Pickups.cpp` | 10 | 10 | 12 | — | — | Partial |
| Game | `source/Game/Timer.cpp` | 10 | 11 | 16 | — | — | Partial |
| Game | `source/Game/Vehicle.cpp` | 4 | 12 | 13 | `0xD82164` | 111 | Partial |
| Game | `source/Game/Cheat.cpp` | 40 | 14 | 21 | — | — | Partial |
| Game | `source/Game/Ped.cpp` | 8 | 18 | 20 | `0xD69844` | 81 | Partial |
| Game | `source/Game/Radar.cpp` | 25 | 20 | 63 | — | — | Partial |
| Game | `source/Game/Font.cpp` | 71 | 22 | 26 | — | — | Partial |
| Game | `source/Game/Game.cpp` | 7 | 29 | 29 | — | — | Skeleton |
| Rage | `source/Rage/phArticulatedCollider.cpp` | 4 | 0 | 0 | `0xEBC78C` | 52 | Pure C++ |
| Rage | `source/Rage/phBound.cpp` | 4 | 0 | 0 | `0xEBB9CC` | 52 | Pure C++ |
| Rage | `source/Rage/phBoundComposite.cpp` | 10 | 0 | 0 | `0xEBBB2C` | 54 | Pure C++ |
| Rage | `source/Rage/phCollider.cpp` | 9 | 0 | 0 | `0xEBBD14` | 50 | Pure C++ |
| Rage | `source/Rage/phConstraint.cpp` | 5 | 0 | 0 | `0xEBC778` | 3 | Pure C++ |
| Rage | `source/Rage/phInst.cpp` | 8 | 0 | 0 | `0xEBBAA4` | 32 | Pure C++ |
| Rage | `source/Rage/phLevelNew.cpp` | 3 | 0 | 0 | `0xEBBE30` | 1 | Pure C++ |
| Rage | `source/Rage/fiStream.cpp` | 7 | 1 | 6 | — | — | Leaf candidate |
| Rage | `source/Rage/phSimulator.cpp` | 1 | 1 | 1 | `0xEBBCF0` | 8 | Leaf candidate |
| Rage | `source/Rage/phBoundBVH.cpp` | 8 | 2 | 5 | `0xEBC2A4` | 63 | Partial |
| Rage | `source/Rage/phArchetype.cpp` | 15 | 3 | 6 | `0xEBC19C` | 21 | Partial |

---

## 3. Major Unimplemented Subsystems (From IDA RTTI)

- **AI & Tasks (`CTask*`):** `331` RTTI classes (ped/vehicle/combat/motion tasks).
- **Audio (`aud*` / `rage::aud*`):** Sound engine, radio, speech, SFX managers.
- **Graphics (`rage::grc*` / `rage::grm*`):** D3D9 device, shaders, drawables, textures.
- **Script Engine (`rage::scrThread`, `CTheScripts`):** Script VM & native registration.
- **World Gameplay Managers:** `CTheZones`, `CGarages`, `CExplosion`, `CPathFind`, `CCarGenerator`, `CRelationship`, `CStuntJumpManager`.

---

## 4. Reference SDKs & Methodology

Just as `gta-reversed` cross-references `plugin-sdk`, we can cross-reference public GTA IV `1.0.7.0` community SDKs:
- **[Zolika1351/iv-sdk](https://github.com/Zolika1351/iv-sdk):** Class headers, struct offsets, enums, and `1.0.7.0` addresses.
- **[ClonkAndre/IV-SDK-DotNet](https://github.com/ClonkAndre/IV-SDK-DotNet):** Managed wrappers and documented field semantics.

**Verification Rule:** Never copy structs blindly. Always verify every offset, calling convention (`__thiscall` `ECX` vs `__cdecl` vs `__usercall` in IDA disassembly), and struct size (`VALIDATE_SIZE` / `VALIDATE_OFFSET`) against `GTAIV.exe 1.0.7.0` before hooking.
