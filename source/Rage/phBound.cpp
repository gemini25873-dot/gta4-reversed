#include "StdInc.h"
#include "Rage/phBound.h"
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

    CVector ReadGlobalVec3(uintptr_t idaAddr)
    {
        return {
            Core::StaticRef<float>(idaAddr),
            Core::StaticRef<float>(idaAddr + 4),
            Core::StaticRef<float>(idaAddr + 8)
        };
    }

    // 0x6D4A70 (__thiscall)
    void ComputeSphereBounds(rage::phBoundSphere* bound)
    {
        const float r = bound->m_fRadiusAroundCentroid;
        bound->m_vecBoundingBoxMax = { r, r, r };
        bound->m_vecBoundingBoxMin = { -r, -r, -r };
        bound->m_fMargin = r;

        if (bound->m_nFlags != 0)
        {
            const CVector& offset = bound->m_vecCentroidOffset;
            bound->m_vecBoundingBoxMax.x += offset.x;
            bound->m_vecBoundingBoxMax.y += offset.y;
            bound->m_vecBoundingBoxMax.z += offset.z;
            bound->m_vecBoundingBoxMin.x += offset.x;
            bound->m_vecBoundingBoxMin.y += offset.y;
            bound->m_vecBoundingBoxMin.z += offset.z;
            bound->m_fMargin += std::sqrt(
                offset.x * offset.x + offset.y * offset.y + offset.z * offset.z);
        }
    }

    // 0x68E490 (__thiscall)
    void ComputeCapsuleBounds(rage::phBoundCapsule* bound)
    {
        const float radius = bound->m_vecUnk80.x;
        const float halfLen = bound->m_vecUnk90.x * 0.5f;
        const float radiusAroundCentroid = halfLen + radius;

        bound->m_fRadiusAroundCentroid = radiusAroundCentroid;
        bound->m_vecBoundingBoxMax = { radius, radiusAroundCentroid, radius };
        bound->m_vecBoundingBoxMin = { -radius, -radiusAroundCentroid, -radius };
        bound->m_fMargin = radiusAroundCentroid;

        if (bound->m_nFlags != 0)
        {
            const CVector& offset = bound->m_vecCentroidOffset;
            bound->m_vecBoundingBoxMax.x += offset.x;
            bound->m_vecBoundingBoxMax.y += offset.y;
            bound->m_vecBoundingBoxMax.z += offset.z;
            bound->m_vecBoundingBoxMin.x += offset.x;
            bound->m_vecBoundingBoxMin.y += offset.y;
            bound->m_vecBoundingBoxMin.z += offset.z;

            const float absY = std::fabs(offset.y) + halfLen;
            bound->m_fMargin = std::sqrt(
                offset.z * offset.z + absY * absY + offset.x * offset.x) + radius;
        }
    }
}

namespace rage
{
    void phBound::InjectHooks()
    {
        phBoundSphere::InjectHooks();
        phBoundCapsule::InjectHooks();
        phBoundBox::InjectHooks();

        Core::Log::Info("rage::phBound hooks yuklendi (4 fonksiyon)");
    }

    void phBound::InitBase()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB9CC));
        m_nType = 0xFF;
        m_nFlags = 0;
        m_nUnk06 = 0;
        m_nUnk07 = 0;
        m_fRadiusAroundCentroid = 0.0f;
        m_fMargin = 0.0f;

        const CVector defaultVec = ReadGlobalVec3(0x18A8C50);
        m_vecBoundingBoxMax = defaultVec;
        m_vecBoundingBoxMin = defaultVec;
        m_vecCentroidOffset = defaultVec;
        m_vecCenterOfGravityOffset = defaultVec;
        m_vecUnk50 = defaultVec;

        const float unk60 = Core::StaticRef<float>(0xEBDED8);
        m_vecUnk60[0] = unk60;
        m_vecUnk60[1] = unk60;
        m_vecUnk60[2] = unk60;
        m_vecUnk60[3] = unk60;

        m_vecUnk70 = ReadGlobalVec3(0x108D600);
        m_nRefCount = 1;
    }

    void phBoundSphere::InjectHooks()
    {
        Core::Hook::Install(0x6D4890, &phBoundSphere::Ctor);
    }

    phBoundSphere* __thiscall phBoundSphere::Ctor(float radius)
    {
        InitBase();
        m_fRadiusAroundCentroid = radius;
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC48C));
        m_nType = static_cast<uint8>(eBoundType::BOUND_SPHERE);
        m_nUnk90 = 0;
        m_vecSphereExtents = { radius, radius, radius };
        m_vecCentroidOffset = { 0.0f, 0.0f, 0.0f };

        ComputeSphereBounds(this);

        m_nUnk94 = 0;
        m_nUnk98 = 0;
        m_nUnk9C = 0;
        return this;
    }

    void phBoundCapsule::InjectHooks()
    {
        Core::Hook::Install(0x68E140, &phBoundCapsule::Ctor);
    }

    phBoundCapsule* __thiscall phBoundCapsule::Ctor()
    {
        InitBase();
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBBC0C));

        const CVector unkVec = ReadGlobalVec3(0x108A340);
        m_vecUnk80 = unkVec;
        m_vecUnk90 = unkVec;
        m_vecUnkC0 = unkVec;
        m_nUnkD0 = 0;

        for (int i = 0; i < 4; ++i)
        {
            const float val = Core::StaticRef<float>(0x18A8C50 + i * 4);
            m_vecUnkA0[i] = val;
            m_vecUnkB0[i] = val;
        }

        m_nType = static_cast<uint8>(eBoundType::BOUND_CAPSULE);
        m_vecCentroidOffset = { 0.0f, 0.0f, 0.0f };

        ComputeCapsuleBounds(this);

        m_nUnkD4 = 0;
        m_nUnkD8 = 0;
        m_nUnkDC = 0;
        return this;
    }

    void phBoundBox::InjectHooks()
    {
        Core::Hook::Install(0x6D7D60, &phBoundBox::DeletingDtor);
        Core::Hook::Install(0x6D7DB0, &phBoundBox::Dtor);
    }

    phBoundBox* __thiscall phBoundBox::DeletingDtor(uint8 flags)
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC56C));
        if (m_pUnkB0)
            RageFree(m_pUnkB0);

        // Base vftable restored to CollisionShape (0x00EBB998)
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB998));
        if ((flags & 1) != 0)
            RageFree(this);

        return this;
    }

    void __thiscall phBoundBox::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC56C));
        if (m_pUnkB0)
            RageFree(m_pUnkB0);

        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB998));
    }
}
