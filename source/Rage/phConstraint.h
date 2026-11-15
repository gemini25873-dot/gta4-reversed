#pragma once
#include "Core/Vector.h"

namespace DiscreteCollisionDetectorInterface
{
    class Result
    {
    public:
        void** m_vftable = nullptr; // +0x00

        static void InjectHooks();

        Result* __thiscall DeletingDtor(uint8 flags);
    };

    VALIDATE_SIZE(Result, 0x04);
    VALIDATE_OFFSET(Result, m_vftable, 0x00);
}

namespace rage
{
    class phManifoldResult : public DiscreteCollisionDetectorInterface::Result
    {
    public:
        static void InjectHooks();
    };

    VALIDATE_SIZE(phManifoldResult, 0x04);

    class phConstraint
    {
    public:
        void** m_vftable = nullptr; // +0x00
        uint8  m_pad04[0x8C]{};     // +0x04..+0x8F
        float  m_fLength = 0.0f;    // +0x90

        static void InjectHooks();

        phConstraint* __thiscall DeletingDtor(uint8 flags);
        double        __thiscall ComputeNormalAndError(const CVector* pointA, const CVector* pointB, CVector* outNormalAB, CVector* outNormalBA);
    };

    VALIDATE_SIZE(phConstraint, 0x94);
    VALIDATE_OFFSET(phConstraint, m_vftable, 0x00);
    VALIDATE_OFFSET(phConstraint, m_fLength, 0x90);

    class phRopeConstraint : public phConstraint
    {
    public:
        uint8  m_pad94[0xAC]{};                 // +0x94..+0x13F
        void** m_pRopeContactVftable = nullptr; // +0x140

        static void InjectHooks();

        phRopeConstraint* __thiscall DeletingDtor(uint8 flags);
    };

    VALIDATE_SIZE(phRopeConstraint, 0x144);
    VALIDATE_OFFSET(phRopeConstraint, m_pRopeContactVftable, 0x140);

    class phRope
    {
    public:
        void**           m_vftable = nullptr; // +0x00
        uint32           m_nUnk04 = 0;        // +0x04
        void*            m_pData08 = nullptr; // +0x08
        void*            m_pData0C = nullptr; // +0x0C
        phRopeConstraint m_constraint{};      // +0x10

        static void InjectHooks();

        void __thiscall Dtor();
    };

    VALIDATE_SIZE(phRope, 0x154);
    VALIDATE_OFFSET(phRope, m_vftable, 0x00);
    VALIDATE_OFFSET(phRope, m_pData08, 0x08);
    VALIDATE_OFFSET(phRope, m_pData0C, 0x0C);
    VALIDATE_OFFSET(phRope, m_constraint, 0x10);
}
