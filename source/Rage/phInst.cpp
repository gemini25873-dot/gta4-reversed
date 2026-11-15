#include "StdInc.h"
#include "Rage/phInst.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include "Core/Pool.h"

namespace rage
{
    void phInst::InjectHooks()
    {
        Core::Hook::Install(0x6869C0, &phInst::Ctor);
        Core::Hook::Install(0x686BE0, &phInst::GetVelocity);
        Core::Hook::Install(0x686C00, &phInst::GetVelocityAtPoint);
        Core::Hook::Install(0x5FDCD0, &phInst::ReturnFalse);
        Core::Hook::Install(0x510B90, &phInst::ReturnMinusOne);

        phInstGta::InjectHooks();

        Core::Log::Info("rage::phInst hooks yuklendi (5 fonksiyon)");
    }

    // 0x6869C0
    phInst* __thiscall phInst::Ctor()
    {
        m_vftable     = reinterpret_cast<void**>(Core::Rebase(0xEBBAA4));
        m_pArchetype  = nullptr;
        m_nLevelIndex = 0xFFFFu;
        m_nFlags      = 0;
        m_pUserData   = nullptr;
        return this;
    }

    // 0x6869E0
    void __thiscall phInst::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBBAA4));
        if (m_pArchetype != nullptr && Core::StaticRef<uint8>(0x18A87F6) != 0)
        {
            auto* archBytes = reinterpret_cast<uint8*>(m_pArchetype);
            auto* refCount = reinterpret_cast<uint16*>(archBytes + 0x1A);
            if (--(*refCount) == 0)
            {
                void** archVftable = *reinterpret_cast<void***>(m_pArchetype);
                using DeletingDtorFn = void(__thiscall*)(void*, uint8);
                reinterpret_cast<DeletingDtorFn>(archVftable[0])(m_pArchetype, 1);
            }
        }
        m_pArchetype = nullptr;
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB998));
    }

    // 0x686BE0
    void __thiscall phInst::GetVelocity(CVector* outVel)
    {
        outVel->x = 0.0f;
        outVel->y = 0.0f;
        outVel->z = 0.0f;
    }

    // 0x686C00
    void __thiscall phInst::GetVelocityAtPoint(const CVector* point, CVector* outVel, int32 /*componentIndex*/)
    {
        using GetVelFn = void(__thiscall*)(phInst*, CVector*);
        reinterpret_cast<GetVelFn>(m_vftable[5])(this, outVel);

        CVector angVel{};
        reinterpret_cast<GetVelFn>(m_vftable[6])(this, &angVel);

        if (angVel.x != 0.0f || angVel.y != 0.0f || angVel.z != 0.0f) {
            const float dx = point->x - m_matTransform.pos.x;
            const float dy = point->y - m_matTransform.pos.y;
            const float dz = point->z - m_matTransform.pos.z;

            outVel->x += dz * angVel.y - dy * angVel.z;
            outVel->y += dx * angVel.z - dz * angVel.x;
            outVel->z += dy * angVel.x - dx * angVel.y;
        }
    }

    // 0x5FDCD0
    bool __thiscall phInst::ReturnFalse()
    {
        return false;
    }

    // 0x510B90
    int32 __thiscall phInst::ReturnMinusOne()
    {
        return -1;
    }
}

void phInstGta::InjectHooks()
{
    Core::Hook::Install(0xC70E60, &phInstGta::Ctor);
    Core::Hook::Install(0xC736C0, &phInstGta::DeletingDtor);
    Core::Hook::Install(0xC70E90, &phInstGta::GetClassType);

    Core::Log::Info("phInstGta hooks yuklendi (3 fonksiyon)");
}

// 0xC70E60
phInstGta* __thiscall phInstGta::Ctor(int32 classType)
{
    rage::phInst::Ctor();
    m_nClassType = classType;
    m_vftable    = reinterpret_cast<void**>(Core::Rebase(0xDC2354));
    m_pUserData  = nullptr;
    return this;
}

// 0xC736C0
phInstGta* __thiscall phInstGta::DeletingDtor(uint8 flags)
{
    // 0xC70EA0: set phInstGta vftable then chain into phInst::Dtor (0x6869E0)
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDC2354));
    rage::phInst::Dtor();

    if (flags & 1) {
        // 0xC79660: free slot in pool (0x1706658)
        auto* pool = Core::StaticRef<CPoolBase*>(0x1706658);
        const int32 idx = pool->GetIndex(this);
        pool->m_byteMap[idx] |= 0x80;
        --pool->m_nUsed;
        if (idx < pool->m_nFirstFree) {
            pool->m_nFirstFree = idx;
        }
    }
    return this;
}

// 0xC70E90
int32 __thiscall phInstGta::GetClassType()
{
    return m_nClassType;
}
