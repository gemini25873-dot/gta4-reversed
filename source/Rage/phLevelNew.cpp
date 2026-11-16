#include "StdInc.h"
#include "Rage/phLevelNew.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <intrin.h>

namespace
{
    void* RageAlloc(size_t size, size_t align, uint32 flags)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using AllocFn = void*(__thiscall*)(void*, size_t, size_t, uint32);
        return reinterpret_cast<AllocFn>(vtable[2])(allocator, size, align, flags);
    }

    int32 RageFree(void* ptr)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using FreeFn = int32(__thiscall*)(void*, void*);
        return reinterpret_cast<FreeFn>(vtable[3])(allocator, ptr);
    }

    void DeleteVirtualObject(void* obj)
    {
        if (obj)
        {
            void** vtable = *reinterpret_cast<void***>(obj);
            using DeletingDtorFn = void(__thiscall*)(void*, int32);
            reinterpret_cast<DeletingDtorFn>(vtable[0])(obj, 1);
        }
    }
}

namespace rage
{
    void phLevelBase::InjectHooks()
    {
        Core::Hook::Install(0x6D75A0, &phLevelBase::DeletingDtor);
    }

    // 0x6D75A0 (retn 4)
    phLevelBase* __thiscall phLevelBase::DeletingDtor(uint8 flags)
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC560));
        DeleteCriticalSection(&m_csLock);
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00D52950));
        if (flags & 1)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x6D75E0 (__usercall: esi = this) - pure C++
    int32 phLevelBase::AllocInstanceStates()
    {
        m_pInstanceStates = static_cast<uint8*>(RageAlloc(m_nMaxInstances, 16, 0));
        int32 count = 0;
        if (m_nMaxInstances != 0)
        {
            do
            {
                m_pInstanceStates[count] = 3;
                ++count;
            }
            while (count < m_nMaxInstances);
        }
        return count;
    }

    void phLevelNew::InjectHooks()
    {
        phLevelBase::InjectHooks();

        Core::Hook::Install(0x6A2480, &phLevelNew::Ctor);
        Core::Hook::Install(0x6A2AF0, &phLevelNew::Shutdown);

        Core::Log::Info("rage::phLevelNew hooks installed (3 functions)");
    }

    // 0x6A2480 (retn)
    phLevelNew* __thiscall phLevelNew::Ctor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC560));
        InitializeCriticalSection(&m_csLock);
        m_nMaxInstances = 0;
        m_nUnk28 = 0;
        m_nUnk2A = 0;
        m_nUnk30 = 0;
        m_nUnk2E = 0;
        m_nUnk2C = 0;
        m_pInstanceStates = nullptr;
        m_bUnk24 = 0;
        m_bUnk44 = 0;

        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBE30));
        m_nUnk40 = -1;
        m_nUnk54 = 0;
        m_nUnk58 = 0;
        m_nUnk50 = 0;
        m_pUnk70 = nullptr;
        m_pUnk74 = nullptr;
        m_nUnk78 = 0;
        m_nUnk7C = 0;
        m_pUnk80 = nullptr;
        m_pUnk84 = nullptr;
        m_nUnk88 = 0;
        m_nUnk8A = 0;
        m_pUnk8C = nullptr;
        m_pUnk94 = nullptr;
        m_nUnk98 = 0;
        m_pUnk9C = nullptr;
        m_nUnk60 = 0;
        m_nMaxOctreeNodes = 10240;
        m_nUnk68 = 5;
        m_nUnk6C = 4;
        m_nUnk90 = -1;
        return this;
    }

    // 0x6A2AF0 (retn)
    int32 __thiscall phLevelNew::Shutdown()
    {
        if (m_pUnk94)
        {
            RageFree(m_pUnk94);
        }
        m_pUnk94 = nullptr;

        if (m_pUnk8C)
        {
            void* raw = static_cast<uint8*>(m_pUnk8C) - 16;
            if (raw)
            {
                RageFree(raw);
            }
        }
        m_pUnk8C = nullptr;
        m_nUnk8A = 0;
        m_nUnk88 = 0;

        if (m_pUnk84)
        {
            RageFree(m_pUnk84);
        }
        m_pUnk84 = nullptr;

        if (m_pUnk80)
        {
            RageFree(m_pUnk80);
        }
        m_pUnk80 = nullptr;

        if (m_pUnk74)
        {
            RageFree(m_pUnk74);
        }
        m_pUnk74 = nullptr;

        DeleteVirtualObject(m_pUnk9C);

        if (m_pUnk70)
        {
            RageFree(m_pUnk70);
        }
        m_pUnk70 = nullptr;

        DeleteVirtualObject(m_pUnkA0);

        int32 result = static_cast<int32>(reinterpret_cast<uintptr_t>(m_pInstanceStates));
        if (m_pInstanceStates)
        {
            result = RageFree(m_pInstanceStates);
        }
        m_pInstanceStates = nullptr;

        auto& activeLevel = Core::StaticRef<phLevelNew*>(0x18A87D8);
        if (activeLevel == this)
        {
            activeLevel = nullptr;
        }

        return result;
    }
}
