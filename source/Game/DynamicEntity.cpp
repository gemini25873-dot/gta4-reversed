#include "StdInc.h"
#include "Game/DynamicEntity.h"
#include "Game/Pools.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>

namespace
{
    struct CVector4
    {
        CVector vec{};
        float w = 0.0f;
    };

    inline uint8* GetModelInfoPtr(int16 modelIndex)
    {
        auto** modelPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
        return modelPtrs[modelIndex];
    }

    inline uint8* GetFragPtr(CDynamicEntity* self)
    {
        using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
        uint8* frag = reinterpret_cast<GetFragFn>(self->m_vftable[40])(self);
        if (!frag)
            frag = static_cast<uint8*>(self->m_pPhysicsInst);
        return frag;
    }

    inline uint8* GetPhArchetypeFromFrag(uint8* frag)
    {
        return *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(frag + 4) + 12);
    }

    inline void UpdateRectBounds(float* outRect, float x, float y)
    {
        if (outRect[0] > x) outRect[0] = x;
        if (x > outRect[2]) outRect[2] = x;
        if (outRect[3] > y) outRect[3] = y;
        if (y > outRect[1]) outRect[1] = y;
    }

    struct CDynRenderTargetHelper
    {
        float  m_fRect[4];     // +0x00..+0x0C
        void*  m_pUnk10;       // +0x10
        void*  m_pUnk14;       // +0x14
        void*  m_pUnk18;       // +0x18
        void*  m_pTexture;     // +0x1C
        uint32 m_nUnk20;       // +0x20
        uint8  m_bDirty;       // +0x24
        uint8  m_nPad25[3];    // +0x25..+0x27

        CDynRenderTargetHelper* __thiscall Init()
        {
            const float one = *reinterpret_cast<const float*>(Core::Rebase(0xD50398));
            m_fRect[0] = 0.0f;
            m_fRect[3] = one;
            m_fRect[2] = one;
            m_fRect[1] = 0.0f;
            m_pTexture = nullptr;
            m_bDirty = 1;
            m_pUnk10 = nullptr;
            m_pUnk14 = nullptr;
            m_pUnk18 = nullptr;
            m_pUnk14 = Core::StaticRef<void*>(0x16394A0);
            return this;
        }

        void __thiscall SetUnk20(uint32 val)
        {
            m_nUnk20 = val;
        }

        void __thiscall SetRect(const float* rect4)
        {
            std::memcpy(m_fRect, rect4, 16);
            m_bDirty = 1;
        }

        bool __thiscall IsReady() const
        {
            return m_pUnk18 != nullptr && m_pUnk14 != nullptr && m_pUnk10 != nullptr;
        }

        void __thiscall EnsureTexture()
        {
            if (m_pTexture != nullptr)
                return;

            void* mgr = Core::StaticRef<void*>(0x16394A0);
            using GetDimFn = int32(__thiscall*)(void*);
            int32 w = mgr ? reinterpret_cast<GetDimFn>((*reinterpret_cast<void***>(mgr))[0x1C / 4])(mgr) : 0x200;

            mgr = Core::StaticRef<void*>(0x16394A0);
            int32 h = mgr ? reinterpret_cast<GetDimFn>((*reinterpret_cast<void***>(mgr))[0x20 / 4])(mgr) : 0x200;

            uint8 desc[0x30]{};
            Core::Hook::CallMethod<void>(0x61DBC0, desc, 0);

            desc[0x12] = 0;
            *reinterpret_cast<uint32*>(desc + 0x14) = 0;
            desc[0x00] = 0;
            desc[0x11] = 0;
            *reinterpret_cast<uint32*>(desc + 0x2C) = 2;
            desc[0x08] = 1;
            desc[0x10] = 1;
            *reinterpret_cast<uint32*>(desc + 0x0C) = 1;
            desc[0x25] = 1;

            void* factory = Core::StaticRef<void*>(0x18A8630);
            using CreateTexFn = void*(__thiscall*)(void*, const char*, int32, int32, int32, int32, void*);
            void* tex = reinterpret_cast<CreateTexFn>((*reinterpret_cast<void***>(factory))[0x38 / 4])(
                factory, reinterpret_cast<const char*>(Core::Rebase(0xD830AC)), 3, w, h, 0x20, desc);

            m_pTexture = tex;
            m_bDirty = 1;
        }
    };

    inline void* GetEntitySkeleton(CDynamicEntity* self)
    {
        using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
        uint8* frag = reinterpret_cast<GetFragFn>(self->m_vftable[40])(self);
        if (frag)
        {
            using GetFragSkelFn = void*(__thiscall*)(uint8*);
            return reinterpret_cast<GetFragSkelFn>((*reinterpret_cast<void***>(frag))[0xE0 / 4])(frag);
        }
        return reinterpret_cast<void*>(self->m_nDynUnk100);
    }

    inline void GetSectorAndListHead(int32 x, int32 y, uint32 flags2, uint8*& outSector, uint32**& outListHead)
    {
        const int32 sectorIdx = (y & 0xF) + ((x & 0xF) << 4);
        outSector = reinterpret_cast<uint8*>(Core::Rebase(0x11A7160)) + sectorIdx * 20;

        const int32 clampedY = (y <= 0) ? 0 : (y < 0x77 ? y : 0x77);
        const int32 clampedX = (x <= 0) ? 0 : (x < 0x77 ? x : 0x77);

        const uint32 entKind = (flags2 >> 6) & 0xF;
        switch (entKind)
        {
        case 2:
            outListHead = reinterpret_cast<uint32**>(outSector);
            break;
        case 3:
            outListHead = reinterpret_cast<uint32**>(outSector + 4);
            break;
        case 4:
            outListHead = reinterpret_cast<uint32**>(outSector + 8);
            break;
        case 6:
            outListHead = reinterpret_cast<uint32**>(
                reinterpret_cast<uint8*>(Core::Rebase(0x11A856C)) + (clampedY + clampedX * 120) * 12);
            break;
        default:
            outListHead = nullptr;
            break;
        }
    }

    inline void UnlinkSectorEntry(uint32** listHead, uint32* entry)
    {
        if (*listHead == entry)
            *listHead = reinterpret_cast<uint32*>(entry[1]);
        if (entry[2])
            *reinterpret_cast<uint32*>(entry[2] + 4) = entry[1];
        if (entry[1])
            *reinterpret_cast<uint32*>(entry[1] + 8) = entry[2];
    }

    inline void LinkSectorEntry(uint32** listHead, uint32* entry)
    {
        entry[2] = 0;
        entry[1] = reinterpret_cast<uint32>(*listHead);
        if (*listHead)
            *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(*listHead) + 8) = reinterpret_cast<uint32>(entry);
        *listHead = entry;
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

    // Slice 1 (0x9E04D0..0x9E17AF)
    Core::Hook::Install(0x9E04D0, &CDynamicEntity::GetWorldBoundCentre);
    Core::Hook::Install(0x9E05C0, &CDynamicEntity::GetBoundCentreTransformed);
    Core::Hook::Install(0x9E0610, &CDynamicEntity::GetBoundSphere);
    Core::Hook::Install(0x9E06C0, &CDynamicEntity::GetWorldBoundRect);
    Core::Hook::Install(0x9E0BD0, &CDynamicEntity::CalcWorldBoundBox);
    Core::Hook::Install(0x9E1190, &CDynamicEntity::UpdateFragState);
    Core::Hook::Install(0x9E12F0, &CDynamicEntity::ResetDynUnkE0);
    Core::Hook::Install(0x9E1360, &CDynamicEntity::ShouldSkipDistanceCheck);
    Core::Hook::Install(0x9E1460, &CDynamicEntity::ShouldSkipOrFlagged);
    Core::Hook::Install(0x9E1480, &CDynamicEntity::CleanupAndNotify);
    Core::Hook::Install(0x9E15B0, &CDynamicEntity::ProcessPostPhysicsUpdate);

    // Slice 2 (0x9E17B0..0x9E2400)
    Core::Hook::Install(0x9E17B0, &CDynamicEntity::InitSkeleton);
    Core::Hook::Install(0x9E1870, &CDynamicEntity::DeleteSkeleton);
    Core::Hook::Install(0x9E18A0, &CDynamicEntity::UpdateSkeletonOrUnk);
    Core::Hook::Install(0x9E1900, &CDynamicEntity::AddAnimRef);
    Core::Hook::Install(0x9E1930, &CDynamicEntity::ReleaseAnimRef);
    Core::Hook::Install(0x9E1960, &CDynamicEntity::AddToWorldSectors);
    Core::Hook::Install(0x9E1B20, &CDynamicEntity::AddToDynList);
    Core::Hook::Install(0x9E1B70, &CDynamicEntity::UpdateAnim);
    Core::Hook::Install(0x9E1CF0, &CDynamicEntity::CreateAnimBlender);
    Core::Hook::Install(0x9E1D30, &CDynamicEntity::InitCrSkeletonOrAnim);
    Core::Hook::Install(0x9E1E70, &CDynamicEntity::ReleaseCrSkeletonOrAnim);
    Core::Hook::Install(0x9E1ED0, &CDynamicEntity::RemoveFromWorldSectors);
    Core::Hook::Install(0x9E1F70, &CDynamicEntity::UpdateWorldSectors);
    Core::Hook::Install(0x9E2220, &CDynamicEntity::RemoveFromDynList);
    Core::Hook::Install(0x9E2270, &CDynamicEntity::DeleteAnimBlender);
    Core::Hook::Install(0x9E22A0, &CDynamicEntity::CleanupDynamicData);
    Core::Hook::Install(0x9E2300, &CDynamicEntity::ScalarDeletingDtor);

    Core::Hook::Install(0x9E2320, &CDynRenderTargetHelper::Init);
    Core::Hook::Install(0x9E2360, &CDynRenderTargetHelper::SetUnk20);
    Core::Hook::Install(0x9E2370, &CDynRenderTargetHelper::SetRect);
    Core::Hook::Install(0x9E2390, &CDynRenderTargetHelper::IsReady);
    Core::Hook::Install(0x9E23B0, &CDynRenderTargetHelper::EnsureTexture);

    Core::Log::Info("CDynamicEntity hooks installed (40 functions)");
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


