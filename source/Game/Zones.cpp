#include "StdInc.h"
#include "Game/Zones.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CTheZones::InjectHooks()
{
    Core::Hook::Install(0x88C280, &CTheZones::Init);
    Core::Hook::Install(0x88BF30, &CTheZones::Save);
    Core::Hook::Install(0x88BF90, &CTheZones::Load);
    Core::Hook::Install(0x88C2A0, &CTheZones::PostZonesInit);
    Core::Hook::Install(0x88BEC0, &CTheZones::PointLiesWithinZone);
    Core::Hook::Install(0x88BFF0, &CTheZones::FindSmallestZoneForPosition);
    Core::Hook::Install(0x88C080, &CTheZones::GetMapZone);
    Core::Hook::Install(0x88C370, &CTheZones::IsPointInZone);

    Core::Log::Info("CTheZones hooks installed (8 functions)");
}

// 0x88C280
int32 __cdecl CTheZones::Init()
{
    return Core::Hook::CallMethod<int32>(
        0xB3CC50,
        &Core::StaticRef<uint32>(0x1692078),
        "Zones",
        reinterpret_cast<void*>(Core::Rebase(0x88BF90)),
        reinterpret_cast<void*>(Core::Rebase(0x88BF30)));
}

// 0x88BF30
bool __cdecl CTheZones::Save()
{
    Core::Hook::Call<char>(0x82DEA0, &ms_nTotalNumberOfMapZones, static_cast<int32>(sizeof(uint32)));

    CZone* zones = GetMapZoneArray();
    for (int32 i = 0; i < MAX_NUM_MAP_ZONES; ++i)
    {
        Core::Hook::Call<char>(0x82DEA0, &zones[i], static_cast<int32>(sizeof(CZone)));
    }

    Core::Hook::Call<char>(0x82DEA0, GetZonesVisited(), MAX_ZONES_VISITED);
    Core::Hook::Call<char>(0x82DEA0, &ms_nZonesRevealed, static_cast<int32>(sizeof(uint32)));
    return true;
}

// 0x88BF90
bool __cdecl CTheZones::Load()
{
    Core::Hook::Call<char>(0x82DEB0, &ms_nTotalNumberOfMapZones, static_cast<int32>(sizeof(uint32)));

    CZone* zones = GetMapZoneArray();
    for (int32 i = 0; i < MAX_NUM_MAP_ZONES; ++i)
    {
        Core::Hook::Call<char>(0x82DEB0, &zones[i], static_cast<int32>(sizeof(CZone)));
    }

    Core::Hook::Call<char>(0x82DEB0, GetZonesVisited(), MAX_ZONES_VISITED);
    Core::Hook::Call<char>(0x82DEB0, &ms_nZonesRevealed, static_cast<int32>(sizeof(uint32)));
    return true;
}

// 0x88C2A0
int32 __cdecl CTheZones::PostZonesInit()
{
    Core::Hook::Call<int32>(0x88BD20);
    ms_nZonesRevealed = 0;
    bool* visited = GetZonesVisited();
    std::memset(visited, 0, MAX_ZONES_VISITED);
    return reinterpret_cast<int32>(visited);
}

// 0x88BEC0
bool __cdecl CTheZones::PointLiesWithinZone(const CVector* pos, const CZone* zone)
{
    if (pos->x >= static_cast<float>(zone->m_nMinX) && static_cast<float>(zone->m_nMaxX) >= pos->x &&
        pos->y >= static_cast<float>(zone->m_nMinY) && static_cast<float>(zone->m_nMaxY) >= pos->y &&
        pos->z >= static_cast<float>(zone->m_nMinZ) && static_cast<float>(zone->m_nMaxZ) >= pos->z)
    {
        return true;
    }
    return false;
}

// 0x88BFF0
CZone* __cdecl CTheZones::FindSmallestZoneForPosition(const CVector* pos)
{
    CZone* zones = GetMapZoneArray();
    uint32 bestSize = static_cast<uint32>(zones[0].m_nMaxX + zones[0].m_nMaxY - zones[0].m_nMinX - zones[0].m_nMinY);
    CZone* bestZone = &zones[0];

    for (uint32 i = 1; i < ms_nTotalNumberOfMapZones; ++i)
    {
        if (PointLiesWithinZone(pos, &zones[i]))
        {
            const uint32 size = static_cast<uint32>(zones[i].m_nMaxX + zones[i].m_nMaxY - zones[i].m_nMinX - zones[i].m_nMinY);
            if (size < bestSize)
            {
                bestZone = &zones[i];
                bestSize = size;
            }
        }
    }

    return bestZone;
}

// 0x88C080
CZone* __cdecl CTheZones::GetMapZone(int32 index)
{
    return &GetMapZoneArray()[index];
}

// 0x88C370
bool __cdecl CTheZones::IsPointInZone(const CVector* pos, uint32 textLabelPart1, uint32 textLabelPart2)
{
    const uint32 count = ms_nTotalNumberOfMapZones;
    for (uint32 i = 0; i < count; ++i)
    {
        const CZone* zone = GetMapZone(static_cast<int32>(i));
        const auto* labelParts = reinterpret_cast<const uint32*>(zone->m_szTextLabel);
        if (labelParts[0] == textLabelPart1 && labelParts[1] == textLabelPart2 && PointLiesWithinZone(pos, zone))
        {
            return true;
        }
    }
    return false;
}
