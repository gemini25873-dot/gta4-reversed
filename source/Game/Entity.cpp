#include "StdInc.h"
#include "Game/Entity.h"
#include "Game/Pools.h"
#include "Game/ModelInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <algorithm>
#include <cmath>

namespace
{
    inline CBaseModelInfo* GetModelInfoPtr(int16 modelIndex)
    {
        return CModelInfo::ms_modelInfoPtrs[modelIndex];
    }

    struct PtrNodeSingle
    {
        CEntity*       entity;
        PtrNodeSingle* next;
    };

    struct PtrNodeDouble
    {
        CEntity*       entity;
        PtrNodeDouble* next;
        PtrNodeDouble* prev;
    };

    inline void CopyMatrix3x4(CMatrix* dst, const CMatrix* src)
    {
        dst->right = src->right;
        dst->up    = src->up;
        dst->at    = src->at;
        dst->pos   = src->pos;
    }

    inline void** GetGridSectorPtr(uint32 listType, int32 x, int32 y)
    {
        if (listType == 1 || listType == 5 || listType == 6)
        {
            const int32 cx = std::clamp(x, 0, 0x77);
            const int32 cy = std::clamp(y, 0, 0x77);
            const int32 idx = cx + cy * 120;
            const uintptr_t base = (listType == 1) ? 0x11A8568u : 0x11A856Cu;
            return reinterpret_cast<void**>(Core::Rebase(base)) + idx * 3;
        }

        const int32 sx = x & 0xF;
        const int32 sy = y & 0xF;
        const int32 idx = (sy << 4) + sx;
        uintptr_t base = 0x11A7160u;
        switch (listType)
        {
        case 2: base = 0x11A7160u; break;
        case 3: base = 0x11A7164u; break;
        case 4: base = 0x11A7168u; break;
        case 7: base = 0x11A716Cu; break;
        case 8: base = 0x11A7170u; break;
        default: break;
        }
        return reinterpret_cast<void**>(Core::Rebase(base)) + idx * 5;
    }

    inline uint32 PackGridBounds(int32 x0, int32 y0, int32 x1, int32 y1)
    {
        const uint32 dx = static_cast<uint32>(x1 - x0) << 22;
        const uint32 dy = static_cast<uint32>(y1 - y0) << 27;
        const uint32 bx = static_cast<uint32>(x0 - 1024) & 0x7FFu;
        const uint32 by = (static_cast<uint32>(y0) * 0x800u - 0x1FF801u) & 0x3FF800u;
        return (dx & 0x7C00000u) | (dy & 0xF8000000u) | bx | by;
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

    Core::Hook::Install(0x9E6480, &CEntity::LockReferences);
    Core::Hook::Install(0x9E6490, &CEntity::UnlockReferences);
    Core::Hook::Install(0x9E64A0, &CEntity::InitReferences);
    Core::Hook::Install(0x9E6530, &CEntity::ShutdownReferences);
    Core::Hook::Install(0x9E6550, &CEntity::RegisterReference);
    Core::Hook::Install(0x9E6580, &CEntity::CleanUpOldReference);
    Core::Hook::Install(0x9E65D0, &CEntity::ResolveReferences);
    Core::Hook::Install(0x9E6630, &CEntity::PruneReferences);
    Core::Hook::Install(0x9E6680, &CEntity::PruneAllImportantReferences);
    Core::Hook::Install(0x9E6720, &CEntity::PruneReferencesInPools);
    Core::Hook::Install(0x9E6830, &CEntity::CopyInteriorInfo);
    Core::Hook::Install(0x9E6860, &CEntity::GetPortalTracker);
    Core::Hook::Install(0x9E6890, &CEntity::Init2dEffects);
    Core::Hook::Install(0x9E69D0, &CEntity::Remove2dEffects);
    Core::Hook::Install(0x9E69E0, &CEntity::SetModelByName);
    Core::Hook::Install(0x9E6A80, &CEntity::DeleteDrawHandler);
    Core::Hook::Install(0x9E6B00, &CEntity::GetBoundMax);
    Core::Hook::Install(0x9E6B10, &CEntity::HasDrawableOrEffects);
    Core::Hook::Install(0x9E6BC0, &CEntity::HasIdentityMatrix);
    Core::Hook::Install(0x9E6BD0, &CEntity::IsOnScreen);
    Core::Hook::Install(0x9E6C80, &CEntity::IsVisibleInFrustum);

    Core::Hook::Install(0x9E6CF0, &CEntity::SetInteriorState);
    Core::Hook::Install(0x9E6D60, &CEntity::ClearInteriorState);
    Core::Hook::Install(0x9E6DD0, &CEntity::MarkModelUsed);
    Core::Hook::Install(0x9E6DF0, &CEntity::GetOffsetVec);
    Core::Hook::Install(0x9E6E20, &CEntity::FindClosest2dEffect);
    Core::Hook::Install(0x9E6F20, &CEntity::HasNetworkIdBelow200);
    Core::Hook::Install(0x9E6F60, &CEntity::IsNetworkIdFFFE);
    Core::Hook::Install(0x9E6FA0, &CEntity::SetPhysicsInst);
    Core::Hook::Install(0x9E6FF0, &CEntity::DeletePhysicsInst);
    Core::Hook::Install(0x9E7080, &CEntity::GetPhysicsCollider);
    Core::Hook::Install(0x9E70C0, &CEntity::IsNotInPhysicsLevel);
    Core::Hook::Install(0x9E7110, &CEntity::RemoveFromPhysicsLevel);
    Core::Hook::Install(0x9E7140, &CEntity::AddEntityToList);
    Core::Hook::Install(0x9E7170, &CEntity::UpdateUnk6C);
    Core::Hook::Install(0x9E71A0, &CEntity::GetNum2dEffects);
    Core::Hook::Install(0x9E71B0, static_cast<void(__thiscall CEntity::*)(float)>(&CEntity::Process2dEffects));
    Core::Hook::Install(0x9E7290, &CEntity::Create2dEffectAudio);
    Core::Hook::Install(0x9E7310, &CEntity::Destroy2dEffectAudio);
    Core::Hook::Install(0x9E7390, &CEntity::Process2dEffectsPtfx);

    Core::Hook::Install(0x9E77F0, &CEntity::TransformPoint2D);
    Core::Hook::Install(0x9E7880, &CEntity::IsInArea2D);
    Core::Hook::Install(0x9E7900, &CEntity::IsInArea3D);
    Core::Hook::Install(0x9E79C0, &CEntity::GetTypeSpecificPtr);
    Core::Hook::Install(0x9E7AB0, &CEntity::CheckAndClearLodFlag);
    Core::Hook::Install(0x9E7B20, &CEntity::SetFlag5EBit1);
    Core::Hook::Install(0x9E7C20, &CEntity::SetModelIndex);
    Core::Hook::Install(0x9E7CF0, &CEntity::CreateDrawHandler);
    Core::Hook::Install(0x9E7EC0, &CEntity::GetBoundRect);
    Core::Hook::Install(0x9E8280, &CEntity::GetBoundCentreAndRadius);
    Core::Hook::Install(0x9E8390, &CEntity::CalcBoundCentreAndRadius);
    Core::Hook::Install(0x9E8490, &CEntity::GetBoundSphere);
    Core::Hook::Install(0x9E85A0, &CEntity::CheckDrawableAndUpdate);
    Core::Hook::Install(0x9E85C0, &CEntity::IsInFrustum);
    Core::Hook::Install(0x9E8630, &CEntity::CanAddToInterior);
    Core::Hook::Install(0x9E86F0, &CEntity::GetPhArchetype);
    Core::Hook::Install(0x9E8710, &CEntity::RemoveFromPhysicsWorld);
    Core::Hook::Install(0x9E8740, &CEntity::InitModelEffects);
    Core::Hook::Install(0x9E87E0, &CEntity::TriggerModelEffectsType1);
    Core::Hook::Install(0x9E8940, &CEntity::TriggerModelEffectsType2);
    Core::Hook::Install(0x9E8DA0, &CEntity::TriggerModelEffectsAtMatrix);

    Core::Hook::Install(0x9E94E0, &CEntity::RemoveMatrix);
    Core::Hook::Install(0x9E9590, &CEntity::GetWorldBoundBox);
    Core::Hook::Install(0x9E9BA0, &CEntity::UpdateRenderAlpha);
    Core::Hook::Install(0x9E9DD0, &CEntity::SetInteriorByName);
    Core::Hook::Install(0x9E9E10, &CEntity::AddToGrid);
    Core::Hook::Install(0x9EA400, &CEntity::AllocateMatrix);
    Core::Hook::Install(0x9EA460, &CEntity::AllocateStaticMatrix);
    Core::Hook::Install(0x9EA4B0, &CEntity::ReleaseMatrix);
    Core::Hook::Install(0x9EA4E0, &CEntity::Dtor);
    Core::Hook::Install(0x9EA540, &CEntity::Render);
    Core::Hook::Install(0x9EA680, &CEntity::RemoveFromGrid);
    Core::Hook::Install(0x9EAB70, &CEntity::CreatePhysicsInst);
    Core::Hook::Install(0x9EAC60, &CEntity::UpdatePhysicsMatrix);
    Core::Hook::Install(0x9EAD00, static_cast<void(__thiscall CEntity::*)(int32, int32, int32)>(&CEntity::Process2dEffects));
    Core::Hook::Install(0x9EAF60, &CEntity::SetMatrix);
    Core::Hook::Install(0x9EB060, &CEntity::DeletingDtor);

    Core::Log::Info("CEntity hooks installed (85 functions)");
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

void __cdecl CEntity::LockReferences()
{
    ++ms_nRefLockCount;
}

void __cdecl CEntity::UnlockReferences()
{
    --ms_nRefLockCount;
}

void __cdecl CEntity::InitReferences()
{
    if (ms_pRefPool)
    {
        Core::Hook::Call<void>(0x5B1C10, ms_pRefPool);
        ms_pRefPool = nullptr;
    }

    ms_nMaxRefs = 0x1F40;
    const size_t allocSize = static_cast<size_t>(ms_nMaxRefs) * sizeof(CEntityReference);
    CEntityReference* pool = Core::Hook::Call<CEntityReference*>(0x4011D0, allocSize);

    ms_pRefPool = pool;
    ms_pRefFreeList = pool;

    for (int32 i = 0; i < ms_nMaxRefs; ++i)
    {
        pool[i].pNext = &pool[i + 1];
        pool[i].ppEntity = nullptr;
    }

    pool[ms_nMaxRefs - 1].pNext = nullptr;
}

void __cdecl CEntity::ShutdownReferences()
{
    if (ms_pRefPool)
    {
        Core::Hook::Call<void>(0x5B1C10, ms_pRefPool);
        ms_pRefPool = nullptr;
    }
}

void __thiscall CEntity::RegisterReference(CEntity** ppRef)
{
    LockReferences();

    CEntityReference* ref = ms_pRefFreeList;
    if (ref)
    {
        ms_pRefFreeList = ref->pNext;
        ref->pNext = reinterpret_cast<CEntityReference*>(m_pReferences);
        m_pReferences = reinterpret_cast<uint32>(ref);
        ref->ppEntity = ppRef;
    }

    UnlockReferences();
}

void __thiscall CEntity::CleanUpOldReference(CEntity** ppRef)
{
    LockReferences();

    CEntityReference** prev = reinterpret_cast<CEntityReference**>(&m_pReferences);
    CEntityReference* cur = *prev;
    while (cur)
    {
        if (cur->ppEntity == ppRef)
        {
            *prev = cur->pNext;
            cur->pNext = ms_pRefFreeList;
            ms_pRefFreeList = cur;
            cur->ppEntity = nullptr;
            break;
        }
        prev = &cur->pNext;
        cur = cur->pNext;
    }

    UnlockReferences();
}

void __thiscall CEntity::ResolveReferences()
{
    LockReferences();

    CEntityReference* cur = reinterpret_cast<CEntityReference*>(m_pReferences);
    while (cur)
    {
        if (*cur->ppEntity == this)
            *cur->ppEntity = nullptr;
        cur = cur->pNext;
    }

    CEntityReference* head = reinterpret_cast<CEntityReference*>(m_pReferences);
    if (head)
    {
        CEntityReference* tail = head;
        tail->ppEntity = nullptr;
        while (tail->pNext)
        {
            tail = tail->pNext;
            tail->ppEntity = nullptr;
        }
        tail->pNext = ms_pRefFreeList;
        ms_pRefFreeList = head;
        m_pReferences = 0;
    }

    UnlockReferences();
}

void __thiscall CEntity::PruneReferences()
{
    LockReferences();

    CEntityReference** prev = reinterpret_cast<CEntityReference**>(&m_pReferences);
    CEntityReference* cur = *prev;
    while (cur)
    {
        if (*cur->ppEntity != this)
        {
            *prev = cur->pNext;
            cur->pNext = ms_pRefFreeList;
            ms_pRefFreeList = cur;
            cur->ppEntity = nullptr;
            cur = *prev;
        }
        else
        {
            prev = &cur->pNext;
            cur = cur->pNext;
        }
    }

    UnlockReferences();
}

void __cdecl CEntity::PruneAllImportantReferences()
{
    CEntity* playerPed = Core::Hook::Call<CEntity*>(0x817F40);
    CEntity* playerVehicle = Core::Hook::Call<CEntity*>(0x817F80);

    if (playerVehicle)
        playerVehicle->ResolveReferences();

    if (!playerPed)
        return;

    playerPed->ResolveReferences();

    const uintptr_t pedPtr = *reinterpret_cast<const uintptr_t*>(reinterpret_cast<const uint8*>(playerPed) + 0x228);
    const uint8* playerData = pedPtr ? reinterpret_cast<const uint8*>(pedPtr + 0x60) : nullptr;
    const int32 idx = *reinterpret_cast<const int32*>(playerData + 0x3D4);
    if (idx >= 0)
    {
        uint8* item = reinterpret_cast<uint8*>(Core::Rebase(0x1662170)) + idx * 0x5C;
        void* sub = item + 8;
        if (!Core::Hook::CallMethod<int32>(0xA66B40, sub))
            Core::Hook::CallMethod<void>(0xA67AA0, sub, playerPed);
        Core::Hook::CallMethod<void>(0xA67D70, item);
    }

    const int32 slot = *reinterpret_cast<const int32*>(Core::Rebase(0xF1CC68));
    void* target = (slot == -1) ? nullptr : reinterpret_cast<void**>(Core::Rebase(0x11A7008))[slot];
    Core::Hook::CallMethod<void>(0x8786F0, target, playerPed);
}

void __cdecl CEntity::PruneReferencesInPools()
{
    const uint32 frameCounter = *reinterpret_cast<const uint32*>(Core::Rebase(0x11DDF38));

    CPoolBase* buildingPool = CPools::ms_pBuildingPool;
    const int32 buildingSize = buildingPool->m_nSize;
    const int32 buildingPhase = static_cast<int32>(frameCounter % 1500);
    const int32 buildingStart = (buildingPhase * buildingSize) / 1500;
    const int32 buildingEnd = ((buildingPhase + 1) * buildingSize) / 1500;

    for (int32 i = buildingStart; i < buildingEnd; ++i)
    {
        if (!buildingPool->IsSlotFree(i))
        {
            CEntity* entity = reinterpret_cast<CEntity*>(
                static_cast<uint8*>(buildingPool->m_pObjects) + i * buildingPool->m_nElementSize);
            if (entity)
                entity->PruneReferences();
        }
    }

    CPoolBase* dummyPool = CPools::ms_pDummyPool;
    const int32 dummySize = dummyPool->m_nSize;
    const int32 dummyPhase = static_cast<int32>(frameCounter % 500);
    const int32 dummyStart = (dummyPhase * dummySize) / 500;
    const int32 dummyEnd = ((dummyPhase + 1) * dummySize) / 500;

    for (int32 i = dummyStart; i < dummyEnd; ++i)
    {
        if (!dummyPool->IsSlotFree(i))
        {
            CEntity* entity = reinterpret_cast<CEntity*>(
                static_cast<uint8*>(dummyPool->m_pObjects) + i * dummyPool->m_nElementSize);
            if (entity)
                entity->PruneReferences();
        }
    }
}

void __thiscall CEntity::CopyInteriorInfo(const CEntity* src, bool copyUnk44)
{
    if (!src)
        return;

    m_nInteriorHash = src->m_nInteriorHash;
    m_nInteriorIndex = src->m_nInteriorIndex;
    if (copyUnk44)
        m_nUnk44 = src->m_nUnk44;
    m_nUnk41 = src->m_nUnk41;
}

void* __thiscall CEntity::GetPortalTracker()
{
    const uint32 type = (m_nFlags2 >> 6) & 0xF;
    if (type == 3)
        return reinterpret_cast<uint8*>(this) + 0x790;
    if (type == 2)
        return reinterpret_cast<uint8*>(this) + 0xE08;
    if (type == 4)
        return reinterpret_cast<uint8*>(this) + 0x264;
    return nullptr;
}

void __thiscall CEntity::Init2dEffects(int32 modelIndex)
{
    if (modelIndex == -1)
        modelIndex = m_nModelIndex;

    CBaseModelInfo* mi = GetModelInfoPtr(static_cast<int16>(modelIndex));
    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* eff = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        void** vtbl = *reinterpret_cast<void***>(eff);

        using GetEffectTypeFn = uint8(__thiscall*)(void*);
        if (reinterpret_cast<GetEffectTypeFn>(vtbl[1])(eff) == 0xF)
        {
            using GetDataFn = const uint8*(__thiscall*)(void*);
            const uint8* data = reinterpret_cast<GetDataFn>(vtbl[19])(eff);

            const float* effFloats = reinterpret_cast<const float*>(eff);
            CVector pos(effFloats[1], effFloats[2], effFloats[3]);

            const float* dataFloats = reinterpret_cast<const float*>(data);
            float quat[4]{ dataFloats[4], dataFloats[5], dataFloats[6], dataFloats[7] };
            const uint32 param = *reinterpret_cast<const uint32*>(data + 0x20);

            Core::Hook::CallMethod<void>(
                0x8B6410, reinterpret_cast<void*>(Core::Rebase(0x1230EC0)), this, param, &pos, quat);
        }
    }

    if ((m_nFlags2 & 0x3C0) == 0x40)
    {
        using GetBoundCentreFn = const CVector*(__thiscall*)(CEntity*);
        const CVector* boundCentre = reinterpret_cast<GetBoundCentreFn>(m_vftable[25])(this);
        const CVector& pos = GetPosition();
        const float z = boundCentre->z + pos.z;

        Core::Hook::CallMethod<void>(
            0xAE2CD0, reinterpret_cast<void*>(Core::Rebase(0x10CC370)), &pos, z);
    }
}

void __thiscall CEntity::Remove2dEffects()
{
    Core::Hook::CallMethod<void>(0x8B6A50, reinterpret_cast<void*>(Core::Rebase(0x1230EC0)), this);
}

int32 __thiscall CEntity::SetModelByName(const char* modelName)
{
    int32 modelIndex = 0;
    Core::Hook::Call<void>(0x98AA80, modelName, &modelIndex);

    using SetModelIndexFn = int32(__thiscall*)(CEntity*, int32);
    return reinterpret_cast<SetModelIndexFn>(m_vftable[14])(this, modelIndex);
}

void __thiscall CEntity::DeleteDrawHandler()
{
    if (!m_pDrawHandler)
        return;

    Core::Hook::CallMethod<void>(0xA4C0B0, reinterpret_cast<void*>(Core::Rebase(0x1660CB8)), m_pDrawHandler);
    m_pDrawHandler = nullptr;

    CBaseModelInfo* mi = GetModelInfoPtr(m_nModelIndex);
    Core::Hook::CallMethod<void>(0x98E470, mi);

    const uint8 modelType = mi->GetModelType();
    if (modelType == 5)
        --(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(mi) + 0xB0));
    else if (mi->GetModelType() == 6)
        --(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(mi) + 0xAC));
}

