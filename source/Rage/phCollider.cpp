#include "StdInc.h"
#include "Rage/phCollider.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>
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

    // 0x5D35A0
    const float* CopyInstanceMatrices(void* collider, const float* mat)
    {
        auto* f = static_cast<float*>(collider);
        for (int32 base = 8; base <= 24; base += 16)
        {
            f[base + 0]  = mat[0];  f[base + 1]  = mat[1];  f[base + 2]  = mat[2];
            f[base + 4]  = mat[4];  f[base + 5]  = mat[5];  f[base + 6]  = mat[6];
            f[base + 8]  = mat[8];  f[base + 9]  = mat[9];  f[base + 10] = mat[10];
            f[base + 12] = mat[12]; f[base + 13] = mat[13]; f[base + 14] = mat[14];
        }
        return mat;
    }

    void SetIdentity3x4(float* dst)
    {
        dst[0] = 1.0f; dst[1] = 0.0f; dst[2] = 0.0f;
        dst[4] = 0.0f; dst[5] = 1.0f; dst[6] = 0.0f;
        dst[8] = 0.0f; dst[9] = 0.0f; dst[10] = 1.0f;
        dst[12] = 0.0f; dst[13] = 0.0f; dst[14] = 0.0f;
    }

    // 0x69CF30
    void ClampLinearVelocity(rage::phCollider* c)
    {
        const float maxSpd = c->m_fMaxSpeed;
        const float sq = c->m_vUnk110.x * c->m_vUnk110.x + c->m_vUnk110.y * c->m_vUnk110.y + c->m_vUnk110.z * c->m_vUnk110.z;
        if (sq > maxSpd * maxSpd)
        {
            const float invLen = (sq == 0.0f) ? 0.0f : (1.0f / std::sqrt(sq));
            const float scale = maxSpd * invLen;
            c->m_vUnk110.x *= scale; c->m_vUnk110.y *= scale; c->m_vUnk110.z *= scale;
            c->m_vUnk130.x *= scale; c->m_vUnk130.y *= scale; c->m_vUnk130.z *= scale;
        }
    }

    // 0x69D040
    void ClampAngularVelocity(rage::phCollider* c)
    {
        const float maxAng = c->m_fMaxAngSpeed;
        const float sq = c->m_vUnk120.x * c->m_vUnk120.x + c->m_vUnk120.y * c->m_vUnk120.y + c->m_vUnk120.z * c->m_vUnk120.z;
        if (sq > maxAng * maxAng)
        {
            const float invLen = (sq == 0.0f) ? 0.0f : (1.0f / std::sqrt(sq));
            const float scale = maxAng * invLen;
            c->m_vUnk120.x *= scale; c->m_vUnk120.y *= scale; c->m_vUnk120.z *= scale;
            c->m_vUnk140.x *= scale; c->m_vUnk140.y *= scale; c->m_vUnk140.z *= scale;
        }
    }
}

namespace rage
{
    void phCollider::InjectHooks()
    {
        Core::Hook::Install(0x69C730, &phCollider::Dtor);
        Core::Hook::Install(0x697DD0, &phCollider::DeletingDtor);
        Core::Hook::Install(0x69C740, &phCollider::Init);
        Core::Hook::Install(0x69C760, &phCollider::InitFromInstance);
        Core::Hook::Install(0x69C080, &phCollider::FixupInstance);
        Core::Hook::Install(0x69D450, &phCollider::SetUnk69D450);
        Core::Hook::Install(0x69ED90, &phCollider::ReturnZeroDouble);
        Core::Hook::Install(0x69EF20, &phCollider::Unk69EF20);
        Core::Hook::Install(0x69F650, &phCollider::GetInvMassMatrix);

        Core::Log::Info("rage::phCollider hooks installed (9 functions)");
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
        const int32 result = InitFromInstance(reinterpret_cast<phInst*>(static_cast<uintptr_t>(a2)));
        m_nUnk18 = a3;
        return result;
    }

