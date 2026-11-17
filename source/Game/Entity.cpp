#include "StdInc.h"
#include "Game/Entity.h"
#include "Game/Pools.h"
#include "Game/ModelInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    inline CBaseModelInfo* GetModelInfoPtr(int16 modelIndex)
    {
        return CModelInfo::ms_modelInfoPtrs[modelIndex];
    }
}

void CEntity::InjectHooks()
{
    Core::Hook::Install(0x9E7B60, &CEntity::Ctor);
    Core::Hook::Install(0x9E6A10, &CEntity::PreRender);
    Core::Hook::Install(0x9E6A40, &CEntity::UpdatePhysics);
    Core::Hook::Install(0x9E6D80, &CEntity::UpdateTransform);
    Core::Hook::Install(0x9E6AE0, &CEntity::GetBoundRadius);
    Core::Hook::Install(0x9E6AF0, &CEntity::GetBoundCentre);
    Core::Hook::Install(0x9E8680, &CEntity::AddToInterior);
    Core::Hook::Install(0x9E86C0, &CEntity::RemoveFromInterior);

    Core::Log::Info("CEntity hooks installed (8 functions)");
}

CEntity* __thiscall CEntity::Ctor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD83BAC));
    m_vecPos = {};
    m_fPad1C = 0.0f;
    m_nUnk6C = 0;

    m_nUnk5C &= 0xC000u;
    m_nUnk5E &= ~1u;

    m_pMatrix = nullptr;
    m_nFlags = 0x80000020u;
    m_nFlags2 = (m_nFlags2 & 0xC00C8001u) | 0x80Cu;

    m_pPhysicsInst = nullptr;
    m_nModelIndex = -1;
    m_pDrawHandler = nullptr;
    m_nInteriorHash = -1;

    m_nInteriorIndex = 63;
    m_nUnk42 = -1;
    m_nUnk44 = -1;
    m_nUnk41 = 0;

    const uint16 seed = static_cast<uint16>(Core::Hook::Call<int32>(0xD0D21A));
    m_nFlags2 &= ~0x80000u;
    m_nUnk5E &= ~2u;
    m_nRandomSeed = seed;

    m_nUnk54 &= 0xFF000000u;
    m_nUnk64 = -1;
    m_nUnk68 = -1;
    m_nUnk63 = -1;
    m_nUnk62 = -1;
    m_nUnk60 = -1;
    m_pReferences = 0;
    m_nUnk61 = 0;
    m_pLod = nullptr;
    m_nUnk50 = 0;
    m_nUnk58 = 0xFF000000u;
    m_nUnk08 = 0x40000000u;

    return this;
}

int32 __thiscall CEntity::PreRender(int32 arg)
{
    if (m_pDrawHandler)
        Core::Hook::CallMethod<int32>(0xA4BEA0, m_pDrawHandler, arg);

    using VirtualFn = int32(__thiscall*)(CEntity*, int32);
    return reinterpret_cast<VirtualFn>(m_vftable[19])(this, arg);
}

void __thiscall CEntity::UpdatePhysics(int32 arg)
{
    void* inst = m_pPhysicsInst;
    if (!inst)
        return;

    void** vtbl = *reinterpret_cast<void***>(inst);
    using GetTypeFn = int32(__thiscall*)(void*);
    const int32 type = reinterpret_cast<GetTypeFn>(vtbl[1])(inst);
    if (type >= 13 || type == 3)
    {
        using UpdateFn = void(__thiscall*)(void*, int32);
        reinterpret_cast<UpdateFn>(vtbl[2])(inst, arg);
    }
}

int32 __thiscall CEntity::UpdateTransform()
{
    uint8 matrixCopy[16]{};
    ms_pCurrentEntity = this;

    if (m_pMatrix)
        m_nFlags2 |= 0x400000u;
    else
        m_nFlags2 &= ~0x400000u;

    using GetMatrixFn = void(__thiscall*)(CEntity*, uint8*);
    reinterpret_cast<GetMatrixFn>(m_vftable[26])(this, matrixCopy);

    using SetMatrixFn = int32(__thiscall*)(CEntity*, uint8*);
    return reinterpret_cast<SetMatrixFn>(m_vftable[5])(this, matrixCopy);
}

double __thiscall CEntity::GetBoundRadius() const
{
    return GetModelInfoPtr(m_nModelIndex)->m_fBoundRadius;
}

const CVector* __thiscall CEntity::GetBoundCentre() const
{
    return &GetModelInfoPtr(m_nModelIndex)->m_vBoundCentre;
}

int32 __thiscall CEntity::AddToInterior()
{
    ms_pCurrentEntity = this;
    if (m_nInteriorHash <= -1)
        return m_nInteriorHash;

    void* interior = CPools::ms_pInteriorInstPool->GetAtHandleRegistered(m_nInteriorHash);
    if (!interior)
        return 0;

    return Core::Hook::CallMethod<int32>(
        0xA40000, interior, this, static_cast<int32>(m_nInteriorIndex), static_cast<int32>(m_nUnk44));
}

int32 __thiscall CEntity::RemoveFromInterior()
{
    if (m_nInteriorHash <= -1 || m_nInteriorIndex <= -1)
        return m_nInteriorHash;

    void* interior = CPools::ms_pInteriorInstPool->GetAtHandleRegistered(m_nInteriorHash);
    if (!interior)
        return 0;

    Core::Hook::CallMethod<void>(0xA3D150, interior, this);
    return 0;
}