const CVector* __thiscall CEntity::GetBoundMax() const
{
    return &GetModelInfoPtr(m_nModelIndex)->m_vMaxBounds;
}

bool __thiscall CEntity::HasDrawableOrEffects() const
{
    CBaseModelInfo* mi = GetModelInfoPtr(m_nModelIndex);
    uint8* drawableStruct = static_cast<uint8*>(mi->m_pDrawableStruct);
    if (drawableStruct && *reinterpret_cast<uint32*>(drawableStruct + 4) != 0)
        return true;

    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* eff = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        void** vtbl = *reinterpret_cast<void***>(eff);

        using GetTypeFn = uint8(__thiscall*)(void*);
        const uint8 type = reinterpret_cast<GetTypeFn>(vtbl[1])(eff);
        if (type == 0 || type == 0x12 || type == 0x13 || type == 0x17)
            return true;

        if (type == 1)
        {
            using GetSubFn = uint8*(__thiscall*)(void*);
            uint8* sub = reinterpret_cast<GetSubFn>(vtbl[7])(eff);
            if (*reinterpret_cast<uint32*>(sub + 0x24) == 0)
                return true;
        }
    }

    return false;
}

bool __thiscall CEntity::HasIdentityMatrix() const
{
    return m_pMatrix == reinterpret_cast<const CMatrix*>(Core::Rebase(0x1639580));
}

bool __thiscall CEntity::IsOnScreen(bool checkShadowOrReflection)
{
    using GetRadiusFn = float(__thiscall*)(CEntity*);
    using GetCentreFn = float*(__thiscall*)(CEntity*, float*);

    const float radius = reinterpret_cast<GetRadiusFn>(m_vftable[22])(this);
    struct { CVector vec; float w; } centre{};
    reinterpret_cast<GetCentreFn>(m_vftable[20])(this, &centre.vec.x);

    const int32 camIdx = *reinterpret_cast<const int32*>(Core::Rebase(0x10F4808));
    uint8* cam = reinterpret_cast<uint8**>(Core::Rebase(0x10F4808))[camIdx];

    if (Core::Hook::CallMethod<int32>(0x62EF60, cam + 0x10, centre.vec, radius, 0) != 0)
        return true;

    if (checkShadowOrReflection && Core::Hook::Call<bool>(0x462F90))
    {
        const float dist = *reinterpret_cast<const float*>(Core::Rebase(0xD8F340));
        if (Core::Hook::CallMethod<bool>(
                0x476120, reinterpret_cast<void*>(Core::Rebase(0x18EDB70)), &centre.vec, radius, dist, 0))
        {
            return true;
        }
    }

    return false;
}

bool __thiscall CEntity::IsVisibleInFrustum(void* viewport)
{
    using GetRadiusFn = float(__thiscall*)(CEntity*);
    using GetCentreFn = float*(__thiscall*)(CEntity*, float*);

    const float radius = reinterpret_cast<GetRadiusFn>(m_vftable[22])(this);
    struct { CVector vec; float w; } centre{};
    reinterpret_cast<GetCentreFn>(m_vftable[20])(this, &centre.vec.x);

    return Core::Hook::CallMethod<int32>(
               0x62EF60, static_cast<uint8*>(viewport) + 0xB0, centre.vec, radius, 0) != 0;
}