    // 0x69C760 (retn 4)
    int32 __thiscall phCollider::InitFromInstance(phInst* instance)
    {
        m_nUnk29C = 0;
        m_pInstance = instance;

        // 0x69CBA0: reset collider state
        auto* rawBytes = reinterpret_cast<uint8*>(this);
        auto* rawFloats = reinterpret_cast<float*>(this);
        auto* rawDwords = reinterpret_cast<uint32*>(this);

        SetIdentity3x4(&m_matCollider.right.x);

        using VoidThisFn = void(__thiscall*)(phCollider*);
        reinterpret_cast<VoidThisFn>(m_vftable[4])(this);

        if (m_pInstance)
        {
            void** instVtable = *reinterpret_cast<void***>(m_pInstance);
            using GetMatFn = const float*(__thiscall*)(void*);
            const float* instMat = reinterpret_cast<GetMatFn>(instVtable[3])(m_pInstance);
            CopyInstanceMatrices(this, instMat);
            reinterpret_cast<VoidThisFn>(m_vftable[10])(this);
        }
        else
        {
            SetIdentity3x4(rawFloats + 8);
        }

        auto* sleepObj = reinterpret_cast<int32*>(static_cast<uintptr_t>(m_nUnk18));
        rawBytes[0x291] = 0;
        rawDwords[166] = 0;
        m_nUnk29C = 0;
        if (sleepObj)
        {
            sleepObj[8] = sleepObj[4] - 2;
            sleepObj[9] = sleepObj[5] - 2;
            reinterpret_cast<uint8*>(sleepObj)[40] = 0;
            void** sleepVtable = *reinterpret_cast<void***>(sleepObj);
            using SleepWakeFn = void(__thiscall*)(void*);
            reinterpret_cast<SleepWakeFn>(sleepVtable[1])(sleepObj);
        }

        rawBytes[0x290] = 0;
        for (int32 off = 116; off <= 160; off += 4)
        {
            rawDwords[off] = 0;
            rawDwords[off + 1] = 0;
            rawDwords[off + 2] = 0;
        }

        if (m_pInstance)
        {
            void* archetype = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(m_pInstance) + 4);
            void** archVtable = *reinterpret_cast<void***>(archetype);

            using GetFloatFn = float(__thiscall*)(void*);
            using GetVecFn = const float*(__thiscall*)(void*);

            // 0x69C930: copy mass & inertia properties from archetype
            m_fMass = reinterpret_cast<GetFloatFn>(archVtable[9])(archetype);
            m_fInvMass = reinterpret_cast<GetFloatFn>(archVtable[10])(archetype);
            const float* ang = reinterpret_cast<GetVecFn>(archVtable[11])(archetype);
            m_vAngInertia = { ang[0], ang[1], ang[2] };
            const float* invAng = reinterpret_cast<GetVecFn>(archVtable[12])(archetype);
            m_vInvAngInertia = { invAng[0], invAng[1], invAng[2] };

            m_fMaxSpeed = reinterpret_cast<GetFloatFn>(archVtable[18])(archetype);
            m_fMaxAngSpeed = reinterpret_cast<GetFloatFn>(archVtable[19])(archetype);

            // 0x69C9A0: apply instance linear/angular velocities and matrix
            alignas(16) float tmpVec[4]{};
            void** instVtable = *reinterpret_cast<void***>(m_pInstance);
            using GetInstVecFn = void(__thiscall*)(void*, float*);
            using SetColVecFn = void(__thiscall*)(phCollider*, const float*);
            using GetMatFn = const float*(__thiscall*)(void*);

            reinterpret_cast<GetInstVecFn>(instVtable[5])(m_pInstance, tmpVec);
            reinterpret_cast<SetColVecFn>(m_vftable[42])(this, tmpVec);
            ClampLinearVelocity(this);

            reinterpret_cast<GetInstVecFn>(instVtable[6])(m_pInstance, tmpVec);
            reinterpret_cast<SetColVecFn>(m_vftable[44])(this, tmpVec);
            ClampAngularVelocity(this);

            const float* instMat = reinterpret_cast<GetMatFn>(instVtable[3])(m_pInstance);
            return static_cast<int32>(reinterpret_cast<uintptr_t>(CopyInstanceMatrices(this, instMat)));
        }

        m_fMass = 1.0f;
        m_fInvMass = 1.0f;
        m_vAngInertia = { 1.0f, 1.0f, 1.0f };
        m_vInvAngInertia = { 1.0f, 1.0f, 1.0f };
        return 0;
    }

    // 0x69C080 (retn 4)
    int32 __thiscall phCollider::FixupInstance(uintptr_t fixupTable)
    {
        const uintptr_t origPtr = reinterpret_cast<uintptr_t>(m_pInstance);
        const auto* table = reinterpret_cast<const uint32*>(fixupTable);
        const int32 count = static_cast<int32>(table[384]);

        int32 result = -1;
        for (int32 i = 0; i < count; ++i)
        {
            if (origPtr >= table[i] && origPtr < table[i + 128])
            {
                result = i;
                break;
            }
        }

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
        if (!sleepObj)
            return 0;
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
