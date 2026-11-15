#pragma once

#include "Rage/phBound.h"

namespace rage
{
    class phBoundGeometry : public phBound
    {
    public:
        uint32  m_nUnk80 = 0;          // +0x80
        void*   m_pUnk84 = nullptr;    // +0x84 (this[33], freed in Dtor)
        void*   m_pUnk88 = nullptr;    // +0x88 (this[34], freed in Dtor)
        void*   m_pUnk8C = nullptr;    // +0x8C (this[35], freed in Dtor)
        uint8   m_pad90[0x10]{};       // +0x90..+0x9F
        CVector m_vecUnkA0{};          // +0xA0..+0xAB (this[40..42])
        float   m_fPadAC = 0.0f;       // +0xAC
        void*   m_pUnkB0 = nullptr;    // +0xB0 (this[44], freed in Dtor)
        void*   m_pUnkB4 = nullptr;    // +0xB4 (this[45], freed in Dtor)
        uint8   m_nUnkB8 = 0;          // +0xB8
        uint8   m_padB9[0x03]{};       // +0xB9..+0xBB
        uint16  m_nUnkBC = 0;          // +0xBC
        uint16  m_nUnkBE = 0;          // +0xBE
        uint32  m_nUnkC0 = 0;          // +0xC0
        uint32  m_nUnkC4 = 0;          // +0xC4
        uint32  m_nUnkC8 = 0;          // +0xC8 (num verts)
        uint32  m_nUnkCC = 0;          // +0xCC (num polys)
        void*   m_pUnkD0 = nullptr;    // +0xD0 (this[52], freed in Dtor)
        uint32  m_nUnkD4 = 0;          // +0xD4
        uint8   m_nUnkD8 = 0;          // +0xD8 (this[216])
        uint8   m_padD9[0x07]{};       // +0xD9..+0xDF

        static void InjectHooks();

        // 0x6B8D00 (__thiscall, retn)
        void __thiscall Dtor();
        // 0x6B8CD0 (__thiscall, retn 4) - vftable[0]
        phBoundGeometry* __thiscall DeletingDtor(uint8 flags);
        // 0x6B8FB0 (__thiscall, retn)
        phBoundGeometry* __thiscall Fixup();
        // 0x6B8C50 (__thiscall, retn)
        int32 __thiscall GetUnkD8() const;
        // 0x6BA400 (__thiscall, retn 4)
        int32 __thiscall ShiftCentroidOffset(const CVector* offset);
    };

    VALIDATE_SIZE(phBoundGeometry, 0xE0);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnk80, 0x80);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnk84, 0x84);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnk88, 0x88);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnk8C, 0x8C);
    VALIDATE_OFFSET(phBoundGeometry, m_vecUnkA0, 0xA0);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnkB0, 0xB0);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnkB4, 0xB4);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnkB8, 0xB8);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnkBC, 0xBC);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnkC0, 0xC0);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnkC8, 0xC8);
    VALIDATE_OFFSET(phBoundGeometry, m_pUnkD0, 0xD0);
    VALIDATE_OFFSET(phBoundGeometry, m_nUnkD8, 0xD8);

    class phBoundBVH : public phBoundGeometry
    {
    public:
        void*  m_pBVH = nullptr;       // +0xE0 (this[56])
        uint32 m_nUnkE4[2]{};          // +0xE4..+0xEB (64-bit zeroed in Ctor)
        uint32 m_nUnkEC = 0;           // +0xEC (this[59])

        static void InjectHooks();

        // 0x6C04F0 (__thiscall, retn)
        phBoundBVH* __thiscall Ctor();
        // 0x6C05B0 (__thiscall, jmp sub_6B8D00)
        void __thiscall Dtor();
        // 0x6C0580 (__thiscall, retn 4) - vftable[0]
        phBoundBVH* __thiscall DeletingDtor(uint8 flags);
    };

    VALIDATE_SIZE(phBoundBVH, 0xF0);
    VALIDATE_OFFSET(phBoundBVH, m_pBVH, 0xE0);
    VALIDATE_OFFSET(phBoundBVH, m_nUnkE4, 0xE4);
    VALIDATE_OFFSET(phBoundBVH, m_nUnkEC, 0xEC);
}
