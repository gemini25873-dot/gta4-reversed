#include "StdInc.h"
#include "Game/Stats.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>

namespace
{
    inline void* GetFloatStatSlot(int32 index)
    {
        return reinterpret_cast<uint8*>(Core::Rebase(0x15C3EB0)) + 4 * index;
    }

    inline void* GetIntStatSlot(int32 index)
    {
        return reinterpret_cast<uint8*>(Core::Rebase(0x15C4460)) + 4 * index;
    }

    inline float ReadFloatStat(void* slot)
    {
        return static_cast<float>(Core::Hook::CallMethod<double>(0x447640, slot));
    }

    inline int32 ReadIntStat(void* slot)
    {
        return Core::Hook::CallMethod<int32>(0x447370, slot);
    }
}

void CStats::InjectHooks()
{
    Core::Hook::Install(0x962A70, &CStats::ResetCounters);
    Core::Hook::Install(0x962A90, &CStats::ResetAllTimers);
    Core::Hook::Install(0x962AC0, &CStats::ResetSessionTimers);
    Core::Hook::Install(0x962B20, &CStats::GetStatMultiplier);
    Core::Hook::Install(0x963320, &CStats::SyncFloatStats);
    Core::Hook::Install(0x963360, &CStats::GetNumFloatStats);
    Core::Hook::Install(0x963370, &CStats::SyncIntStats);
    Core::Hook::Install(0x9633B0, &CStats::GetNumIntStats);
    Core::Hook::Install(0x9633C0, &CStats::GetStatHistoryPtr);
    Core::Hook::Install(0x9633D0, &CStats::GetStatHistoryCount);
    Core::Hook::Install(0x9633E0, &CStats::GetStatValue);
    Core::Hook::Install(0x963480, &CStats::GetStatString);
    Core::Hook::Install(0x9634B0, &CStats::GetGameProgress);
    Core::Hook::Install(0x9634C0, &CStats::GetShootingAccuracy);
    Core::Hook::Install(0x963570, &CStats::GetCriminalRating);
    Core::Hook::Install(0x963700, &CStats::CheckStatPopup);
    Core::Hook::Install(0x963820, &CStats::UpdateAverageSpeedStat);
    Core::Hook::Install(0x963D70, &CStats::SetStatString);
    Core::Hook::Install(0x963DB0, &CStats::SetStatMinValue);

    Core::Log::Info("CStats hooks yuklendi (19 fonksiyon)");
}

void __cdecl CStats::ResetCounters()
{
    m_Counter1 = 0;
    m_Counter2 = 0;
    m_Counter3 = 0;
}

int32 __cdecl CStats::ResetAllTimers()
{
    m_Counter1 = 0;
    m_Counter2 = 0;
    m_Counter3 = 0;
    m_SessionTimer2 = 0;
    m_SessionTimer1 = 0;
    return 0;
}

int32 __cdecl CStats::ResetSessionTimers()
{
    m_Counter1 = 0;
    m_Counter2 = 0;
    m_Counter3 = 0;
    m_SessionTimer2 = 0;
    return 0;
}

double __cdecl CStats::GetStatMultiplier(int32 type)
{
    switch (type)
    {
    case 0:
        return 0.0;
    case 2:
    case 8:
        return 1.0f;
    case 7:
        return 600.0f;
    default:
        return 1.0f;
    }
}

float* __cdecl CStats::SyncFloatStats()
{
    float* out = reinterpret_cast<float*>(Core::Rebase(0x15C2770));
    const int32 count = m_NumFloatStats;
    for (int32 i = 0; i < count; ++i)
        out[i] = ReadFloatStat(GetFloatStatSlot(i));
    return out;
}

int32 __cdecl CStats::GetNumFloatStats()
{
    return m_NumFloatStats;
}

int32* __cdecl CStats::SyncIntStats()
{
    int32* out = reinterpret_cast<int32*>(Core::Rebase(0x15C2140));
    const int32 count = m_NumIntStats;
    for (int32 i = 0; i < count; ++i)
        out[i] = ReadIntStat(GetIntStatSlot(i));
    return out;
}

int32 __cdecl CStats::GetNumIntStats()
{
    return m_NumIntStats;
}

