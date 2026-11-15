#pragma once

class CCheat
{
public:
    static inline uint16* const m_CheatString = reinterpret_cast<uint16*>(0); // accessed via Rebase in .cpp
    static inline uint32& m_LastStatFrame = Core::StaticRef<uint32>(0x15E3690);
    static inline uint32& m_LastBlockedMsgTime = Core::StaticRef<uint32>(0x15E3694);
    static inline uint32& m_ResetFlag = Core::StaticRef<uint32>(0x15E3604);
    static inline bool& m_bHasPlayerCheated = Core::StaticRef<bool>(0x15E3688);
    static inline bool& m_bSlowMo = Core::StaticRef<bool>(0x15E3602);
    static inline bool& m_bExtraCheatToggle = Core::StaticRef<bool>(0x15E3603);

    static void InjectHooks();

    static void __cdecl Reset();
    static void __cdecl ResetCheats();
    static void __cdecl AddToCheatString(uint16 ch);
    static bool __cdecl WasCheatActivatedRecently(int32 cheatId, uint32 timeWindowMs);
    static void __cdecl RecordCheatStat();
    static int32 __cdecl ShowCheatNotification(int32 cheatId);
    static void __cdecl DoCheats();

    static void __cdecl WantedLevelDownCheat();
    static void __cdecl ChangeWeatherCheat();
    static int32 __cdecl ToggleSlowMoCheat();
    static int32 __cdecl ToggleExtraCheat();
    static int32 __cdecl ActivateCheat(int32 cheatId);
};
