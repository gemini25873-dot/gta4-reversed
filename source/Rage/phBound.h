#pragma once

#include "Core/Vector.h"

namespace rage
{
    enum class eBoundType : uint8
    {
        BOUND_SPHERE           = 0,
        BOUND_CAPSULE          = 1,
        BOUND_BOX              = 3,
        BOUND_GEOMETRY         = 4,
        BOUND_BVH              = 8,
        BOUND_COMPOSITE        = 10,
        BOUND_INVALID          = 0xFF,
    };

    class phBound
    {
    public:
        void**  m_vftable = nullptr;               // +0x00 (rage::phBound::`vftable' @ 0x00EBB9CC)
        uint8   m_nType = 0xFF;                    // +0x04 (0=Sphere, 1=Capsule, 3=Box)
        uint8   m_nFlags = 0;                      // +0x05
        uint8   m_nUnk06 = 0;                      // +0x06
        uint8   m_nUnk07 = 0;                      // +0x07
        float   m_fRadiusAroundCentroid = 0.0f;    // +0x08
        float   m_fMargin = 0.0f;                  // +0x0C
        CVector m_vecBoundingBoxMax{};             // +0x10..+0x1B
        float   m_fPad1C = 0.0f;                   // +0x1C
        CVector m_vecBoundingBoxMin{};             // +0x20..+0x2B
        float   m_fPad2C = 0.0f;                   // +0x2C
        CVector m_vecCentroidOffset{};             // +0x30..+0x3B
        float   m_fPad3C = 0.0f;                   // +0x3C
        CVector m_vecCenterOfGravityOffset{};      // +0x40..+0x4B
        float   m_fPad4C = 0.0f;                   // +0x4C
        CVector m_vecUnk50{};                      // +0x50..+0x5B
        float   m_fPad5C = 0.0f;                   // +0x5C
        float   m_vecUnk60[4]{};                   // +0x60..+0x6F
        CVector m_vecUnk70{};                      // +0x70..+0x7B
        uint32  m_nRefCount = 1;                   // +0x7C

        static void InjectHooks();

        // Note: 0x684B20 (sub_684B20) is __usercall(eax = this), not __thiscall.
        // Do not hook 0x684B20 directly with Core::Hook::Install.
        void InitBase();

        eBoundType GetType() const { return static_cast<eBoundType>(m_nType); }
        uint32 GetRefCount() const { return m_nRefCount; }
    };

    VALIDATE_SIZE(phBound, 0x80);
    VALIDATE_OFFSET(phBound, m_vftable, 0x00);
    VALIDATE_OFFSET(phBound, m_nType, 0x04);
    VALIDATE_OFFSET(phBound, m_fRadiusAroundCentroid, 0x08);
    VALIDATE_OFFSET(phBound, m_fMargin, 0x0C);
    VALIDATE_OFFSET(phBound, m_vecBoundingBoxMax, 0x10);
    VALIDATE_OFFSET(phBound, m_vecBoundingBoxMin, 0x20);
    VALIDATE_OFFSET(phBound, m_vecCentroidOffset, 0x30);
    VALIDATE_OFFSET(phBound, m_vecCenterOfGravityOffset, 0x40);
    VALIDATE_OFFSET(phBound, m_vecUnk50, 0x50);
    VALIDATE_OFFSET(phBound, m_vecUnk60, 0x60);
    VALIDATE_OFFSET(phBound, m_vecUnk70, 0x70);
    VALIDATE_OFFSET(phBound, m_nRefCount, 0x7C);

    class phBoundSphere : public phBound
    {
    public:
        CVector m_vecSphereExtents{};              // +0x80..+0x8B (radius, radius, radius)
        float   m_fPad8C = 0.0f;                   // +0x8C
        uint32  m_nUnk90 = 0;                      // +0x90
        uint32  m_nUnk94 = 0;                      // +0x94
        uint32  m_nUnk98 = 0;                      // +0x98
        uint32  m_nUnk9C = 0;                      // +0x9C

        static void InjectHooks();

        // 0x6D4890 (__thiscall, retn 4)
        phBoundSphere* __thiscall Ctor(float radius);
    };

