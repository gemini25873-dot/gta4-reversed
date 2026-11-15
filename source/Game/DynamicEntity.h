#pragma once
#include "Game/Entity.h"

class CDynamicEntity : public CEntity
{
public:
    void*  m_pAnimBlender = nullptr;      // +0x70
    uint32 m_nDynUnk74 = 0;               // +0x74
    uint32 m_nDynUnk78 = 0;               // +0x78
    uint32 m_nDynPad7C = 0;               // +0x7C
    uint8  m_portalTracker[0x50]{};       // +0x80..+0xCF (CPortalTracker)
    uint32 m_nDynUnkD0 = 0;               // +0xD0
    uint32 m_nDynUnkD4 = 0;               // +0xD4
    uint32 m_nDynPadD8 = 0;               // +0xD8
    uint32 m_nDynPadDC = 0;               // +0xDC
    uint32 m_nDynUnkE0 = 0;               // +0xE0
    uint32 m_nDynUnkE4 = 0;               // +0xE4
    uint32 m_nDynUnkE8 = 0;               // +0xE8
    uint32 m_nDynUnkEC = 0;               // +0xEC
    uint32 m_nDynUnkF0 = 0;               // +0xF0
    uint8  m_nDynUnkF4 = 0;               // +0xF4
    uint8  m_nDynPadF5[3]{};              // +0xF5..+0xF7
    uint32 m_nDynUnkF8 = 0x7F7FFFFF;      // +0xF8
    float  m_fBoundRadiusOffset = 0.0f;   // +0xFC
    uint32 m_nDynUnk100 = 0;              // +0x100
    uint32 m_nDynUnk104 = 0;              // +0x104
    uint32 m_nDynUnk108 = 0;              // +0x108
    uint32 m_nDynPad10C = 0;              // +0x10C

    static void InjectHooks();

    CDynamicEntity* __thiscall Ctor();
    int32 __thiscall Dtor();
    int32 __thiscall AddToInterior();
    int32 __thiscall RemoveFromInterior();
    double __thiscall GetBoundRadius();
    const CVector* __thiscall GetBoundCentre();
    const CVector* __thiscall GetBoundBoxMin();
};

VALIDATE_SIZE(CDynamicEntity, 0x110);
VALIDATE_OFFSET(CDynamicEntity, m_pAnimBlender, 0x70);
VALIDATE_OFFSET(CDynamicEntity, m_portalTracker, 0x80);
VALIDATE_OFFSET(CDynamicEntity, m_nDynUnkD0, 0xD0);
VALIDATE_OFFSET(CDynamicEntity, m_nDynUnkF8, 0xF8);
VALIDATE_OFFSET(CDynamicEntity, m_fBoundRadiusOffset, 0xFC);
VALIDATE_OFFSET(CDynamicEntity, m_nDynUnk108, 0x108);