void __thiscall CEntity::SetInteriorState(int32 interiorHash, int32 roomIndex, int32 portalIndex)
{
    if (interiorHash < 0)
    {
        m_nUnk41 = 3;
        return;
    }

    if (roomIndex > 0)
    {
        m_nInteriorHash = interiorHash;
        m_nInteriorIndex = static_cast<int8>(roomIndex);
        m_nUnk41 = 5;
        return;
    }

    if (portalIndex > -1)
    {
        m_nInteriorHash = interiorHash;
        m_nUnk44 = static_cast<int16>(portalIndex);
        m_nUnk41 = 6;
        return;
    }

    if (roomIndex == 0)
    {
        const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;
        if (entityType == 4u || entityType == 5u)
            return;

        m_nInteriorHash = interiorHash;
        m_nInteriorIndex = 0;
        m_nUnk41 = 7;
        return;
    }

    m_nUnk41 = 3;
}

void __thiscall CEntity::ClearInteriorState()
{
    if ((m_nFlags >> 25) & 1u)
    {
        m_nUnk41 = 4;
        *(reinterpret_cast<uint8*>(this) + 0xBE) = 1;
    }
    else
    {
        m_nUnk41 = 2;
    }
}

void __thiscall CEntity::MarkModelUsed()
{
    m_nFlags = (m_nFlags & ~1u) | 0x40u;
    GetModelInfoPtr(m_nModelIndex)->m_nIDEFlags |= 0x20u;
}

CVector* __thiscall CEntity::GetOffsetVec(CVector* outVec, int32 /*arg*/) const
{
    Core::Hook::CallMethod<int32>(0x98EA60, GetModelInfoPtr(m_nModelIndex));
    *outVec = CVector(0.0f, 0.0f, 0.0f);
    return outVec;
}

void* __thiscall CEntity::FindClosest2dEffect(int32 effectType, const CVector* pos) const
{
    CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
    void* bestEffect = nullptr;
    float bestDistSq = 0.0f;

    int32 count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, modelInfo, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        const uint8 type = reinterpret_cast<uint8(__thiscall*)(void*)>(vtbl[1])(effect);
        if (static_cast<int32>(type) == effectType)
        {
            CVector effectPos{};
            Core::Hook::CallMethod<void>(0xBBDF20, effect, &effectPos, this);

            const CVector delta = effectPos - *pos;
            const float distSq = delta.MagnitudeSquared();
            if (!bestEffect || distSq < bestDistSq)
            {
                bestEffect = effect;
                bestDistSq = distSq;
            }
        }
        count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    }

    return bestEffect;
}

bool __thiscall CEntity::HasNetworkIdBelow200() const
{
    const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;
    if (entityType == 4u)
        return *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(this) + 0x220) < 0x200u;
    if (entityType == 2u)
        return *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(this) + 0x10C4) < 0x200u;
    return false;
}

bool __thiscall CEntity::IsNetworkIdFFFE() const
{
    const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;
    if (entityType == 4u)
        return *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(this) + 0x220) == 0xFFFEu;
    if (entityType == 2u)
        return *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(this) + 0x10C4) == 0xFFFEu;
    return false;
}

void __thiscall CEntity::SetPhysicsInst(void* inst, bool addModelRef)
{
    m_pPhysicsInst = inst;
    reinterpret_cast<void**>(inst)[3] = this;
    Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x40, 1);

    if (addModelRef)
    {
        CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
        if (*reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(modelInfo) + 0x52) > -1)
            Core::Hook::CallMethod<void>(0x98ED50, modelInfo);

        if (m_nFlags & 0x4000000u)
            Core::Hook::CallMethod<void>(0x98E460, modelInfo);
    }
}

void __thiscall CEntity::DeletePhysicsInst(int32 /*unused*/)
{
    if (m_pPhysicsInst)
    {
        void** vtbl = *reinterpret_cast<void***>(m_pPhysicsInst);
        const uint32 type = reinterpret_cast<uint32(__thiscall*)(void*)>(vtbl[1])(m_pPhysicsInst);
        if (type != 0x12u)
        {
            if (type >= 0xDu)
            {
                Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x800, 1);
                if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(m_pPhysicsInst) + 0x60) & 1u) &&
                    Core::StaticRef<void*>(0x1666888) != nullptr)
                {
                    void** vtbl2 = *reinterpret_cast<void***>(m_pPhysicsInst);
                    reinterpret_cast<void(__thiscall*)(void*)>(vtbl2[0xA4 / 4])(m_pPhysicsInst);
                }
            }

            if (m_pPhysicsInst)
            {
                void** vtbl3 = *reinterpret_cast<void***>(m_pPhysicsInst);
                reinterpret_cast<void(__thiscall*)(void*, int32)>(vtbl3[0])(m_pPhysicsInst, 1);
            }
        }

        m_pPhysicsInst = nullptr;
        CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
        if (*reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(modelInfo) + 0x52) > -1)
            Core::Hook::CallMethod<void>(0x98ED60, modelInfo);

        if (m_nFlags & 0x4000000u)
            Core::Hook::CallMethod<void>(0x98E470, modelInfo);
    }
}

void* __thiscall CEntity::GetPhysicsCollider() const
{
    if (!m_pPhysicsInst)
        return nullptr;

    const uint16 levelIndex = *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIndex == 0xFFFFu)
        return nullptr;

    uint8* sim = Core::StaticRef<uint8*>(0x1666884);
    uint8* level = *reinterpret_cast<uint8**>(sim + 4);
    const int32 idx = static_cast<int32>(levelIndex);
    if (idx < 0 || idx >= static_cast<int32>(*reinterpret_cast<const uint16*>(level + 0x26)))
        return nullptr;

    const uint8* states = *reinterpret_cast<const uint8* const*>(level + 0x70);
    if (states[idx * 8 + 4] & 3u)
        return nullptr;

    void** colliders = *reinterpret_cast<void***>(level + 0x80);
    return colliders[idx];
}

bool __thiscall CEntity::IsNotInPhysicsLevel() const
{
    if (!m_pPhysicsInst)
        return true;

    const uint16 levelIndex = *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIndex == 0xFFFFu)
        return true;

    const int32 idx = static_cast<int32>(levelIndex);
    uint8* level = Core::StaticRef<uint8*>(0x1666880);
    if (idx < 0 || idx >= static_cast<int32>(*reinterpret_cast<const uint16*>(level + 0x26)))
        return true;

    const uint8* states = *reinterpret_cast<const uint8* const*>(level + 0x70);
    return (states[idx * 8 + 4] & 3u) != 0;
}

void __thiscall CEntity::RemoveFromPhysicsLevel()
{
    if (!m_pPhysicsInst)
        return;

    const uint16 levelIndex = *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIndex != 0xFFFFu)
        Core::Hook::CallMethod<void>(0x69A470, Core::StaticRef<void*>(0x1666884), static_cast<int32>(levelIndex), 0);
}

void __stdcall CEntity::AddEntityToList(void** array, int32* count, int32 maxCount, void* item)
{
    if (item && *count < maxCount)
    {
        array[*count] = item;
        (*count)++;
    }
}

void __thiscall CEntity::UpdateUnk6C(int32 arg)
{
    void* obj = reinterpret_cast<void*>(m_nUnk6C);
    if (!obj)
        return;

    void** vtbl = *reinterpret_cast<void***>(obj);
    if (reinterpret_cast<int32(__thiscall*)(void*)>(vtbl[1])(obj) != 0)
        Core::Hook::CallMethod<void>(0x494080, obj, 1, arg);
}

int32 __thiscall CEntity::GetNum2dEffects() const
{
    return Core::Hook::CallMethod<int32>(0x98EA60, GetModelInfoPtr(m_nModelIndex));
}

void __thiscall CEntity::Process2dEffects(float arg)
{
    CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
    const bool hasFlag = (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(modelInfo) + 0x5B) & 1u) != 0;
    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);

    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, modelInfo, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        using GetTypeFn = uint8(__thiscall*)(void*);
        using GetAttrFn = void*(__thiscall*)(void*);

        if (hasFlag && reinterpret_cast<GetTypeFn>(vtbl[1])(effect) == 1)
        {
            void* attr = reinterpret_cast<GetAttrFn>(vtbl[7])(effect);
            if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(attr) + 0x24) == 0)
            {
                void* attr2 = reinterpret_cast<GetAttrFn>(vtbl[7])(effect);
                Core::Hook::CallMethod<void>(0xC84E90, reinterpret_cast<void*>(Core::Rebase(0x170F670)), this, attr2, i);
                continue;
            }
        }

        const uint8 type = reinterpret_cast<GetTypeFn>(vtbl[1])(effect);
        if (type == 0)
        {
            Core::Hook::CallMethod<void>(0x8B7890, reinterpret_cast<void*>(Core::Rebase(0x1230EC0)), this, effect, i);
        }
        else if (type == 0x13)
        {
            const int32 val = reinterpret_cast<int32(__thiscall*)(void*)>(vtbl[0x5C / 4])(effect);
            Core::Hook::Call<void>(0x972AE0, this, val, arg);
        }
    }
}

void __thiscall CEntity::Create2dEffectAudio()
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
    if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(modelInfo) + 0x5B) & 0x20u) == 0)
        return;

    int32 count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, modelInfo, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        if (reinterpret_cast<uint8(__thiscall*)(void*)>(vtbl[1])(effect) == 1)
        {
            void* attr = reinterpret_cast<void*(__thiscall*)(void*)>(vtbl[7])(effect);
            if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(attr) + 0x24) == 5)
            {
                void* attr2 = reinterpret_cast<void*(__thiscall*)(void*)>(vtbl[7])(effect);
                Core::Hook::CallMethod<void>(0xC86E40, reinterpret_cast<void*>(Core::Rebase(0x170F670)), this, attr2, i);
            }
        }
        count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    }
}

void __thiscall CEntity::Destroy2dEffectAudio()
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
    if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(modelInfo) + 0x5B) & 0x20u) == 0)
        return;

    int32 count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, modelInfo, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        if (reinterpret_cast<uint8(__thiscall*)(void*)>(vtbl[1])(effect) == 1)
        {
            void* attr = reinterpret_cast<void*(__thiscall*)(void*)>(vtbl[7])(effect);
            if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(attr) + 0x24) == 5)
            {
                void* attr2 = reinterpret_cast<void*(__thiscall*)(void*)>(vtbl[7])(effect);
                Core::Hook::CallMethod<void>(0xC87240, reinterpret_cast<void*>(Core::Rebase(0x170F670)), this, attr2);
            }
        }
        count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    }
}