// --- Slice 1 (0x9E04D0..0x9E17AF) ---

// 0x9E04D0 (ret 0x4)
CVector* __thiscall CDynamicEntity::GetWorldBoundCentre(CVector* out)
{
    uint8* frag = GetFragPtr(this);
    if (frag)
    {
        uint8* phArchetype = GetPhArchetypeFromFrag(frag);
        const CVector& centre = *reinterpret_cast<const CVector*>(phArchetype + 0x30);
        const CMatrix& fragMat = *reinterpret_cast<const CMatrix*>(frag + 0x10);

        out->x = fragMat.right.x * centre.x + fragMat.up.x * centre.y + fragMat.at.x * centre.z + fragMat.pos.x;
        out->y = fragMat.right.y * centre.x + fragMat.up.y * centre.y + fragMat.at.y * centre.z + fragMat.pos.y;
        out->z = fragMat.right.z * centre.x + fragMat.up.z * centre.y + fragMat.at.z * centre.z + fragMat.pos.z;
        return out;
    }

    Core::Hook::CallMethod<void>(0x9E8280, this, out);
    return out;
}

// 0x9E05C0 (ret 0x4)
CVector* __thiscall CDynamicEntity::GetBoundCentreTransformed(CVector* out)
{
    uint8* frag = GetFragPtr(this);
    if (frag)
    {
        uint8* phArchetype = GetPhArchetypeFromFrag(frag);
        return Core::Hook::CallMethod<CVector*>(0x7D6880, this, out, phArchetype + 0x30);
    }

    return Core::Hook::CallMethod<CVector*>(0x9E8390, this, out);
}

