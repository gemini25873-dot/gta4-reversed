#pragma once

class CGame
{
public:
    static inline int32&  ms_nCurrentEpisode = Core::StaticRef<int32>(0x10F8044);
    static inline int32&  ms_nLastEpisode    = Core::StaticRef<int32>(0x10F8048);
    static inline int32&  ms_nActiveEpisode  = Core::StaticRef<int32>(0x10F8064);
    static inline int32&  ms_nTargetEpisode  = Core::StaticRef<int32>(0x10F8078);
    static inline uint8&  ms_bEpisodeFlag    = Core::StaticRef<uint8>(0xF1D128);
    static inline bool&   ms_bGamePaused     = Core::StaticRef<bool>(0x18A81AF);
    static inline uint64& ms_qwShutdownTime  = Core::StaticRef<uint64>(0x18A8240);

    static void InjectHooks();

    static int32  __cdecl ShutdownCore();
    static int32  __cdecl OnEpisodeChanged();
    static int32  __cdecl ShutdownRenderWare();
    static int32  __cdecl ShutdownViewport();
    static char   __cdecl ReloadEpisode(char skipShutdown);
    static char   __cdecl RestartEpisode();
    static uint32 __cdecl ProcessTimeStep();
};
