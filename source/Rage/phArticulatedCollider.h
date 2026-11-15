#pragma once
#include "Rage/phCollider.h"

namespace rage
{
    struct phArticulatedArray
    {
        void*  m_pData = nullptr;   // +0x00
        uint16 m_nCount = 0;        // +0x04
        uint16 m_nCapacity = 0;     // +0x06
    };

    VALIDATE_SIZE(phArticulatedArray, 0x08);

    class phArticulatedCollider : public phCollider
    {
    public:
        void*              m_pBody = nullptr;           // +0x2A0 (672)
        uint8              m_pad2A4[0x04]{};            // +0x2A4 (676)
        int32              m_nUnk2A8 = 0;               // +0x2A8 (680)
        phArticulatedArray m_aUnk2AC{};                 // +0x2AC (684)
        phArticulatedArray m_aUnk2B4{};                 // +0x2B4 (692)
        phArticulatedArray m_aUnk2BC{};                 // +0x2BC (700)
        phArticulatedArray m_aUnk2C4{};                 // +0x2C4 (708)
        phArticulatedArray m_aComponentToLink{};        // +0x2CC (716)
        phArticulatedArray m_aUnk2D4{};                 // +0x2D4 (724)
        phArticulatedArray m_aUnk2DC{};                 // +0x2DC (732)
        uint8              m_pad2E4[0x08]{};            // +0x2E4 (740)
        uintptr_t          m_nUnk2EC = 0;               // +0x2EC (748)

        static void InjectHooks();

        // Pure C++ implementation of 0x6FE9E0 (__usercall esi = this)
        void                   Dtor();
        phArticulatedCollider* __thiscall DeletingDtor(uint8 flags);
        int32                  __thiscall FixupInstance(uintptr_t fixupTable);
        float*                 __thiscall GetMatrix3x3_7029D0(float* outMat, int32 componentIdx);
        float*                 __thiscall GetMatrix3x3_702A60(float* outMat, int32 componentIdx);

    private:
        float* CopyLinkMatrix3x3(float* outMat, int32 componentIdx, size_t linkMatOffset);
    };

    VALIDATE_SIZE(phArticulatedCollider, 0x2F0);
    VALIDATE_OFFSET(phArticulatedCollider, m_pBody, 0x2A0);
    VALIDATE_OFFSET(phArticulatedCollider, m_nUnk2A8, 0x2A8);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2AC, 0x2AC);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2B4, 0x2B4);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2BC, 0x2BC);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2C4, 0x2C4);
    VALIDATE_OFFSET(phArticulatedCollider, m_aComponentToLink, 0x2CC);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2D4, 0x2D4);
    VALIDATE_OFFSET(phArticulatedCollider, m_aUnk2DC, 0x2DC);
    VALIDATE_OFFSET(phArticulatedCollider, m_nUnk2EC, 0x2EC);
}
