#include "StdInc.h"
#include "Game/Cheat.h"
#include "Game/Weather.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstdio>

namespace
{
    constexpr uint32 kCheatCount = 32;
    constexpr uint32 kCheatBufLen = 30;
    constexpr int32 kStatTimesCheated = 0x103;

    using CheatFn = int32(__cdecl*)();

    struct WeaponPackEntry
    {
        int32 weaponType;
        int32 ammo;
    };

    inline uint16* GetCheatStringBuf()
    {
        return reinterpret_cast<uint16*>(Core::Rebase(0x15E35A8));
    }

    inline uint8* GetCheatsActiveArray()
    {
        return reinterpret_cast<uint8*>(Core::Rebase(0x15E35E4));
    }

    inline uint32* GetCheatActivationTimes()
    {
        return reinterpret_cast<uint32*>(Core::Rebase(0x15E3608));
    }

    inline uint32* GetCheatHashes()
    {
        return reinterpret_cast<uint32*>(Core::Rebase(0xF2AA10));
    }

    inline CheatFn* GetCheatHandlers()
    {
        return reinterpret_cast<CheatFn*>(Core::Rebase(0xF2A990));
    }

    inline void* GetTextInstance()
    {
        return reinterpret_cast<void*>(Core::Rebase(0x10F4CE8));
    }

    inline void* GetHelpMsgInstance()
    {
        return reinterpret_cast<void*>(Core::Rebase(0xF1DC40));
    }

    inline uint32* GetInputQueueObject()
    {
        return reinterpret_cast<uint32*>(Core::Rebase(0x110A230));
    }

    inline uint32 GetFrameCounter()
    {
        return Core::StaticRef<uint32>(0x11DDF38);
    }

    inline uint32 GetTimeMs()
    {
        return static_cast<uint32>(Core::StaticRef<uint64>(0x11DDE74));
    }

    // 0x7E6F10
    int32 WideStrLen(const uint16* str)
    {
        int32 len = 0;
        while (*str)
        {
            ++str;
            ++len;
        }
        return len;
    }

    // 0x7E6E20
    void WideToAscii(const uint16* src, char* dst)
    {
        if (!src)
            return;

        while (*src)
        {
            *dst++ = static_cast<char>(*src & 0xFF);
            ++src;
        }
        *dst = '\0';
    }

    // 0x7BDBC0
    uint32 HashCheatString(const char* str)
    {
        const auto* crcTable = reinterpret_cast<const uint32*>(Core::Rebase(0xF0FE30));
        uint32 result = 0xFFFFFFFFu;
        for (; *str; ++str)
            result = crcTable[static_cast<uint8>(result ^ static_cast<uint8>(*str))] ^ (result >> 8);
        return result;
    }

    // 0x467E30
    bool IsAchievementUnlocked(int32 id)
    {
        const int32 count = Core::StaticRef<int32>(0x18A8344);
        const auto* unlocked = reinterpret_cast<const int32*>(Core::Rebase(0x1848B18));
        for (int32 i = 0; i < count; ++i)
        {
            if (unlocked[i] == id)
                return true;
        }
        return false;
    }

    // 0x75D2B0
    bool IsProfileSignedIn(const void* profile)
    {
        return (static_cast<const uint8*>(profile)[92] & 1) != 0;
    }

    // 0x9DEC10
    const uint16* GetInputQueue(int32 mode)
    {
        const uint32* queueObj = GetInputQueueObject();
        if (mode == static_cast<int32>(queueObj[1]) || mode == 2)
            return reinterpret_cast<const uint16*>(queueObj + 2);
        return nullptr;
    }

    // 0x8788D0 / 0x817F40
    uint8* FindPlayerPed()
    {
        const int32 activeProfile = Core::StaticRef<int32>(0xF1CC68);
        if (activeProfile == -1)
            return nullptr;

        auto** profiles = reinterpret_cast<uint8**>(Core::Rebase(0x11A7008));
        uint8* profile = profiles[activeProfile];
        return profile ? *reinterpret_cast<uint8**>(profile + 1420) : nullptr;
    }

    // 0x878890
    uint8* FindPlayerVehicle()
    {
        uint8* ped = FindPlayerPed();
        if (ped && (ped[620] & 4) != 0)
            return *reinterpret_cast<uint8**>(ped + 2880);
        return nullptr;
    }

