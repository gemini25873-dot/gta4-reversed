#pragma once

class CLoadingScreens
{
public:
    static inline int32& m_nCurrentScreen = Core::StaticRef<int32>(0x18A8254);
    static inline int32& m_nNumScreens    = Core::StaticRef<int32>(0x18A8258);
    static inline bool&  m_bIsInitialLoad = Core::StaticRef<bool>(0x18A825E);
    static inline float& m_fTimer         = Core::StaticRef<float>(0x18A824C);
    static inline uint8& m_bWaitForIntro  = Core::StaticRef<uint8>(0xFC7F7F);

    static inline bool ms_bSkipIntroLogos = true;

    static void InjectHooks();

    static int32* __cdecl ResetScreenAnimations();
};
