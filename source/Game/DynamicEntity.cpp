#include "StdInc.h"
#include "Game/DynamicEntity.h"
#include "Game/Pools.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    inline uint8* GetModelInfoPtr(int16 modelIndex)
    {
        auto** modelPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
        return modelPtrs[modelIndex];
    }
}

void CDynamicEntity::InjectHooks()
{
    Core::Hook::Install(0x9E0340, &CDynamicEntity::Ctor);
    Core::Hook::Install(0x9E22D0, &CDynamicEntity::Dtor);
    Core::Hook::Install(0x9E0410, &CDynamicEntity::AddToInterior);
    Core::Hook::Install(0x9E0440, &CDynamicEntity::RemoveFromInterior);
    Core::Hook::Install(0x9E0490, &CDynamicEntity::GetBoundRadius);
    Core::Hook::Install(0x9E0660, &CDynamicEntity::GetBoundCentre);
    Core::Hook::Install(0x9E0690, &CDynamicEntity::GetBoundBoxMin);

    Core::Log::Info("CDynamicEntity hooks yuklendi (7 fonksiyon)");
}

CDynamicEntity* __thiscall CDynamicEntity::Ctor()
{
    CEntity::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD82E14));
    m_pAnimBlender = nullptr;
    Core::Hook::CallMethod<void*>(0xB29670, m_portalTracker);
    m_nFlags |= 0x2000000u;

    m_nDynUnk74 = 0;
    m_nDynUnkD0 = 0;
    m_nDynUnk100 = 0;
    m_nDynUnkD4 = 0;
    m_nDynUnk78 = 0;
    m_nDynUnkE0 = 0;
    m_nDynUnkE4 = 0;
    m_nDynUnkE8 = 0;
    m_nDynUnkF0 = 0;
    m_nDynUnkF4 = 0;
    m_nDynUnkF8 = 0x7F7FFFFFu;
    m_fBoundRadiusOffset = 0.0f;
    m_nDynUnk104 = 0;
    m_nDynUnk108 = 0;

    void** trackerVtbl = *reinterpret_cast<void***>(m_portalTracker);
    using TrackerAttachFn = void(__thiscall*)(void*, CDynamicEntity*);
    reinterpret_cast<TrackerAttachFn>(trackerVtbl[3])(m_portalTracker, this);

    Core::Hook::CallMethod<void>(0xB2A8F0, m_portalTracker, 0);
    m_nUnk58 |= 0xFF000000u;

    return this;
}

int32 __thiscall CDynamicEntity::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD82E14));
    Core::Hook::CallMethod<void>(0x9E22A0, this);
    Core::Hook::CallMethod<void>(0xB2A080, m_portalTracker);
    Core::Hook::CallMethod<void>(0xC79700, &m_pAnimBlender);
    return Core::Hook::CallMethod<int32>(0x9EA4E0, this);
}

int32 __thiscall CDynamicEntity::AddToInterior()
{
    if (m_nInteriorHash <= -1)
        return m_nInteriorHash;

    void* interior = Core::Hook::CallMethod<void*>(0xA4C560, CPools::ms_pInteriorInstPool, m_nInteriorHash);
    Core::StaticRef<void*>(0x1639450) = interior;
    if (!interior)
        return 0;

    return Core::Hook::CallMethod<int32>(0xA40220, interior, this, static_cast<int32>(m_nUnk44));
}

int32 __thiscall CDynamicEntity::RemoveFromInterior()
{
    int32 result = m_nInteriorHash;
    if (result > -1)
    {
        void* interior = Core::Hook::CallMethod<void*>(0xA4C560, CPools::ms_pInteriorInstPool, result);
        if (interior)
            result = Core::Hook::CallMethod<int32>(0xA41600, interior, this);
    }

    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xB0) = 0;
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xB8) = 0;
    m_nInteriorIndex = 0;
    m_nInteriorHash = -1;
    return result;
}

double __thiscall CDynamicEntity::GetBoundRadius()
{
    using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
    uint8* frag = reinterpret_cast<GetFragFn>(m_vftable[40])(this);
    if (!frag)
        frag = static_cast<uint8*>(m_pPhysicsInst);

    if (frag)
    {
        uint8* phArchetype = *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(frag + 4) + 12);
        return *reinterpret_cast<float*>(phArchetype + 8) + m_fBoundRadiusOffset;
    }

    return *reinterpret_cast<float*>(GetModelInfoPtr(m_nModelIndex) + 28) + m_fBoundRadiusOffset;
}

const CVector* __thiscall CDynamicEntity::GetBoundCentre()
{
    using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
    uint8* frag = reinterpret_cast<GetFragFn>(m_vftable[40])(this);
    if (!frag)
        frag = static_cast<uint8*>(m_pPhysicsInst);

    if (frag)
    {
        uint8* phArchetype = *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(frag + 4) + 12);
        return reinterpret_cast<const CVector*>(phArchetype + 32);
    }

    return CEntity::GetBoundCentre();
}

const CVector* __thiscall CDynamicEntity::GetBoundBoxMin()
{
    using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
    uint8* frag = reinterpret_cast<GetFragFn>(m_vftable[40])(this);
    if (!frag)
        frag = static_cast<uint8*>(m_pPhysicsInst);

    if (frag)
    {
        uint8* phArchetype = *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(frag + 4) + 12);
        return reinterpret_cast<const CVector*>(phArchetype + 16);
    }

    return Core::Hook::CallMethod<const CVector*>(0x9E6B00, this);
}