// 0x9E0610 (ret 0x4)
double __thiscall CDynamicEntity::GetBoundSphere(CVector* outCentre)
{
    uint8* frag = GetFragPtr(this);
    if (frag)
    {
        uint8* phArchetype = GetPhArchetypeFromFrag(frag);
        Core::Hook::CallMethod<CVector*>(0x7D6880, this, outCentre, phArchetype + 0x30);
        return *reinterpret_cast<float*>(phArchetype + 0x8);
    }

    return Core::Hook::CallMethod<double>(0x9E8490, this, outCentre);
}

// 0x9E06C0 (ret 0x4)
float* __thiscall CDynamicEntity::GetWorldBoundRect(float* outRect)
{
    uint8* modelInfo = GetModelInfoPtr(m_nModelIndex);

    outRect[0] = 1000000.0f;
    outRect[3] = 1000000.0f;
    outRect[2] = -1000000.0f;
    outRect[1] = -1000000.0f;

    CVector4 boxMax{};
    CVector4 boxMin{};

    uint8* frag = GetFragPtr(this);
    if (frag)
    {
        uint8* phArchetype = GetPhArchetypeFromFrag(frag);
        boxMax = *reinterpret_cast<const CVector4*>(phArchetype + 0x20);
        boxMin = *reinterpret_cast<const CVector4*>(phArchetype + 0x10);
    }
    else
    {
        boxMax = *reinterpret_cast<const CVector4*>(modelInfo + 0x20);
        boxMin = *reinterpret_cast<const CVector4*>(modelInfo + 0x30);
    }

    auto transformCorner = [&](float cx, float cy, float cz, float cw)
    {
        if (m_pMatrix)
        {
            float wx = m_pMatrix->right.x * cx + m_pMatrix->up.x * cy + m_pMatrix->at.x * cz + m_pMatrix->pos.x;
            float wy = m_pMatrix->right.y * cx + m_pMatrix->up.y * cy + m_pMatrix->at.y * cz + m_pMatrix->pos.y;
            UpdateRectBounds(outRect, wx, wy);
        }
        else
        {
            CVector4 localCorner{ { cx, cy, cz }, cw };
            CVector4 worldCorner{};
            Core::Hook::Call<float*>(0x9E77F0, &worldCorner, &m_vecPos, &localCorner);
            UpdateRectBounds(outRect, worldCorner.vec.x, worldCorner.vec.y);
        }
    };

    transformCorner(boxMax.vec.x, boxMax.vec.y, boxMax.vec.z, boxMax.w);
    transformCorner(boxMin.vec.x, boxMin.vec.y, boxMin.vec.z, boxMin.w);
    transformCorner(boxMin.vec.x, boxMax.vec.y, boxMax.vec.z, boxMax.w);
    transformCorner(boxMax.vec.x, boxMin.vec.y, boxMin.vec.z, boxMin.w);

    return outRect;
}

