#pragma once
#include "Game/Physical.h"

class CPed;

class CVehicle : public CPhysical
{
public:
    uint8  pad210[0x40];                  // +0x210..+0x24F
    uint8  m_nVehUnk250;                  // +0x250
    uint8  pad251[0xF];                   // +0x251..+0x25F
    uint8  m_vehSubObj260[0xB50];         // +0x260..+0xDAF
    uint8  m_vehAudioEntity[0x58];        // +0xDB0..+0xE07
    uint8  m_audPlaceableTracker[0x1C];   // +0xE08..+0xE23
    uint8  m_vehSubObjE24[0x140];         // +0xE24..+0xF63
    uint8  m_nVehFlagsF64;                // +0xF64
    uint8  m_nVehFlagsF65;                // +0xF65
    uint8  m_nVehFlagsF66;                // +0xF66
    uint8  m_nVehFlagsF67;                // +0xF67
    uint8  m_nVehFlagsF68;                // +0xF68
    uint8  m_nVehFlagsF69;                // +0xF69
    uint8  m_nVehFlagsF6A;                // +0xF6A
    uint8  m_nVehFlagsF6B;                // +0xF6B
    uint8  m_nVehFlagsF6C;                // +0xF6C
    uint8  m_nVehFlagsF6D;                // +0xF6D
    uint8  m_nVehFlagsF6E;                // +0xF6E
    uint8  m_nVehFlagsF6F;                // +0xF6F
    uint8  m_nVehFlagsF70;                // +0xF70
    uint8  m_nVehFlagsF71;                // +0xF71
    uint8  padF72[2];                     // +0xF72..+0xF73
    uint32 m_nVehUnkF74;                  // +0xF74
    int32  m_nVehUnkF78;                  // +0xF78
    uint32 m_nVehUnkF7C;                  // +0xF7C
    uint32 m_nVehUnkF80;                  // +0xF80
    float  m_fVehUnkF84;                  // +0xF84
    uint32 m_nVehUnkF88;                  // +0xF88
    uint8  padF8C[0xC];                   // +0xF8C..+0xF97
    int16  m_nVehUnkF98;                  // +0xF98
    int16  m_nVehUnkF9A;                  // +0xF9A
    int16  m_nVehUnkF9C;                  // +0xF9C
    uint8  padF9E[2];                     // +0xF9E..+0xF9F
    CPed*  m_pDriver;                     // +0xFA0
    CPed*  m_pPassengers[8];              // +0xFA4..+0xFC3
    float  m_fVehUnkFC4;                  // +0xFC4
    uint8  padFC8[4];                     // +0xFC8..+0xFCB
    uint8  m_nVehUnkFCC;                  // +0xFCC
    uint8  padFCD[3];                     // +0xFCD..+0xFCF
    uint32 m_nVehUnkFD0;                  // +0xFD0
    uint32 m_nVehUnkFD4;                  // +0xFD4
    uint32 m_nVehUnkFD8;                  // +0xFD8
    uint32 m_nVehUnkFDC;                  // +0xFDC
    uint32 m_nVehUnkFE0;                  // +0xFE0
    uint8  padFE4[8];                     // +0xFE4..+0xFEB
    uint32 m_nVehUnkFEC;                  // +0xFEC
    uint8  padFF0[4];                     // +0xFF0..+0xFF3
    int32  m_nVehUnkFF4;                  // +0xFF4
    uint32 m_nVehUnkFF8;                  // +0xFF8
    uint8  m_vehSubObjFFC[8 * 0x14];      // +0xFFC..+0x109B
    uint32 m_nVehUnk109C;                 // +0x109C
    float  m_fVehUnk10A0;                 // +0x10A0
    float  m_fVehUnk10A4;                 // +0x10A4
    float  m_fVehUnk10A8;                 // +0x10A8
    uint8  pad10AC[4];                    // +0x10AC..+0x10AF
    float  m_fVehUnk10B0;                 // +0x10B0
    float  m_fVehUnk10B4;                 // +0x10B4
    float  m_fVehUnk10B8;                 // +0x10B8
    uint8  pad10BC[4];                    // +0x10BC..+0x10BF
    uint8  m_nMaxPassengers;              // +0x10C0
    uint8  m_nVehUnk10C1;                 // +0x10C1
    uint8  m_nVehUnk10C2;                 // +0x10C2
    uint8  pad10C3;                       // +0x10C3
    int16  m_nVehUnk10C4;                 // +0x10C4
    uint8  pad10C6[2];                    // +0x10C6..+0x10C7
    float  m_fVehUnk10C8;                 // +0x10C8
    float  m_fVehUnk10CC;                 // +0x10CC
    float  m_fVehUnk10D0;                 // +0x10D0
    float  m_fVehUnk10D4;                 // +0x10D4
    float  m_fVehUnk10D8;                 // +0x10D8
    float  m_fVehUnk10DC;                 // +0x10DC
    uint8  m_vehSubObj10E0[0x28];         // +0x10E0..+0x1107
    uint8  m_nCreatedBy;                  // +0x1108
    uint8  pad1109[3];                    // +0x1109..+0x110B
    uint32 m_nVehUnk110C;                 // +0x110C
    int16  m_nVehUnk1110;                 // +0x1110
    uint8  m_nVehUnk1112;                 // +0x1112
    uint8  pad1113;                       // +0x1113
    int16  m_nVehUnk1114;                 // +0x1114
    uint8  pad1116[2];                    // +0x1116..+0x1117
    float  m_fVehUnk1118;                 // +0x1118
    uint8  pad111C[4];                    // +0x111C..+0x111F
    uint8  m_vehSubObj1120[0x1C0];        // +0x1120..+0x12DF
    uint32 m_nVehUnk12E0;                 // +0x12E0
    uint32 m_nVehUnk12E4;                 // +0x12E4
    uint32 m_nVehUnk12E8;                 // +0x12E8
    uint32 m_nVehUnk12EC;                 // +0x12EC
    uint32 m_nVehUnk12F0;                 // +0x12F0
    uint32 m_nVehUnk12F4;                 // +0x12F4
    uint32 m_nVehUnk12F8;                 // +0x12F8
    uint32 m_nVehUnk12FC;                 // +0x12FC
    uint32 m_nVehUnk1300;                 // +0x1300
    float  m_fVehUnk1304;                 // +0x1304
    uint32 m_nVehUnk1308;                 // +0x1308
    int16  m_nVehUnk130C;                 // +0x130C
    int16  m_nVehUnk130E;                 // +0x130E
    uint32 m_nVehUnk1310;                 // +0x1310
    uint32 m_nVehUnk1314;                 // +0x1314
    float  m_fVehUnk1318;                 // +0x1318
    float  m_fVehUnk131C;                 // +0x131C
    int32  m_nVehUnk1320;                 // +0x1320
    float  m_fVehUnk1324;                 // +0x1324
    float  m_fVehUnk1328;                 // +0x1328
    float  m_fVehUnk132C;                 // +0x132C
    float  m_fVehUnk1330;                 // +0x1330
    uint8  pad1334;                       // +0x1334
    uint8  m_nVehUnk1335;                 // +0x1335
    uint8  m_nVehUnk1336;                 // +0x1336
    uint8  m_nVehUnk1337;                 // +0x1337
    uint32 m_nVehUnk1338;                 // +0x1338
    uint32 m_nVehUnk133C;                 // +0x133C
    uint8  m_nVehUnk1340;                 // +0x1340
    uint8  m_nVehUnk1341;                 // +0x1341
    uint8  m_nVehUnk1342;                 // +0x1342
    uint8  m_nVehUnk1343;                 // +0x1343
    uint8  m_nVehUnk1344;                 // +0x1344
    uint8  pad1345[3];                    // +0x1345..+0x1347
    float  m_fVehUnk1348;                 // +0x1348
    uint8  m_nVehUnk134C;                 // +0x134C
    uint8  pad134D[3];                    // +0x134D..+0x134F