void __thiscall CEntity::Process2dEffectsPtfx()
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* modelInfo = GetModelInfoPtr(m_nModelIndex);
    if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(modelInfo) + 0x5B) & 0x1Cu) == 0)
        return;

    int32 count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, modelInfo, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        if (reinterpret_cast<uint8(__thiscall*)(void*)>(vtbl[1])(effect) == 2)
        {
            void* data = reinterpret_cast<void*(__thiscall*)(void*)>(vtbl[9])(effect);
            const int32 boneId = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(data) + 0x28);

            if (boneId >= 0)
            {
                uint8 flag = 1;
                const CVector* boneVec = Core::Hook::Call<const CVector*>(0xB1E560, this, boneId, 1, &flag);
                if (!boneVec)
                    goto next;
                if (boneVec->x == 0.0f && boneVec->y == 0.0f && boneVec->z == 0.0f)
                    goto next;
            }

            const int32 ptfxId = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(data) + 0x20);
            if (m_nFlags2 & 0x100000u)
            {
                const uint8* table = reinterpret_cast<const uint8*>(Core::Rebase(0x15C07C8));
                if (table[ptfxId * 0x70] == 0)
                    goto next;
            }

            {
                CVector vec(0.0f, 0.0f, *reinterpret_cast<const float*>(Core::Rebase(0xD50398)));
                int32 unk27C = 0;
                if ((m_nFlags2 & 0x3C0u) == 0x100u)
                {
                    unk27C = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(this) + 0x27C);
                    if (!unk27C)
                    {
                        CEntity* attachParent = *reinterpret_cast<CEntity* const*>(reinterpret_cast<const uint8*>(this) + 0x280);
                        if (attachParent && (attachParent->m_nFlags2 & 0x3C0u) == 0x100u)
                            unk27C = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(attachParent) + 0x27C);
                    }
                }

                const CVector* pos = &GetPosition();
                const float scale = *reinterpret_cast<const float*>(Core::Rebase(0xEBDED4));
                Core::Hook::Call<void>(
                    0x9609F0,
                    0, 0, ptfxId, 1.0f, pos, 0, 0, 1, scale, 0, 0,
                    &vec, this, 0, 0, (unk27C == 0) ? 1 : 0, unk27C, 0, -1);

                if (!Core::Hook::Call<bool>(0x462F90) || Core::Hook::Call<bool>(0x463000))
                {
                    m_nFlags2 |= 0x100000u;
                    if ((m_nFlags2 & 0x3C0u) == 0x100u)
                    {
                        CEntity* attachParent = *reinterpret_cast<CEntity* const*>(reinterpret_cast<const uint8*>(this) + 0x280);
                        if (attachParent)
                            attachParent->m_nFlags2 |= 0x100000u;
                    }
                }
                break;
            }
        }
    next:
        count = Core::Hook::CallMethod<int32>(0x98EA60, modelInfo);
    }
}

const CVector* __cdecl CEntity::TransformPoint2D(CVector* out, const float* posHeading, const CVector* offset)
{
    const double heading = static_cast<double>(posHeading[3]);
    const float c = static_cast<float>(std::cos(heading));
    const float s = static_cast<float>(std::sin(heading));

    out->x = offset->x * c - offset->y * s + posHeading[0];
    out->y = offset->y * c + offset->x * s + posHeading[1];
    out->z = posHeading[2] + offset->z;
    return offset;
}

bool __thiscall CEntity::IsInArea2D(float x1, float y1, float x2, float y2) const
{
    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const float minX = (x1 <= x2) ? x1 : x2;
    const float maxX = (x1 <= x2) ? x2 : x1;
    const float minY = (y1 <= y2) ? y1 : y2;
    const float maxY = (y1 <= y2) ? y2 : y1;

    return pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY;
}

bool __thiscall CEntity::IsInArea3D(float x1, float y1, float z1, float x2, float y2, float z2) const
{
    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const float minX = (x1 <= x2) ? x1 : x2;
    const float maxX = (x1 <= x2) ? x2 : x1;
    const float minY = (y1 <= y2) ? y1 : y2;
    const float maxY = (y1 <= y2) ? y2 : y1;
    const float minZ = (z1 <= z2) ? z1 : z2;
    const float maxZ = (z1 <= z2) ? z2 : z1;

    return pos.x >= minX && pos.x <= maxX
        && pos.y >= minY && pos.y <= maxY
        && pos.z >= minZ && pos.z <= maxZ;
}

void* __thiscall CEntity::GetTypeSpecificPtr()
{
    const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;
    if (entityType == 3)
        return Core::Hook::CallMethod<void*>(0xBE61F0, reinterpret_cast<uint8*>(this) + 0x3D0);
    if (entityType == 2)
        return *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0xC80);
    return nullptr;
}

bool __thiscall CEntity::CheckAndClearLodFlag()
{
    CEntity* lod = static_cast<CEntity*>(m_pLod);
    CEntity* superLod = lod ? static_cast<CEntity*>(lod->m_pLod) : nullptr;

    if (Core::StaticRef<int32>(0x16B7760) == 0)
        return false;

    CEntity* target = nullptr;
    if (lod && (lod->m_nFlags & 0x80u) != 0 && ((lod->m_nFlags2 >> 25) & 1u) == 0)
    {
        target = lod;
    }
    else if (superLod && (superLod->m_nFlags & 0x80u) != 0 && ((superLod->m_nFlags2 >> 25) & 1u) == 0)
    {
        target = superLod;
    }
    else
    {
        return false;
    }

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[target->m_nModelIndex];
    if ((mi->m_nIDEFlags & 8u) == 0)
        return false;

    target->m_nUnk5C &= 0xDFFFu;
    return static_cast<uint8>(target->m_nUnk63) >= 0xEFu;
}

void __thiscall CEntity::SetFlag5EBit1(bool enable)
{
    if (enable)
    {
        if ((m_nUnk5E & 2u) == 0)
        {
            m_nUnk5E |= 2u;
            Core::Hook::Call<void>(0x7DB880, this);
        }
    }
    else
    {
        if ((m_nUnk5E & 2u) != 0)
        {
            m_nUnk5E &= 0xFFFDu;
            Core::Hook::Call<void>(0x7DFEA0, this);
        }
    }
}

void __thiscall CEntity::SetModelIndex(int32 modelIndex)
{
    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[modelIndex];
    m_nModelIndex = static_cast<int16>(modelIndex);

    if ((m_nFlags2 & 0x10u) == 0)
    {
        const uint8 res = Core::Hook::CallMethod<uint8>(0x9E6B10, this);
        m_nFlags2 = (m_nFlags2 & ~0x10u) | ((static_cast<uint32>(res) << 4) & 0x10u);
    }

    if ((mi->m_nIDEFlags & 0x2000u) != 0)
        m_nFlags2 |= 0x4000000u;

    if (Core::Hook::CallMethod<int32>(0x98EA30, mi) > 0
        || (reinterpret_cast<uint8*>(mi)[0x5B] & 0x40u) != 0
        || Core::Hook::CallMethod<int32>(0x98ED70, mi, 0x12) > 0)
    {
        m_nFlags |= 0x800000u;
    }

    if (Core::StaticRef<float>(0xDCE03C) > mi->m_fBoundRadius || (m_nFlags2 & 0x3C0u) == 0x40u)
        m_nFlags |= 0x40000000u;

    if (mi->GetModelType() == 2 || modelIndex == Core::StaticRef<int32>(0x1690BA8))
        m_nFlags2 |= 0x8000u;
    else
        m_nFlags2 &= ~0x8000u;

    *reinterpret_cast<float*>(&m_nUnk50) = mi->m_fDrawDistance;

    if (*reinterpret_cast<int16*>(reinterpret_cast<uint8*>(mi) + 0x52) != -1)
        m_nFlags2 |= 0x8000000u;
}

bool __thiscall CEntity::CreateDrawHandler()
{
    if ((m_nFlags & 0x20u) == 0 || m_pDrawHandler != nullptr)
        return false;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    void* drawable = nullptr;
    if (mi->m_pFragType)
    {
        drawable = *reinterpret_cast<void**>(static_cast<uint8*>(mi->m_pFragType) + 0xB4);
        m_nFlags |= 0x4000000u;
    }
    else if (mi->m_pDrawableStruct)
    {
        drawable = *reinterpret_cast<void**>(mi->m_pDrawableStruct);
    }

    if (!drawable)
        return false;

    m_pDrawHandler = Core::Hook::CallMethod<void*>(
        0xA4C020, reinterpret_cast<void*>(Core::Rebase(0x1660CB8)), this, drawable);
    if (!m_pDrawHandler)
        return false;

    Core::Hook::CallMethod<void>(0x98E460, mi);

    const uint8 modelType = mi->GetModelType();
    if (modelType == 5)
        ++(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(mi) + 0xB0));
    else if (mi->GetModelType() == 6)
        ++(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(mi) + 0xAC));

    if ((mi->m_nIDEFlags & 0x1000u) != 0)
        m_nFlags |= 0x10000u;
    if ((mi->m_nIDEFlags & 0x10u) != 0)
        m_nFlags |= 0x20000u;

    int32 drawFlags = -1;
    if (mi->m_pFragType)
    {
        drawFlags = Core::Hook::CallMethod<int32>(0x521220, mi->m_pFragType);
    }
    else
    {
        void** vtbl = *reinterpret_cast<void***>(drawable);
        using GetFlagsFn = int32(__thiscall*)(void*, int32);
        drawFlags = reinterpret_cast<GetFlagsFn>(vtbl[0x40 / 4])(drawable, 0);
    }

    if (drawFlags != -1)
    {
        if (drawFlags & 0x02) m_nFlags |= 0x1000u;
        if (drawFlags & 0x04) m_nFlags |= 0x2000u;
        if (drawFlags & 0x08) m_nFlags |= 0x4000u;
        if (drawFlags & 0x10) m_nFlags |= 0x8000u;
        if (drawFlags & 0x20) m_nFlags |= 0x8000u;
    }

    Core::Hook::CallMethod<void>(0xA4BCA0, m_pDrawHandler, mi, this);

    if ((m_nFlags2 & 0x10u) == 0)
    {
        const uint8 res = Core::Hook::CallMethod<uint8>(0x9E6B10, this);
        m_nFlags2 = (m_nFlags2 & ~0x10u) | ((static_cast<uint32>(res) << 4) & 0x10u);
    }

    if ((mi->m_nIDEFlags & 0x800u) != 0 && static_cast<uint8>(m_nUnk58 >> 24) == 0xFFu)
    {
        const CVector* pos = m_pMatrix ? &m_pMatrix->pos : &m_vecPos;
        const float alphaF = Core::Hook::Call<float>(
            0x888B00, pos, m_nInteriorHash, static_cast<int32>(m_nInteriorIndex));
        uint8 alphaByte = 0xFEu;
        if (alphaF < 1.0f)
            alphaByte = static_cast<uint8>(static_cast<int32>(alphaF * Core::StaticRef<float>(0xEBE020)));
        m_nUnk58 = (m_nUnk58 & 0x00FFFFFFu) | (static_cast<uint32>(alphaByte) << 24);
    }

    if (Core::Hook::CallMethod<int32>(0x98EA30, mi) > 0
        || (reinterpret_cast<uint8*>(mi)[0x5B] & 0x40u) != 0
        || Core::Hook::CallMethod<int32>(0x98ED70, mi, 0x12) > 0)
    {
        m_nFlags |= 0x800000u;
    }

    if ((m_nFlags & 0x10000u) != 0 && (m_nFlags & 0x80u) == 0 && (m_nFlags & 0x8000u) == 0)
        m_nFlags &= ~0x10000u;

    return true;
}

float* __thiscall CEntity::GetBoundRect(float* outRect) const
{
    const CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    const CVector& minB = mi->m_vBoundCentre;
    const CVector& maxB = mi->m_vMaxBounds;

    const float initMax = Core::StaticRef<float>(0xEBE030);
    const float initMin = Core::StaticRef<float>(0xEBE02C);
    outRect[0] = initMax;
    outRect[3] = initMax;
    outRect[2] = initMin;
    outRect[1] = initMin;

    const CVector corners[4] = {
        {minB.x, minB.y, minB.z},
        {maxB.x, maxB.y, maxB.z},
        {maxB.x, minB.y, minB.z},
        {minB.x, maxB.y, maxB.z},
    };

    for (const CVector& corner : corners)
    {
        float wx = 0.0f;
        float wy = 0.0f;
        if (m_pMatrix)
        {
            wx = m_pMatrix->right.x * corner.x + m_pMatrix->up.x * corner.y + m_pMatrix->at.x * corner.z + m_pMatrix->pos.x;
            wy = m_pMatrix->right.y * corner.x + m_pMatrix->up.y * corner.y + m_pMatrix->at.y * corner.z + m_pMatrix->pos.y;
        }
        else
        {
            CVector tmp{};
            TransformPoint2D(&tmp, &m_vecPos.x, &corner);
            wx = tmp.x;
            wy = tmp.y;
        }

        if (outRect[0] > wx) outRect[0] = wx;
        if (wx > outRect[2]) outRect[2] = wx;
        if (outRect[3] > wy) outRect[3] = wy;
        if (wy > outRect[1]) outRect[1] = wy;
    }

    return outRect;
}