// 0x9E0BD0 (ret 0x4)
float* __thiscall CDynamicEntity::CalcWorldBoundBox(float* outBox)
{
    CVector v1{};
    CVector v2{};

    uint8* frag = GetFragPtr(this);
    if (frag)
    {
        uint8* phArchetype = GetPhArchetypeFromFrag(frag);
        v1 = *reinterpret_cast<const CVector*>(phArchetype + 0x10);
        v2 = *reinterpret_cast<const CVector*>(phArchetype + 0x20);
    }
    else
    {
        uint8* modelInfo = GetModelInfoPtr(m_nModelIndex);
        v1 = *reinterpret_cast<const CVector*>(modelInfo + 0x30);
        v2 = *reinterpret_cast<const CVector*>(modelInfo + 0x20);
    }

    CMatrix localMat{};
    const CMatrix* mat = m_pMatrix;
    if (!mat)
    {
        Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, &localMat);
        mat = &localMat;
    }

    CVector centre = (v1 + v2) * 0.5f;
    CVector halfExt = (v1 - v2) * 0.5f;

    float wx = mat->right.x * centre.x + mat->up.x * centre.y + mat->at.x * centre.z + mat->pos.x;
    float wy = mat->right.y * centre.x + mat->up.y * centre.y + mat->at.y * centre.z + mat->pos.y;
    float wz = mat->right.z * centre.x + mat->up.z * centre.y + mat->at.z * centre.z + mat->pos.z;

    float ex = std::fabsf(mat->right.x) * halfExt.x + std::fabsf(mat->up.x) * halfExt.y + std::fabsf(mat->at.x) * halfExt.z;
    float ey = std::fabsf(mat->right.y) * halfExt.x + std::fabsf(mat->up.y) * halfExt.y + std::fabsf(mat->at.y) * halfExt.z;
    float ez = std::fabsf(mat->right.z) * halfExt.x + std::fabsf(mat->up.z) * halfExt.y + std::fabsf(mat->at.z) * halfExt.z;

    outBox[0] = wx - ex;
    outBox[1] = wy - ey;
    outBox[2] = wz - ez;
    outBox[3] = 0.0f;

    outBox[4] = wx + ex;
    outBox[5] = wy + ey;
    outBox[6] = wz + ez;
    outBox[7] = 0.0f;

    return outBox;
}

// 0x9E1190 (ret)
void __thiscall CDynamicEntity::UpdateFragState()
{
    if (m_nDynUnk78)
    {
        auto* unk78 = reinterpret_cast<uint8*>(m_nDynUnk78);
        if (unk78[0] & 1)
            Core::Hook::CallMethod<void>(0x509FF0, reinterpret_cast<void*>(Core::Rebase(0x16397D0)), unk78 + 0x1A04, 1);
    }

    using GetFragOnlyFn = uint8*(__thiscall*)(CDynamicEntity*);
    auto getFragOnly = reinterpret_cast<GetFragOnlyFn>(m_vftable[40]);

    uint8* frag = getFragOnly(this);
    void* obj = nullptr;
    if (frag)
    {
        uint8* f = getFragOnly(this);
        using FragFn56 = void*(__thiscall*)(void*);
        void** fVtbl = *reinterpret_cast<void***>(f);
        obj = reinterpret_cast<FragFn56>(fVtbl[56])(f);
    }
    else
    {
        obj = reinterpret_cast<void*>(m_nDynUnk100);
    }

    if (!obj || !(m_nDynUnkF4 & 0x2))
        return;

    frag = getFragOnly(this);
    if (frag)
    {
        uint8* f = getFragOnly(this);
        using FragFn56 = void*(__thiscall*)(void*);
        void** fVtbl = *reinterpret_cast<void***>(f);
        obj = reinterpret_cast<FragFn56>(fVtbl[56])(f);
    }
    else
    {
        obj = reinterpret_cast<void*>(m_nDynUnk100);
    }

    Core::Hook::CallMethod<void>(0x63DD30, obj);

    uint8* f = getFragOnly(this);
    if (f)
    {
        uint8* c = *reinterpret_cast<uint8**>(f + 0x64);
        if (c)
        {
            if ((m_nFlags2 & 0x3C0) == 0xC0)
            {
                uint16 idx = *reinterpret_cast<uint16*>(f + 0x8);
                uint8* globalObj = *reinterpret_cast<uint8**>(Core::Rebase(0x1666880));
                uint16 count = *reinterpret_cast<uint16*>(globalObj + 0x26);
                if (idx < count)
                {
                    uint8* arr = *reinterpret_cast<uint8**>(globalObj + 0x70);
                    if (arr[idx * 8 + 4] & 0x3)
                    {
                        m_nDynUnkF4 |= 0x28;
                        return;
                    }
                }
            }

            uint8* sub = c + 0x100;
            if (*reinterpret_cast<uint32*>(sub + 0x74) != 0)
            {
                uint8* f2 = getFragOnly(this);
                using FragFn49 = void(__thiscall*)(void*);
                void** f2Vtbl = *reinterpret_cast<void***>(f2);
                reinterpret_cast<FragFn49>(f2Vtbl[49])(f2);
                m_nDynUnkF4 |= 0x28;
                return;
            }

            uint8* p168 = *reinterpret_cast<uint8**>(c + 0x168);
            uint32 val = p168 ? *reinterpret_cast<uint32*>(p168 + 0x14) : 0;
            uint8* a1 = Core::Hook::CallMethod<uint8*>(0x447A20, this);
            uint32 arg3 = *reinterpret_cast<uint32*>(a1 + 0x10);
            void* arg0 = Core::Hook::CallMethod<void*>(0x447A60, this, 0);
            Core::Hook::CallMethod<void>(0x51B0B0, f, arg0, val, arg3);
        }
    }

    m_nDynUnkF4 |= 0x28;
}

