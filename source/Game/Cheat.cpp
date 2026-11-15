#include "StdInc.h"
#include "Game/Cheat.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstdio>

namespace
{
    constexpr uint32 kCheatCount = 32;
    constexpr uint32 kCheatBufLen = 30;
    constexpr int32 kStatTimesCheated = 0x103;

    using CheatFn = int32(__cdecl*)();

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

    inline void* GetInputQueueInstance()
    {
        return reinterpret_cast<void*>(Core::Rebase(0x110A230));
    }

    inline uint32 GetFrameCounter()
    {
        return Core::StaticRef<uint32>(0x11DDF38);
    }

    inline uint32 GetTimeMs()
    {
        return static_cast<uint32>(Core::StaticRef<uint64>(0x11DDE74));
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

    bool IsAchievementUnlocked(int32 id)
    {
        return Core::Hook::Call<uint8>(0x467E30, id) != 0;
    }

    bool IsNetworkGame()
    {
        return Core::Hook::Call<uint8>(0x462F90) != 0;
    }
}

void CCheat::InjectHooks()
{
    Core::Hook::Install(0x96D980, &CCheat::Reset);
    Core::Hook::Install(0x96DA40, &CCheat::ResetCheats);
    Core::Hook::Install(0x96DA80, &CCheat::AddToCheatString);
    Core::Hook::Install(0x96DBB0, &CCheat::WasCheatActivatedRecently);
    Core::Hook::Install(0x96DBE0, &CCheat::RecordCheatStat);
    Core::Hook::Install(0x96DC10, &CCheat::ShowCheatNotification);
    Core::Hook::Install(0x96DE30, &CCheat::DoCheats);
    Core::Hook::Install(0x96E6A0, &CCheat::WantedLevelDownCheat);
    Core::Hook::Install(0x96E6D0, &CCheat::ChangeWeatherCheat);
    Core::Hook::Install(0x96F9D0, &CCheat::ToggleSlowMoCheat);
    Core::Hook::Install(0x96FA00, &CCheat::ToggleExtraCheat);
    Core::Hook::Install(0x96FA30, &CCheat::ActivateCheat);

    Core::Log::Info("CCheat hooks yuklendi (12 fonksiyon)");
}

void __cdecl CCheat::Reset()
{
    m_ResetFlag = 0;
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

    const int32 len = Core::Hook::Call<int32>(0x7E6F10, buf);
    if (len < 6)
        return;

    char asciiBuf[32]{};
    Core::Hook::Call<char*>(0x7E6E20, buf, asciiBuf);

    const uint32* hashes = GetCheatHashes();
    CheatFn* handlers = GetCheatHandlers();
    uint8* active = GetCheatsActiveArray();

    for (int32 curLen = len; curLen >= 6; --curLen)
    {
        const uint32 hash = Core::Hook::Call<uint32>(0x7BDBC0, asciiBuf);
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
        if (profile && Core::Hook::CallMethod<uint8>(0x75D2B0, profile) != 0)
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

    const uint16* queue = Core::Hook::CallMethod<const uint16*>(0x9DEC10, GetInputQueueInstance(), 0);
    if (!queue)
        return;

    while (*queue)
    {
        AddToCheatString(*queue);
        --queue;
    }
}

void __cdecl CCheat::WantedLevelDownCheat()
{
    GetCheatActivationTimes()[4] = GetTimeMs();
    ShowCheatNotification(4);
    RecordCheatStat();

    void* playerInfo = Core::Hook::Call<void*>(0x8788D0, 0);
    Core::Hook::CallMethod<void>(0x9BEF50, playerInfo, 0);
}

void __cdecl CCheat::ChangeWeatherCheat()
{
    RecordCheatStat();
    Core::Hook::Call<void>(0x9E42A0);
    GetCheatActivationTimes()[6] = GetTimeMs();
    ShowCheatNotification(6);
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
    Core::Hook::Call<void>(0x81E610, buf);

    return ShowCheatNotification(cheatId);
}