float* __thiscall CEntity::GetBoundCentreAndRadius(float* out) const
{
    CalcBoundCentreAndRadius(out);
    return out;
}

float* __thiscall CEntity::CalcBoundCentreAndRadius(float* out) const
{
    const CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    const float* src = reinterpret_cast<const float*>(mi) + 4; // +0x10..+0x1C
    const CVector localPos(src[0], src[1], src[2]);

    if (m_pMatrix)
    {
        out[0] = m_pMatrix->right.x * localPos.x + m_pMatrix->up.x * localPos.y + m_pMatrix->at.x * localPos.z + m_pMatrix->pos.x;
        out[1] = m_pMatrix->right.y * localPos.x + m_pMatrix->up.y * localPos.y + m_pMatrix->at.y * localPos.z + m_pMatrix->pos.y;
        out[2] = m_pMatrix->right.z * localPos.x + m_pMatrix->up.z * localPos.y + m_pMatrix->at.z * localPos.z + m_pMatrix->pos.z;
        out[3] = src[3];
    }
    else
    {
        TransformPoint2D(reinterpret_cast<CVector*>(out), &m_vecPos.x, &localPos);
    }
    return out;
}

double __thiscall CEntity::GetBoundSphere(float* out) const
{
    CalcBoundCentreAndRadius(out);
    return static_cast<double>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]->m_fBoundRadius);
}

bool __thiscall CEntity::CheckDrawableAndUpdate()
{
    if ((m_pDrawHandler != nullptr || (m_nFlags & 0x4000000u) != 0) && (m_nFlags & 0x20u) != 0)
        return Core::Hook::CallMethod<bool>(0x9E6BD0, this, 0);
    return false;
}

bool __thiscall CEntity::IsInFrustum(int32 index)
{
    using GetRadiusFn = double(__thiscall*)(CEntity*);
    const float radius = static_cast<float>(reinterpret_cast<GetRadiusFn>(m_vftable[0x58 / 4])(this));

    struct { CVector vec; float w; } centre{};
    using GetCentreFn = void(__thiscall*)(CEntity*, float*);
    reinterpret_cast<GetCentreFn>(m_vftable[0x50 / 4])(this, &centre.vec.x);

    void* viewport = Core::StaticRef<void**>(0x10F47E0)[index];
    return Core::Hook::CallMethod<int32>(
        0x62EF60, reinterpret_cast<uint8*>(viewport) + 0x10, centre.vec, radius, 0) != 0;
}

bool __thiscall CEntity::CanAddToInterior() const
{
    if ((m_nFlags2 & 0x3C0u) == 0x140u && (m_nUnk5C & 0x1FFFu) > 0)
        return false;

    if (m_nUnk41 != 5 && m_nUnk41 != 6 && m_nUnk41 != 7)
        return false;

    if (m_nInteriorIndex < 0 && m_nUnk44 <= -1)
        return false;

    return Core::Hook::CallMethod<void*>(0xA4C560, CPools::ms_pInteriorInstPool, m_nInteriorHash) != nullptr;
}

void* __thiscall CEntity::GetPhArchetype() const
{
    if (m_pPhysicsInst)
    {
        void* arch = *reinterpret_cast<void**>(static_cast<uint8*>(m_pPhysicsInst) + 4);
        if (arch)
            return arch;
    }
    return CModelInfo::ms_modelInfoPtrs[m_nModelIndex]->m_pArchetype;
}

void __thiscall CEntity::RemoveFromPhysicsWorld()
{
    if (m_pPhysicsInst && *reinterpret_cast<void**>(static_cast<uint8*>(m_pPhysicsInst) + 4) != nullptr)
    {
        using Fn = void(__thiscall*)(CEntity*, int32);
        reinterpret_cast<Fn>(m_vftable[0xB4 / 4])(this, 1);
        Core::Hook::CallMethod<void>(0x69A2C0, Core::StaticRef<void*>(0x1666884), m_pPhysicsInst, 0);
    }
}

void __thiscall CEntity::InitModelEffects()
{
    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        using GetTypeFn = uint8(__thiscall*)(void*);
        const uint8 type = reinterpret_cast<GetTypeFn>(vtbl[1])(effect);
        if (type == 0 || type == 1 || type == 18)
        {
            m_nFlags2 |= 0x10u;
        }
        else if (type == 10)
        {
            using InitFn = void(__thiscall*)(void*);
            reinterpret_cast<InitFn>(vtbl[11])(effect);
        }
    }
}

void __thiscall CEntity::TriggerModelEffectsType1(int32 arg0, int32 arg1, int32 boneIndex, float arg3)
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((reinterpret_cast<uint8*>(mi)[0x5B] & 2u) == 0)
        return;

    using GetAnimFn = void*(__thiscall*)(CEntity*);
    void* anim = reinterpret_cast<GetAnimFn>(m_vftable[0xA0 / 4])(this);
    bool checkZero = false;
    uint32 boneTag = 0;

    if (anim)
    {
        void* skelOwner = reinterpret_cast<GetAnimFn>(m_vftable[0xA0 / 4])(this);
        void* crSkel = Core::Hook::CallMethod<void*>(0x51DDD0, skelOwner);
        void** boneArray = *reinterpret_cast<void***>(static_cast<uint8*>(crSkel) + 0xD4);
        void* bone = boneArray[boneIndex];
        if (!bone)
            return;

        void* animData = *reinterpret_cast<void**>(static_cast<uint8*>(anim) + 0x64);
        if (animData)
        {
            const uint8 boneIdx = *reinterpret_cast<uint8*>(static_cast<uint8*>(bone) + 0x0C);
            const float* weights = *reinterpret_cast<float**>(static_cast<uint8*>(animData) + 0x104);
            if (!(0.0f < weights[boneIdx]))
                checkZero = true;
        }

        const int32 parentIdx = *reinterpret_cast<int16*>(static_cast<uint8*>(bone) + 0x0E);
        if (parentIdx == -1)
            return;

        void* boneMap = *reinterpret_cast<void**>(static_cast<uint8*>(crSkel) + 0x1E0);
        if (!boneMap)
            return;

        uint8* basePtr = **reinterpret_cast<uint8***>(static_cast<uint8*>(boneMap) + 4);
        uint8* boneInfo = basePtr + parentIdx * 0xE0;
        if (!boneInfo)
            return;

        boneTag = *reinterpret_cast<uint16*>(boneInfo + 0x16);
    }

    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        void** vtbl = *reinterpret_cast<void***>(effect);
        using GetTypeFn = uint8(__thiscall*)(void*);
        if (reinterpret_cast<GetTypeFn>(vtbl[1])(effect) != 1)
            continue;

        using GetDataFn = uint8*(__thiscall*)(void*);
        uint8* data = reinterpret_cast<GetDataFn>(vtbl[7])(effect);
        if (checkZero && data[0x38] != 0)
            continue;
        if (*reinterpret_cast<int32*>(data + 0x24) != 1)
            continue;

        const uint32 effBone = *reinterpret_cast<uint32*>(data + 0x28);
        if (effBone != boneTag && effBone != 0xFFFFFFFFu)
            continue;

        Core::Hook::CallMethod<void>(
            0xC85750, reinterpret_cast<void*>(Core::Rebase(0x170F670)),
            this, data, arg0, arg1, boneTag, arg3);
    }
}

void __thiscall CEntity::TriggerModelEffectsType2(int32 arg0, int32 arg1, int32 boneIndex)
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((reinterpret_cast<uint8*>(mi)[0x5B] & 4u) == 0)
        return;

    using GetAnimFn = void*(__thiscall*)(CEntity*);
    void* anim = reinterpret_cast<GetAnimFn>(m_vftable[0xA0 / 4])(this);
    bool checkZero = false;
    uint32 boneTag = 0;

    if (anim)
    {
        void* crSkel = Core::Hook::CallMethod<void*>(0x51DDD0, anim);
        void** boneArray = *reinterpret_cast<void***>(static_cast<uint8*>(crSkel) + 0xD4);
        void* bone = boneArray[boneIndex];
        if (!bone)
            return;

        void* animData = *reinterpret_cast<void**>(static_cast<uint8*>(anim) + 0x64);
        if (animData)
        {
            const uint8 boneIdx = *reinterpret_cast<uint8*>(static_cast<uint8*>(bone) + 0x0C);
            const float* weights = *reinterpret_cast<float**>(static_cast<uint8*>(animData) + 0x104);
            if (!(0.0f < weights[boneIdx]))
                checkZero = true;
        }

        const int32 parentIdx = *reinterpret_cast<int16*>(static_cast<uint8*>(bone) + 0x0E);
        if (parentIdx == -1)
            return;

        void* boneMap = *reinterpret_cast<void**>(static_cast<uint8*>(crSkel) + 0x1E0);
        if (!boneMap)
            return;

        uint8* basePtr = **reinterpret_cast<uint8***>(static_cast<uint8*>(boneMap) + 4);
        uint8* boneInfo = basePtr + parentIdx * 0xE0;
        if (!boneInfo)
            return;

        boneTag = *reinterpret_cast<uint16*>(boneInfo + 0x16);
    }

    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        const CVector offset = *reinterpret_cast<const CVector*>(static_cast<const uint8*>(effect) + 4);
        void** vtbl = *reinterpret_cast<void***>(effect);
        using GetTypeFn = uint8(__thiscall*)(void*);
        const uint8 effectType = reinterpret_cast<GetTypeFn>(vtbl[1])(effect);

        if (effectType == 1)
        {
            using GetDataFn = uint8*(__thiscall*)(void*);
            uint8* data = reinterpret_cast<GetDataFn>(vtbl[7])(effect);
            if (checkZero && data[0x38] != 0)
                continue;
            const uint32 effBone = *reinterpret_cast<uint32*>(data + 0x28);
            if (effBone != boneTag && effBone != 0xFFFFFFFFu)
                continue;
            if (*reinterpret_cast<int32*>(data + 0x24) != 2)
                continue;

            Core::Hook::CallMethod<void>(
                0xC85E50, reinterpret_cast<void*>(Core::Rebase(0x170F670)),
                this, data, arg0, arg1, boneTag);
        }
        else if (effectType == 2)
        {
            using GetDataFn = uint8*(__thiscall*)(void*);
            uint8* data = reinterpret_cast<GetDataFn>(vtbl[9])(effect);
            if (checkZero && data[0x2C] != 0)
                continue;
            const uint32 effBone = *reinterpret_cast<uint32*>(data + 0x28);
            if (effBone != boneTag && effBone != 0xFFFFFFFFu)
                continue;

            const int32 fxType = *reinterpret_cast<int32*>(data + 0x24);
            const int32 fxId = *reinterpret_cast<int32*>(data + 0x20);

            auto getOwner = [this]() -> void* {
                if ((m_nFlags2 & 0x3C0u) != 0x100u)
                    return nullptr;
                void* owner = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x27C);
                if (!owner)
                {
                    CEntity* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                    if (attach && (attach->m_nFlags2 & 0x3C0u) == 0x100u)
                        owner = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(attach) + 0x27C);
                }
                return owner;
            };

            auto markFxFlag = [this]() {
                if (!Core::Hook::Call<bool>(0x462F90) || Core::Hook::Call<bool>(0x463000))
                {
                    m_nFlags2 |= 0x100000u;
                    if ((m_nFlags2 & 0x3C0u) == 0x100u)
                    {
                        CEntity* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                        if (attach)
                            attach->m_nFlags2 |= 0x100000u;
                    }
                }
            };

            if (fxType == 0)
            {
                if ((m_nFlags2 & 0x100000u) != 0 && Core::StaticRef<uint8>(0x15C07C8 + fxId * 0x70) == 0)
                    continue;

                void* owner = getOwner();
                Core::Hook::Call<void>(
                    0x9609F0,
                    0, 0, fxId, 1.0f, arg0, 0, 0, 1,
                    Core::StaticRef<float>(0xEBDED4), 0, 0, arg1,
                    this, boneTag, 0, owner == nullptr ? 1 : 0, owner, 0, -1);
                markFxFlag();
            }
            else if (fxType == 3)
            {
                if ((m_nFlags2 & 0x100000u) != 0 && Core::StaticRef<uint8>(0x15C07C8 + fxId * 0x70) == 0)
                    continue;

                uint8 localBool = 1;
                void* fxInst = Core::Hook::Call<void*>(0xB1E560, this, boneTag, 1, &localBool);
                if (!fxInst || Core::Hook::CallMethod<bool>(0x4967C0, fxInst))
                    continue;

                float quat[4] = {
                    *reinterpret_cast<float*>(data + 0x10),
                    *reinterpret_cast<float*>(data + 0x14),
                    *reinterpret_cast<float*>(data + 0x18),
                    *reinterpret_cast<float*>(data + 0x1C),
                };
                CMatrix localMat{};
                Core::Hook::CallMethod<void>(0x5BE050, &localMat, quat);
                localMat.pos = offset;
                localMat.pad3 = 0.0f;
                Core::Hook::CallMethod<void>(0x513430, &localMat, fxInst);

                void* owner = getOwner();
                Core::Hook::Call<void>(
                    0x9609F0,
                    0, 0, fxId, 1.0f, &localMat.pos, 0, 0, 1,
                    Core::StaticRef<float>(0xEBDED4), 0, 0,
                    reinterpret_cast<uint8*>(&localMat) + 0x18,
                    this, boneTag, 0, owner == nullptr ? 1 : 0, owner, 0, -1);
                markFxFlag();
            }
        }
    }
}

