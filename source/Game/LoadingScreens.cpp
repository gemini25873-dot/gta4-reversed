#include "StdInc.h"
#include "Game/LoadingScreens.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    constexpr int32 kFlagNormal      = 0;
    constexpr int32 kFlagLegal       = 1;
    constexpr int32 kFlagLegal2      = 2;
    constexpr int32 kFlagStartIntro  = 3;
    constexpr int32 kFlagEndIntro    = 4;
    constexpr int32 kFlagInitialMain = 6;
    constexpr int32 kScreenStrideDwords = 100;

    inline int32* GetScreenDurations()
    {
        return reinterpret_cast<int32*>(Core::Rebase(0x18A8F40));
    }

    inline int32* GetScreenFlags()
    {
        return reinterpret_cast<int32*>(Core::Rebase(0x18A8F48));
    }

    inline int32* GetAnimBase()
    {
        return reinterpret_cast<int32*>(Core::Rebase(0x18A8FA0));
    }
}

void CLoadingScreens::InjectHooks()
{
    Core::Hook::Install(0x4240E0, &CLoadingScreens::ResetScreenAnimations);

    Core::Log::Info("CLoadingScreens hooks installed (1 function, intro skip enabled)");
}

int32* __cdecl CLoadingScreens::ResetScreenAnimations()
{
    const int32 count = m_nNumScreens;
    int32* result = reinterpret_cast<int32*>(static_cast<intptr_t>(count));

    if (count > 0)
    {
        int32* base = GetAnimBase();
        for (int32 i = 0; i < count; ++i)
        {
            int32* cur = base;
            for (int32 j = 0; j < 4; ++j)
            {
                cur[0] = cur[-8];
                cur[1] = cur[-7];
                cur[2] = cur[-4];
                cur[3] = cur[-3];
                cur[-10] = 0;
                cur[-9] = 0;
                cur += 24;
            }
            result = cur;
            base += kScreenStrideDwords;
        }
    }

    if (ms_bSkipIntroLogos && m_bIsInitialLoad && count > 0)
    {
        int32* flags = GetScreenFlags();
        int32* durations = GetScreenDurations();

        const int32 curIdx = m_nCurrentScreen;
        if (curIdx >= 0 && curIdx < count)
        {
            const int32 curFlag = flags[kScreenStrideDwords * curIdx];
            if (curFlag == kFlagLegal || curFlag == kFlagLegal2 ||
                curFlag == kFlagStartIntro || curFlag == kFlagEndIntro)
            {
                for (int32 i = 0; i < count; ++i)
                {
                    const int32 f = flags[kScreenStrideDwords * i];
                    if (f == kFlagLegal || f == kFlagLegal2 ||
                        f == kFlagStartIntro || f == kFlagEndIntro)
                    {
                        durations[kScreenStrideDwords * i] = 0;
                    }
                    else if (f == kFlagInitialMain)
                    {
                        m_nCurrentScreen = i;
                        m_bWaitForIntro = 0;
                        m_fTimer = 0.0f;
                        break;
                    }
                }
            }
        }
    }

    return result;
}
