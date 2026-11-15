#pragma once

#include "Rage/phBound.h"

namespace rage
{
    class phBoundComposite : public phBound
    {
    public:
        phBound** m_ppBounds = nullptr;              // +0x80 (128)
        void*     m_pCurrentMatrices = nullptr;      // +0x84 (132)
        void*     m_pLastMatrices = nullptr;         // +0x88 (136)
        void*     m_pBoundLocalBBoxes = nullptr;     // +0x8C (140)
        uint16    m_nNumBounds = 0;                  // +0x90 (144)
        uint16    m_nUnk92 = 0;                      // +0x92 (146)
        void*     m_pTypeAndIncludeFlags = nullptr;  // +0x94 (148)
        uint64    m_nUnk98 = 0;                      // +0x98 (152)

        static void InjectHooks();

        // Note: 0x688670 (sub_688670) is __usercall(esi = this), not __thiscall.
        // Do not hook 0x688670 directly with Core::Hook::Install.
        void Dtor();

        // 0x6883C0 (__thiscall, retn)
        phBoundComposite* __thiscall Ctor();
        // 0x688410 (__thiscall, retn 4) - vftable[0]
        phBoundComposite* __thiscall DeletingDtor(uint8 flags);

        // 0x686780 (__thiscall, retn)
        int32 __thiscall GetBoundType() const;
        // 0x686240 (__thiscall, retn)
        int32 __thiscall IsDefaultType() const;
        // 0x6867F0 (__thiscall, retn 4)
        uint32 __thiscall SetUnk70(int32 val);
        // 0x6848F0 (__thiscall, retn 4)
        uint8 __thiscall SetUnk07(uint8 val);
        // 0x684900 (__thiscall, retn)
        uint8 __thiscall GetUnk07() const;
        // 0x68C020 (__thiscall, retn)
        int32 __thiscall GetFirstBoundVFunc21();
        // 0x68BF70 (__thiscall, retn 4)
        int32 __thiscall VFunc52Wrapper(int32 arg);
        // 0x68BFD0 (__thiscall, retn 4)
        int32 __thiscall VFunc53Wrapper(int32 arg);
    };

    VALIDATE_SIZE(phBoundComposite, 0xA0);
    VALIDATE_OFFSET(phBoundComposite, m_ppBounds, 0x80);
    VALIDATE_OFFSET(phBoundComposite, m_pCurrentMatrices, 0x84);
    VALIDATE_OFFSET(phBoundComposite, m_pLastMatrices, 0x88);
    VALIDATE_OFFSET(phBoundComposite, m_pBoundLocalBBoxes, 0x8C);
    VALIDATE_OFFSET(phBoundComposite, m_nNumBounds, 0x90);
    VALIDATE_OFFSET(phBoundComposite, m_nUnk92, 0x92);
    VALIDATE_OFFSET(phBoundComposite, m_pTypeAndIncludeFlags, 0x94);
    VALIDATE_OFFSET(phBoundComposite, m_nUnk98, 0x98);
}