    const void* GetTextString(const char* key)
    {
        return Core::Hook::CallMethod<const void*>(0x7B54C0, GetTextInstance(), key);
    }

    int32 ShowHelpText(const void* text)
    {
        return Core::Hook::CallMethod<int32>(
            0x85D1A0, GetHelpMsgInstance(),
            text, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, -1);
    }

    bool IsNetworkGame()
    {
        return Core::Hook::Call<uint8>(0x462F90) != 0;
    }

    void SendCheatTelemetry(const char* value)
    {
        Core::Hook::Call<void>(0x81E610, value);
    }

    void GiveWeaponPack(const WeaponPackEntry* entries, size_t count)
    {
        const int32 streamingFlags = Core::StaticRef<int32>(0x15F73A0);

        for (size_t i = 0; i < count; ++i)
        {
            void* weaponInfo = Core::Hook::Call<void*>(0x924E80, entries[i].weaponType);
            const int32 modelIdx = Core::Hook::CallMethod<int32>(0x925120, weaponInfo);
            Core::Hook::Call<void>(0x832C40, modelIdx, streamingFlags, 2);
        }

        Core::Hook::Call<void>(0x832C20, 0);

        for (size_t i = 0; i < count; ++i)
        {
            uint8* ped = FindPlayerPed();
            Core::Hook::CallMethod<void>(0x9AB750, ped + 0x2B0, entries[i].weaponType, entries[i].ammo, 0, 0, 0);
        }

        for (size_t i = 0; i < count; ++i)
        {
            void* weaponInfo = Core::Hook::Call<void*>(0x924E80, entries[i].weaponType);
            const int32 modelIdx = Core::Hook::CallMethod<int32>(0x925120, weaponInfo);
            Core::Hook::Call<void>(0x832AF0, modelIdx, streamingFlags);
        }
    }

    int32 SpawnVehicleCheat(uint32 modelIndexAddress, int32 cheatId)
    {
        const int32 modelIndex = Core::StaticRef<int32>(modelIndexAddress);
        Core::Hook::Call<void*>(0x96E6F0, modelIndex);
        GetCheatActivationTimes()[cheatId] = GetTimeMs();
        return CCheat::ShowCheatNotification(cheatId);
    }
}

void CCheat::InjectHooks()
{
    Core::Hook::Install(0x96D980, &CCheat::Reset);
    Core::Hook::Install(0x96D990, &CCheat::Init);
    Core::Hook::Install(0x96DA40, &CCheat::ResetCheats);
    Core::Hook::Install(0x96DA80, &CCheat::AddToCheatString);
    Core::Hook::Install(0x96DBB0, &CCheat::WasCheatActivatedRecently);
    Core::Hook::Install(0x96DBE0, &CCheat::RecordCheatStat);
    Core::Hook::Install(0x96DC10, &CCheat::ShowCheatNotification);
    Core::Hook::Install(0x96DE30, &CCheat::DoCheats);
    Core::Hook::Install(0x96DE70, &CCheat::WeaponSet1Cheat);
    Core::Hook::Install(0x96E1B0, &CCheat::WeaponSet2Cheat);
    Core::Hook::Install(0x96E500, &CCheat::HealthArmourAmmoCheat);
    Core::Hook::Install(0x96E610, &CCheat::WantedLevelUpCheat);
    Core::Hook::Install(0x96E6A0, &CCheat::WantedLevelDownCheat);
    Core::Hook::Install(0x96E6D0, &CCheat::ChangeWeatherCheat);
    Core::Hook::Install(0x96F200, &CCheat::SpawnAnnihilatorCheat);
    Core::Hook::Install(0x96F250, &CCheat::SpawnNrg900Cheat);
    Core::Hook::Install(0x96F280, &CCheat::SpawnFbiBuffaloCheat);
    Core::Hook::Install(0x96F2B0, &CCheat::SpawnJetmaxCheat);
    Core::Hook::Install(0x96F300, &CCheat::SpawnCometCheat);
    Core::Hook::Install(0x96F330, &CCheat::SpawnTurismoCheat);
    Core::Hook::Install(0x96F360, &CCheat::SpawnCognoscentiCheat);
    Core::Hook::Install(0x96F390, &CCheat::SpawnSuperGtCheat);
    Core::Hook::Install(0x96F3C0, &CCheat::SpawnSanchezCheat);
    Core::Hook::Install(0x96F3F0, &CCheat::SpawnInnovationCheat);
    Core::Hook::Install(0x96F420, &CCheat::SpawnHexerCheat);
    Core::Hook::Install(0x96F450, &CCheat::SpawnHakuchou2Cheat);
    Core::Hook::Install(0x96F480, &CCheat::SpawnDouble2Cheat);
    Core::Hook::Install(0x96F4B0, &CCheat::SpawnGangBurritoCheat);
    Core::Hook::Install(0x96F4E0, &CCheat::SpawnSlamvanCheat);
    Core::Hook::Install(0x96F510, &CCheat::WeaponSet3Cheat);
    Core::Hook::Install(0x96F850, &CCheat::SpawnFloaterCheat);
    Core::Hook::Install(0x96F880, &CCheat::SpawnVaderCheat);
    Core::Hook::Install(0x96F8B0, &CCheat::SpawnAkumaCheat);
    Core::Hook::Install(0x96F8E0, &CCheat::SpawnApcCheat);
    Core::Hook::Install(0x96F910, &CCheat::SpawnBuzzardCheat);
    Core::Hook::Install(0x96F940, &CCheat::SpawnBulletCheat);
    Core::Hook::Install(0x96F970, &CCheat::ParachuteCheat);
    Core::Hook::Install(0x96F9D0, &CCheat::ToggleSlowMoCheat);
    Core::Hook::Install(0x96FA00, &CCheat::ToggleExtraCheat);
    Core::Hook::Install(0x96FA30, &CCheat::ActivateCheat);

    Core::Log::Info("CCheat hooks installed (40 functions)");
}