void __thiscall CEntity::TriggerModelEffectsAtMatrix(int32 boneTag, const CMatrix* mat)
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((reinterpret_cast<uint8*>(mi)[0x5B] & 0x10u) == 0)
        return;

    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* effect = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        const CVector offset = *reinterpret_cast<const CVector*>(static_cast<const uint8*>(effect) + 4);
        void** vtbl = *reinterpret_cast<void***>(effect);
        using GetTypeFn = uint8(__thiscall*)(void*);
        const uint8 effectType = reinterpret_cast<GetTypeFn>(vtbl[1])(effect);

        if (effectType == 1)
        {
            using GetDataFn = uint8*(__thiscall*)(void*);
            uint8* data = reinterpret_cast<GetDataFn>(vtbl[7])(effect);
            if (*reinterpret_cast<int32*>(data + 0x24) != 4)
                continue;
            const int32 effBone = *reinterpret_cast<int32*>(data + 0x28);
            if (effBone != boneTag && effBone != -1)
                continue;

            Core::Hook::CallMethod<void>(
                0xC86A60, reinterpret_cast<void*>(Core::Rebase(0x170F670)),
                this, data, mat);
        }
        else if (effectType == 2)
        {
            using GetDataFn = uint8*(__thiscall*)(void*);
            uint8* data = reinterpret_cast<GetDataFn>(vtbl[9])(effect);
            if (*reinterpret_cast<int32*>(data + 0x24) != 2)
                continue;
            const int32 effBone = *reinterpret_cast<int32*>(data + 0x28);
            if (effBone != boneTag && effBone != -1)
                continue;

            const int32 fxId = *reinterpret_cast<int32*>(data + 0x20);
            const bool hasFxFlag = (m_nFlags2 & 0x100000u) != 0;
            const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;

            if (Core::StaticRef<uint8>(0x15C07C8 + fxId * 0x70) == 0)
            {
                if ((m_nFlags2 & 0x3C0u) == 0x100u)
                {
                    CEntity* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                    if (attach && (attach->m_nFlags2 & 0x100000u) != 0)
                        continue;
                }
                if (hasFxFlag)
                    continue;
            }

            CVector worldPos{};
            worldPos.x = mat->right.x * offset.x + mat->up.x * offset.y + mat->at.x * offset.z + mat->pos.x;
            worldPos.y = mat->right.y * offset.x + mat->up.y * offset.y + mat->at.y * offset.z + mat->pos.y;
            worldPos.z = mat->right.z * offset.x + mat->up.z * offset.y + mat->at.z * offset.z + mat->pos.z;

            void* owner = nullptr;
            if (entityType == 4)
            {
                owner = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x27C);
                if (!owner)
                {
                    CEntity* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                    if (attach && (attach->m_nFlags2 & 0x3C0u) == 0x100u)
                        owner = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(attach) + 0x27C);
                }
            }

            int32 extraArg = 0;
            if (entityType == 4 || entityType == 2 || entityType == 3)
            {
                if (*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x1E8) == Core::StaticRef<uint32>(0x11DDE74))
                    extraArg = *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0x1E4);
            }

            Core::Hook::Call<void>(
                0x9609F0,
                0, extraArg, fxId, 1.0f, &worldPos, 0, 0, 1,
                Core::StaticRef<float>(0xEBDED4), 0, 0, &mat->at,
                this, boneTag, 0, owner == nullptr ? 1 : 0, owner, 0, -1);

            if (!Core::Hook::Call<bool>(0x462F90) || Core::Hook::Call<bool>(0x463000))
            {
                m_nFlags2 |= 0x100000u;
                if ((m_nFlags2 & 0x3C0u) == 0x100u)
                {
                    CEntity* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                    if (attach)
                        attach->m_nFlags2 |= 0x100000u;
                }
            }
        }
    }
}

void __thiscall CEntity::RemoveMatrix()
{
    CMatrix* mat = m_pMatrix;
    auto** ownerPtr = reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(mat) + 0x40);
    if (!*ownerPtr)
    {
        m_pMatrix = nullptr;
        return;
    }

    m_vecPos = mat->pos;
    m_fPad1C = static_cast<float>(std::atan2(-mat->up.x, mat->up.y));
    m_pMatrix = nullptr;
    *ownerPtr = nullptr;

    Core::Hook::CallMethod<void>(0x9E77D0, reinterpret_cast<void*>(Core::Rebase(0x16395C0)), mat);
}

float* __thiscall CEntity::GetWorldBoundBox(float* outBox)
{
    if (m_nModelIndex == -1)
    {
        outBox[0] = 0.0f;
        outBox[1] = 0.0f;
        outBox[2] = 0.0f;
        outBox[4] = 0.0f;
        outBox[5] = 0.0f;
        outBox[6] = 0.0f;
        return outBox;
    }

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    const CVector& minB = mi->m_vBoundCentre;
    const CVector& maxB = mi->m_vMaxBounds;

    const CVector center(
        (minB.x + maxB.x) * Core::StaticRef<float>(0x108A350),
        (minB.y + maxB.y) * Core::StaticRef<float>(0x108A354),
        (minB.z + maxB.z) * Core::StaticRef<float>(0x108A358));
    const CVector halfExtents(
        (maxB.x - minB.x) * Core::StaticRef<float>(0x108A350),
        (maxB.y - minB.y) * Core::StaticRef<float>(0x108A354),
        (maxB.z - minB.z) * Core::StaticRef<float>(0x108A358));

    CMatrix tempMat{};
    const CMatrix* mat = m_pMatrix;
    if (!mat)
    {
        Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, &tempMat);
        mat = &tempMat;
    }

    const float wx = mat->right.x * center.x + mat->up.x * center.y + mat->at.x * center.z + mat->pos.x;
    const float wy = mat->right.y * center.x + mat->up.y * center.y + mat->at.y * center.z + mat->pos.y;
    const float wz = mat->right.z * center.x + mat->up.z * center.y + mat->at.z * center.z + mat->pos.z;

    const float ex = std::fabs(mat->right.x) * halfExtents.x + std::fabs(mat->up.x) * halfExtents.y + std::fabs(mat->at.x) * halfExtents.z;
    const float ey = std::fabs(mat->right.y) * halfExtents.x + std::fabs(mat->up.y) * halfExtents.y + std::fabs(mat->at.y) * halfExtents.z;
    const float ez = std::fabs(mat->right.z) * halfExtents.x + std::fabs(mat->up.z) * halfExtents.y + std::fabs(mat->at.z) * halfExtents.z;

    outBox[0] = wx - ex;
    outBox[1] = wy - ey;
    outBox[2] = wz - ez;
    outBox[4] = wx + ex;
    outBox[5] = wy + ey;
    outBox[6] = wz + ez;
    return outBox;
}

int32 __thiscall CEntity::UpdateRenderAlpha()
{
    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((m_nFlags2 & 0x10u) == 0)
        return 0;

    if (m_pDrawHandler)
    {
        const uint32 entityType = (m_nFlags2 >> 6) & 0xFu;
        if (entityType != 6
            && !Core::Hook::Call<bool>(0x7955D0)
            && (!Core::Hook::Call<bool>(0x405180) || entityType != 3))
        {
            void* sub = *reinterpret_cast<void**>(static_cast<uint8*>(m_pDrawHandler) + 4);
            if (sub)
            {
                void** vtbl = *reinterpret_cast<void***>(sub);
                using Fn = void(__thiscall*)(void*, CEntity*);
                reinterpret_cast<Fn>(vtbl[3])(sub, this);
            }
        }
    }

    if (Core::Hook::CallMethod<int32>(0x98EA60, mi) != 0)
    {
        const float oneOver255 = Core::StaticRef<float>(0xEBE134);
        float alpha = static_cast<float>(static_cast<uint8>(m_nUnk63)) * oneOver255;
        const bool bInInterior = ((m_nFlags >> 27) & 1u) != 0;

        if (bInInterior && m_nInteriorHash != -1)
        {
            void* interior = Core::Hook::CallMethod<void*>(0xA4C560, CPools::ms_pInteriorInstPool, m_nInteriorHash);
            if (interior)
            {
                auto* portalEnt = *reinterpret_cast<CEntity**>(static_cast<uint8*>(interior) + 0xF4);
                if (portalEnt)
                {
                    alpha = static_cast<float>(static_cast<uint8>(portalEnt->m_nUnk63)) * oneOver255;
                    const float blend = *reinterpret_cast<float*>(static_cast<uint8*>(interior) + 0x10C);
                    if (this == portalEnt)
                    {
                        alpha *= blend;
                    }
                    else
                    {
                        auto* secondaryEnt = *reinterpret_cast<CEntity**>(static_cast<uint8*>(interior) + 0xF8);
                        if (this == secondaryEnt)
                            alpha = static_cast<float>(static_cast<uint8>(secondaryEnt->m_nUnk63)) * oneOver255;
                        else
                            alpha = Core::StaticRef<float>(0xD50398) - blend * alpha;
                    }
                }
            }
        }
        else if (!bInInterior && m_pLod)
        {
            auto* lod = static_cast<CEntity*>(m_pLod);
            if ((lod->m_nFlags & 0x800000u) != 0
                && ((lod->m_nFlags2 & 0x80000u) != 0 || (lod->m_nUnk5C & 0x2000u) != 0)
                && Core::StaticRef<uint8>(0xF346C5) == 1)
            {
                const float lodAlpha = static_cast<float>(0xFF - static_cast<uint8>(lod->m_nUnk63)) * oneOver255;
                if (alpha > lodAlpha)
                    alpha = lodAlpha;
            }
        }

        if ((m_nFlags & 0x800000u) != 0 && (bInInterior || !Core::Hook::Call<bool>(0xB37BA0)))
        {
            const uint32 modelUnk5C = *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(mi) + 0x5C);
            const float limit = static_cast<float>(modelUnk5C) * Core::StaticRef<float>(0xD83C78);
            const float minAlpha = (limit <= alpha) ? limit : alpha;
            Core::Hook::CallMethod<void>(0x7E2600, this, minAlpha);
        }

        Core::Hook::CallMethod<void>(0x9E71B0, this, alpha);
    }

    CBaseModelInfo* mi2 = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((mi2->m_nIDEFlags & 0xFF000000u) == 0x03000000u)
        Core::Hook::Call<void>(0x88CCD0, this);

    return 0;
}

