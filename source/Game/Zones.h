#pragma once
#include "Core/Vector.h"

class CZone
{
public:
    char   m_szInfoLabel[8]{};      // +0x00..+0x07
    char   m_szTextLabel[8]{};      // +0x08..+0x0F
    int16  m_nMinX = 0;             // +0x10
    int16  m_nMinY = 0;             // +0x12
    int16  m_nMinZ = 0;             // +0x14
    int16  m_nMaxX = 0;             // +0x16
    int16  m_nMaxY = 0;             // +0x18
    int16  m_nMaxZ = 0;             // +0x1A
    int16  m_nCenterX = 0;          // +0x1C
    int16  m_nCenterY = 0;          // +0x1E
    uint8  m_nPopulationType = 0;   // +0x20
    uint8  m_nScumminess = 0;       // +0x21
    uint8  m_pad22[0x06]{};         // +0x22..+0x27
};

VALIDATE_SIZE(CZone, 0x28);
VALIDATE_OFFSET(CZone, m_szInfoLabel, 0x00);
VALIDATE_OFFSET(CZone, m_szTextLabel, 0x08);
VALIDATE_OFFSET(CZone, m_nMinX, 0x10);
VALIDATE_OFFSET(CZone, m_nMinY, 0x12);
VALIDATE_OFFSET(CZone, m_nMinZ, 0x14);
VALIDATE_OFFSET(CZone, m_nMaxX, 0x16);
VALIDATE_OFFSET(CZone, m_nMaxY, 0x18);
VALIDATE_OFFSET(CZone, m_nMaxZ, 0x1A);
VALIDATE_OFFSET(CZone, m_nCenterX, 0x1C);
VALIDATE_OFFSET(CZone, m_nCenterY, 0x1E);
VALIDATE_OFFSET(CZone, m_nPopulationType, 0x20);
VALIDATE_OFFSET(CZone, m_nScumminess, 0x21);

class CTheZones
{
public:
    static constexpr int32 MAX_NUM_MAP_ZONES = 300;
    static constexpr int32 MAX_ZONES_VISITED = 100;

    static inline uint32& ms_nTotalNumberOfMapZones = Core::StaticRef<uint32>(0x1205CA4);
    static inline uint32& ms_nZonesRevealed         = Core::StaticRef<uint32>(0x1205C90);

    static CZone* GetMapZoneArray()
    {
        return reinterpret_cast<CZone*>(Core::Rebase(0x1205D10));
    }

    static bool* GetZonesVisited()
    {
        return reinterpret_cast<bool*>(Core::Rebase(0x1205CA8));
    }

    static void InjectHooks();

    static int32  __cdecl Init();
    static bool   __cdecl Save();
    static bool   __cdecl Load();
    static int32  __cdecl PostZonesInit();
    static bool   __cdecl PointLiesWithinZone(const CVector* pos, const CZone* zone);
    static CZone* __cdecl FindSmallestZoneForPosition(const CVector* pos);
    static CZone* __cdecl GetMapZone(int32 index);
    static bool   __cdecl IsPointInZone(const CVector* pos, uint32 textLabelPart1, uint32 textLabelPart2);
};
