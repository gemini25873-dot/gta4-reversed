#include "StdInc.h"
#include "Hook.h"
#include "Log.h"
#include <vector>
#include <array>

namespace Core::Hook {
    struct PatchRecord {
        uintptr_t             addr;
        std::array<uint8, 5>  origBytes;
    };

    static std::vector<PatchRecord> g_Patches;

    bool InstallJmp(uintptr_t idaAddr, void* detour) {
        const uintptr_t addr = Rebase(idaAddr);
        DWORD oldProtect = 0;
        if (!VirtualProtect(reinterpret_cast<void*>(addr), 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            Log::Error("VirtualProtect failed at 0x%08X (IDA 0x%08X)", addr, idaAddr);
            return false;
        }

        PatchRecord rec{};
        rec.addr = addr;
        std::memcpy(rec.origBytes.data(), reinterpret_cast<const void*>(addr), 5);
        g_Patches.push_back(rec);

        const int32 rel = static_cast<int32>(reinterpret_cast<uintptr_t>(detour) - (addr + 5));
        uint8* p = reinterpret_cast<uint8*>(addr);
        p[0] = 0xE9;
        std::memcpy(p + 1, &rel, 4);

        DWORD dummy = 0;
        VirtualProtect(reinterpret_cast<void*>(addr), 5, oldProtect, &dummy);
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(addr), 5);
        return true;
    }

    void UninstallAll() {
        for (auto it = g_Patches.rbegin(); it != g_Patches.rend(); ++it) {
            DWORD oldProtect = 0;
            if (VirtualProtect(reinterpret_cast<void*>(it->addr), 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                std::memcpy(reinterpret_cast<void*>(it->addr), it->origBytes.data(), 5);
                DWORD dummy = 0;
                VirtualProtect(reinterpret_cast<void*>(it->addr), 5, oldProtect, &dummy);
                FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(it->addr), 5);
            }
        }
        g_Patches.clear();
    }
}