// 0x9E12F0 (ret)
void __thiscall CDynamicEntity::ResetDynUnkE0()
{
    uint8 al = m_nDynUnkF4;
    uint8 dl = static_cast<uint8>((al & 0x20) << 1);
    m_nDynUnkF4 = static_cast<uint8>((al & 0x9F) | dl);

    m_nDynUnkE0 = Core::StaticRef<uint32>(0x18A8C60);
    m_nDynUnkE4 = Core::StaticRef<uint32>(0x18A8C64);
    m_nDynUnkE8 = Core::StaticRef<uint32>(0x18A8C68);
    m_nDynUnkEC = Core::StaticRef<uint32>(0x18A8C6C);
    m_nDynUnkF0 = 0;
}

// 0x9E1360 (ret)
bool __thiscall CDynamicEntity::ShouldSkipDistanceCheck()
{
    if ((m_nFlags2 & 0x3C0) != 0xC0)
        return false;
    if (Core::StaticRef<uint8>(0xF0C511) == 0)
        return false;
    if (Core::StaticRef<uint8>(0x10D0189) != 0)
        return false;

    if ((*(reinterpret_cast<uint8*>(this) + 0x26C) & 1) == 0 && !Core::Hook::CallMethod<bool>(0x426A30, this))
        return true;

    CVector pos = GetPosition();
    float dx = pos.x - Core::StaticRef<float>(0x11DC4E0);
    float dy = pos.y - Core::StaticRef<float>(0x11DC4E4);
    float dz = pos.z - Core::StaticRef<float>(0x11DC4E8);
    float distSq = dx * dx + dy * dy + dz * dz;

    uint8* player = Core::Hook::Call<uint8*>(0x817F40);
    if (player && *reinterpret_cast<void**>(player + 0x20) != nullptr)
    {
        uint8* playerMat = Core::Hook::CallMethod<uint8*>(0x447980, player);
        distSq = static_cast<float>(Core::Hook::CallMethod<double>(0x444270, &pos, reinterpret_cast<const CVector*>(playerMat + 0x30)));
    }

    float thresh = static_cast<float>(Core::Hook::Call<double>(0x799240));
    return distSq > thresh;
}

// 0x9E1460 (ret)
bool __thiscall CDynamicEntity::ShouldSkipOrFlagged()
{
    if (m_nDynUnkF4 & 0x4)
        return true;

    return ShouldSkipDistanceCheck();
}

// 0x9E1480 (ret)
void* __thiscall CDynamicEntity::CleanupAndNotify()
{
    void* res = Core::Hook::CallMethod<void*>(0x9DFF40, this);
    if (!res)
    {
        using NotifyFn = void(__thiscall*)(CDynamicEntity*);
        reinterpret_cast<NotifyFn>(m_vftable[51])(this);
    }
    return res;
}

// 0x9E15B0 (ret)
int32 __thiscall CDynamicEntity::ProcessPostPhysicsUpdate()
{
    Core::Hook::CallMethod<void>(0x9E14A0, this);
    if (Core::StaticRef<uint8>(0x15BFF22) == 0)
        Core::Hook::CallMethod<void>(0x9DFED0, this);

    return Core::Hook::CallMethod<int32>(0x9E9BA0, this);
}

// --- Slice 2 (0x9E17B0..0x9E2400) ---

void __thiscall CDynamicEntity::InitSkeleton()
{
    using GetFragFn = uint8*(__thiscall*)(CDynamicEntity*);
    if (reinterpret_cast<GetFragFn>(m_vftable[40])(this))
        return;

    if (!m_pDrawHandler)
        return;

    uint8* rmcDrawable = *reinterpret_cast<uint8**>(m_pDrawHandler);
    if (!rmcDrawable)
        return;

    uint8* skelData = *reinterpret_cast<uint8**>(rmcDrawable + 0xC);
    if (!skelData)
        return;

    const uint16 numBones = *reinterpret_cast<uint16*>(skelData + 0x14);
    void* mem = Core::Hook::Call<void*>(0x4011D0, 0x28);
    void* skel = mem ? Core::Hook::CallMethod<void*>(0x63D490, mem, static_cast<uint32>(numBones), 1) : nullptr;
    m_nDynUnk100 = reinterpret_cast<uint32>(skel);
    Core::Hook::CallMethod<void>(0x63D710, skel, skelData, 0, 0);

    if (!m_pMatrix)
    {
        Core::Hook::CallMethod<void>(0x9EA460, this);
        Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
    }

    Core::Hook::CallMethod<void>(0x9EA400, this);

    if (!m_pMatrix)
    {
        Core::Hook::CallMethod<void>(0x9EA460, this);
        Core::Hook::CallMethod<void>(0x9E9440, &m_vecPos, m_pMatrix);
    }

    *reinterpret_cast<uint32*>(m_nDynUnk100 + 8) = reinterpret_cast<uint32>(m_pMatrix);
    m_nDynUnkF4 |= 0x2;
}

void __thiscall CDynamicEntity::DeleteSkeleton()
{
    void* skel = reinterpret_cast<void*>(m_nDynUnk100);
    if (skel)
    {
        Core::Hook::CallMethod<void>(0x63D5C0, skel);
        Core::Hook::Call<void>(0x5B1C10, skel);
    }
    m_nDynUnk100 = 0;
}

