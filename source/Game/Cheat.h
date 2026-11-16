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
    static void __cdecl Init();
    static void __cdecl ResetCheats();
    static void __cdecl AddToCheatString(uint16 ch);
    static bool __cdecl WasCheatActivatedRecently(int32 cheatId, uint32 timeWindowMs);
    static void __cdecl RecordCheatStat();
    static int32 __cdecl ShowCheatNotification(int32 cheatId);
    static void __cdecl DoCheats();

    static int32 __cdecl WeaponSet1Cheat();
    static int32 __cdecl WeaponSet2Cheat();
    static int32 __cdecl WeaponSet3Cheat();
    static int32 __cdecl HealthArmourAmmoCheat();
    static int32 __cdecl WantedLevelUpCheat();
    static void __cdecl WantedLevelDownCheat();
    static void __cdecl ChangeWeatherCheat();
    static int32 __cdecl ParachuteCheat();
    static int32 __cdecl ToggleSlowMoCheat();
    static int32 __cdecl ToggleExtraCheat();
    static int32 __cdecl ActivateCheat(int32 cheatId);

    static void __cdecl SpawnAnnihilatorCheat();
    static int32 __cdecl SpawnNrg900Cheat();
    static int32 __cdecl SpawnFbiBuffaloCheat();
    static void __cdecl SpawnJetmaxCheat();
    static int32 __cdecl SpawnCometCheat();
    static int32 __cdecl SpawnTurismoCheat();
    static int32 __cdecl SpawnCognoscentiCheat();
    static int32 __cdecl SpawnSuperGtCheat();
    static int32 __cdecl SpawnSanchezCheat();
    static int32 __cdecl SpawnInnovationCheat();
    static int32 __cdecl SpawnHexerCheat();
    static int32 __cdecl SpawnHakuchou2Cheat();
    static int32 __cdecl SpawnDouble2Cheat();
    static int32 __cdecl SpawnGangBurritoCheat();
    static int32 __cdecl SpawnSlamvanCheat();
    static int32 __cdecl SpawnFloaterCheat();
    static int32 __cdecl SpawnVaderCheat();
    static int32 __cdecl SpawnAkumaCheat();
    static int32 __cdecl SpawnApcCheat();
    static int32 __cdecl SpawnBuzzardCheat();
    static int32 __cdecl SpawnBulletCheat();
};