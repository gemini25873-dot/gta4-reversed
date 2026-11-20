#pragma once
#include "Game/Wanted.h"

class CEntity;
class CPed;
class CVehicle;
class CVector;

class CPlayerInfo
{
public:
    static inline auto&   ms_apPlayers         = Core::StaticRef<CPlayerInfo*[32]>(0x11A7008);
    static inline int32&  ms_nLocalPlayerIndex = Core::StaticRef<int32>(0xF1CC68);
    static inline int32&  ms_nLocalPlayerIndex2 = Core::StaticRef<int32>(0xF1CC6C);
    static inline CPed*&  ms_pCurrentPed       = Core::StaticRef<CPed*>(0x11A7118);

    uint8    m_gamerInfo[0x60]{};             // +0x000..+0x05F
    CWanted  m_Wanted{};                      // +0x060..+0x4AF
    uint32   m_nUnk4B0 = 0;                   // +0x4B0
    uint32   m_nUnk4B4 = 0;                   // +0x4B4
    uint32   m_nUnk4B8 = 0;                   // +0x4B8
    uint32   m_nFlags = 0;                    // +0x4BC
    uint8    m_pad4C0[8]{};                   // +0x4C0..+0x4C7
    uint32   m_nUnk4C8 = 0;                   // +0x4C8
    float    m_fUnk4CC = 0.0f;                // +0x4CC
    uint32   m_nUnk4D0 = 0;                   // +0x4D0
    uint32   m_nUnk4D4 = 0;                   // +0x4D4
    uint8    m_nUnk4D8 = 0;                   // +0x4D8
    uint8    m_nUnk4D9 = 0;                   // +0x4D9
    int8     m_nPlayerIndex = -1;             // +0x4DA
    uint8    m_pad4DB = 0;                    // +0x4DB
    int32    m_nPlayerState = 0;              // +0x4DC
    uint8    m_nUnk4E0 = 0;                   // +0x4E0
    uint8    m_nUnk4E1 = 0;                   // +0x4E1
    uint8    m_nUnk4E2 = 0;                   // +0x4E2
    uint8    m_pad4E3 = 0;                    // +0x4E3
    uint32   m_nUnk4E4 = 0;                   // +0x4E4
    uint32   m_nUnk4E8 = 0;                   // +0x4E8
    uint32   m_nUnk4EC = 0;                   // +0x4EC
    uint32   m_nUnk4F0 = 0;                   // +0x4F0
    uint32   m_nUnk4F4 = 0;                   // +0x4F4
    uint32   m_nUnk4F8 = 0;                   // +0x4F8
    uint32   m_nUnk4FC = 0;                   // +0x4FC
    float    m_fUnk500 = 0.0f;                // +0x500
    uint32   m_nUnk504 = 0;                   // +0x504
    uint32   m_nUnk508 = 0;                   // +0x508
    float    m_fUnk50C = 0.0f;                // +0x50C
    uint32   m_nUnk510 = 0;                   // +0x510
    float    m_fUnk514 = 0.0f;                // +0x514
    uint32   m_nUnk518 = 0;                   // +0x518
    uint32   m_nUnk51C = 0;                   // +0x51C
    float    m_fUnk520 = 0.0f;                // +0x520
    uint32   m_nUnk524 = 0;                   // +0x524
    float    m_fUnk528 = 0.0f;                // +0x528
    uint32   m_nUnk52C = 0;                   // +0x52C
    float    m_fUnk530 = 0.0f;                // +0x530
    uint16   m_nUnk534 = 0;                   // +0x534
    uint8    m_pad536[2]{};                   // +0x536..+0x537
    uint32   m_nUnk538 = 0;                   // +0x538
    uint32   m_nUnk53C = 0;                   // +0x53C
    uint32   m_nUnk540 = 0;                   // +0x540
    uint16   m_nUnk544 = 0;                   // +0x544
    uint8    m_nUnk546 = 0;                   // +0x546
    uint8    m_nUnk547 = 0;                   // +0x547
    uint8    m_nUnk548 = 0;                   // +0x548
    uint8    m_pad549 = 0;                    // +0x549
    uint16   m_nMaxHealth = 200;              // +0x54A
    uint16   m_nMaxArmour = 100;              // +0x54C
    uint8    m_nUnk54E = 0;                   // +0x54E
    uint8    m_nUnk54F = 0;                   // +0x54F
    uint8    m_nUnk550 = 0;                   // +0x550
    uint8    m_nUnk551 = 0;                   // +0x551
    uint8    m_pad552[2]{};                   // +0x552..+0x553
    uint32   m_nUnk554 = 0;                   // +0x554
    uint8    m_nUnk558 = 0;                   // +0x558
    uint8    m_pad559 = 0;                    // +0x559
    uint16   m_nUnk55A = 0;                   // +0x55A
    uint32   m_nUnk55C = 0;                   // +0x55C
    uint8    m_nUnk560 = 0;                   // +0x560
    uint8    m_pad561[11]{};                  // +0x561..+0x56B
    uint32   m_nUnk56C = 0;                   // +0x56C
    uint32   m_nUnk570 = 0;                   // +0x570
    int32    m_nUnk574 = -1;                  // +0x574
    uint32   m_nUnk578 = 0;                   // +0x578
    uint32   m_nUnk57C = 0;                   // +0x57C
    uint8    m_nUnk580 = 0;                   // +0x580
    uint8    m_nUnk581 = 0;                   // +0x581
    uint8    m_pad582[2]{};                   // +0x582..+0x583
    uint8    m_nUnk584 = 0;                   // +0x584
    uint8    m_pad585[3]{};                   // +0x585..+0x587
    uint32   m_nUnk588 = 0;                   // +0x588
    CPed*    m_pPed = nullptr;                // +0x58C
    uint8    m_pad590[4]{};                   // +0x590..+0x593
    CEntity* m_pRemoteVehicle = nullptr;      // +0x594
    float    m_fUnk598[5]{};                  // +0x598..+0x5AB
    uint8    m_bUnk5AC = 0;                   // +0x5AC
    uint8    m_pad5AD[3]{};                   // +0x5AD..+0x5AF
    int32    m_nUnk5B0 = 0;                   // +0x5B0
    void*    m_pUnk5B4 = nullptr;             // +0x5B4
    uint32   m_nMoneyEncrypted = 0;           // +0x5B8
    uint32   m_nDisplayMoneyEncrypted = 0;    // +0x5BC