void __cdecl CCheat::Reset()
{
    m_ResetFlag = 0;
}

void __cdecl CCheat::Init()
{
    uint32* times = GetCheatActivationTimes();
    for (uint32 i = 0; i < kCheatCount; ++i)
        times[i] = 0;
}

void __cdecl CCheat::ResetCheats()
{
    uint8* active = GetCheatsActiveArray();
    for (uint32 i = 0; i < kCheatCount; ++i)
        active[i] = 0;

    GetCheatStringBuf()[0] = 0;
    m_bHasPlayerCheated = false;
}

void __cdecl CCheat::AddToCheatString(uint16 ch)
{
    uint16* buf = GetCheatStringBuf();
    for (int32 i = kCheatBufLen - 1; i >= 1; --i)
        buf[i] = buf[i - 1];

    buf[0] = ch;
    buf[kCheatBufLen - 1] = 0;

    const int32 len = WideStrLen(buf);
    if (len < 6)
        return;

    char asciiBuf[32]{};
    WideToAscii(buf, asciiBuf);

    const uint32* hashes = GetCheatHashes();
    CheatFn* handlers = GetCheatHandlers();
    uint8* active = GetCheatsActiveArray();

    for (int32 curLen = len; curLen >= 6; --curLen)
    {
        const uint32 hash = HashCheatString(asciiBuf);
        for (uint32 i = 0; i < kCheatCount; ++i)
        {
            if (hashes[i] == hash)
            {
                const char* msgKey = active[i] ? "CHEAT2" : "CHEAT1";
                ShowHelpText(GetTextString(msgKey));

                m_bHasPlayerCheated = true;
                if (handlers[i])
                    handlers[i]();
                else
                    active[i] = (active[i] == 0) ? 1 : 0;

                buf[0] = 0;
                return;
            }
        }
        asciiBuf[curLen - 1] = 0;
    }
}

bool __cdecl CCheat::WasCheatActivatedRecently(int32 cheatId, uint32 timeWindowMs)
{
    const uint32 activatedAt = GetCheatActivationTimes()[cheatId];
    return activatedAt != 0 && GetTimeMs() <= activatedAt + timeWindowMs;
}

void __cdecl CCheat::RecordCheatStat()
{
    const uint32 frame = GetFrameCounter();
    if (frame != m_LastStatFrame)
    {
        Core::Hook::Call<void>(0x962B90, kStatTimesCheated, 1.0f);
        m_LastStatFrame = GetFrameCounter();
    }
}

