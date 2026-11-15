#pragma once

class CClock {
public:
    static inline int32&  ms_nForcedGameClockMinutes    = *reinterpret_cast<int32*>(Core::Rebase(0x11D52F0));
    static inline int32&  ms_nStoredGameClockHours      = *reinterpret_cast<int32*>(Core::Rebase(0x11D52F4));
    static inline int32&  ms_nGameClockMonths           = *reinterpret_cast<int32*>(Core::Rebase(0x11D52F8));
    static inline int32&  ms_nGameClockMinutes          = *reinterpret_cast<int32*>(Core::Rebase(0x11D52FC));
    static inline int32&  ms_nGameClockHours            = *reinterpret_cast<int32*>(Core::Rebase(0x11D5300));
    static inline int32&  ms_nMillisecondsPerGameMinute = *reinterpret_cast<int32*>(Core::Rebase(0x11D5304));
    static inline int32&  ms_nForcedGameClockHours      = *reinterpret_cast<int32*>(Core::Rebase(0x11D5308));
    static inline int32&  ms_nGameClockSeconds          = *reinterpret_cast<int32*>(Core::Rebase(0x11D530C));
    static inline int32&  CurrentDay                    = *reinterpret_cast<int32*>(Core::Rebase(0x11D5310));
    static inline int32&  ms_nGameClockDays             = *reinterpret_cast<int32*>(Core::Rebase(0x11D5314));
    static inline uint32& ms_nLastClockTick             = *reinterpret_cast<uint32*>(Core::Rebase(0x11D5318));

    static void InjectHooks();

    static void   __cdecl Initialise(int32 millisecondsPerGameMinute);
    static int32  __cdecl GetGameClockHours();
    static int32  __cdecl GetGameClockMinutes();
    static bool   __cdecl GetIsTimeInRange(int32 hourA, int32 hourB);
    static uint32 __cdecl GetGameClockMinutesUntil(int32 hours, int32 minutes);
    static void   __cdecl SetGameClockExact(int32 day, int32 hours, int32 minutes, int32 seconds);
    static void   __cdecl SetGameClock(int32 hours, int32 minutes, int32 seconds, int32 day);
    static void   __cdecl OffsetClockByADayOrAdvance(int32 hours, int32 minutes);
    static void   __cdecl Update();
};