void __thiscall CDynamicEntity::UpdateSkeletonOrUnk(void* arg)
{
    Core::Hook::CallMethod<void>(0x9E6A10, this, arg);

    if (m_nDynUnk100)
        Core::Hook::CallMethod<void>(0x53BDA0, arg, reinterpret_cast<void*>(m_nDynUnk100 + 4));

    if (m_nDynUnk78)
        Core::Hook::CallMethod<void>(0x9EBE30, reinterpret_cast<void*>(m_nDynUnk78), arg);

    if (m_nDynUnkD4 && m_nDynUnkD0)
    {
        uint8* item = Core::Hook::CallMethod<uint8*>(0x59A640, reinterpret_cast<void*>(m_nDynUnkD0), 3);
        if (item)
            Core::Hook::CallMethod<void>(0xCBA1F0, arg, item + 0x18);
    }
}

void __cdecl CDynamicEntity::AddAnimRef(int32 index)
{
    uint8* pool = Core::StaticRef<uint8*>(0x1711150);
    uint8* flags = *reinterpret_cast<uint8**>(pool + 4);
    uint8* elem = (flags[index] & 0x80)
        ? nullptr
        : (*reinterpret_cast<uint8**>(pool) + index * (*reinterpret_cast<int32*>(pool + 0xC)));
    if (elem)
        *reinterpret_cast<int32*>(elem + 4) += 1;
}

void __cdecl CDynamicEntity::ReleaseAnimRef(int32 index)
{
    uint8* pool = Core::StaticRef<uint8*>(0x1711150);
    uint8* flags = *reinterpret_cast<uint8**>(pool + 4);
    uint8* elem = (flags[index] & 0x80)
        ? nullptr
        : (*reinterpret_cast<uint8**>(pool) + index * (*reinterpret_cast<int32*>(pool + 0xC)));
    if (elem)
        *reinterpret_cast<int32*>(elem + 4) -= 1;
}

void __thiscall CDynamicEntity::AddToWorldSectors()
{
    if ((m_nFlags2 & 0x3C0) == 0x40)
    {
        Core::Hook::CallMethod<void>(0x9E6D80, this);
        return;
    }

    int32 minY = 0, minX = 0, maxY = 0, maxX = 0;
    Core::Hook::CallMethod<void>(0x9E0090, this, &minY, &minX, &maxY, &maxX);

    void* pool1 = Core::StaticRef<void*>(0x1706E98);
    void* pool2 = Core::StaticRef<void*>(0x16B775C);

    for (int32 x = minX; x <= maxX; ++x)
    {
        for (int32 y = minY; y <= maxY; ++y)
        {
            uint8* sector = nullptr;
            uint32** listHead = nullptr;
            GetSectorAndListHead(x, y, m_nFlags2, sector, listHead);

            uint32* node = Core::Hook::CallMethod<uint32*>(0x96D520, pool1);
            if (node)
            {
                uint32* entry = Core::Hook::CallMethod<uint32*>(0x96D520, pool2);
                if (entry)
                    entry[0] = reinterpret_cast<uint32>(this);

                LinkSectorEntry(listHead, entry);
                node[1] = reinterpret_cast<uint32>(entry);
                node[2] = reinterpret_cast<uint32>(sector);
                node[0] = reinterpret_cast<uint32>(listHead);
            }

            node[3] = 0;
            node[4] = reinterpret_cast<uint32>(m_pAnimBlender);
            if (m_pAnimBlender)
                *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pAnimBlender) + 12) = reinterpret_cast<uint32>(node);
            m_pAnimBlender = node;
        }
    }
}

void __thiscall CDynamicEntity::AddToDynList()
{
    if (m_nDynUnk74 != 0)
        return;

    void* pool = Core::StaticRef<void*>(0x16B775C);
    uint32* node = Core::Hook::CallMethod<uint32*>(0x96D520, pool);
    if (node)
        node[0] = reinterpret_cast<uint32>(this);

    m_nDynUnk74 = reinterpret_cast<uint32>(node);
    node[2] = 0;

    uint32*& head = Core::StaticRef<uint32*>(0x11D286C);
    node[1] = reinterpret_cast<uint32>(head);
    if (head)
        head[2] = reinterpret_cast<uint32>(node);
    head = node;
}

void __thiscall CDynamicEntity::UpdateAnim()
{
    m_nDynUnkF4 = (m_nDynUnkF4 & 0x9F) | static_cast<uint8>((m_nDynUnkF4 & 0x20) << 1);

    m_nDynUnkE0 = Core::StaticRef<uint32>(0x18A8C60);
    m_nDynUnkE4 = Core::StaticRef<uint32>(0x18A8C64);
    m_nDynUnkE8 = Core::StaticRef<uint32>(0x18A8C68);
    m_nDynUnkEC = Core::StaticRef<uint32>(0x18A8C6C);

    m_nDynUnkF4 &= 0xEF;
    m_nDynUnkF0 = 0;

    const bool hasPending = (m_nDynUnkF4 & 0x84) != 0;
    m_nDynUnkF4 &= 0x7F;

    using FlagFn = bool(__thiscall*)(CDynamicEntity*);
    if (!hasPending)
    {
        const uint32 entType = (m_nFlags2 >> 6) & 0xF;
        if (entType != 3 && entType != 6)
        {
            if (!reinterpret_cast<FlagFn>(m_vftable[9])(this) &&
                !reinterpret_cast<FlagFn>(m_vftable[10])(this) &&
                !reinterpret_cast<FlagFn>(m_vftable[11])(this))
            {
                return;
            }
        }
    }

    if (Core::Hook::Call<bool>(0x7955D0))
        return;

    if (!GetEntitySkeleton(this) || !m_nDynUnk78 || !(m_nDynUnkF4 & 0x1))
        return;

    float delta = 0.0f;
    if (!reinterpret_cast<FlagFn>(m_vftable[9])(this) &&
        !reinterpret_cast<FlagFn>(m_vftable[11])(this) &&
        !reinterpret_cast<FlagFn>(m_vftable[10])(this))
    {
        delta = Core::StaticRef<float>(0x11DDE7C);
    }

    if (Core::StaticRef<uint8>(0x10C7CD0) != 0)
        delta = Core::StaticRef<float>(0x11DDE8C);

    Core::Hook::CallMethod<void>(0x9EFB80, reinterpret_cast<void*>(m_nDynUnk78), this, delta);
}