int32 __cdecl CCheat::ShowCheatNotification(int32 cheatId)
{
    const void* msg = GetTextString("CHEAT1");
    bool isSignedIn = false;

    const int32 activeProfile = Core::StaticRef<int32>(0xF1CC68);
    if (activeProfile != -1)
    {
        void** profiles = reinterpret_cast<void**>(Core::Rebase(0x11A7008));
        void* profile = profiles[activeProfile];
        if (profile && IsProfileSignedIn(profile))
            isSignedIn = true;
    }

    const int32 netState = Core::StaticRef<int32>(0x10F8044);
    if (!IsNetworkGame() && isSignedIn && netState == 0)
    {
        switch (cheatId)
        {
        case 0:
        case 1:
        case 22:
            if (m_LastBlockedMsgTime == GetTimeMs())
                return cheatId;
            if (!IsAchievementUnlocked(19))
                msg = GetTextString("ACHBLOCKED1D");
            break;

        case 2:
            m_LastBlockedMsgTime = GetTimeMs();
            if (!IsAchievementUnlocked(5) && !IsAchievementUnlocked(15) && !IsAchievementUnlocked(11))
                msg = GetTextString("ACHBLOCKED1B");
            else if (!IsAchievementUnlocked(15) && !IsAchievementUnlocked(11))
                msg = GetTextString("ACHBLOCKED1C");
            else if (!IsAchievementUnlocked(5))
                msg = GetTextString("ACHBLOCKED1E");
            else if (!IsAchievementUnlocked(15))
                msg = GetTextString("ACHBLOCKED1F");
            else if (!IsAchievementUnlocked(11))
                msg = GetTextString("ACHBLOCKED1G");
            break;

        case 4:
        case 7:
        case 26:
        case 27:
        case 30:
        case 31:
            if (!IsAchievementUnlocked(15) && !IsAchievementUnlocked(11))
                msg = GetTextString("ACHBLOCKED1C");
            else if (!IsAchievementUnlocked(15))
                msg = GetTextString("ACHBLOCKED1F");
            else if (!IsAchievementUnlocked(11))
                msg = GetTextString("ACHBLOCKED1G");
            break;

        case 5:
            if (!IsAchievementUnlocked(25))
                msg = GetTextString("ACHBLOCKED2");
            break;

        default:
            break;
        }
    }

    return ShowHelpText(msg);
}

void __cdecl CCheat::DoCheats()
{
    if (IsNetworkGame())
        return;

    const uint16* queue = GetInputQueue(0);
    if (!queue)
        return;

    while (*queue)
    {
        AddToCheatString(*queue);
        --queue;
    }
}

int32 __cdecl CCheat::WeaponSet1Cheat()
{
    RecordCheatStat();

    static constexpr WeaponPackEntry kWeapons[] = {
        { 3, 1 },
        { 5, 10 },
        { 7, 120 },
        { 10, 50 },
        { 12, 150 },
        { 14, 100 },
        { 16, 200 },
        { 18, 20 },
    };
    GiveWeaponPack(kWeapons, sizeof(kWeapons) / sizeof(kWeapons[0]));

    GetCheatActivationTimes()[0] = GetTimeMs();
    return ShowCheatNotification(0);
}

int32 __cdecl CCheat::WeaponSet2Cheat()
{
    RecordCheatStat();

    const int32 episode = Core::StaticRef<int32>(0x10F8044);
    const int32 slot2Weapon = (episode == 2) ? 36 : 4;

    const WeaponPackEntry weapons[] = {
        { 1, 1 },
        { slot2Weapon, 10 },
        { 9, 120 },
        { 11, 50 },
        { 13, 150 },
        { 15, 100 },
        { 16, 200 },
        { 18, 20 },
    };
    GiveWeaponPack(weapons, sizeof(weapons) / sizeof(weapons[0]));

    GetCheatActivationTimes()[1] = GetTimeMs();
    return ShowCheatNotification(1);
}

