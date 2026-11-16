#include "StdInc.h"
#include "Rage/phSimulator.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <intrin.h>

namespace
{
    void RageFree(void* ptr)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using FreeFn = void(__thiscall*)(void*, void*);
        reinterpret_cast<FreeFn>(vtable[3])(allocator, ptr);
    }
}

namespace rage
{
    void phSimulator::InjectHooks()
    {
        Core::Hook::Install(0x697780, &phSimulator::DeletingDtor);

        // Deferred functions (due to LTCG __usercall/__userpurge callees or missing vftable/dump):
        // - 0x6975F0 (Ctor): vftable address and nullsub_1 address missing from dump
        // - 0x6977B0 (Dtor): missing vftable address + __usercall callees (0x738BF0, 0x6B0AD0)
        // - 0x69A730 (Update): __usercall callees (0x69A0C0, 0x6992F0)
        // - 0x6983F0: ecx/this ambiguous for 0x6A3960 (__thiscall)
        // - 0x698560 / 0x698220: __usercall callee (0x698260) and 0x698220 missing from dump
        // - 0x699D50: __userpurge callees (0x6B1E00, 0x6B36A0) and __usercall (0x738DA0)
        // - 0x6993C0: __usercall callee (0x712980)

        Core::Log::Info("rage::phSimulator hooks installed (1 function)");
    }

    // 0x697780 (retn 4)
    phSimulator* __thiscall phSimulator::DeletingDtor(uint8 flags)
    {
        Core::Hook::CallMethod<void>(0x6977B0, this);
        if ((flags & 1) && this != nullptr)
        {
            RageFree(this);
        }
        return this;
    }
}
