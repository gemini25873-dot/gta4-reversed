#include "StdInc.h"
#include "Game/Wanted.h"
#include "Game/PlayerInfo.h"
#include "Core/Vector.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CWanted::InjectHooks()
{
    Core::Hook::Install(0x921010, &CWanted::Ctor);
    Core::Hook::Install(0x920CA0, &CWanted::Dtor);
    Core::Hook::Install(0x920CD0, &CWanted::SetPed);
    Core::Hook::Install(0x920CE0, &CWanted::IsTimeSinceLastSeenOver1s);
    Core::Hook::Install(0x920CF0, &CWanted::ResetSearchTimers);
    Core::Hook::Install(0x920D00, &CWanted::SetMaximumWantedLevel);
    Core::Hook::Install(0x920F20, &CWanted::ClearCrimes);
    Core::Hook::Install(0x920FF0, &CWanted::GetWantedLevel);
    Core::Hook::Install(0x921000, &CWanted::SetWantedLevelRaw);
    Core::Hook::Install(0x921FC0, &CWanted::SetWantedLevel);

    Core::Log::Info("CWanted hooks installed (10 functions)");
}

// 0x921010
CWanted* __thiscall CWanted::Ctor()
{
    Core::Hook::CallMethod<void>(0x447290, &m_nWantedLevelEncrypted);
    Core::Hook::CallMethod<void>(0x447290, &m_nChaosEncrypted);
    m_nFlags2 &= ~1u;
    m_pPed = nullptr;
    m_nWantedUnk44 = 0;
    m_nWantedUnk48 = 0;
    m_nWantedUnk40 = 0;
    return this;
}

// 0x920CA0
int32 __thiscall CWanted::Dtor()
{
    m_pPed = nullptr;
    Core::Hook::CallMethod<void>(0x4475F0, &m_nChaosEncrypted);
    return Core::Hook::CallMethod<int32>(0x4475F0, &m_nWantedLevelEncrypted);
}

// 0x920CD0
CPed* __thiscall CWanted::SetPed(CPed* ped)
{
    m_pPed = ped;
    return ped;
}

// 0x920CE0
bool __thiscall CWanted::IsTimeSinceLastSeenOver1s() const
{
    return m_nLastTimeSeen > 0x3E8u;
}

// 0x920CF0
int32 __thiscall CWanted::ResetSearchTimers()
{
    m_nFlags2 &= ~2u;
    m_nWantedUnk18 = 0;
    m_nWantedUnk04 = 0;
    m_nWantedUnk6C = 0;
    return 0;
}

// 0x920D00
int32 __cdecl CWanted::SetMaximumWantedLevel(int32 level)
{
    switch (level)
    {
    case 0:
        ms_nMaximumWantedLevel = 0;
        ms_nMaximumChaos = 0;
        return 0;
    case 1:
        ms_nMaximumWantedLevel = 1;
        ms_nMaximumChaos = 115;
        break;
    case 2:
        ms_nMaximumWantedLevel = 2;
        ms_nMaximumChaos = 365;
        break;
    case 3:
        ms_nMaximumWantedLevel = 3;
        ms_nMaximumChaos = 875;
        break;
    case 4:
        ms_nMaximumWantedLevel = 4;
        ms_nMaximumChaos = 1800;
        break;
    case 5:
        ms_nMaximumWantedLevel = 5;
        ms_nMaximumChaos = 3500;
        break;
    case 6:
        ms_nMaximumWantedLevel = 6;
        ms_nMaximumChaos = 6900;
        break;
    default:
        break;
    }
    return level;
}

// 0x920F20
void __thiscall CWanted::ClearCrimes()
{
    auto* entries = reinterpret_cast<uint32*>(m_crimeEntries);
    for (int32 i = 0; i < 16; ++i)
    {
        entries[i * 12] = 0;
        entries[i * 12 + 1] = 0;
    }
}

// 0x920FF0
int32 __thiscall CWanted::GetWantedLevel()
{
    return Core::Hook::CallMethod<int32>(0x447370, &m_nWantedLevelEncrypted);
}

// 0x921000
void __thiscall CWanted::SetWantedLevelRaw(int32 level)
{
    Core::Hook::CallMethod<void>(0x447350, &m_nWantedLevelEncrypted, level);
}

// 0x921FC0
void __thiscall CWanted::SetWantedLevel(int32 level)
{
    if (level <= 6)
    {
        if (level > ms_nMaximumWantedLevel)
            SetMaximumWantedLevel(level);

        CVector pos{};
        CPlayerInfo::FindPlayerCoors(&pos);
        Core::Hook::CallMethod<void>(0x921E00, this, &pos, level, 0, 1);

        CPlayerInfo::FindPlayerCoors(&pos);
        Core::Hook::CallMethod<void>(0x921B50, this, &pos, 1, 1);
    }
}
