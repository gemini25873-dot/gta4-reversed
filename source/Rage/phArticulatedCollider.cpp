#include "StdInc.h"
#include "Rage/phArticulatedCollider.h"
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

    void FreeArticulatedArray(rage::phArticulatedArray& arr)
    {
        if (arr.m_nCapacity != 0 && arr.m_pData != nullptr)
        {
            RageFree(arr.m_pData);
        }
    }

    // 0x43B510 (__thiscall)
    int32 FindFixupIndex(uintptr_t fixupTable, uintptr_t addr)
    {
        const auto* table = reinterpret_cast<const uint32*>(fixupTable);
        const int32 count = static_cast<int32>(table[384]);
        for (int32 i = 0; i < count; ++i)
        {
            if (addr >= table[i] && addr < table[i + 128])
            {
                return i;
            }
        }
        return -1;
    }
}

namespace rage
{
    void phArticulatedCollider::InjectHooks()
    {
        Core::Hook::Install(0x6FE9B0, &phArticulatedCollider::DeletingDtor);
        Core::Hook::Install(0x6FEB70, &phArticulatedCollider::FixupInstance);
        Core::Hook::Install(0x7029D0, &phArticulatedCollider::GetMatrix3x3_7029D0);
        Core::Hook::Install(0x702A60, &phArticulatedCollider::GetMatrix3x3_702A60);

        Core::Log::Info("rage::phArticulatedCollider hooks installed (4 functions)");
    }

    // 0x6FE9E0 (__usercall esi = this) rewritten in pure C++
    void phArticulatedCollider::Dtor()
    {
        FreeArticulatedArray(m_aUnk2DC);
        FreeArticulatedArray(m_aUnk2D4);
        FreeArticulatedArray(m_aComponentToLink);
        FreeArticulatedArray(m_aUnk2C4);
        FreeArticulatedArray(m_aUnk2BC);
        FreeArticulatedArray(m_aUnk2B4);
        FreeArticulatedArray(m_aUnk2AC);
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBD14));
    }

    // 0x6FE9B0 (retn 4)
    phArticulatedCollider* __thiscall phArticulatedCollider::DeletingDtor(uint8 flags)
    {
        Dtor();
        if ((flags & 1) != 0)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x6FEB70 (retn 4)
    int32 __thiscall phArticulatedCollider::FixupInstance(uintptr_t fixupTable)
    {
        const uintptr_t origInst = reinterpret_cast<uintptr_t>(m_pInstance);
        const int32 idx1 = FindFixupIndex(fixupTable, origInst);
        if (idx1 != -1)
        {
            const int32 offset = *reinterpret_cast<const int32*>(fixupTable + 4 * idx1 + 1024);
            m_pInstance = reinterpret_cast<phInst*>(origInst + offset);
        }

        const uintptr_t orig2EC = m_nUnk2EC;
        const int32 idx2 = FindFixupIndex(fixupTable, orig2EC);
        if (idx2 != -1)
        {
            const int32 offset = *reinterpret_cast<const int32*>(fixupTable + 4 * idx2 + 1024);
            m_nUnk2EC = orig2EC + offset;
        }

        return idx2;
    }

    float* phArticulatedCollider::CopyLinkMatrix3x3(float* outMat, int32 componentIdx, size_t linkMatOffset)
    {
        int32 linkIdx = 0;
        if (componentIdx >= 0)
        {
            const auto* inst = reinterpret_cast<const uint8*>(m_pInstance);
            const auto* archetype = *reinterpret_cast<const uint8* const*>(inst + 0x04);
            const auto* bound = *reinterpret_cast<const uint8* const*>(archetype + 0x0C);
            const uint16 numBounds = *reinterpret_cast<const uint16*>(bound + 0x92);
            if (componentIdx < static_cast<int32>(numBounds))
            {
                const auto* body = reinterpret_cast<const int32*>(m_pBody);
                const int32 numLinks = body[0];
                const auto* componentToLink = static_cast<const int32*>(m_aComponentToLink.m_pData);
                const int32 mapped = componentToLink[componentIdx];
                if (mapped >= 0 && mapped < numLinks)
                {
                    linkIdx = mapped;
                }
            }
        }

        const auto* linkPtr = reinterpret_cast<const uint8*>(reinterpret_cast<const uintptr_t*>(m_pBody)[linkIdx + 1]);
        auto* srcMat = reinterpret_cast<float*>(const_cast<uint8*>(linkPtr + linkMatOffset));

        outMat[0]  = srcMat[0];
        outMat[1]  = srcMat[1];
        outMat[2]  = srcMat[2];
        outMat[4]  = srcMat[4];
        outMat[5]  = srcMat[5];
        outMat[6]  = srcMat[6];
        outMat[8]  = srcMat[8];
        outMat[9]  = srcMat[9];
        outMat[10] = srcMat[10];
        return srcMat;
    }

    // 0x7029D0 (retn 8)
    float* __thiscall phArticulatedCollider::GetMatrix3x3_7029D0(float* outMat, int32 componentIdx)
    {
        return CopyLinkMatrix3x3(outMat, componentIdx, 0x170);
    }

    // 0x702A60 (retn 8)
    float* __thiscall phArticulatedCollider::GetMatrix3x3_702A60(float* outMat, int32 componentIdx)
    {
        return CopyLinkMatrix3x3(outMat, componentIdx, 0x2E0);
    }
}
