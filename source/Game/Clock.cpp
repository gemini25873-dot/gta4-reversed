#include "StdInc.h"
#include "Clock.h"
#include "Timer.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace {
    // 0x7955D0
    bool IsClockUpdateBlocked() {
        const int32  state1 = Core::StaticRef<int32>(0xF0E4A0);
        const HANDLE handle = Core::StaticRef<HANDLE>(0x10E1984);
        const uint8  flag1  = Core::StaticRef<uint8>(0x10D00D8);
        const uint8  flag2  = Core::StaticRef<uint8>(0xF0C512);
        const int32  state2 = Core::StaticRef<int32>(0xF0E4A4);

        const bool cond = !state1
            || handle != INVALID_HANDLE_VALUE
            || state1 == 18
            || ((state1 == 2 || state1 == 7) && flag1 != 0)
            || (Core::Hook::Call<uint8>(0x794330) != 0 && flag2 == 0);

        return cond
            && state2 != 5
            && state2 != 13
            && state2 != 6
            && state2 != 14;
    }
}

void CClock::InjectHooks() {
    Core::Hook::Install(0x820E50, &CClock::Initialise);
    Core::Hook::Install(0x4051F0, &CClock::GetGameClockHours);
    Core::Hook::Install(0x405200, &CClock::GetGameClockMinutes);
    Core::Hook::Install(0x821060, &CClock::GetIsTimeInRange);
    Core::Hook::Install(0x821020, &CClock::GetGameClockMinutesUntil);
    Core::Hook::Install(0x820EE0, &CClock::SetGameClockExact);
    Core::Hook::Install(0x8210B0, &CClock::StepDay);
    Core::Hook::Install(0x821200, &CClock::SetGameClock);
    Core::Hook::Install(0x821260, &CClock::OffsetClockByADayOrAdvance);
    Core::Hook::Install(0x8212E0, &CClock::Update);
    Core::Log::Info("CClock hooks installed.");
}

// 0x820E50
void __cdecl CClock::Initialise(int32 millisecondsPerGameMinute) {
    ms_nMillisecondsPerGameMinute = millisecondsPerGameMinute;
    ms_nGameClockMonths           = 1;
    ms_nGameClockDays             = 1;
    ms_nGameClockMinutes          = 0;
    ms_nGameClockSeconds          = 0;
    ms_nLastClockTick             = CTimer::m_snTimeInMilliseconds;
    ms_nGameClockHours            = 12;
    ms_nStoredGameClockHours      = -1;
    ms_nForcedGameClockHours      = -1;
    ms_nForcedGameClockMinutes    = -1;
    CurrentDay                    = 3;
}

// 0x4051F0
int32 __cdecl CClock::GetGameClockHours() {
    return (ms_nForcedGameClockHours == -1) ? ms_nGameClockHours : ms_nForcedGameClockHours;
}

// 0x405200
int32 __cdecl CClock::GetGameClockMinutes() {
    return (ms_nForcedGameClockMinutes == -1) ? ms_nGameClockMinutes : ms_nForcedGameClockMinutes;
}

// 0x821060
bool __cdecl CClock::GetIsTimeInRange(int32 hourA, int32 hourB) {
    if (hourA > hourB) {
        return ms_nGameClockHours >= hourA || ms_nGameClockHours < hourB;
    }
    return ms_nGameClockHours >= hourA && ms_nGameClockHours < hourB;
}

// 0x821020
uint32 __cdecl CClock::GetGameClockMinutesUntil(int32 hours, int32 minutes) {
    const uint32 now    = static_cast<uint32>(ms_nGameClockMinutes + 60 * ms_nGameClockHours);
    uint32       target = static_cast<uint32>(minutes + 60 * hours);
    if (target < now) {
        target += 1440;
    }
    return target - now;
}

// 0x820EE0
void __cdecl CClock::SetGameClockExact(int32 day, int32 hours, int32 minutes, int32 seconds) {
    CurrentDay           = day;
    ms_nGameClockHours   = hours;
    ms_nGameClockMinutes = minutes;
    ms_nGameClockSeconds = seconds;
    ms_nLastClockTick    = CTimer::m_snTimeInMilliseconds;
}

