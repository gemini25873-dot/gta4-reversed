#include "StdInc.h"
#include "Rage/phBoundComposite.h"
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
    void phBoundComposite::InjectHooks()
    {
        Core::Hook::Install(0x6883C0, &phBoundComposite::Ctor);
        Core::Hook::Install(0x688410, &phBoundComposite::DeletingDtor);
        Core::Hook::Install(0x686780, &phBoundComposite::GetBoundType);
        Core::Hook::Install(0x686240, &phBoundComposite::IsDefaultType);
        Core::Hook::Install(0x6867F0, &phBoundComposite::SetUnk70);
        Core::Hook::Install(0x6848F0, &phBoundComposite::SetUnk07);
        Core::Hook::Install(0x684900, &phBoundComposite::GetUnk07);
        Core::Hook::Install(0x68C020, &phBoundComposite::GetFirstBoundVFunc21);
        Core::Hook::Install(0x68BF70, &phBoundComposite::VFunc52Wrapper);
        Core::Hook::Install(0x68BFD0, &phBoundComposite::VFunc53Wrapper);

        Core::Log::Info("rage::phBoundComposite hooks yuklendi (10 fonksiyon)");
    }

    // 0x6883C0 (retn)
    phBoundComposite* __thiscall phBoundComposite::Ctor()
    {
        InitBase();
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBB2C));
        m_nType = 12;
        m_ppBounds = nullptr;
        m_pCurrentMatrices = nullptr;
        m_pLastMatrices = nullptr;
        m_pBoundLocalBBoxes = nullptr;
        m_pTypeAndIncludeFlags = nullptr;
        m_vecCentroidOffset = { 0.0f, 0.0f, 0.0f };
        m_nUnk98 = 0;
        return this;
    }

    void phBoundComposite::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBB2C));
        if (m_ppBounds)
        {
            auto& refCountEnabled = Core::StaticRef<uint8>(0x18A87F6);
            for (uint16 i = 0; i < m_nNumBounds; ++i)
            {
                phBound* bound = m_ppBounds[i];
                if (bound && refCountEnabled)
                {
                    --bound->m_nRefCount;
                    if (refCountEnabled && bound->m_nRefCount == 0)
                    {
                        using DeletingDtorFn = void* (__thiscall*)(phBound*, uint8);
                        reinterpret_cast<DeletingDtorFn>(bound->m_vftable[0])(bound, 1);
                    }
                }
                m_ppBounds[i] = nullptr;
            }
            if (m_ppBounds)
            {
                RageFree(m_ppBounds);
            }
        }
        if (m_pLastMatrices != m_pCurrentMatrices && m_pLastMatrices)
        {
            RageFree(m_pLastMatrices);
        }
        if (m_pCurrentMatrices)
        {
            RageFree(m_pCurrentMatrices);
        }
        if (m_pBoundLocalBBoxes)
        {
            RageFree(m_pBoundLocalBBoxes);
        }
        if (m_pTypeAndIncludeFlags)
        {
            RageFree(m_pTypeAndIncludeFlags);
        }
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB998));
    }

    // 0x688410 (retn 4)
    phBoundComposite* __thiscall phBoundComposite::DeletingDtor(uint8 flags)
    {
        Dtor();
        if ((flags & 1) != 0 && this)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x686780 (retn)
    int32 __thiscall phBoundComposite::GetBoundType() const
    {
        return m_nType;
    }

    // 0x686240 (retn)
    int32 __thiscall phBoundComposite::IsDefaultType() const
    {
        using VFunc5Fn = uint8(__thiscall*)(const phBoundComposite*);
        return reinterpret_cast<VFunc5Fn>(m_vftable[5])(this) == 0 ? 1 : 0;
    }

    // 0x6867F0 (retn 4)
    uint32 __thiscall phBoundComposite::SetUnk70(int32 val)
    {
        const uint32 result = Core::StaticRef<uint8>(0x18A87F6) ? m_nRefCount : 1;
        auto* pUnk70 = reinterpret_cast<int32*>(&m_vecUnk70);
        pUnk70[0] = val;
        pUnk70[1] = val;
        pUnk70[2] = val;
        m_nRefCount = result;
        return result;
    }

    // 0x6848F0 (retn 4)
    uint8 __thiscall phBoundComposite::SetUnk07(uint8 val)
    {
        m_nUnk07 = val;
        return val;
    }

    // 0x684900 (retn)
    uint8 __thiscall phBoundComposite::GetUnk07() const
    {
        return m_nUnk07;
    }

    // 0x68C020 (retn)
    int32 __thiscall phBoundComposite::GetFirstBoundVFunc21()
    {
        phBound* first = m_ppBounds[0];
        if (first)
        {
            using VFunc21Fn = int32(__thiscall*)(phBound*);
            return reinterpret_cast<VFunc21Fn>(first->m_vftable[21])(first);
        }
        return 0;
    }

    // 0x68BF70 (retn 4)
    int32 __thiscall phBoundComposite::VFunc52Wrapper(int32 arg)
    {
        using VFunc52Fn = int32(__thiscall*)(phBoundComposite*, int32, int32);
        return reinterpret_cast<VFunc52Fn>(m_vftable[52])(this, arg, 0);
    }

    // 0x68BFD0 (retn 4)
    int32 __thiscall phBoundComposite::VFunc53Wrapper(int32 arg)
    {
        using VFunc53Fn = int32(__thiscall*)(phBoundComposite*, int32, int32);
        return reinterpret_cast<VFunc53Fn>(m_vftable[53])(this, arg, 0);
    }
}