int32 __cdecl CCheat::WeaponSet3Cheat()
{
    RecordCheatStat();

    static constexpr WeaponPackEntry kWeapons[] = {
        { 3, 1 },
        { 36, 10 },
        { 29, 120 },
        { 30, 50 },
        { 32, 150 },
        { 34, 100 },
        { 35, 200 },
        { 21, 20 },
    };
    GiveWeaponPack(kWeapons, sizeof(kWeapons) / sizeof(kWeapons[0]));

    GetCheatActivationTimes()[22] = GetTimeMs();
    return ShowCheatNotification(22);
}

int32 __cdecl CCheat::HealthArmourAmmoCheat()
{
    RecordCheatStat();

    uint8* ped = FindPlayerPed();
    const uint8* playerInfo = *reinterpret_cast<const uint8* const*>(ped + 552);
    const float maxHealth = static_cast<float>(*reinterpret_cast<const uint16*>(playerInfo + 1354));

    using SetHealthFn = void(__thiscall*)(void*, float, int32);
    void** pedVtable = *reinterpret_cast<void***>(ped);
    reinterpret_cast<SetHealthFn>(pedVtable[61])(ped, maxHealth, 0);

    const float maxArmour = static_cast<float>(*reinterpret_cast<const uint16*>(playerInfo + 1356));
    Core::Hook::CallMethod<void>(0x89E640, ped, maxArmour);

    uint8* veh = FindPlayerVehicle();
    if (veh)
    {
        const uint8* vehSub = *reinterpret_cast<const uint8* const*>(veh + 108);
        const uint32 vehFlags = *reinterpret_cast<const uint32*>(veh + 40);
        if ((!vehSub || vehSub[14] == 0) && (vehFlags & 0x7C00u) != 0xC00u)
        {
            void** vehVtable = *reinterpret_cast<void***>(veh);
            reinterpret_cast<SetHealthFn>(vehVtable[61])(veh, 1000.0f, 0);

            using FixVehicleFn = void(__thiscall*)(void*);
            reinterpret_cast<FixVehicleFn>(vehVtable[101])(veh);
        }
    }

    SendCheatTelemetry("H");
    GetCheatActivationTimes()[2] = GetTimeMs();
    return ShowCheatNotification(2);
}

int32 __cdecl CCheat::WantedLevelUpCheat()
{
    RecordCheatStat();

    uint8* ped = FindPlayerPed();
    uint8* playerInfo = *reinterpret_cast<uint8**>(ped + 552);
    void* wanted = playerInfo ? (playerInfo + 96) : nullptr;

    const int32 currentWanted = Core::Hook::CallMethod<int32>(0x920FF0, wanted);
    const int32 targetWanted = (currentWanted + 1 >= 6) ? 6 : (currentWanted + 1);

    Core::Hook::CallMethod<void>(0x9BEF50, ped, targetWanted);
    GetCheatActivationTimes()[3] = GetTimeMs();
    return ShowCheatNotification(3);
}

void __cdecl CCheat::WantedLevelDownCheat()
{
    GetCheatActivationTimes()[4] = GetTimeMs();
    ShowCheatNotification(4);
    RecordCheatStat();

    uint8* ped = FindPlayerPed();
    Core::Hook::CallMethod<void>(0x9BEF50, ped, 0);
}

void __cdecl CCheat::ChangeWeatherCheat()
{
    RecordCheatStat();
    CWeather::AdvanceWeather();
    GetCheatActivationTimes()[6] = GetTimeMs();
    ShowCheatNotification(6);
}

int32 __cdecl CCheat::ParachuteCheat()
{
    uint8* ped = FindPlayerPed();
    Core::Hook::CallMethod<void>(0x9AB750, ped + 0x2B0, 41, 1, 0, 0, 0);

    const int32 streamingFlags = Core::StaticRef<int32>(0x15F73A0);
    void* weaponInfo = Core::Hook::Call<void*>(0x924E80, 41);
    const int32 modelIdx = Core::Hook::CallMethod<int32>(0x925120, weaponInfo);
    Core::Hook::Call<void>(0x832AF0, modelIdx, streamingFlags);

    GetCheatActivationTimes()[29] = GetTimeMs();
    return ShowCheatNotification(29);
}

int32 __cdecl CCheat::ToggleSlowMoCheat()
{
    m_bSlowMo = !m_bSlowMo;
    GetCheatActivationTimes()[30] = GetTimeMs();
    return ShowCheatNotification(30);
}