    static void InjectHooks();

    CVehicle* __thiscall Ctor(uint8 createdBy);
    CPed*     __thiscall GetPassenger(int32 seat);

    static int32 __stdcall SeatToPassengerIndex(int32 seat);
    static int32 __stdcall PassengerIndexToSeat(int32 index);

    void        __thiscall Unk_9C52C0();
    bool        __thiscall Unk_9C5360();
    bool        __thiscall Unk_9C5370(bool updateTimer);
    bool        __thiscall Unk_9C53C0(float* outProgress);
    bool        __thiscall Unk_9C5430();
    static int32 __stdcall Unk_9C6DF0(int32 seat);
    int32       __thiscall GetPedSeat(CPed* pPed);
    int32       __thiscall Unk_9C6E90(int32 targetId);
    int32       __thiscall Unk_9C6EF0(int32 a1);
    const char* __thiscall Unk_9C6F20(int32 id);
    void        __thiscall Unk_9C6F90(float t);
    bool        __thiscall IsPlayerInVehicle();

    bool        __thiscall Unk_9C5440();
    bool        __thiscall Unk_9C5480();
    bool        __thiscall Unk_9C54C0();
    void*       __thiscall Unk_9C5510(int32 key);
    int32       __thiscall Unk_9C5570();
    void*       __thiscall Unk_9C55E0(int32 key);
    static void __cdecl    Unk_9C5630(int32 modelIdx, float* outA, float* outB, uint8 a4);
    void        __thiscall Unk_9C5650();
    static bool __stdcall  Unk_9C56E0(const uint8* pObj);
    void        __thiscall Unk_9C5760(CPed* pPed);
    void*       __thiscall Unk_9C6AF0(int32 idx);
    int32       __thiscall Unk_9C6B20();