void __thiscall CEntity::SetInteriorByName(void* interiorNameOrPtr, int32 interiorIndex, int32 arg3)
{
    if (!interiorNameOrPtr)
    {
        m_nUnk41 = 3;
        return;
    }

    const int32 handle = Core::Hook::CallMethod<int32>(
        0x88F5D0, CPools::ms_pInteriorInstPool, interiorNameOrPtr);
    Core::Hook::CallMethod<void>(0x9E6CF0, this, handle, interiorIndex, arg3);
}

void __thiscall CEntity::AddToGrid(const float* boundRect)
{
    const float worldMin = Core::StaticRef<float>(0xEBE168);
    const float worldMax1 = Core::StaticRef<float>(0xEBE164);
    const float worldMax2 = Core::StaticRef<float>(0xD83C7C);

    float minX = (worldMin > boundRect[0]) ? worldMin : boundRect[0];
    float minY = (worldMin > boundRect[3]) ? worldMin : boundRect[3];
    float maxX = (boundRect[2] >= worldMax1) ? worldMax2 : boundRect[2];
    float maxY = (boundRect[1] >= worldMax1) ? worldMax2 : boundRect[1];

    if ((m_nFlags & 0x40u) != 0)
    {
        const float scale = Core::StaticRef<float>(0xEBE284);
        const float offset = Core::StaticRef<float>(0xD50960);

        const int32 x0 = static_cast<int32>(minX * scale + offset);
        const int32 x1 = static_cast<int32>(maxX * scale + offset);
        const int32 y0 = static_cast<int32>(minY * scale + offset);
        const int32 y1 = static_cast<int32>(maxY * scale + offset);

        m_nUnk04 = PackGridBounds(x0, y0, x1, y1);
        if (y0 > y1)
            return;

        auto** rowBase = reinterpret_cast<PtrNodeSingle**>(Core::Rebase(0x11D2870)) + y0 * 30 + x0;
        for (int32 y = y0; y <= y1; ++y)
        {
            for (int32 x = x0; x <= x1; ++x)
            {
                auto* node = static_cast<PtrNodeSingle*>(
                    Core::Hook::CallMethod<void*>(0x96D520, CPools::ms_pPtrNodeSinglePool));
                if (node)
                    node->entity = this;
                PtrNodeSingle** slot = &rowBase[x - x0];
                node->next = *slot;
                *slot = node;
            }
            rowBase += 30;
        }
        return;
    }

    const float scale = Core::StaticRef<float>(0xD529C4);
    const float offset = Core::StaticRef<float>(0xDCE040);

    const int32 x0 = static_cast<int32>(std::floor(minX * scale + offset));
    const int32 x1 = static_cast<int32>(std::floor(maxX * scale + offset));
    const int32 y0 = static_cast<int32>(std::floor(minY * scale + offset));
    const int32 y1 = static_cast<int32>(std::floor(maxY * scale + offset));

    m_nUnk04 = PackGridBounds(x0, y0, x1, y1);

    int32 zLevel = 0;
    if ((m_nFlags2 & 0x3C0u) == 0x40u)
    {
        CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
        const CVector& minB = mi->m_vBoundCentre;
        const CVector& maxB = mi->m_vMaxBounds;
        CMatrix mat{};
        Core::Hook::CallMethod<void>(0x5090F0, this, &mat);

        const float z1 = mat.right.z * minB.x + mat.up.z * minB.y + mat.at.z * minB.z + mat.pos.z;
        const float z2 = mat.right.z * maxB.x + mat.up.z * maxB.y + mat.at.z * maxB.z + mat.pos.z;
        zLevel = std::max(0, static_cast<int32>((z1 > z2) ? z1 : z2));
    }

    if (y0 > y1 || x0 > x1)
        return;

    const uint32 listType = (m_nFlags2 >> 6) & 0xFu;
    for (int32 y = y0; y <= y1; ++y)
    {
        for (int32 x = x0; x <= x1; ++x)
        {
            if (listType == 1 && (m_nFlags & 0x40u) == 0)
                Core::Hook::Call<void>(0x818110, x, y, zLevel, this);

            if (listType == 1)
            {
                auto** slot = reinterpret_cast<PtrNodeSingle**>(GetGridSectorPtr(1, x, y));
                auto* node = static_cast<PtrNodeSingle*>(
                    Core::Hook::CallMethod<void*>(0x96D520, CPools::ms_pPtrNodeSinglePool));
                if (node)
                    node->entity = this;
                node->next = *slot;
                *slot = node;
            }
            else if (listType >= 2 && listType <= 8)
            {
                auto** slot = reinterpret_cast<PtrNodeDouble**>(GetGridSectorPtr(listType, x, y));
                auto* node = static_cast<PtrNodeDouble*>(
                    Core::Hook::CallMethod<void*>(0x96D520, CPools::ms_pPtrNodeDoublePool));
                if (node)
                    node->entity = this;
                node->prev = nullptr;
                node->next = *slot;
                if (*slot)
                    (*slot)->prev = node;
                *slot = node;
            }
        }
    }
}

void __thiscall CEntity::AllocateMatrix()
{
    Core::Hook::Call<void>(0x524F30, reinterpret_cast<void*>(Core::Rebase(0x1639570)));
    void* list = reinterpret_cast<void*>(Core::Rebase(0x16395C0));

    if (m_pMatrix)
    {
        Core::Hook::CallMethod<void>(0x9E77B0, list, m_pMatrix);
    }
    else
    {
        void* mat = Core::Hook::CallMethod<void*>(0x9E7770, list);
        if (!mat)
        {
            void* tail = Core::StaticRef<void*>(0x1639654);
            auto* owner = *reinterpret_cast<CEntity**>(static_cast<uint8*>(tail) + 0x40);
            owner->RemoveMatrix();
            mat = Core::Hook::CallMethod<void*>(0x9E7770, list);
        }
        *reinterpret_cast<CEntity**>(static_cast<uint8*>(mat) + 0x40) = this;
        m_pMatrix = static_cast<CMatrix*>(mat);
    }

    Core::StaticRef<uint32>(0x1639570) = 0;
}

void __thiscall CEntity::AllocateStaticMatrix()
{
    Core::Hook::Call<void>(0x524F30, reinterpret_cast<void*>(Core::Rebase(0x1639570)));

    if (!m_pMatrix)
    {
        void* list = reinterpret_cast<void*>(Core::Rebase(0x16395C0));
        void* mat = Core::Hook::CallMethod<void*>(0x9E7740, list);
        if (!mat)
        {
            void* tail = Core::StaticRef<void*>(0x1639654);
            auto* owner = *reinterpret_cast<CEntity**>(static_cast<uint8*>(tail) + 0x40);
            owner->RemoveMatrix();
            mat = Core::Hook::CallMethod<void*>(0x9E7740, list);
        }
        *reinterpret_cast<CEntity**>(static_cast<uint8*>(mat) + 0x40) = this;
        m_pMatrix = static_cast<CMatrix*>(mat);
    }

    Core::StaticRef<uint32>(0x1639570) = 0;
}

void __thiscall CEntity::ReleaseMatrix()
{
    Core::Hook::Call<void>(0x524F30, reinterpret_cast<void*>(Core::Rebase(0x1639570)));
    RemoveMatrix();
    Core::StaticRef<uint32>(0x1639570) = 0;
}

void __thiscall CEntity::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD83BAC));

    if ((m_nFlags & 0x8000000u) != 0)
        RemoveFromInterior();

    if (m_pLod)
    {
        auto* lod = static_cast<CEntity*>(m_pLod);
        lod->m_nUnk61 = static_cast<uint8>(lod->m_nUnk61 - 1);
    }

    Core::Hook::CallMethod<void>(0x9E6A80, this);
    Core::Hook::CallMethod<void>(0x9E65D0, this);
    Core::Hook::CallMethod<void>(0x9E6FF0, this, 1);

    if (m_pMatrix)
        ReleaseMatrix();

    const bool bHasExtra = (m_nUnk5E & 2u) != 0;
    m_pMatrix = reinterpret_cast<CMatrix*>(Core::Rebase(0x1639580));
    if (bHasExtra)
        Core::Hook::CallMethod<void>(0x9E7B20, this, 0);
}

int32 __thiscall CEntity::Render(int32 arg0, int32 arg1, int32 arg2, int32 arg3)
{
    if (!m_pDrawHandler)
        return 0;

    void* handlerBase = *reinterpret_cast<void**>(m_pDrawHandler);
    if (handlerBase && (m_nFlags & 0x4000000u) == 0)
    {
        void* sub = *reinterpret_cast<void**>(static_cast<uint8*>(handlerBase) + 0x0C);
        if (sub && *reinterpret_cast<uint16*>(static_cast<uint8*>(sub) + 0x14) > 1)
            return 0;
    }

    Core::Hook::CallMethod<void>(0xA4BD30, m_pDrawHandler, arg1);

    if ((m_nFlags & 0x4000000u) != 0)
    {
        CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
        if (mi->GetModelType() == 6)
        {
            Core::Hook::CallMethod<void>(0xA4BD50, m_pDrawHandler, arg1);
            return 0;
        }

        void* renderObj = Core::Hook::Call<void*>(0x7BDD80, 0x60, 0);
        if (renderObj)
        {
            const uint8 alpha = static_cast<uint8>(m_nUnk58 >> 24);
            const int32 a = Core::Hook::CallMethod<int32>(0x447980, this);
            const int32 b = Core::Hook::CallMethod<int32>(0x4479A0, this);
            Core::Hook::CallMethod<void>(
                0x7BE560, renderObj, static_cast<int32>(m_nModelIndex), arg2, static_cast<uint32>(alpha), b, a, arg0, arg1);
        }
        Core::Hook::CallMethod<void>(0x404E90, renderObj);
    }
    else
    {
        if (!m_pMatrix)
        {
            AllocateStaticMatrix();
            Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
        }

        uint8 localMat[0x40]{};
        Core::Hook::CallMethod<void>(0x441ED0, localMat, m_pMatrix);

        void* renderObj = Core::Hook::Call<void*>(0x7BDD80, 0x40, 0);
        if (renderObj)
        {
            const uint8 alpha = static_cast<uint8>(m_nUnk58 >> 24);
            const int32 b = Core::Hook::CallMethod<int32>(0x4479A0, this);
            Core::Hook::CallMethod<void>(
                0x7BFAA0, renderObj, static_cast<int32>(m_nModelIndex), arg2, static_cast<uint32>(alpha), b, localMat, arg0, arg1);
        }
        Core::Hook::CallMethod<void>(0x404E90, renderObj);
    }

    Core::Hook::CallMethod<void>(0xA4BD50, m_pDrawHandler, arg1);
    return 0;
}