    static void InjectHooks();

    bool IsSignedIn() const { return (m_gamerInfo[92] & 1) != 0; }

    CPlayerInfo* __thiscall Ctor();
    void         __thiscall Clear();
    void         __thiscall Process();
    void         __thiscall UpdateAsyncProbe();
    void         __thiscall ProcessWorldBounds();
    void         __thiscall SetPlayerControl(bool bCutsceneOrScript, bool bDisable, float fPosZ, bool bSetPedTask, bool bClearArea);
    void         __thiscall SetPlayerPed(CPed* ped);
    void         __thiscall SetRemoteVehicle(CEntity* veh);
    CVector*     __thiscall GetPos(CVector* outPos) const;
    bool         __thiscall IsWasted() const;
    bool         __thiscall IsBusted() const;
    void         __thiscall SetWasted();
    void         __thiscall SetBusted();
    bool         __thiscall HasFlagsFBFF() const;
    bool         __thiscall HasFlagsCFFE() const;
    bool         __thiscall HasFlagsEBFF() const;
    bool         __thiscall CanControlPlayer() const;
    int32        __thiscall GetMoney();
    void         __thiscall SetMoney(int32 amount);
    void         __thiscall AddMoney(int32 delta);
    int32        __thiscall GetDisplayMoney();
    void         __thiscall SetDisplayMoney(int32 amount);

    static bool         __cdecl IsUnk16394E4Ready();
    static void         __cdecl ScoreVehicleCandidate(CVehicle* veh, CPed* ped, const CVector* vehPos, float dist, float* outBestScore, CVehicle** outBestVeh, const CVector* searchPos, bool bFlag);
    static bool         __cdecl VehicleSearchCallback(CVehicle* veh, void* ctx);
    static bool         __cdecl HeliSearchCallback(CVehicle* veh, void* ctx);
    static CVehicle*    __cdecl FindVehicleToEnter(CPed* ped, const CVector* pos, bool bFlag);
    static CVehicle*    __cdecl FindHeliToEnter(CPed* ped, const CVector* pos, bool bFlag);
    static int32        __cdecl GetPlayerIndexFromPed(const CPed* ped);
    static CPlayerInfo* __cdecl GetPlayerInfo(uint32 index);
    static CPed*        __cdecl FindPlayerPedFast();
    static CVector*     __cdecl FindPlayerCoors(CVector* outPos);
    static CVehicle*    __cdecl FindPlayerVehicle(CPlayerInfo* info = nullptr);
    static CPed*        __cdecl FindPlayerPed(CPlayerInfo* info = nullptr);
};

VALIDATE_SIZE(CPlayerInfo, 0x5C0);
VALIDATE_OFFSET(CPlayerInfo, m_Wanted, 0x060);
VALIDATE_OFFSET(CPlayerInfo, m_nFlags, 0x4BC);
VALIDATE_OFFSET(CPlayerInfo, m_nPlayerIndex, 0x4DA);
VALIDATE_OFFSET(CPlayerInfo, m_nPlayerState, 0x4DC);
VALIDATE_OFFSET(CPlayerInfo, m_nMaxHealth, 0x54A);
VALIDATE_OFFSET(CPlayerInfo, m_nMaxArmour, 0x54C);
VALIDATE_OFFSET(CPlayerInfo, m_pPed, 0x58C);
VALIDATE_OFFSET(CPlayerInfo, m_pRemoteVehicle, 0x594);
VALIDATE_OFFSET(CPlayerInfo, m_fUnk598, 0x598);
VALIDATE_OFFSET(CPlayerInfo, m_nMoneyEncrypted, 0x5B8);
VALIDATE_OFFSET(CPlayerInfo, m_nDisplayMoneyEncrypted, 0x5BC);