    bool        __thiscall Unk_9C6B40();
    int32       __thiscall Unk_9C6B70(int32 key);
    void*       __thiscall Unk_9C6BA0(int32 idx);
    void*       __thiscall Unk_9C6BC0(int32 val);
    static void __cdecl    Unk_9C6C00();
    int32       __thiscall GetPassengerCount();
    void        __thiscall Unk_9C7010(bool setFlag);
    void        __thiscall Unk_9C7170(const float* pVec3);
    void        __thiscall Unk_9C71C0(const float* pVec3);
    void        __thiscall Unk_9C7210(const float* pA, const float* pB);
    bool        __thiscall Unk_9C72B0();
    void        __thiscall Unk_9C72D0();

    void        __thiscall Unk_9C7380();
    int32       __thiscall Unk_9C7560();
    void        __thiscall Unk_9C7580(int32 val);
    void*       __thiscall Unk_9C7710(int32 boneIdx);

    static void __cdecl    InitSystem();
    int32       __thiscall Unk_9C80B0(int32 a2, int32 a3, int32 a4, int32 a5, int32 a6);
    int32       __thiscall Unk_9C80C0();
    int32       __thiscall Unk_9C80D0();
    void*       __thiscall Unk_9C83A0(int32 a2, int32 a3, int32 a4, int32 a5);
    bool        __thiscall Unk_9C8700(int32 boneIdx, float* outVec4);
    void        __thiscall Unk_9C8900(int32 boneId, float* outMat4x4);
    void        __thiscall Unk_9C8A70();
    bool        __thiscall Unk_9C8C70(bool a2, bool a3, int32 a4, int32 a5, bool a6, bool a7, bool a8);
    bool        __thiscall Unk_9C8D80();
    bool        __thiscall Unk_9C8DC0(bool a2, bool a3, int32 a4);

    bool        __thiscall Unk_9C8F30();
    bool        __thiscall Unk_9C9020();
    void        __thiscall Unk_9C91A0();
    bool        __thiscall Unk_9C9210();
    void        __thiscall Unk_9C93B0(void** outArr, int32* pCount, int32 maxCount, void* pExtra);
    bool        __thiscall Unk_9C9BA0();
    void        __thiscall Unk_9C9E60(uint8* pArg, bool bFlag);
    bool        __thiscall Unk_9CA080(CPed* pPed);
    bool        __thiscall Unk_9CA250(CPed* pPed, uint32 seatIdx);
    void        __thiscall Unk_9CA440(void* a2, int32 a3);
    bool        __thiscall Unk_9CA600();
    bool        __thiscall Unk_9CA620();

    bool        __thiscall Unk_9CA650(uint8* pPed);
    bool        __thiscall Unk_9CA710(bool bSkipVelCheck);
    bool        __thiscall Unk_9CA8E0(CPed* pPed);
    bool        __thiscall Unk_9CA9B0();
    static bool __cdecl    Unk_9CA9E0(int32 modelIdx);
    void        __thiscall Unk_9CAA30();
    void        __thiscall Unk_9CC990(uint8* pPed, int32 a3);
    bool        __thiscall Unk_9D4A50();
    CVehicle*   __thiscall ScalarDeletingDtor(uint8 flags);
    void        __thiscall Unk_9D5F00();
    void        __thiscall Unk_9D5FC0();
    void        __thiscall Unk_9D6170(void* a2, float angle, int32 a4);

private:
    void        ApplyPassengerImpulse(CPed* pPed);
};

VALIDATE_SIZE(CVehicle, 0x1350);
VALIDATE_OFFSET(CVehicle, m_vehSubObj260, 0x260);
VALIDATE_OFFSET(CVehicle, m_vehAudioEntity, 0xDB0);
VALIDATE_OFFSET(CVehicle, m_audPlaceableTracker, 0xE08);
VALIDATE_OFFSET(CVehicle, m_pDriver, 0xFA0);
VALIDATE_OFFSET(CVehicle, m_pPassengers, 0xFA4);
VALIDATE_OFFSET(CVehicle, m_nMaxPassengers, 0x10C0);
VALIDATE_OFFSET(CVehicle, m_nCreatedBy, 0x1108);
VALIDATE_OFFSET(CVehicle, m_nVehUnk134C, 0x134C);
