#include "StdInc.h"
#include "Core/Hook.h"
#include "Core/Log.h"
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
#include "Game/Camera.h"
#include "Game/FileMgr.h"
#include "Game/DummyObject.h"
#include "Game/IplStore.h"
#include "Game/Font.h"
#include "Game/Radar.h"
#include "Rage/fiStream.h"
#include "Game/PlayerInfo.h"
#include "Game/Wanted.h"
#include "Game/Building.h"
#include "Game/ModelInfo.h"
#include "Game/TxdStore.h"
#include "Game/Streaming.h"
#include "Game/Pad.h"
#include "Game/Pickups.h"
#include "Game/Game.h"
#include "Game/Zones.h"
#include "Game/Explosion.h"
#include "Game/Garages.h"
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
#include "Tools/DebugMenu.h"

#include <DbgHelp.h>

namespace {
    using SymSetOptionsFn              = DWORD(WINAPI*)(DWORD);
    using SymInitializeFn              = BOOL(WINAPI*)(HANDLE, PCSTR, BOOL);
    using SymFromAddrFn                = BOOL(WINAPI*)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
    using SymGetLineFromAddr64Fn       = BOOL(WINAPI*)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
    using StackWalk64Fn                = BOOL(WINAPI*)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
                                                       PREAD_PROCESS_MEMORY_ROUTINE64,
                                                       PFUNCTION_TABLE_ACCESS_ROUTINE64,
                                                       PGET_MODULE_BASE_ROUTINE64,
                                                       PTRANSLATE_ADDRESS_ROUTINE64);
    using SymFunctionTableAccess64Fn   = PVOID(WINAPI*)(HANDLE, DWORD64);
    using SymGetModuleBase64Fn         = DWORD64(WINAPI*)(HANDLE, DWORD64);

    struct DbgHelpApi {
        HMODULE                    mod = nullptr;
        SymSetOptionsFn            pSymSetOptions = nullptr;
        SymInitializeFn            pSymInitialize = nullptr;
        SymFromAddrFn              pSymFromAddr = nullptr;
        SymGetLineFromAddr64Fn     pSymGetLineFromAddr64 = nullptr;
        StackWalk64Fn              pStackWalk64 = nullptr;
        SymFunctionTableAccess64Fn pSymFunctionTableAccess64 = nullptr;
        SymGetModuleBase64Fn       pSymGetModuleBase64 = nullptr;
        bool                       triedInit = false;
        bool                       ready = false;
    } g_DbgHelp{};

    HMODULE g_SelfModule = nullptr;

    void EnsureDbgHelpInitialized() {
        if (g_DbgHelp.triedInit) {
            return;
        }
        g_DbgHelp.triedInit = true;

        g_DbgHelp.mod = LoadLibraryA("dbghelp.dll");
        if (!g_DbgHelp.mod) {
            return;
        }

        g_DbgHelp.pSymSetOptions = reinterpret_cast<SymSetOptionsFn>(GetProcAddress(g_DbgHelp.mod, "SymSetOptions"));
        g_DbgHelp.pSymInitialize = reinterpret_cast<SymInitializeFn>(GetProcAddress(g_DbgHelp.mod, "SymInitialize"));
        g_DbgHelp.pSymFromAddr = reinterpret_cast<SymFromAddrFn>(GetProcAddress(g_DbgHelp.mod, "SymFromAddr"));
        g_DbgHelp.pSymGetLineFromAddr64 = reinterpret_cast<SymGetLineFromAddr64Fn>(GetProcAddress(g_DbgHelp.mod, "SymGetLineFromAddr64"));
        g_DbgHelp.pStackWalk64 = reinterpret_cast<StackWalk64Fn>(GetProcAddress(g_DbgHelp.mod, "StackWalk64"));
        g_DbgHelp.pSymFunctionTableAccess64 = reinterpret_cast<SymFunctionTableAccess64Fn>(GetProcAddress(g_DbgHelp.mod, "SymFunctionTableAccess64"));
        g_DbgHelp.pSymGetModuleBase64 = reinterpret_cast<SymGetModuleBase64Fn>(GetProcAddress(g_DbgHelp.mod, "SymGetModuleBase64"));

        if (!g_DbgHelp.pSymInitialize || !g_DbgHelp.pSymFromAddr) {
            return;
        }

        if (g_DbgHelp.pSymSetOptions) {
            g_DbgHelp.pSymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
        }

        char symSearchPath[MAX_PATH * 2]{};
        if (g_SelfModule) {
            char selfPath[MAX_PATH]{};
            if (GetModuleFileNameA(g_SelfModule, selfPath, sizeof(selfPath))) {
                char* lastSlash = nullptr;
                for (char* p = selfPath; *p; ++p) {
                    if (*p == '\\' || *p == '/') {
                        lastSlash = p;
                    }
                }
                if (lastSlash) {
                    *lastSlash = '\0';
                    std::snprintf(symSearchPath, sizeof(symSearchPath), "%s", selfPath);
                }
            }
        }

        g_DbgHelp.ready = g_DbgHelp.pSymInitialize(
            GetCurrentProcess(),
            symSearchPath[0] ? symSearchPath : nullptr,
            TRUE) != FALSE;
    }

    const char* GetExceptionName(DWORD code) {
        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:    return "Access Violation";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal Instruction";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:  return "Integer Divide by Zero";
        case EXCEPTION_STACK_OVERFLOW:      return "Stack Overflow";
        case EXCEPTION_PRIV_INSTRUCTION:    return "Privileged Instruction";
        default:                            return "Unhandled Exception";
        }
    }

    const char* GetBaseName(const char* path) {
        if (!path) return "";
        const char* name = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '\\' || *p == '/') {
                name = p + 1;
            }
        }
        return name;
    }
}

