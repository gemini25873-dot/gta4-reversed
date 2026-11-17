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
    uint8    m_pad4B0[0x0C]{};                // +0x4B0..+0x4BB
    uint32   m_nFlags = 0;                    // +0x4BC
    uint8    m_pad4C0[0x1A]{};                // +0x4C0..+0x4D9
    int8     m_nPlayerIndex = -1;             // +0x4DA
    uint8    m_pad4DB = 0;                    // +0x4DB
    int32    m_nPlayerState = 0;              // +0x4DC
    uint8    m_pad4E0[0x6A]{};                // +0x4E0..+0x549
    uint16   m_nMaxHealth = 200;              // +0x54A
    uint16   m_nMaxArmour = 100;              // +0x54C
    uint8    m_pad54E[0x1E]{};                // +0x54E..+0x56B
    uint32   m_nUnk56C = 0;                   // +0x56C
    uint32   m_nUnk570 = 0;                   // +0x570
    int32    m_nUnk574 = -1;                  // +0x574
    uint32   m_nUnk578 = 0;                   // +0x578
    uint32   m_nUnk57C = 0;                   // +0x57C
    uint8    m_nUnk580 = 0;                   // +0x580
    uint8    m_nUnk581 = 0;                   // +0x581
    uint8    m_pad582[2]{};                   // +0x582..+0x583
    uint8    m_nUnk584 = 0;                   // +0x584
    uint8    m_pad585[7]{};                   // +0x585..+0x58B
    CPed*    m_pPed = nullptr;                // +0x58C
    uint8    m_pad590[4]{};                   // +0x590..+0x593
    CEntity* m_pRemoteVehicle = nullptr;      // +0x594
    float    m_fUnk598[5]{};                  // +0x598..+0x5AB
    uint8    m_pad5AC[8]{};                   // +0x5AC..+0x5B3
    void*    m_pUnk5B4 = nullptr;             // +0x5B4
    uint32   m_nMoneyEncrypted = 0;           // +0x5B8
    uint32   m_nDisplayMoneyEncrypted = 0;    // +0x5BC

    static void InjectHooks();

    bool IsSignedIn() const { return (m_gamerInfo[92] & 1) != 0; }

    CPlayerInfo* __thiscall Ctor();
    void         __thiscall SetPlayerPed(CPed* ped);
    void         __thiscall SetRemoteVehicle(CEntity* veh);
    CVector*     __thiscall GetPos(CVector* outPos) const;
    bool         __thiscall IsWasted() const;
    bool         __thiscall IsBusted() const;
    bool         __thiscall HasFlagsFBFF() const;
    bool         __thiscall HasFlagsCFFE() const;
    int32        __thiscall GetMoney();
    void         __thiscall SetMoney(int32 amount);
    int32        __thiscall GetDisplayMoney();
    void         __thiscall SetDisplayMoney(int32 amount);

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
