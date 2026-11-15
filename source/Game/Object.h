#pragma once
#include "Game/Physical.h"

class CObject : public CPhysical
{
public:
    uint32 m_nObjUnk210;                  // +0x210
    uint32 m_nObjFlags214;                // +0x214
    uint32 m_nObjUnk218;                  // +0x218
    uint32 m_nObjUnk21C;                  // +0x21C
    int16  m_nObjUnk220;                  // +0x220
    uint8  pad222[2];                     // +0x222..+0x223
    uint32 m_nObjUnk224;                  // +0x224
    int16  m_nObjUnk228;                  // +0x228
    uint8  m_nCreatedBy;                  // +0x22A
    uint8  m_nObjUnk22B;                  // +0x22B
    uint8  pad22C[0x14];                  // +0x22C..+0x23F
    uint32 m_nObjUnk240;                  // +0x240
    uint32 m_nObjUnk244;                  // +0x244
    uint32 m_nObjUnk248;                  // +0x248
    uint8  pad24C[4];                     // +0x24C..+0x24F
    float  m_fObjScale250;                // +0x250
    uint32 m_nObjUnk254;                  // +0x254
    uint32 m_nObjUnk258;                  // +0x258
    void*  m_pObjUnk25C;                  // +0x25C
    void*  m_pObjUnk260;                  // +0x260
    uint8  m_audPlaceableTracker[0xC];    // +0x264..+0x26F
    uint8  m_audObjectAudioEntity[0x30];  // +0x270..+0x29F

    static void InjectHooks();

    CObject* __thiscall Ctor();
    CObject* __thiscall CtorWithModel(int32 modelIndex, uint8 bInitBounds, uint8 bCreatePhysics);
    CObject* __thiscall DeletingDtor(uint8 flags);
    void     __thiscall Init();
};

VALIDATE_SIZE(CObject, 0x2A0);
VALIDATE_OFFSET(CObject, m_nObjUnk210, 0x210);
VALIDATE_OFFSET(CObject, m_nCreatedBy, 0x22A);
VALIDATE_OFFSET(CObject, m_fObjScale250, 0x250);
VALIDATE_OFFSET(CObject, m_pObjUnk260, 0x260);
VALIDATE_OFFSET(CObject, m_audPlaceableTracker, 0x264);
VALIDATE_OFFSET(CObject, m_audObjectAudioEntity, 0x270);
