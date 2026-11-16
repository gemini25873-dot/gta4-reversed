#include "StdInc.h"
#include "Rage/phCollider.h"
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
    void phCollider::InjectHooks()
    {
        Core::Hook::Install(0x69C730, &phCollider::Dtor);
        Core::Hook::Install(0x697DD0, &phCollider::DeletingDtor);
        Core::Hook::Install(0x69C740, &phCollider::Init);
        Core::Hook::Install(0x69C080, &phCollider::FixupInstance);
        Core::Hook::Install(0x69D450, &phCollider::SetUnk69D450);
        Core::Hook::Install(0x69ED90, &phCollider::ReturnZeroDouble);
        Core::Hook::Install(0x69EF20, &phCollider::Unk69EF20);
        Core::Hook::Install(0x69F650, &phCollider::GetInvMassMatrix);

        Core::Log::Info("rage::phCollider hooks installed (8 functions)");
    }

    // 0x69C730 (retn)
    void __thiscall phCollider::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBD14));
    }

    // 0x697DD0 (retn 4)
    phCollider* __thiscall phCollider::DeletingDtor(uint8 flags)
    {
        if (flags & 2)
        {
            auto* rawBase = reinterpret_cast<uint8*>(this) - 16;
            const uint32 count = *reinterpret_cast<uint32*>(rawBase);
            for (uint32 i = count; i > 0; --i)
            {
                this[i - 1].Dtor();
            }
            if ((flags & 1) && rawBase != nullptr)
            {
                RageFree(rawBase);
            }
            return reinterpret_cast<phCollider*>(rawBase);
        }

        Dtor();
        if (flags & 1)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x69C740 (retn 8)
    int32 __thiscall phCollider::Init(int32 a2, int32 a3)
    {
        const int32 result = Core::Hook::CallMethod<int32>(0x69C760, this, a2);
        m_nUnk18 = a3;
        return result;
    }

    // 0x69C080 (retn 4)
    int32 __thiscall phCollider::FixupInstance(uintptr_t fixupTable)
    {
        const uintptr_t origPtr = reinterpret_cast<uintptr_t>(m_pInstance);
        int32 result = Core::Hook::CallMethod<int32>(0x43B510, reinterpret_cast<void*>(fixupTable), origPtr);
        if (result != -1)
        {
            const int32 offset = *reinterpret_cast<const int32*>(fixupTable + 4 * result + 1024);
            result = static_cast<int32>(origPtr) + offset;
            m_pInstance = reinterpret_cast<phInst*>(static_cast<uintptr_t>(result));
        }
        return result;
    }

    // 0x69D450 (retn 4) - sub_69B280 (phSleep::Update) rewritten in pure C++
    int32 __thiscall phCollider::SetUnk69D450(float dt)
    {
        auto* sleepObj = reinterpret_cast<int32*>(static_cast<uintptr_t>(m_nUnk18));
        void** vtable = *reinterpret_cast<void***>(sleepObj);
        using CheckSleepFn = uint8(__thiscall*)(void*, float, float);
        using WakeFn = int32(__thiscall*)(void*);

        if (reinterpret_cast<CheckSleepFn>(vtable[2])(sleepObj, dt, dt))
        {
            int32 result = 0;
            if (sleepObj[6] == 1)
            {
                result = ++sleepObj[8];
                if (result >= sleepObj[4])
                {
                    sleepObj[6] = 0;
                }
            }
            if (sleepObj[6] != 1)
            {
                result = ++sleepObj[9];
                if (result >= sleepObj[5])
                {
                    sleepObj[6] = 2;
                }
            }
            return result;
        }
        if (sleepObj[6] == 1)
        {
            sleepObj[8] = 0;
            sleepObj[9] = 0;
            return 0;
        }
        return reinterpret_cast<WakeFn>(vtable[1])(sleepObj);
    }

    // 0x69ED90 (retn 4)
    double __stdcall phCollider::ReturnZeroDouble(int32 /*a1*/)
    {
        return 0.0;
    }

    // 0x69EF20 (retn 8) - sub_732110 (R * diag(m_vInvAngInertia) * R^T) rewritten in pure C++
    int32 __thiscall phCollider::Unk69EF20(int32 a2, int32 /*a3*/)
    {
        float* a1 = reinterpret_cast<float*>(static_cast<uintptr_t>(a2));
        const float* d = &m_vInvAngInertia.x;
        const float* r = &m_matCollider.right.x;

        const float r00 = r[0], r01 = r[1], r02 = r[2];
        const float r10 = r[4], r11 = r[5], r12 = r[6];
        const float r20 = r[8], r21 = r[9], r22 = r[10];

        const float d0 = d[0], d1 = d[1], d2 = d[2];

        a1[0]  = r20 * (d2 * r20) + r10 * (d1 * r10) + (d0 * r00) * r00;
        a1[1]  = (d2 * r21) * r20 + (d1 * r11) * r10 + (d0 * r00) * r01;
        a1[2]  = (d2 * r22) * r20 + (d1 * r12) * r10 + (d0 * r00) * r02;

        a1[4]  = r11 * (d1 * r10) + r01 * (d0 * r00) + (d2 * r20) * r21;
        a1[5]  = (d1 * r11) * r11 + (d0 * r01) * r01 + (d2 * r21) * r21;
        a1[6]  = (d1 * r12) * r11 + (d0 * r02) * r01 + (d2 * r22) * r21;

        a1[8]  = r22 * (d2 * r20) + r12 * (d1 * r10) + r02 * (d0 * r00);
        a1[9]  = r22 * (d2 * r21) + r12 * (d1 * r11) + (d0 * r01) * r02;
        a1[10] = (d2 * r22) * r22 + (d1 * r12) * r12 + (d0 * r02) * r02;

        return static_cast<int32>(reinterpret_cast<uintptr_t>(&m_matCollider));
    }

    // 0x69F650 (retn 8)
    float* __thiscall phCollider::GetInvMassMatrix(float* outMat, int32 /*a3*/)
    {
        outMat[0]  = m_fInvMass;
        outMat[1]  = 0.0f;
        outMat[2]  = 0.0f;
        outMat[4]  = 0.0f;
        outMat[5]  = m_fInvMass;
        outMat[6]  = 0.0f;
        outMat[8]  = 0.0f;
        outMat[9]  = 0.0f;
        outMat[10] = m_fInvMass;
        return outMat;
    }
}
