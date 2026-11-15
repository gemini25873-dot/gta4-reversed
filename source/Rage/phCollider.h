#pragma once
#include "Core/Matrix.h"

namespace rage
{
    class phInst;

    class phCollider
    {
    public:
        void**  m_vftable = nullptr;                            // +0x000
        uint8   m_pad04[0x0C]{};                                // +0x004
        phInst* m_pInstance = nullptr;                          // +0x010
        int32   m_nUnk14 = 0;                                   // +0x014
        int32   m_nUnk18 = 0;                                   // +0x018
        uint8   m_pad1C[0x84]{};                                // +0x01C
        CVector m_vAngInertia{ 1.0f, 1.0f, 1.0f };              // +0x0A0
        float   m_fPadAC = 0.0f;                                // +0x0AC
        CVector m_vInvAngInertia{ 1.0f, 1.0f, 1.0f };           // +0x0B0
        float   m_fPadBC = 0.0f;                                // +0x0BC
        float   m_fMass = 1.0f;                                 // +0x0C0
        float   m_fInvMass = 1.0f;                              // +0x0C4
        float   m_fMaxSpeed = 80.0f;                            // +0x0C8
        float   m_fMaxAngSpeed = 6.2831854820251465f;           // +0x0CC
        CMatrix m_matCollider{};                                // +0x0D0
        CVector m_vUnk110{};                                    // +0x110
        float   m_fPad11C = 0.0f;                               // +0x11C
        CVector m_vUnk120{};                                    // +0x120
        float   m_fPad12C = 0.0f;                               // +0x12C
        CVector m_vUnk130{};                                    // +0x130
        float   m_fPad13C = 0.0f;                               // +0x13C
        CVector m_vUnk140{};                                    // +0x140
        float   m_fPad14C = 0.0f;                               // +0x14C
        uint8   m_pad150[0x40]{};                               // +0x150
        CVector m_vUnk190{};                                    // +0x190
        float   m_fPad19C = 0.0f;                               // +0x19C
        uint8   m_pad1A0[0x10]{};                               // +0x1A0
        CVector m_vUnk1B0{};                                    // +0x1B0
        float   m_fPad1BC = 0.0f;                               // +0x1BC
        uint8   m_pad1C0[0xD2]{};                               // +0x1C0
        uint8   m_nUnk292 = 0;                                  // +0x292
        uint8   m_nUnk293 = 0;                                  // +0x293
        uint8   m_nUnk294 = 0;                                  // +0x294
        uint8   m_pad295[0x07]{};                               // +0x295
        int32   m_nUnk29C = 0;                                  // +0x29C

        static void InjectHooks();

        void        __thiscall Dtor();
        phCollider* __thiscall DeletingDtor(uint8 flags);
        int32       __thiscall Init(int32 a2, int32 a3);
        int32       __thiscall FixupInstance(uintptr_t fixupTable);
        int32       __thiscall Unk69EF20(int32 a2, int32 a3);
        float*      __thiscall GetInvMassMatrix(float* outMat, int32 a3);
        int32       __thiscall SetUnk69D450(float dt);

        static double __stdcall ReturnZeroDouble(int32 a1);
    };

    VALIDATE_SIZE(phCollider, 0x2A0);
    VALIDATE_OFFSET(phCollider, m_vftable, 0x000);
    VALIDATE_OFFSET(phCollider, m_pInstance, 0x010);
    VALIDATE_OFFSET(phCollider, m_nUnk14, 0x014);
    VALIDATE_OFFSET(phCollider, m_nUnk18, 0x018);
    VALIDATE_OFFSET(phCollider, m_vAngInertia, 0x0A0);
    VALIDATE_OFFSET(phCollider, m_vInvAngInertia, 0x0B0);
    VALIDATE_OFFSET(phCollider, m_fMass, 0x0C0);
    VALIDATE_OFFSET(phCollider, m_fInvMass, 0x0C4);
    VALIDATE_OFFSET(phCollider, m_fMaxSpeed, 0x0C8);
    VALIDATE_OFFSET(phCollider, m_fMaxAngSpeed, 0x0CC);
    VALIDATE_OFFSET(phCollider, m_matCollider, 0x0D0);
    VALIDATE_OFFSET(phCollider, m_vUnk110, 0x110);
    VALIDATE_OFFSET(phCollider, m_vUnk120, 0x120);
    VALIDATE_OFFSET(phCollider, m_vUnk130, 0x130);
    VALIDATE_OFFSET(phCollider, m_vUnk140, 0x140);
    VALIDATE_OFFSET(phCollider, m_vUnk190, 0x190);
    VALIDATE_OFFSET(phCollider, m_vUnk1B0, 0x1B0);
    VALIDATE_OFFSET(phCollider, m_nUnk292, 0x292);
    VALIDATE_OFFSET(phCollider, m_nUnk293, 0x293);
    VALIDATE_OFFSET(phCollider, m_nUnk294, 0x294);
    VALIDATE_OFFSET(phCollider, m_nUnk29C, 0x29C);
}
