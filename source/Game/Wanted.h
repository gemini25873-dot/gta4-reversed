#pragma once

class CPed;

class CWanted
{
public:
    static inline int32& ms_nMaximumWantedLevel = Core::StaticRef<int32>(0xF27A14);
    static inline int32& ms_nMaximumChaos       = Core::StaticRef<int32>(0xF27A18);

    uint32 m_nWantedUnk00 = 0;               // +0x000
    uint32 m_nWantedUnk04 = 0;               // +0x004
    uint32 m_nLastWantedLevelChangeTime = 0; // +0x008
    uint32 m_nWantedUnk0C = 0;               // +0x00C
    int32  m_nWantedUnk10 = 0;               // +0x010
    uint32 m_nWantedUnk14 = 0;               // +0x014
    uint32 m_nWantedUnk18 = 0;               // +0x018
    float  m_fMultiplier = 1.0f;             // +0x01C
    uint8  m_nMaxCopCars = 0;                // +0x020
    uint8  m_pad21[0x1F]{};                  // +0x021..+0x03F
    uint32 m_nWantedUnk40 = 0;               // +0x040
    uint32 m_nWantedUnk44 = 0;               // +0x044
    uint32 m_nWantedUnk48 = 0;               // +0x048
    uint32 m_nWantedUnk4C = 0;               // +0x04C
    uint32 m_nLastTimeSeen = 0;              // +0x050
    uint32 m_nWantedUnk54 = 0;               // +0x054
    uint16 m_nMaxCopHelis = 0;               // +0x058
    uint8  m_nFlags = 0;                     // +0x05A
    uint8  m_nFlags2 = 0;                    // +0x05B
    uint8  m_pad5C[0x10]{};                  // +0x05C..+0x06B
    uint32 m_nWantedUnk6C = 0;               // +0x06C
    int32  m_nChaosLevelThreshold = 0;       // +0x070
    uint8  m_pad74[0x0C]{};                  // +0x074..+0x07F
    uint8  m_crimeEntries[16 * 48]{};        // +0x080..+0x37F
    uint8  m_nMaxCops = 0;                   // +0x380
    uint8  m_pad381[3]{};                    // +0x381..+0x383
    uint32 m_nWantedLevelEncrypted = 0;      // +0x384
    uint32 m_nChaosEncrypted = 0;            // +0x388
    uint8  m_nWantedUnk38C = 0;              // +0x38C
    uint8  m_pad38D[3]{};                    // +0x38D..+0x38F
    CPed*  m_pPed = nullptr;                 // +0x390
    uint8  m_pad394[0xBC]{};                 // +0x394..+0x44F

    static void InjectHooks();

    CWanted* __thiscall Ctor();
    int32    __thiscall Dtor();
    CPed*    __thiscall SetPed(CPed* ped);
    bool     __thiscall IsTimeSinceLastSeenOver1s() const;
    int32    __thiscall ResetSearchTimers();
    void     __thiscall ClearCrimes();
    int32    __thiscall GetWantedLevel();
    void     __thiscall SetWantedLevelRaw(int32 level);
    void     __thiscall SetWantedLevel(int32 level);

    static int32 __cdecl SetMaximumWantedLevel(int32 level);
};

VALIDATE_SIZE(CWanted, 0x450);
VALIDATE_OFFSET(CWanted, m_nLastWantedLevelChangeTime, 0x008);
VALIDATE_OFFSET(CWanted, m_nMaxCopCars, 0x020);
VALIDATE_OFFSET(CWanted, m_nLastTimeSeen, 0x050);
VALIDATE_OFFSET(CWanted, m_nMaxCopHelis, 0x058);
VALIDATE_OFFSET(CWanted, m_nFlags, 0x05A);
VALIDATE_OFFSET(CWanted, m_nFlags2, 0x05B);
VALIDATE_OFFSET(CWanted, m_nChaosLevelThreshold, 0x070);
VALIDATE_OFFSET(CWanted, m_crimeEntries, 0x080);
VALIDATE_OFFSET(CWanted, m_nMaxCops, 0x380);
VALIDATE_OFFSET(CWanted, m_nWantedLevelEncrypted, 0x384);
VALIDATE_OFFSET(CWanted, m_nChaosEncrypted, 0x388);
VALIDATE_OFFSET(CWanted, m_pPed, 0x390);