static void FormatAddressDetailed(uintptr_t addr, char* out, size_t outSize) {
    const uintptr_t gameBase = Core::GetBaseAddr();
    char modPart[160]{};
    HMODULE mod = nullptr;
    if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(addr),
            &mod) && mod) {
        char path[MAX_PATH]{};
        GetModuleFileNameA(mod, path, sizeof(path));
        const char* name = GetBaseName(path);
        const uintptr_t modBase = reinterpret_cast<uintptr_t>(mod);
        if (modBase == gameBase) {
            std::snprintf(modPart, sizeof(modPart), "%s+0x%08X (IDA: 0x%08X)",
                name,
                static_cast<uint32>(addr - modBase),
                static_cast<uint32>(0x400000u + (addr - modBase)));
        } else {
            std::snprintf(modPart, sizeof(modPart), "%s+0x%08X", name, static_cast<uint32>(addr - modBase));
        }
    } else {
        std::snprintf(modPart, sizeof(modPart), "unknown");
    }

    if (g_DbgHelp.ready && mod && g_DbgHelp.pSymFromAddr) {
        alignas(SYMBOL_INFO) uint8 symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
        auto* sym = reinterpret_cast<PSYMBOL_INFO>(symBuf);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = MAX_SYM_NAME;

        DWORD64 disp = 0;
        if (g_DbgHelp.pSymFromAddr(GetCurrentProcess(), static_cast<DWORD64>(addr), &disp, sym)) {
            char linePart[160]{};
            if (g_DbgHelp.pSymGetLineFromAddr64) {
                IMAGEHLP_LINE64 line{};
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD lineDisp = 0;
                if (g_DbgHelp.pSymGetLineFromAddr64(GetCurrentProcess(), static_cast<DWORD64>(addr), &lineDisp, &line) && line.FileName) {
                    std::snprintf(linePart, sizeof(linePart), " [%s:%lu]", GetBaseName(line.FileName), line.LineNumber);
                }
            }
            std::snprintf(out, outSize, "%s -> %s+0x%llX%s", modPart, sym->Name, disp, linePart);
            return;
        }
    }

    std::snprintf(out, outSize, "%s", modPart);
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ex) {
    if (!ex || !ex->ExceptionRecord || !ex->ContextRecord) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    EnsureDbgHelpInitialized();

    const EXCEPTION_RECORD* rec = ex->ExceptionRecord;
    const CONTEXT* ctx = ex->ContextRecord;
    const uintptr_t addr = reinterpret_cast<uintptr_t>(rec->ExceptionAddress);

    char addrInfo[384]{};
    FormatAddressDetailed(addr, addrInfo, sizeof(addrInfo));

    Core::Log::Error("=== CRASH DETECTED ===");
    Core::Log::Error("Exception: %s (0x%08X) at 0x%08X (%s)",
        GetExceptionName(rec->ExceptionCode),
        static_cast<uint32>(rec->ExceptionCode),
        static_cast<uint32>(addr),
        addrInfo);

    if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && rec->NumberParameters >= 2) {
        const ULONG_PTR op = rec->ExceptionInformation[0];
        const char* opName = (op == 0) ? "READ" : (op == 1) ? "WRITE" : (op == 8) ? "DEP/EXECUTE" : "UNKNOWN";
        Core::Log::Error("Reason: Invalid memory %s at address 0x%08X%s",
            opName,
            static_cast<uint32>(rec->ExceptionInformation[1]),
            (rec->ExceptionInformation[1] < 0x10000u) ? " (likely NULL/offset pointer dereference)" : "");
    }

    Core::Log::Error("Registers: EAX=%08X EBX=%08X ECX=%08X EDX=%08X",
        ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx);
    Core::Log::Error("           ESI=%08X EDI=%08X EBP=%08X ESP=%08X EIP=%08X",
        ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp, ctx->Eip);

    if (g_DbgHelp.ready && g_DbgHelp.pStackWalk64 && g_DbgHelp.pSymFunctionTableAccess64 && g_DbgHelp.pSymGetModuleBase64) {
        CONTEXT walkCtx = *ctx;
        STACKFRAME64 frame{};
        frame.AddrPC.Offset = ctx->Eip;
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Offset = ctx->Ebp;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Offset = ctx->Esp;
        frame.AddrStack.Mode = AddrModeFlat;

        Core::Log::Error("Call Stack (PDB symbols %s):", g_DbgHelp.ready ? "loaded" : "unavailable");
        const HANDLE proc = GetCurrentProcess();
        const HANDLE thd = GetCurrentThread();
        for (int i = 0; i < 20; ++i) {
            if (!g_DbgHelp.pStackWalk64(
                    IMAGE_FILE_MACHINE_I386, proc, thd, &frame, &walkCtx, nullptr,
                    g_DbgHelp.pSymFunctionTableAccess64, g_DbgHelp.pSymGetModuleBase64, nullptr)) {
                break;
            }
            if (frame.AddrPC.Offset == 0) {
                break;
            }
            char frameInfo[384]{};
            FormatAddressDetailed(static_cast<uintptr_t>(frame.AddrPC.Offset), frameInfo, sizeof(frameInfo));
            Core::Log::Error("  #%02d 0x%08X (%s)", i, static_cast<uint32>(frame.AddrPC.Offset), frameInfo);
        }
    }

    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(reinterpret_cast<LPCVOID>(ctx->Esp), &mbi, sizeof(mbi)) &&
        mbi.State == MEM_COMMIT &&
        (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
        Core::Log::Error("Raw Stack (ESP):");
        const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
        for (int i = 0; i < 12; ++i) {
            const uintptr_t sp = static_cast<uintptr_t>(ctx->Esp) + static_cast<uintptr_t>(i * 4);
            if (sp + 4 > regionEnd) {
                break;
            }
            const uint32 val = *reinterpret_cast<const uint32*>(sp);
            char stackMod[384]{};
            FormatAddressDetailed(val, stackMod, sizeof(stackMod));
            Core::Log::Error("  Stack[+%02X]: 0x%08X (%s)", i * 4, val, stackMod);
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

static void InstallCompatibilityHooks() {
    Core::Hook::Install(0x821500, &HookedLoadRadioColoredLogo);
    Core::Hook::Install(0x821520, &HookedLoadRadioNonColoredLogo);
    Core::Hook::Install(0x5A76D0, &HookedLookupNative);

    Core::Log::Info("Complete Edition compatibility hooks installed");
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
    rage::fiStream::InjectHooks();
    CFileMgr::InjectHooks();
    CDummyObject::InjectHooks();
    CIplStore::InjectHooks();
    CRadar::InjectHooks();
    CFont::InjectHooks();
    CCamera::InjectHooks();
    CPlayerInfo::InjectHooks();
    CWanted::InjectHooks();
    CBuilding::InjectHooks();
    CModelInfo::InjectHooks();
    CTxdStore::InjectHooks();
    CStreaming::InjectHooks();
    CPad::InjectHooks();
    CPickups::InjectHooks();
    CGame::InjectHooks();
    CTheZones::InjectHooks();
    CExplosion::InjectHooks();
    CGarages::InjectHooks();
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

    Tools::DebugMenu::Init();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_SelfModule = hModule;
        DisableThreadLibraryCalls(hModule);
        InitPlugin();
    } else if (reason == DLL_PROCESS_DETACH) {
        Tools::DebugMenu::Shutdown();
        Core::Hook::UninstallAll();
        Core::Log::Shutdown();
    }
    return TRUE;
}
