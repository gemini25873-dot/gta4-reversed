#pragma once

class CStats
{
public:
    static inline uint8& m_bPopupGuard = Core::StaticRef<uint8>(0x15C2CF8);
    static inline int32& m_Counter1 = Core::StaticRef<int32>(0x15C2CFC);
    static inline int32& m_Counter2 = Core::StaticRef<int32>(0x15C2D00);
    static inline int32& m_Counter3 = Core::StaticRef<int32>(0x15C2D04);
    static inline int32& m_SessionTimer1 = Core::StaticRef<int32>(0x15C2D14);
    static inline int32& m_SessionTimer2 = Core::StaticRef<int32>(0x15C2D20);
    static inline int32& m_NumFloatStats = Core::StaticRef<int32>(0x15C42A4);
    static inline int32& m_NumIntStats = Core::StaticRef<int32>(0x15C4A90);
    static inline int32& m_StatHistoryCount = Core::StaticRef<int32>(0x15C2DAC);

    static void InjectHooks();

    static void __cdecl ResetCounters();
    static int32 __cdecl ResetAllTimers();
    static int32 __cdecl ResetSessionTimers();
    static double __cdecl GetStatMultiplier(int32 type);

    static float* __cdecl SyncFloatStats();
    static int32 __cdecl GetNumFloatStats();
    static int32* __cdecl SyncIntStats();
    static int32 __cdecl GetNumIntStats();
    static int32* __cdecl GetStatHistoryPtr();
    static int32 __cdecl GetStatHistoryCount();

    static double __cdecl GetStatValue(int32 statId);
    static const char* __cdecl GetStatString(int32 statId);
    static double __cdecl GetGameProgress();
    static double __cdecl GetShootingAccuracy();
    static int32 __cdecl GetCriminalRating();
    static void __cdecl CheckStatPopup(int32 unused, int32 statId);
    static void __cdecl UpdateAverageSpeedStat();
    static int32 __cdecl SetStatString(int32 statId, const char* str);
    static int32 __cdecl SetStatMinValue(int32 statId, int32 value);
};