void __thiscall CEntity::RemoveFromGrid()
{
    if ((m_nFlags2 & 0x400000u) != 0)
    {
        if (!m_pMatrix)
        {
            AllocateStaticMatrix();
            Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
        }
    }
    else if (m_pMatrix)
    {
        ReleaseMatrix();
    }

    float rect[8]{};
    using GetBoxFn = void(__thiscall*)(CEntity*, float*);
    reinterpret_cast<GetBoxFn>(m_vftable[26])(this, rect);

    const float worldMin = Core::StaticRef<float>(0xEBE168);
    const float worldMax1 = Core::StaticRef<float>(0xEBE164);
    const float worldMax2 = Core::StaticRef<float>(0xD83C7C);

    const float minX = (worldMin > rect[0]) ? worldMin : rect[0];
    const float minY = (worldMin > rect[3]) ? worldMin : rect[3];
    const float maxX = (rect[2] >= worldMax1) ? worldMax2 : rect[2];
    const float maxY = (rect[1] >= worldMax1) ? worldMax2 : rect[1];

    if ((m_nFlags & 0x40u) != 0)
    {
        const float scale = Core::StaticRef<float>(0xEBE284);
        const float offset = Core::StaticRef<float>(0xD50960);

        const int32 x0 = static_cast<int32>(minX * scale + offset);
        const int32 x1 = static_cast<int32>(maxX * scale + offset);
        const int32 y0 = static_cast<int32>(minY * scale + offset);
        const int32 y1 = static_cast<int32>(maxY * scale + offset);

        if (y0 > y1 || x0 > x1)
            return;

        auto** rowBase = reinterpret_cast<PtrNodeSingle**>(Core::Rebase(0x11D2870)) + y0 * 30 + x0;
        for (int32 y = y0; y <= y1; ++y)
        {
            for (int32 x = x0; x <= x1; ++x)
            {
                PtrNodeSingle** head = &rowBase[x - x0];
                PtrNodeSingle* prev = nullptr;
                PtrNodeSingle* cur = *head;
                while (cur && cur->entity != this)
                {
                    prev = cur;
                    cur = cur->next;
                }
                if (cur)
                {
                    if (cur == *head)
                        *head = cur->next;
                    else if (prev)
                        prev->next = cur->next;
                    Core::Hook::CallMethod<void>(0xC79660, CPools::ms_pPtrNodeSinglePool, cur);
                }
            }
            rowBase += 30;
        }
        return;
    }

    const float scale = Core::StaticRef<float>(0xD529C4);
    const float offset = Core::StaticRef<float>(0xDCE040);

    const int32 x0 = static_cast<int32>(std::floor(minX * scale + offset));
    const int32 x1 = static_cast<int32>(std::floor(maxX * scale + offset));
    const int32 y0 = static_cast<int32>(std::floor(minY * scale + offset));
    const int32 y1 = static_cast<int32>(std::floor(maxY * scale + offset));

    if (y0 > y1 || x0 > x1)
        return;

    const uint32 listType = (m_nFlags2 >> 6) & 0xFu;
    for (int32 y = y0; y <= y1; ++y)
    {
        for (int32 x = x0; x <= x1; ++x)
        {
            if (listType == 1)
            {
                void** slot = GetGridSectorPtr(1, x, y);
                Core::Hook::CallMethod<void>(0x7DB8D0, slot, this);
            }
            else if (listType >= 2 && listType <= 8)
            {
                auto** head = reinterpret_cast<PtrNodeDouble**>(GetGridSectorPtr(listType, x, y));
                PtrNodeDouble* cur = *head;
                while (cur && cur->entity != this)
                    cur = cur->next;
                if (cur)
                {
                    if (cur == *head)
                        *head = cur->next;
                    if (cur->prev)
                        cur->prev->next = cur->next;
                    if (cur->next)
                        cur->next->prev = cur->prev;
                    Core::Hook::CallMethod<void>(0xC79660, CPools::ms_pPtrNodeDoublePool, cur);
                }
            }
        }
    }
}

void __thiscall CEntity::CreatePhysicsInst()
{
    if (m_nModelIndex == -1 || m_pPhysicsInst)
        return;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if (mi->m_pFragType)
    {
        m_nFlags |= 0x4000000u;
        if (!m_pMatrix)
        {
            AllocateStaticMatrix();
            Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
        }

        void* mem = Core::Hook::CallMethod<void*>(0x96D520, Core::StaticRef<void*>(0x170665C));
        void* inst = mem ? Core::Hook::CallMethod<void*>(0xC711F0, mem, 0x11, mi->m_pFragType, m_pMatrix, 0) : nullptr;
        *reinterpret_cast<uint32*>(static_cast<uint8*>(inst) + 0x60) |= 8u;
        Core::Hook::CallMethod<void>(0x9E6FA0, this, inst, 1);

        void** vtbl = *reinterpret_cast<void***>(inst);
        using InitFn = void(__thiscall*)(void*, int32);
        reinterpret_cast<InitFn>(vtbl[40])(inst, 0);
        return;
    }

    if (mi->m_pArchetype)
    {
        void* mem = Core::Hook::CallMethod<void*>(0x96D520, Core::StaticRef<void*>(0x1706658));
        void* inst = mem ? Core::Hook::CallMethod<void*>(0xC70E60, mem, 6) : nullptr;
        void* mat = Core::Hook::CallMethod<void*>(0x447980, this);
        Core::Hook::CallMethod<void>(0x686A20, inst, mi->m_pArchetype, mat);
        Core::Hook::CallMethod<void>(0x9E6FA0, this, inst, 1);
        return;
    }

    if (*reinterpret_cast<int16*>(reinterpret_cast<uint8*>(mi) + 0x52) != -1)
        Core::Hook::Call<void>(0xA73370, this);
}

void __thiscall CEntity::UpdatePhysicsMatrix(int32 arg)
{
    if (!m_pPhysicsInst)
        return;

    if (!m_pMatrix)
    {
        AllocateStaticMatrix();
        Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
    }

    auto* dstMat = reinterpret_cast<CMatrix*>(static_cast<uint8*>(m_pPhysicsInst) + 0x10);
    CopyMatrix3x4(dstMat, m_pMatrix);

    const uint16 levelIndex = *reinterpret_cast<uint16*>(static_cast<uint8*>(m_pPhysicsInst) + 8);
    if (levelIndex != 0xFFFFu)
        Core::Hook::CallMethod<void>(0x6A3960, Core::StaticRef<void*>(0x1666880), static_cast<uint32>(levelIndex), 0);
}

void __thiscall CEntity::Process2dEffects(int32 arg0, int32 arg1, int32 arg2)
{
    if (m_nModelIndex == -1)
        return;

    CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if ((*reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(mi) + 0x5B) & 8u) == 0)
        return;

    const int32 count = Core::Hook::CallMethod<int32>(0x98EA60, mi);
    for (int32 i = 0; i < count; ++i)
    {
        void* fx = Core::Hook::CallMethod<void*>(0x98EAA0, mi, i);
        CVector pos = *reinterpret_cast<CVector*>(static_cast<uint8*>(fx) + 4);
        void** vtbl = *reinterpret_cast<void***>(fx);

        using GetTypeFn = uint8(__thiscall*)(void*);
        const uint8 type = reinterpret_cast<GetTypeFn>(vtbl[1])(fx);

        if (type == 1)
        {
            using GetDataFn = void*(__thiscall*)(void*);
            void* data = reinterpret_cast<GetDataFn>(vtbl[7])(fx);
            if (*reinterpret_cast<uint8*>(static_cast<uint8*>(data) + 0x39) != static_cast<uint8>(arg1)
                && *reinterpret_cast<uint32*>(static_cast<uint8*>(data) + 0x24) == 3)
            {
                const int32 fxId = *reinterpret_cast<int32*>(static_cast<uint8*>(data) + 0x28);
                if (fxId == arg0 || fxId == -1)
                {
                    Core::Hook::CallMethod<void>(
                        0xC865E0, reinterpret_cast<void*>(Core::Rebase(0x170F670)), this, data, arg0, arg1, arg2);
                }
            }
        }
        else if (type == 2)
        {
            using GetDataFn = void*(__thiscall*)(void*);
            void* data = reinterpret_cast<GetDataFn>(vtbl[9])(fx);
            const uint8 flag2D = *reinterpret_cast<uint8*>(static_cast<uint8*>(data) + 0x2D);
            if (flag2D != static_cast<uint8>(arg1)
                && *reinterpret_cast<uint32*>(static_cast<uint8*>(data) + 0x24) == 1)
            {
                const int32 fxId = *reinterpret_cast<int32*>(static_cast<uint8*>(data) + 0x28);
                if (fxId != arg0 && fxId != -1)
                    continue;

                if ((m_nFlags2 & 0x100000u) != 0)
                {
                    const int32 soundIdx = *reinterpret_cast<int32*>(static_cast<uint8*>(data) + 0x20);
                    if (Core::StaticRef<uint8>(0x15C07C8u + static_cast<uintptr_t>(soundIdx) * 0x70u) == 0)
                        continue;
                }

                const int32 id = (flag2D != 0) ? arg2 : arg0;
                uint8 tmpByte = 0;
                void* snd = Core::Hook::Call<void*>(0xB1E560, this, id, 1, &tmpByte);
                if (!snd || Core::Hook::CallMethod<bool>(0x4967C0, snd))
                    continue;

                struct { CVector vec; float w; } worldPos{};
                Core::Hook::CallMethod<void>(0x516BA0, snd, &pos, &worldPos.vec);

                void* param = nullptr;
                if ((m_nFlags2 & 0x3C0u) == 0x100u)
                {
                    param = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x27C);
                    if (!param)
                    {
                        auto* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                        if (attach && (attach->m_nFlags2 & 0x3C0u) == 0x100u)
                            param = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(attach) + 0x27C);
                    }
                }

                auto* mat = static_cast<uint8*>(Core::Hook::CallMethod<void*>(0x447980, this));
                const uint32 sndParam = *reinterpret_cast<uint32*>(static_cast<uint8*>(data) + 0x20);
                Core::Hook::Call<void>(
                    0x9609F0,
                    0,
                    0,
                    sndParam,
                    1.0f,
                    &worldPos.vec,
                    0,
                    0,
                    1,
                    Core::StaticRef<float>(0xEBDED4),
                    0,
                    0,
                    mat + 0x20,
                    this,
                    id,
                    0,
                    param ? 0 : 1,
                    param,
                    0,
                    -1);

                if (!Core::Hook::Call<bool>(0x462F90) || Core::Hook::Call<bool>(0x463000))
                {
                    m_nFlags2 |= 0x100000u;
                    if ((m_nFlags2 & 0x3C0u) == 0x100u)
                    {
                        auto* attach = *reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0x280);
                        if (attach)
                            attach->m_nFlags2 |= 0x100000u;
                    }
                }
            }
        }
    }
}

void __thiscall CEntity::SetMatrix(const CMatrix* mat, int32 arg1, int32 arg2)
{
    if (!m_pMatrix)
    {
        if (mat->at.z == Core::StaticRef<float>(0xD50398))
        {
            m_vecPos = mat->pos;
            m_fPad1C = static_cast<float>(std::atan2(-mat->up.x, mat->up.y));
            return;
        }
        AllocateStaticMatrix();
    }

    CopyMatrix3x4(m_pMatrix, mat);
}

CEntity* __thiscall CEntity::DeletingDtor(uint8 flags)
{
    Dtor();
    if ((flags & 1u) != 0)
        Core::Hook::Call<void>(0x5B1C10, this);
    return this;
}