void __thiscall CDynamicEntity::CreateAnimBlender(void* arg)
{
    void* pool = Core::StaticRef<void*>(0x16397B0);
    void* obj = Core::Hook::CallMethod<void*>(0x96D520, pool);
    if (obj)
        obj = Core::Hook::CallMethod<void*>(0x9EED30, obj, this, arg);

    m_nDynUnkF4 |= 0x1;
    m_nDynUnk78 = reinterpret_cast<uint32>(obj);
}

void __thiscall CDynamicEntity::InitCrSkeletonOrAnim(void* arg)
{
    uint8* modelInfo = GetModelInfoPtr(m_nModelIndex);
    const int32 animIndex = *reinterpret_cast<int16*>(modelInfo + 0x58);
    if (animIndex != -1 && Core::Hook::Call<bool>(0xC8AC60, animIndex))
    {
        const uint32 animRes = Core::Hook::Call<uint32>(0xC8AEF0, animIndex);
        m_nDynUnkD4 = animRes;
        if (animRes != 0)
            AddAnimRef(animIndex);
    }

    void* mem = Core::Hook::Call<void*>(0x4011D0, 0x14);
    void* obj = nullptr;
    if (mem)
    {
        void* skel = GetEntitySkeleton(this);
        obj = Core::Hook::CallMethod<void*>(0x59A1C0, mem, arg, reinterpret_cast<void*>(m_nDynUnkD4), skel, 0, 0);
    }

    m_nDynUnkD0 = reinterpret_cast<uint32>(obj);

    uint8* player = Core::Hook::Call<uint8*>(0x59B9E0, 4);
    Core::Hook::CallMethod<void>(0x59B760, player, obj, 0);

    void* slotC = *reinterpret_cast<void**>(player + 0xC);
    Core::Hook::CallMethod<void>(0x58D270, slotC, 0, 1, 0);
    Core::Hook::CallMethod<void>(0x58D670, slotC, 0xF1, 0, 1);
    Core::Hook::CallMethod<void>(0x58D550, slotC, 0);
    Core::Hook::CallMethod<void>(0x58D8D0, slotC, 0xF1, 0);

    void* slot10 = *reinterpret_cast<void**>(player + 0x10);
    Core::Hook::CallMethod<void>(0x58D270, slot10, 0, 1, 0);
    Core::Hook::CallMethod<void>(0x58D550, slot10, 0);

    Core::Hook::CallMethod<void>(0x59A6B0, obj, player);
    uint8* node = Core::Hook::CallMethod<uint8*>(0x59A640, obj, 1);
    if (node)
        node[0x10] = 1;
}

void __thiscall CDynamicEntity::ReleaseCrSkeletonOrAnim()
{
    if (m_nDynUnkD4 != 0)
    {
        uint8* modelInfo = GetModelInfoPtr(m_nModelIndex);
        const int32 animIndex = *reinterpret_cast<int16*>(modelInfo + 0x58);
        if (animIndex != -1)
            ReleaseAnimRef(animIndex);
    }

    m_nDynUnkD4 = 0;
    void* obj = reinterpret_cast<void*>(m_nDynUnkD0);
    if (obj)
    {
        Core::Hook::CallMethod<void>(0x59A210, obj);
        Core::Hook::Call<void>(0x5B1C10, obj);
    }
    m_nDynUnkD0 = 0;
}

void __thiscall CDynamicEntity::RemoveFromWorldSectors()
{
    if ((m_nFlags2 & 0x3C0) == 0x40)
    {
        Core::Hook::CallMethod<void>(0x9EA680, this);
        return;
    }

    void* pool1 = Core::StaticRef<void*>(0x1706E98);
    void* pool2 = Core::StaticRef<void*>(0x16B775C);

    uint32* node = reinterpret_cast<uint32*>(m_pAnimBlender);
    while (node)
    {
        uint32** listHead = reinterpret_cast<uint32**>(node[0]);
        uint32* entry = reinterpret_cast<uint32*>(node[1]);
        UnlinkSectorEntry(listHead, entry);
        Core::Hook::CallMethod<void>(0xC79660, pool2, entry);

        uint32* cur = node;
        node = reinterpret_cast<uint32*>(node[4]);
        if (m_pAnimBlender == cur)
            m_pAnimBlender = node;
        if (cur[3])
            *reinterpret_cast<uint32*>(cur[3] + 0x10) = reinterpret_cast<uint32>(node);
        if (node)
            node[3] = cur[3];

        Core::Hook::CallMethod<void>(0xC79660, pool1, cur);
    }
}

