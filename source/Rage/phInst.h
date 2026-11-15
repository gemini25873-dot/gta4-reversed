#pragma once
#include "Core/Matrix.h"

class CEntity;

namespace rage
{
    class phArchetype;

    class phInst
    {
    public:
        void**       m_vftable = nullptr;       // +0x00
        phArchetype* m_pArchetype = nullptr;    // +0x04
        uint16       m_nLevelIndex = 0xFFFFu;   // +0x08
        uint16       m_nFlags = 0;              // +0x0A
        CEntity*     m_pUserData = nullptr;     // +0x0C
        CMatrix      m_matTransform{};          // +0x10

        static void InjectHooks();

        phInst* __thiscall Ctor();
        void    __thiscall Dtor();
        void    __thiscall GetVelocity(CVector* outVel);
        void    __thiscall GetVelocityAtPoint(const CVector* point, CVector* outVel, int32 componentIndex);
        bool    __thiscall ReturnFalse();
        int32   __thiscall ReturnMinusOne();

        phArchetype*   GetArchetype() const { return m_pArchetype; }
        const CMatrix& GetMatrix() const { return m_matTransform; }
        void           SetMatrix(const CMatrix& mat) { m_matTransform = mat; }
        CEntity*       GetEntity() const { return m_pUserData; }
    };

    VALIDATE_SIZE(phInst, 0x50);
    VALIDATE_OFFSET(phInst, m_vftable, 0x00);
    VALIDATE_OFFSET(phInst, m_pArchetype, 0x04);
    VALIDATE_OFFSET(phInst, m_nLevelIndex, 0x08);
    VALIDATE_OFFSET(phInst, m_nFlags, 0x0A);
    VALIDATE_OFFSET(phInst, m_pUserData, 0x0C);
    VALIDATE_OFFSET(phInst, m_matTransform, 0x10);
}

class phInstGta : public rage::phInst
{
public:
    int32 m_nClassType = 0; // +0x50

    static void InjectHooks();

    phInstGta* __thiscall Ctor(int32 classType);
    phInstGta* __thiscall DeletingDtor(uint8 flags);
    int32      __thiscall GetClassType();
};

VALIDATE_SIZE(phInstGta, 0x54);
VALIDATE_OFFSET(phInstGta, m_nClassType, 0x50);