    VALIDATE_SIZE(phBoundSphere, 0xA0);
    VALIDATE_OFFSET(phBoundSphere, m_vecSphereExtents, 0x80);
    VALIDATE_OFFSET(phBoundSphere, m_nUnk90, 0x90);
    VALIDATE_OFFSET(phBoundSphere, m_nUnk9C, 0x9C);

    class phBoundCapsule : public phBound
    {
    public:
        CVector m_vecUnk80{};                      // +0x80..+0x8B
        float   m_fPad8C = 0.0f;                   // +0x8C
        CVector m_vecUnk90{};                      // +0x90..+0x9B
        float   m_fPad9C = 0.0f;                   // +0x9C
        float   m_vecUnkA0[4]{};                   // +0xA0..+0xAF
        float   m_vecUnkB0[4]{};                   // +0xB0..+0xBF
        CVector m_vecUnkC0{};                      // +0xC0..+0xCB
        float   m_fPadCC = 0.0f;                   // +0xCC
        uint32  m_nUnkD0 = 0;                      // +0xD0
        uint32  m_nUnkD4 = 0;                      // +0xD4
        uint32  m_nUnkD8 = 0;                      // +0xD8
        uint32  m_nUnkDC = 0;                      // +0xDC

        static void InjectHooks();

        // 0x68E140 (__thiscall, retn)
        phBoundCapsule* __thiscall Ctor();
    };

    VALIDATE_SIZE(phBoundCapsule, 0xE0);
    VALIDATE_OFFSET(phBoundCapsule, m_vecUnk80, 0x80);
    VALIDATE_OFFSET(phBoundCapsule, m_vecUnk90, 0x90);
    VALIDATE_OFFSET(phBoundCapsule, m_vecUnkA0, 0xA0);
    VALIDATE_OFFSET(phBoundCapsule, m_vecUnkB0, 0xB0);
    VALIDATE_OFFSET(phBoundCapsule, m_vecUnkC0, 0xC0);
    VALIDATE_OFFSET(phBoundCapsule, m_nUnkD0, 0xD0);
    VALIDATE_OFFSET(phBoundCapsule, m_nUnkDC, 0xDC);

    class phBoundBox : public phBound
    {
    public:
        uint8   m_polyPad80[0x0C]{};               // +0x80..+0x8B (phBoundPolyhedron base fields)
        void*   m_pPolys = nullptr;                // +0x8C (points to m_aPolys)
        uint8   m_polyPad90[0x20]{};               // +0x90..+0xAF
        void*   m_pUnkB0 = nullptr;                // +0xB0 (freed via sysMemAllocator in Dtor)
        uint8   m_polyPadB4[0x14]{};               // +0xB4..+0xC7
        uint32  m_nNumVerts = 8;                   // +0xC8
        uint32  m_nNumPolys = 6;                   // +0xCC
        CVector m_vecHalfExtents{};                // +0xD0..+0xDB
        float   m_fPadDC = 0.0f;                   // +0xDC
        uint8   m_aVerts[0x80]{};                  // +0xE0..+0x15F (8 verts @ +0xF0..+0x158)
        uint8   m_aPolys[0xC0]{};                  // +0x160..+0x21F (6 * 0x20-byte polys)
        uint32  m_nUnk220 = 0;                     // +0x220

        static void InjectHooks();

        // 0x6D7D60 (__thiscall, retn 4) - vftable[0]
        phBoundBox* __thiscall DeletingDtor(uint8 flags);
        // 0x6D7DB0 (__thiscall, retn)
        void __thiscall Dtor();
    };

    VALIDATE_OFFSET(phBoundBox, m_pPolys, 0x8C);
    VALIDATE_OFFSET(phBoundBox, m_pUnkB0, 0xB0);
    VALIDATE_OFFSET(phBoundBox, m_nNumVerts, 0xC8);
    VALIDATE_OFFSET(phBoundBox, m_nNumPolys, 0xCC);
    VALIDATE_OFFSET(phBoundBox, m_vecHalfExtents, 0xD0);
    VALIDATE_OFFSET(phBoundBox, m_aPolys, 0x160);
    VALIDATE_OFFSET(phBoundBox, m_nUnk220, 0x220);
}
