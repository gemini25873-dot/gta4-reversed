#include "StdInc.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include "Core/SteamAuth.h"
#include "Core/Vector.h"
#include "Core/Matrix.h"
#include "Core/Pool.h"
#include "Game/Timer.h"
#include "Game/Clock.h"
#include "Game/Weather.h"
#include "Game/Cheat.h"
#include "Game/Stats.h"
#include "Game/General.h"
#include "Game/Entity.h"
#include "Game/DynamicEntity.h"
#include "Game/Physical.h"
#include "Game/Ped.h"
#include "Game/Vehicle.h"
#include "Game/Object.h"
#include "Game/LoadingScreens.h"
#include "Game/World.h"
#include "Game/WeaponInfo.h"
#include "Game/Cam.h"
#include "Rage/phBound.h"
#include "Rage/phBoundComposite.h"
#include "Rage/phBoundBVH.h"
#include "Rage/phArchetype.h"
#include "Rage/phInst.h"
#include "Rage/phCollider.h"
#include "Rage/phArticulatedCollider.h"
#include "Rage/phConstraint.h"
#include "Rage/phLevelNew.h"
#include "Rage/phSimulator.h"

static void FormatAddressModule(uintptr_t addr, char* out, size_t outSize) {
    const uintptr_t gameBase = Core::GetBaseAddr();
    HMODULE mod = nullptr;
    if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(addr),
            &mod) && mod) {
        char path[MAX_PATH]{};
        GetModuleFileNameA(mod, path, sizeof(path));
        const char* name = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '\\' || *p == '/') {
                name = p + 1;
            }
        }
        const uintptr_t modBase = reinterpret_cast<uintptr_t>(mod);
        if (modBase == gameBase) {
            std::snprintf(out, outSize, "%s+0x%08X (IDA: 0x%08X)",
                name,
                static_cast<uint32>(addr - modBase),
                static_cast<uint32>(0x400000u + (addr - modBase)));
        } else {
            std::snprintf(out, outSize, "%s+0x%08X", name, static_cast<uint32>(addr - modBase));
        }
    } else {
        std::snprintf(out, outSize, "unknown");
    }
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ex) {
    if (!ex || !ex->ExceptionRecord || !ex->ContextRecord) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    const EXCEPTION_RECORD* rec = ex->ExceptionRecord;
    const CONTEXT* ctx = ex->ContextRecord;
    const uintptr_t addr = reinterpret_cast<uintptr_t>(rec->ExceptionAddress);

    char modInfo[160]{};
    FormatAddressModule(addr, modInfo, sizeof(modInfo));

    Core::Log::Error("=== CRASH DETECTED ===");
    Core::Log::Error("Code: 0x%08X | Addr: 0x%08X (%s)",
        static_cast<uint32>(rec->ExceptionCode),
        static_cast<uint32>(addr),
        modInfo);
    Core::Log::Error("EAX=%08X EBX=%08X ECX=%08X EDX=%08X",
        ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx);
    Core::Log::Error("ESI=%08X EDI=%08X EBP=%08X ESP=%08X EIP=%08X",
        ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp, ctx->Eip);

    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(reinterpret_cast<LPCVOID>(ctx->Esp), &mbi, sizeof(mbi)) &&
        mbi.State == MEM_COMMIT &&
        (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
        const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
        for (int i = 0; i < 8; ++i) {
            const uintptr_t sp = static_cast<uintptr_t>(ctx->Esp) + static_cast<uintptr_t>(i * 4);
            if (sp + 4 > regionEnd) {
                break;
            }
            const uint32 val = *reinterpret_cast<const uint32*>(sp);
            char stackMod[160]{};
            FormatAddressModule(val, stackMod, sizeof(stackMod));
            Core::Log::Error("Stack[+%02X]: 0x%08X (%s)", i * 4, val, stackMod);
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI VectoredCrashHandler(EXCEPTION_POINTERS* ex) {
    if (!ex || !ex->ExceptionRecord) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    const DWORD code = ex->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_ACCESS_VIOLATION &&
        code != EXCEPTION_ILLEGAL_INSTRUCTION &&
        code != EXCEPTION_INT_DIVIDE_BY_ZERO &&
        code != EXCEPTION_STACK_OVERFLOW &&
        code != EXCEPTION_PRIV_INSTRUCTION) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    static thread_local bool t_inCrash = false;
    if (t_inCrash) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    t_inCrash = true;
    const LONG result = CrashHandler(ex);
    t_inCrash = false;
    return result;
}

// Complete Edition -> 1.0.7.0 script native compatibility (0x5A76D0, __usercall @<eax>(hash@<esi>)):
// Complete Edition script.img calls newer/multiplayer natives (e.g. 0x2C89A1F9 / 747217401 on phone open)
// that don't exist in 1.0.7.0, causing SCRT70. Stub missing natives to safely return 0.
static void __cdecl StubNativeReturnZero(uint32* ctx) {
    if (ctx && ctx[0]) {
        *reinterpret_cast<uint32*>(ctx[0]) = 0;
    }
}

static uint32 __stdcall LookupOrStubNative(uint32 hash) {
    const uint32 capacity = Core::StaticRef<uint32>(0x198332C);
    const uint32* table = Core::StaticRef<uint32*>(0x1983328);
    if (capacity != 0 && table != nullptr) {
        uint32 idx = hash % capacity;
        uint32 probe = hash;
        uint32 entryHash = table[2 * idx];
        if (entryHash != hash) {
            while (entryHash != 0) {
                probe = (probe >> 1) + 1;
                idx = (probe + idx) % capacity;
                entryHash = table[2 * idx];
                if (entryHash == hash) {
                    break;
                }
            }
        }
        if (entryHash == hash) {
            return table[2 * idx + 1];
        }
    }

    Core::Log::Info("[Script] Stubbed missing 1.0.7.0 native: 0x%08X (%u)", hash, hash);
    return reinterpret_cast<uint32>(&StubNativeReturnZero);
}

static void __declspec(naked) HookedLookupNative() {
    __asm {
        push esi
        call LookupOrStubNative
        ret
    }
}

// Complete Edition multi-WTD radio logo fix (0x821500 / 0x821520, __thiscall):
// radiohud.dat lists both base and _eflc WTDs; don't overwrite an already-loaded sprite with null
// when scanning a WTD that doesn't contain that station's logo.
static char __fastcall HookedLoadRadioColoredLogo(uint8* self) {
    if (*reinterpret_cast<uint32*>(self + 4) != 0) {
        return 1;
    }
    if (self[0x10F] == 0) {
        return 0;
    }
    Core::Hook::CallMethod<int>(0x8534A0, self + 4, reinterpret_cast<const char*>(self + 0x10F));
    return 1;
}

static char __fastcall HookedLoadRadioNonColoredLogo(uint8* self) {
    if (*reinterpret_cast<uint32*>(self + 8) != 0) {
        return 1;
    }
    if (self[0x10] == 0) {
        return 0;
    }
    Core::Hook::CallMethod<int>(0x8534A0, self + 8, reinterpret_cast<const char*>(self + 0x10));
    return 1;
}

// Complete Edition local DLC mounting (0x813F30, __thiscall): mounts TLAD & TBoGT folders directly
static void MountLocalDlcFolder(uint8* self, const char* folder, const char* listFile, char episodeFlag, uintptr_t guidIdaAddr) {
    char checkPath[MAX_PATH]{};
    std::snprintf(checkPath, sizeof(checkPath), "%s\\%s", folder, listFile);
    if (GetFileAttributesA(checkPath) == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    uint8 entry[360]{};
    std::strcpy(reinterpret_cast<char*>(entry), folder);
    if (Core::Hook::CallStdcall<char>(0x813480, episodeFlag, entry, 1)) {
        const int slot = Core::Hook::CallMethod<int>(0x813E20, self, entry);
        auto* guidDst = reinterpret_cast<uint8*>(Core::Rebase(0x11A32D8) + 32 * slot);
        std::memset(guidDst, 0, 32);
        std::memcpy(guidDst + 12, reinterpret_cast<const void*>(Core::Rebase(guidIdaAddr)), 16);
        Core::Log::Info("Mounted local DLC: %s (slot=%d)", folder, slot);
    }
}

static int __fastcall HookedLoadDLCs(uint8* self) {
    const uint16 count = *reinterpret_cast<const uint16*>(self + 336);
    for (uint16 i = 0; i < count; ++i) {
        Core::Hook::CallMethod<int>(0x8137A0, self, static_cast<int>(i));
    }

    Core::Hook::Call<void>(0x5AC2C0);
    Core::Hook::Call<void>(0x5AADE0);

    if (Core::SteamAuth::IsVerified()) {
        MountLocalDlcFolder(self, "TLAD", "E1_List.txt", 2, 0xF1CAC4);
        MountLocalDlcFolder(self, "TBoGT", "E2_List.txt", 4, 0xF1CAD8);
    } else {
        Core::Log::Info("[SteamAuth] Skipping local DLC mount (ownership not verified).");
    }

    self[358] = 0;
    const int res = Core::Hook::CallMethod<int>(0x813BD0, self);
    self[358] = 0;
    return res;
}

static void InstallCompatibilityHooks() {
    Core::Hook::Install(0x813F30, &HookedLoadDLCs);
    Core::Hook::Install(0x821500, &HookedLoadRadioColoredLogo);
    Core::Hook::Install(0x821520, &HookedLoadRadioNonColoredLogo);
    Core::Hook::Install(0x5A76D0, &HookedLookupNative);

    Core::Log::Info("Complete Edition compatibility hooks installed (Steam verification: %s)",
        Core::SteamAuth::IsVerified() ? "VERIFIED" : "FAILED");
}

static void InitPlugin() {
    const uintptr_t base = Core::GetBaseAddr();
    Core::Log::Init();
    AddVectoredExceptionHandler(1, VectoredCrashHandler);
    SetUnhandledExceptionFilter(CrashHandler);

    Core::Log::Info("gta4_reversed loaded!");
    Core::Log::Info("GTAIV.exe Base: 0x%08X (ASLR delta: 0x%08X)",
        static_cast<uint32>(base),
        static_cast<uint32>(base - 0x400000u));

    const bool steamOk = Core::SteamAuth::VerifyOwnership();
    Core::Log::Info("[SteamAuth] Result: %s",
        steamOk ? "AppID 12210 ownership verified" : "Ownership not verified");

    InstallCompatibilityHooks();

    CVector::InjectHooks();
    CMatrix::InjectHooks();
    CPoolBase::InjectHooks();
    CGeneral::InjectHooks();
    CTimer::InjectHooks();
    CClock::InjectHooks();
    CWeather::InjectHooks();
    CCheat::InjectHooks();
    CStats::InjectHooks();
    CEntity::InjectHooks();
    CDynamicEntity::InjectHooks();
    CPhysical::InjectHooks();
    CPed::InjectHooks();
    CVehicle::InjectHooks();
    CObject::InjectHooks();
    CLoadingScreens::InjectHooks();
    CWorld::InjectHooks();
    CWeaponInfo::InjectHooks();
    CCam::InjectHooks();
    rage::phBound::InjectHooks();
    rage::phBoundComposite::InjectHooks();
    rage::phBoundGeometry::InjectHooks();
    rage::phBoundBVH::InjectHooks();
    rage::phArchetype::InjectHooks();
    rage::phInst::InjectHooks();
    rage::phCollider::InjectHooks();
    rage::phArticulatedCollider::InjectHooks();
    rage::phConstraint::InjectHooks();
    rage::phLevelNew::InjectHooks();
    rage::phSimulator::InjectHooks();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        InitPlugin();
    } else if (reason == DLL_PROCESS_DETACH) {
        Core::Hook::UninstallAll();
        Core::Log::Shutdown();
    }
    return TRUE;
}
