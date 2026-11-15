#pragma once

#include "Core/Vector.h"

namespace rage
{
    class phBound;

    class phArchetypeBase
    {
    public:
        void** m_vftable = nullptr; // +0x00

        phArchetypeBase* __thiscall DeletingDtor(uint8 flags);
    };

    VALIDATE_SIZE(phArchetypeBase, 0x04);
    VALIDATE_OFFSET(phArchetypeBase, m_vftable, 0x00);

    class phArchetype : public phArchetypeBase
    {
    public:
        enum eArchetypeType : int32
        {
            ARCHETYPE_BASE = 0,
            ARCHETYPE_PHYS = 1,
            ARCHETYPE_DAMP = 2,
            ARCHETYPE_GTA  = 3,
        };

        int32       m_nType          = ARCHETYPE_BASE; // +0x04
        const char* m_pszName        = nullptr;        // +0x08
        phBound*    m_pBound         = nullptr;        // +0x0C
        uint32      m_nTypeFlags     = 1;              // +0x10
        uint32      m_nIncludeFlags  = 0xFFFFFFFFu;    // +0x14
        uint16      m_nPropertyFlags = 0;              // +0x18
        uint16      m_nRefCount      = 0;              // +0x1A

        static void InjectHooks();

        void         __thiscall Dtor();
        phArchetype* __thiscall DeletingDtor(uint8 flags);
        void         __thiscall Shutdown();
        void         __thiscall SetBound(phBound* bound);

        phBound* GetBound() const { return m_pBound; }
    };

    VALIDATE_SIZE(phArchetype, 0x1C);
    VALIDATE_OFFSET(phArchetype, m_nType, 0x04);
    VALIDATE_OFFSET(phArchetype, m_pszName, 0x08);
    VALIDATE_OFFSET(phArchetype, m_pBound, 0x0C);
    VALIDATE_OFFSET(phArchetype, m_nTypeFlags, 0x10);
    VALIDATE_OFFSET(phArchetype, m_nIncludeFlags, 0x14);
    VALIDATE_OFFSET(phArchetype, m_nPropertyFlags, 0x18);
    VALIDATE_OFFSET(phArchetype, m_nRefCount, 0x1A);

    class phArchetypePhys : public phArchetype
    {
    public:
        float   m_fMass          = 1.0f;                 // +0x1C
        float   m_fInvMass       = 1.0f;                 // +0x20
        float   m_fGravityFactor = 1.0f;                 // +0x24
        float   m_fMaxSpeed      = 80.0f;                // +0x28
        float   m_fMaxAngSpeed   = 6.2831855f;           // +0x2C (2 * PI)
        CVector m_vAngInertia{ 1.0f, 1.0f, 1.0f };       // +0x30..+0x3B
        float   m_fPad3C         = 0.0f;                 // +0x3C
        CVector m_vInvAngInertia{ 1.0f, 1.0f, 1.0f };    // +0x40..+0x4B
        float   m_fPad4C         = 0.0f;                 // +0x4C

        phArchetypePhys* __thiscall Ctor();
        float            __thiscall GetMass() const;
        float            __thiscall GetInvMass() const;
        float            __thiscall GetGravityFactor() const;
        void             __thiscall SetGravityFactor(float factor);
        float            __thiscall GetMaxSpeed() const;
        float            __thiscall GetMaxAngSpeed() const;
    };

    VALIDATE_SIZE(phArchetypePhys, 0x50);
    VALIDATE_OFFSET(phArchetypePhys, m_fMass, 0x1C);
    VALIDATE_OFFSET(phArchetypePhys, m_fInvMass, 0x20);
    VALIDATE_OFFSET(phArchetypePhys, m_fGravityFactor, 0x24);
    VALIDATE_OFFSET(phArchetypePhys, m_fMaxSpeed, 0x28);
    VALIDATE_OFFSET(phArchetypePhys, m_fMaxAngSpeed, 0x2C);
    VALIDATE_OFFSET(phArchetypePhys, m_vAngInertia, 0x30);
    VALIDATE_OFFSET(phArchetypePhys, m_vInvAngInertia, 0x40);

    class phArchetypeDamp : public phArchetypePhys
    {
    public:
        float m_avDamping[6][4]{};    // +0x50..+0xAF (6 x Vec4)
        uint8 m_abDampingActive[6]{}; // +0xB0..+0xB5
        uint8 m_nDampPadB6[10]{};     // +0xB6..+0xBF

        phArchetypeDamp* __thiscall Ctor();
    };

    VALIDATE_SIZE(phArchetypeDamp, 0xC0);
    VALIDATE_OFFSET(phArchetypeDamp, m_avDamping, 0x50);
    VALIDATE_OFFSET(phArchetypeDamp, m_abDampingActive, 0xB0);
}

class phArchetypeGta : public rage::phArchetypeDamp
{
public:
    phArchetypeGta* __thiscall Ctor();
    void            __thiscall Dtor();
    phArchetypeGta* __thiscall DeletingDtor(uint8 flags);
};

VALIDATE_SIZE(phArchetypeGta, 0xC0);