void __thiscall CDynamicEntity::UpdateWorldSectors()
{
    if (m_nFlags & 0x8000000)
        return;

    if ((m_nFlags2 & 0x3C0) == 0x40)
    {
        Core::Hook::CallMethod<void>(0x9EA680, this);
        Core::Hook::CallMethod<void>(0x9E6D80, this);
        return;
    }

    uint32* node = reinterpret_cast<uint32*>(m_pAnimBlender);
    if (!node)
        return;

    int32 minY = 0, minX = 0, maxY = 0, maxX = 0;
    Core::Hook::CallMethod<void>(0x9E0090, this, &minY, &minX, &maxY, &maxX);

    void* pool1 = Core::StaticRef<void*>(0x1706E98);
    void* pool2 = Core::StaticRef<void*>(0x16B775C);

    for (int32 x = minX; x <= maxX; ++x)
    {
        for (int32 y = minY; y <= maxY; ++y)
        {
            uint8* sector = nullptr;
            uint32** listHead = nullptr;
            GetSectorAndListHead(x, y, m_nFlags2, sector, listHead);

            if (node)
            {
                uint32** oldHead = reinterpret_cast<uint32**>(node[0]);
                uint32* entry = reinterpret_cast<uint32*>(node[1]);
                UnlinkSectorEntry(oldHead, entry);
                LinkSectorEntry(listHead, entry);

                node[2] = reinterpret_cast<uint32>(sector);
                node[0] = reinterpret_cast<uint32>(listHead);
                node = reinterpret_cast<uint32*>(node[4]);
            }
            else
            {
                uint32* entry = Core::Hook::CallMethod<uint32*>(0x96D520, pool2);
                if (entry)
                    entry[0] = reinterpret_cast<uint32>(this);
                LinkSectorEntry(listHead, entry);

                uint32* newNode = Core::Hook::CallMethod<uint32*>(0x96D520, pool1);
                if (newNode)
                {
                    newNode[2] = reinterpret_cast<uint32>(sector);
                    newNode[0] = reinterpret_cast<uint32>(listHead);
                    newNode[1] = reinterpret_cast<uint32>(entry);
                }

                newNode[3] = 0;
                newNode[4] = reinterpret_cast<uint32>(m_pAnimBlender);
                if (m_pAnimBlender)
                    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pAnimBlender) + 12) = reinterpret_cast<uint32>(newNode);
                m_pAnimBlender = newNode;
            }
        }
    }

    while (node)
    {
        uint32** listHead = reinterpret_cast<uint32**>(node[0]);
        uint32* entry = reinterpret_cast<uint32*>(node[1]);
        UnlinkSectorEntry(listHead, entry);
        Core::Hook::CallMethod<void>(0xC79660, pool2, entry);

        uint32* cur = node;
        node = reinterpret_cast<uint32*>(node[4]);
        if (m_pAnimBlender == cur)
            m_pAnimBlender = node;
        if (cur[3])
            *reinterpret_cast<uint32*>(cur[3] + 0x10) = reinterpret_cast<uint32>(node);
        if (node)
            node[3] = cur[3];

        Core::Hook::CallMethod<void>(0xC79660, pool1, cur);
    }
}

void __thiscall CDynamicEntity::RemoveFromDynList()
{
    uint32* node = reinterpret_cast<uint32*>(m_nDynUnk74);
    if (!node)
        return;

    uint32*& head = Core::StaticRef<uint32*>(0x11D286C);
    if (head == node)
        head = reinterpret_cast<uint32*>(node[1]);
    if (node[2])
        *reinterpret_cast<uint32*>(node[2] + 4) = node[1];
    if (node[1])
        *reinterpret_cast<uint32*>(node[1] + 8) = node[2];

    void* pool = Core::StaticRef<void*>(0x16B775C);
    Core::Hook::CallMethod<void>(0xC79660, pool, node);
    m_nDynUnk74 = 0;
}

void __thiscall CDynamicEntity::DeleteAnimBlender()
{
    void* blender = reinterpret_cast<void*>(m_nDynUnk78);
    if (blender)
    {
        Core::Hook::CallMethod<void>(0x9EFA80, blender);
        void* pool = Core::StaticRef<void*>(0x16397B0);
        Core::Hook::CallMethod<void>(0xC79660, pool, blender);
    }
    m_nDynUnk78 = 0;
}

void __thiscall CDynamicEntity::CleanupDynamicData()
{
    m_nDynUnkF4 &= 0xFE;
    DeleteAnimBlender();
    Core::Hook::CallMethod<void>(0x9E6A80, this);
    ReleaseCrSkeletonOrAnim();
    DeleteSkeleton();
}

CDynamicEntity* __thiscall CDynamicEntity::ScalarDeletingDtor(uint8 flags)
{
    Dtor();
    if (flags & 1)
        Core::Hook::Call<void>(0x5B1C10, this);
    return this;
}
