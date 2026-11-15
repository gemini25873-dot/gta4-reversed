#pragma once
#include "Core/Matrix.h"

namespace rage
{
    class phCollider;
    class phLevelNew;

    class phSimulator
    {
    public:
        void**      m_vftable = nullptr;        // +0x00
        phLevelNew* m_pLevel = nullptr;         // +0x04
        void*       m_pManifoldMgr = nullptr;   // +0x08
        void*       m_pUnk0C = nullptr;         // +0x0C
        void*       m_pUnk10 = nullptr;         // +0x10
        void*       m_pUnk14 = nullptr;         // +0x14
        uint32      m_nMaxColliders = 0;        // +0x18
        uint32      m_nUnk1C = 0;               // +0x1C
        phCollider* m_pColliders = nullptr;     // +0x20
        uint8*      m_pColliderFlags = nullptr; // +0x24
        void*       m_pUnk28 = nullptr;         // +0x28
        void*       m_pUnk2C = nullptr;         // +0x2C
        void*       m_pUnk30 = nullptr;         // +0x30
        uint16      m_nUnk34 = 0;               // +0x34
        uint16      m_nUnk36 = 0;               // +0x36
        uint32      m_nUnk38Count = 0;          // +0x38
        uint16*     m_pUnk3C = nullptr;         // +0x3C
        uint32      m_nUnk40 = 0;               // +0x40
        void*       m_pUnk44 = nullptr;         // +0x44
        uint8       m_pad48[0x60]{};            // +0x48 (6x 16-byte callback/functor slots)
        void*       m_pArrayA8 = nullptr;       // +0xA8
        uint16      m_nCountAC = 0;             // +0xAC
        uint16      m_nCapacityAE = 0;          // +0xAE
        void*       m_pArrayB0 = nullptr;       // +0xB0
        uint16      m_nCountB4 = 0;             // +0xB4
        uint16      m_nCapacityB6 = 0;          // +0xB6
        void*       m_pArrayB8 = nullptr;       // +0xB8
        uint16      m_nCountBC = 0;             // +0xBC
        uint16      m_nCapacityBE = 0;          // +0xBE

        static void InjectHooks();

        phSimulator* __thiscall DeletingDtor(uint8 flags);
    };

    VALIDATE_SIZE(phSimulator, 0xC0);
    VALIDATE_OFFSET(phSimulator, m_vftable, 0x00);
    VALIDATE_OFFSET(phSimulator, m_pLevel, 0x04);
    VALIDATE_OFFSET(phSimulator, m_pManifoldMgr, 0x08);
    VALIDATE_OFFSET(phSimulator, m_pUnk0C, 0x0C);
    VALIDATE_OFFSET(phSimulator, m_pUnk10, 0x10);
    VALIDATE_OFFSET(phSimulator, m_pUnk14, 0x14);
    VALIDATE_OFFSET(phSimulator, m_nMaxColliders, 0x18);
    VALIDATE_OFFSET(phSimulator, m_nUnk1C, 0x1C);
    VALIDATE_OFFSET(phSimulator, m_pColliders, 0x20);
    VALIDATE_OFFSET(phSimulator, m_pColliderFlags, 0x24);
    VALIDATE_OFFSET(phSimulator, m_pUnk28, 0x28);
    VALIDATE_OFFSET(phSimulator, m_pUnk2C, 0x2C);
    VALIDATE_OFFSET(phSimulator, m_pUnk30, 0x30);
    VALIDATE_OFFSET(phSimulator, m_nUnk34, 0x34);
    VALIDATE_OFFSET(phSimulator, m_nUnk36, 0x36);
    VALIDATE_OFFSET(phSimulator, m_nUnk38Count, 0x38);
    VALIDATE_OFFSET(phSimulator, m_pUnk3C, 0x3C);
    VALIDATE_OFFSET(phSimulator, m_nUnk40, 0x40);
    VALIDATE_OFFSET(phSimulator, m_pUnk44, 0x44);
    VALIDATE_OFFSET(phSimulator, m_pad48, 0x48);
    VALIDATE_OFFSET(phSimulator, m_pArrayA8, 0xA8);
    VALIDATE_OFFSET(phSimulator, m_nCountAC, 0xAC);
    VALIDATE_OFFSET(phSimulator, m_nCapacityAE, 0xAE);
    VALIDATE_OFFSET(phSimulator, m_pArrayB0, 0xB0);
    VALIDATE_OFFSET(phSimulator, m_nCountB4, 0xB4);
    VALIDATE_OFFSET(phSimulator, m_nCapacityB6, 0xB6);
    VALIDATE_OFFSET(phSimulator, m_pArrayB8, 0xB8);
    VALIDATE_OFFSET(phSimulator, m_nCountBC, 0xBC);
    VALIDATE_OFFSET(phSimulator, m_nCapacityBE, 0xBE);
}
