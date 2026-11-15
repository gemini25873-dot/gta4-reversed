#pragma once
#include "Game/Physical.h"

class CPed : public CPhysical
{
public:
    uint8  m_nPedUnk210;                   // +0x210
    uint8  m_nPedUnk211;                   // +0x211
    uint8  m_nPedUnk212;                   // +0x212
    uint8  pad213;                         // +0x213
    float  m_fPedUnk214;                   // +0x214
    uint8  m_nPedUnk218;                   // +0x218
    uint8  m_nPedUnk219;                   // +0x219
    uint8  pad21A[2];                      // +0x21A..+0x21B
    void*  m_pPedUnk21C;                   // +0x21C
    uint8  m_bPedUnk220;                   // +0x220
    uint8  pad221[3];                      // +0x221..+0x223
    void*  m_pPedUnk224;                   // +0x224
    uint8  pad228[4];                      // +0x228..+0x22B
    void*  m_pPedUnk22C;                   // +0x22C
    uint8  pad230[8];                      // +0x230..+0x237
    uint8  m_pedSubObj238[0x28];           // +0x238..+0x25F
    uint32 m_nPedFlags;                    // +0x260
    uint8  pad264[8];                      // +0x264..+0x26B
    uint32 m_nPedFlags2;                   // +0x26C
    uint8  pad270[0x40];                   // +0x270..+0x2AF
    uint8  m_pedSubObj2B0[0x120];          // +0x2B0..+0x3CF
    uint8  m_pedSubObj3D0[0x1B0];          // +0x3D0..+0x57F
    uint8  m_pedSpeechAudio[0x210];        // +0x580..+0x78F
    uint8  m_audPlaceableTracker[0x30];    // +0x790..+0x7BF
    void*  m_pPedUnk7C0;                   // +0x7C0
    uint8  pad7C4[4];                      // +0x7C4..+0x7C7
    int32  m_nPedUnk7C8;                   // +0x7C8
    uint8  pad7CC[0x2B8];                  // +0x7CC..+0xA83
    int32  m_nPedState;                    // +0xA84
    uint8  padA88[8];                      // +0xA88..+0xA8F
    void*  m_pPedIntelligence;             // +0xA90
    uint8  padA94[0xAC];                   // +0xA94..+0xB3F
    uint32 m_nPedUnkB40[19];               // +0xB40..+0xB8B
    uint8  padB8C[8];                      // +0xB8C..+0xB93
    uint32 m_nPedUnkB94;                   // +0xB94
    uint32 m_nPedUnkB98;                   // +0xB98
    uint8  padB9C[0x24];                   // +0xB9C..+0xBBF
    uint8  m_pedSubObjBC0[0x1D4];          // +0xBC0..+0xD93
    int16  m_nPedUnkD94;                   // +0xD94
    uint8  padD96[2];                      // +0xD96..+0xD97
    uint8  m_pedSubObjD98[0x7C];           // +0xD98..+0xE13
    void*  m_pPedUnkE14;                   // +0xE14
    float  m_fPedUnkE18;                   // +0xE18
    float  m_fPedUnkE1C;                   // +0xE1C
    uint8  m_nPedUnkE20;                   // +0xE20
    uint8  padE21[0xF];                    // +0xE21..+0xE2F
    CVector m_vecPedUnkE30;                // +0xE30..+0xE3B
    uint8  padE3C[4];                      // +0xE3C..+0xE3F
    CVector m_vecPedUnkE40;                // +0xE40..+0xE4B
    uint8  padE4C[4];                      // +0xE4C..+0xE4F
    float  m_fPedUnkE50;                   // +0xE50
    int32  m_nPedUnkE54;                   // +0xE54
    int32  m_nPedUnkE58;                   // +0xE58
    int32  m_nPedUnkE5C;                   // +0xE5C
    uint8  padE60[0xA0];                   // +0xE60..+0xEFF

    static void InjectHooks();

    CPed* __thiscall Ctor(const uint8* initParams, int32 modelIndex);
    CPed* __thiscall DeletingDtor(uint8 flags);
    bool  __thiscall IsValidOrPlayer();
    void  __thiscall SetPedState(int32 state);
};

VALIDATE_SIZE(CPed, 0xF00);
VALIDATE_OFFSET(CPed, m_nPedUnk210, 0x210);
VALIDATE_OFFSET(CPed, m_pPedUnk21C, 0x21C);
VALIDATE_OFFSET(CPed, m_nPedFlags, 0x260);
VALIDATE_OFFSET(CPed, m_nPedFlags2, 0x26C);
VALIDATE_OFFSET(CPed, m_pedSpeechAudio, 0x580);
VALIDATE_OFFSET(CPed, m_audPlaceableTracker, 0x790);
VALIDATE_OFFSET(CPed, m_nPedState, 0xA84);
VALIDATE_OFFSET(CPed, m_pPedIntelligence, 0xA90);
VALIDATE_OFFSET(CPed, m_nPedUnkB40, 0xB40);
VALIDATE_OFFSET(CPed, m_nPedUnkD94, 0xD94);
VALIDATE_OFFSET(CPed, m_fPedUnkE50, 0xE50);