int32 __cdecl CCheat::ToggleExtraCheat()
{
    m_bExtraCheatToggle = !m_bExtraCheatToggle;
    GetCheatActivationTimes()[31] = GetTimeMs();
    return ShowCheatNotification(31);
}

int32 __cdecl CCheat::ActivateCheat(int32 cheatId)
{
    GetCheatHandlers()[cheatId]();

    char buf[32]{};
    std::snprintf(buf, sizeof(buf), "%d", static_cast<int32>(GetCheatHashes()[cheatId]));
    SendCheatTelemetry(buf);

    return ShowCheatNotification(cheatId);
}

void __cdecl CCheat::SpawnAnnihilatorCheat()
{
    if (Core::StaticRef<uint8>(0x1690C11) != 0)
        return;

    const uint8* ped = FindPlayerPed();
    if (ped && (ped[620] & 4) != 0)
    {
        const uint8* veh = *reinterpret_cast<const uint8* const*>(ped + 2880);
        if (veh && *reinterpret_cast<const int32*>(veh + 4948) == 3)
            return;
    }

    SpawnVehicleCheat(0x12309DC, 7);
}

int32 __cdecl CCheat::SpawnNrg900Cheat()     { return SpawnVehicleCheat(0x12302C8, 8); }
int32 __cdecl CCheat::SpawnFbiBuffaloCheat() { return SpawnVehicleCheat(0x123022C, 9); }

void __cdecl CCheat::SpawnJetmaxCheat()
{
    if (Core::StaticRef<uint8>(0x1690C11) != 0)
        return;

    const uint8* ped = FindPlayerPed();
    if (ped && (ped[620] & 4) != 0)
    {
        const uint8* veh = *reinterpret_cast<const uint8* const*>(ped + 2880);
        if (veh && *reinterpret_cast<const int32*>(veh + 4948) == 3)
            return;
    }

    SpawnVehicleCheat(0x1230568, 10);
}

int32 __cdecl CCheat::SpawnCometCheat()       { return SpawnVehicleCheat(0x1230628, 11); }
int32 __cdecl CCheat::SpawnTurismoCheat()     { return SpawnVehicleCheat(0x1230688, 12); }
int32 __cdecl CCheat::SpawnCognoscentiCheat() { return SpawnVehicleCheat(0x1230994, 13); }
int32 __cdecl CCheat::SpawnSuperGtCheat()     { return SpawnVehicleCheat(0x123043C, 14); }
int32 __cdecl CCheat::SpawnSanchezCheat()     { return SpawnVehicleCheat(0x1230388, 15); }
int32 __cdecl CCheat::SpawnInnovationCheat()  { return SpawnVehicleCheat(0x12305C8, 16); }
int32 __cdecl CCheat::SpawnHexerCheat()       { return SpawnVehicleCheat(0x1230AA8, 17); }
int32 __cdecl CCheat::SpawnHakuchou2Cheat()   { return SpawnVehicleCheat(0x12307B4, 18); }
int32 __cdecl CCheat::SpawnDouble2Cheat()     { return SpawnVehicleCheat(0x1230844, 19); }
int32 __cdecl CCheat::SpawnGangBurritoCheat() { return SpawnVehicleCheat(0x12307F0, 20); }
int32 __cdecl CCheat::SpawnSlamvanCheat()     { return SpawnVehicleCheat(0x12307D8, 21); }
int32 __cdecl CCheat::SpawnFloaterCheat()     { return SpawnVehicleCheat(0x1230934, 23); }
int32 __cdecl CCheat::SpawnVaderCheat()       { return SpawnVehicleCheat(0x12303E8, 24); }
int32 __cdecl CCheat::SpawnAkumaCheat()       { return SpawnVehicleCheat(0x12303DC, 25); }
int32 __cdecl CCheat::SpawnApcCheat()         { return SpawnVehicleCheat(0x1230478, 26); }
int32 __cdecl CCheat::SpawnBuzzardCheat()     { return SpawnVehicleCheat(0x1230364, 27); }
int32 __cdecl CCheat::SpawnBulletCheat()      { return SpawnVehicleCheat(0x12307CC, 28); }