int32* __cdecl CStats::GetStatHistoryPtr()
{
    return reinterpret_cast<int32*>(Core::Rebase(0x15C2D28));
}

int32 __cdecl CStats::GetStatHistoryCount()
{
    return m_StatHistoryCount;
}

double __cdecl CStats::GetStatValue(int32 statId)
{
    if (static_cast<uint32>(statId - 649) <= 0x23)
    {
        if (statId == 682)
        {
            const int32 s0 = ReadIntStat(reinterpret_cast<void*>(Core::Rebase(0x15C4510)));
            const int32 s1 = ReadIntStat(reinterpret_cast<void*>(Core::Rebase(0x15C450C)));
            const int32 s2 = ReadIntStat(reinterpret_cast<void*>(Core::Rebase(0x15C4508)));
            const int32 s3 = ReadIntStat(reinterpret_cast<void*>(Core::Rebase(0x15C4504)));
            return static_cast<float>(s0 + s1 + s2 + s3);
        }
        return 1.0;
    }

    if (statId >= 253)
        return static_cast<double>(ReadIntStat(GetIntStatSlot(statId - 253)));

    return static_cast<double>(ReadFloatStat(GetFloatStatSlot(statId)));
}

const char* __cdecl CStats::GetStatString(int32 statId)
{
    if (static_cast<uint32>(statId - 649) > 0x20)
        return nullptr;

    const char** table = reinterpret_cast<const char**>(Core::Rebase(0x15C2304));
    const char* str = table[statId];
    if (!str || !*str)
        return nullptr;

    return str;
}

double __cdecl CStats::GetGameProgress()
{
    return GetStatValue(0);
}

double __cdecl CStats::GetShootingAccuracy()
{
    const float shotsFired = static_cast<float>(GetStatValue(291));
    const float shotsHit = static_cast<float>(GetStatValue(41));
    if (shotsFired == 0.0f || shotsHit == 0.0f)
        return 0.0;

    const float pct = (shotsHit / shotsFired) * 100.0f;
    return static_cast<float>(std::floor(pct));
}

int32 __cdecl CStats::GetCriminalRating()
{
    const float val = static_cast<float>(GetStatValue(115));
    if (val < 10.0f)
        return 0;
    if (val < 60.0f)
        return 2;
    if (val < 160.0f)
        return 3;
    if (val < 330.0f)
        return 4;
    if (val < 540.0f)
        return 5;
    if (val < 800.0f)
        return 6;
    return 7;
}

void __cdecl CStats::CheckStatPopup(int32 /*unused*/, int32 statId)
{
    if (m_bPopupGuard)
    {
        m_bPopupGuard = 0;
        return;
    }

    if (Core::Hook::Call<uint8>(0xB525D0, statId) != 0)
        GetStatValue(statId);
}

void __cdecl CStats::UpdateAverageSpeedStat()
{
    const float dist = static_cast<float>(GetStatValue(77)) + static_cast<float>(GetStatValue(78));
    const double timeMs = GetStatValue(71) + static_cast<float>(GetStatValue(73));
    if (timeMs > 0.0 && dist > 0.0f)
    {
        const float timeF = static_cast<float>(timeMs);
        const float hours = ((timeF * 0.001f) * (1.0f / 60.0f)) * (1.0f / 60.0f);
        Core::Hook::Call<int32>(0x963D00, 42, dist / hours);
    }
}

int32 __cdecl CStats::SetStatString(int32 statId, const char* str)
{
    const int32 rel = statId - 649;
    if (static_cast<uint32>(rel) > 0x20 || !str)
        return str ? reinterpret_cast<int32>(str) : rel;

    char** table = reinterpret_cast<char**>(Core::Rebase(0x15C2304));
    Core::Hook::Call<char*>(0x468E40, table[statId], str, 128);
    return Core::Hook::Call<int32>(0x963AF0, statId);
}

int32 __cdecl CStats::SetStatMinValue(int32 statId, int32 value)
{
    if (GetStatValue(statId) == 0.0)
        return Core::Hook::Call<int32>(0x963D00, statId, static_cast<float>(value));

    const float current = static_cast<float>(GetStatValue(statId));
    const float target = (static_cast<float>(value) <= current) ? static_cast<float>(value) : current;
    return Core::Hook::Call<int32>(0x963D00, statId, target);
}