// 0x8210B0
void __cdecl CClock::StepDay(bool forward) {
    const auto* daysInMonth = reinterpret_cast<const uint8*>(Core::Rebase(0xF1CD94));

    if (forward) {
        if (++ms_nGameClockDays > daysInMonth[ms_nGameClockMonths - 1]) {
            ms_nGameClockDays = 1;
            if (++ms_nGameClockMonths > 12) {
                ms_nGameClockMonths = 1;
            }
        }
        CurrentDay = (CurrentDay + 1) % 7;
        Core::Hook::Call<void>(0x962B90, 0x104, 1.0f);
    } else {
        if (--ms_nGameClockDays == 0) {
            if (--ms_nGameClockMonths == 0) {
                ms_nGameClockMonths = 12;
            }
            ms_nGameClockDays = daysInMonth[ms_nGameClockMonths - 1];
        }
        if (--CurrentDay < 0) {
            CurrentDay = 6;
        }
        Core::Hook::Call<void>(0x962B90, 0x104, -1.0f);
    }
}

// 0x821200
void __cdecl CClock::SetGameClock(int32 hours, int32 minutes, int32 seconds, int32 day) {
    ms_nGameClockHours   = hours;
    ms_nLastClockTick    = CTimer::m_snTimeInMilliseconds;
    ms_nGameClockMinutes = minutes;

    if (day >= 0) {
        const int32 delta = (day - CurrentDay + 7) % 7;
        CurrentDay        = day;
        ms_nGameClockDays += delta;
    }

    ms_nGameClockSeconds = seconds;
    if (seconds >= 60) {
        ms_nGameClockMinutes += seconds / 60;
        ms_nGameClockSeconds  = seconds % 60;
    }

    if (ms_nGameClockMinutes >= 60) {
        ms_nGameClockHours   += ms_nGameClockMinutes / 60;
        ms_nGameClockMinutes %= 60;
    }

    if (ms_nGameClockHours >= 24) {
        ms_nGameClockDays  += ms_nGameClockHours / 24;
        ms_nGameClockHours %= 24;
    }

    const auto* daysInMonth = reinterpret_cast<const uint8*>(Core::Rebase(0xF1CD94));
    int32       month       = ms_nGameClockMonths;
    if (ms_nGameClockDays > daysInMonth[month - 1]) {
        ms_nGameClockDays = 1;
        do {
            month = (month % 12) + 1;
        } while (!daysInMonth[month - 1]);
        ms_nGameClockMonths = month;
    }

    if (month > 12) {
        ms_nGameClockMonths = 1;
    }
}

// 0x821260
void __cdecl CClock::OffsetClockByADayOrAdvance(int32 hours, int32 minutes) {
    int32 h = hours;
    int32 m = minutes;
    if (m > 59) {
        const uint32 extraHours = static_cast<uint32>(m - 60) / 60u + 1u;
        m -= 60 * static_cast<int32>(extraHours);
        for (uint32 i = 0; i < extraHours; ++i) {
            if (++h >= 24) {
                h -= 24;
                StepDay(true);
            }
        }
    }
    if (h < ms_nGameClockHours || (h == ms_nGameClockHours && m < ms_nGameClockMinutes)) {
        StepDay(true);
    }
    SetGameClock(h, m, 0, -1);
}

// 0x8212E0
void __cdecl CClock::Update() {
    if (CTimer::GetIsPaused() || IsClockUpdateBlocked()) {
        return;
    }

    uint32       now  = CTimer::m_snTimeInMilliseconds;
    uint32       last = ms_nLastClockTick;
    const int32  step = ms_nMillisecondsPerGameMinute;

    if (static_cast<int32>(now - last) > step) {
        do {
            last += step;
            ms_nLastClockTick = last;
            if (++ms_nGameClockMinutes >= 60) {
                ms_nGameClockMinutes = 0;
                if (++ms_nGameClockHours >= 24) {
                    ms_nGameClockHours = 0;
                    StepDay(true);
                    last = ms_nLastClockTick;
                    now  = CTimer::m_snTimeInMilliseconds;
                }
            }
        } while (static_cast<int32>(now - last) > step);
    }

    if (last > now) {
        last              = now;
        ms_nLastClockTick = now;
    }

    ms_nGameClockSeconds = static_cast<int32>(
        static_cast<double>(now - last) * 60.0 / static_cast<double>(static_cast<uint32>(step))
    );

    const int32  h         = GetGameClockHours();
    const int32  m         = GetGameClockMinutes();
    const float  totalSecs = static_cast<float>(m) * 60.0f + static_cast<float>(h) * 3600.0f + static_cast<float>(ms_nGameClockSeconds);
    const int32  ep        = Core::StaticRef<int32>(0x10F9AC0);
    auto*        birdCycle = reinterpret_cast<float*>(Core::Rebase(0xF1CDA0));

    constexpr float kInvHalfDay = 1.0f / 43200.0f;
    if (totalSecs >= 43200.0f) {
        birdCycle[ep] = 1.0f - (totalSecs - 43200.0f) * kInvHalfDay;
    } else {
        birdCycle[ep] = totalSecs * kInvHalfDay;
    }
}