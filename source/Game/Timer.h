#pragma once

class CTimer {
public:
    static inline uint32& m_snTimeInMilliseconds = *reinterpret_cast<uint32*>(Core::Rebase(0x11DDF3C));
    static inline float&  ms_fTimeStep           = *reinterpret_cast<float*>(Core::Rebase(0x11DDE8C));
    static inline uint32& ms_nTimeMs             = *reinterpret_cast<uint32*>(Core::Rebase(0x11DDE74));
    static inline float&  ms_fFrameTimeStep      = *reinterpret_cast<float*>(Core::Rebase(0x11DDE7C));
    static inline float&  ms_fTimeScale          = *reinterpret_cast<float*>(Core::Rebase(0xF1D9D8));
    static inline uint32& m_FrameCounter         = *reinterpret_cast<uint32*>(Core::Rebase(0x11DDF38));
    static inline bool&   m_UserPause            = *reinterpret_cast<bool*>(Core::Rebase(0x11DDF41));
    static inline bool&   m_CodePause            = *reinterpret_cast<bool*>(Core::Rebase(0x11DDF42));

    static void InjectHooks();

    static bool  __cdecl GetIsPaused();
    static int32 __cdecl GetTimeStepInMilliseconds();
    static void  __cdecl StartUserPause();
    static void  __cdecl EndUserPause();
    static bool  __cdecl GetIsUserPaused();
    static void  __cdecl StartCodePause();
    static void  __cdecl EndCodePause();
    static bool  __cdecl GetIsCodePaused();
    static bool  __cdecl Stop();
    static void  __cdecl Update();
};
