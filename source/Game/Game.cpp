#include "StdInc.h"
#include "Game/Game.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    char CallLoadEpisode(int32 episodeId, int32 a2, char skipShutdown, char a4)
    {
        const uintptr_t fn = Core::Rebase(0x4205B0);
        const int32 argSkip = static_cast<int32>(skipShutdown);
        const int32 arg4 = static_cast<int32>(a4);
        char result = 0;
        __asm {
            push arg4
            push argSkip
            push a2
            mov eax, episodeId
            call fn
            add esp, 12
            mov result, al
        }
        return result;
    }

    void NotifyActiveThreadOnEpisodeSwitch()
    {
        const int32 idx = Core::StaticRef<int32>(0xF10A08);
        if (idx != -1)
        {
            uint8* base = reinterpret_cast<uint8*>(Core::Rebase(0x110A590));
            void* obj = base + 188 * idx;
            if (obj)
            {
                Core::Hook::CallMethod<void>(0x897220, obj, 1);
            }
        }
    }
} // namespace

void CGame::InjectHooks()
{
    Core::Hook::Install(0x41FE30, &CGame::ShutdownCore);
    Core::Hook::Install(0x4202A0, &CGame::OnEpisodeChanged);
    Core::Hook::Install(0x420580, &CGame::ShutdownRenderWare);
    Core::Hook::Install(0x420E10, &CGame::ShutdownViewport);
    Core::Hook::Install(0x420F60, &CGame::ReloadEpisode);
    Core::Hook::Install(0x420FB0, &CGame::RestartEpisode);
    Core::Hook::Install(0x4217D0, &CGame::ProcessTimeStep);

    Core::Log::Info("CGame hooks installed (7 functions)");
}

// 0x41FE30
int32 __cdecl CGame::ShutdownCore()
{
    Core::Log::InfoOnce("game_shutdown_core", "CGame::ShutdownCore executed");
    ms_qwShutdownTime = 0ULL;
    Core::Hook::Call<void>(0x8DE520);
    Core::Hook::Call<void>(0x8AD090);
    Core::Hook::Call<void>(0x80BAB0);
    Core::Hook::Call<void>(0x7DBA00);
    Core::Hook::Call<void>(0x832A60);
    Core::Hook::Call<void>(0x824D60);
    Core::Hook::Call<void>(0x902DA0);
    Core::Hook::Call<void>(0x794360);
    Core::Hook::Call<void>(0x984040);
    Core::Hook::Call<void>(0x88F700);

    int32 result = Core::StaticRef<int32>(0x11DC188);
    if (result)
    {
        result = Core::Hook::Call<int32>(0x5B1C10, result);
        Core::StaticRef<int32>(0x11DC188) = 0;
    }
    Core::StaticRef<int32>(0x11DC18C) = 0;
    return result;
}

// 0x4202A0
int32 __cdecl CGame::OnEpisodeChanged()
{
    Core::Log::InfoOnce("game_episode_changed", "CGame::OnEpisodeChanged executed (target=%d)", ms_nTargetEpisode);
    const int32 prevEpisode = ms_nCurrentEpisode;
    ms_nCurrentEpisode = ms_nTargetEpisode;
    ms_nActiveEpisode = ms_nTargetEpisode;

    Core::Hook::Call<void>(0x7B2DF0);

    uint32* playerPed = Core::Hook::Call<uint32*>(0x8788D0, 0);
    if (playerPed)
    {
        void** vtable = *reinterpret_cast<void***>(playerPed);
        using GetVehicleFn = int32(__thiscall*)(uint32*);
        int32 veh = reinterpret_cast<GetVehicleFn>(vtable[40])(playerPed);
        int32 node = 0;
        if (veh)
        {
            veh = reinterpret_cast<GetVehicleFn>(vtable[40])(playerPed);
            void** vehVtable = *reinterpret_cast<void***>(veh);
            using GetModelFn = int32(__thiscall*)(int32);
            node = reinterpret_cast<GetModelFn>(vehVtable[56])(veh);
        }
        else
        {
            node = static_cast<int32>(playerPed[64]);
        }
        Core::Hook::Call<void>(0x930CA0, *reinterpret_cast<uint32*>(node + 4));
    }

    Core::Hook::CallMethod<void>(0x814300, Core::StaticRef<void*>(0x19AB8F0));

    if (prevEpisode != ms_nTargetEpisode)
    {
        Core::Hook::Call<void>(0x848390);
        Core::Hook::Call<void>(0x42E380);
    }

    const int32 result = Core::Hook::Call<int32>(0x82DF50);
    ms_bEpisodeFlag = 0;
    return result;
}

// 0x420580
int32 __cdecl CGame::ShutdownRenderWare()
{
    void* obj = Core::StaticRef<void*>(0x19AB8F0);
    if (obj)
    {
        Core::Hook::CallMethod<void>(0x814EF0, obj);
    }
    return Core::Hook::CallMethod<int32>(0x8015C0, &Core::StaticRef<uint32>(0x18A8D78), 0);
}

// 0x420E10
int32 __cdecl CGame::ShutdownViewport()
{
    const uint32 vp = Core::StaticRef<uint32>(0x18A8DD8);
    if (vp)
    {
        uint32* inner = *reinterpret_cast<uint32**>(vp + 4);
        void** vtable = *reinterpret_cast<void***>(inner);
        using Fn8 = void(__thiscall*)(uint32*, int32);
        reinterpret_cast<Fn8>(vtable[2])(inner, 1);

        const uint32 sub = *reinterpret_cast<uint32*>(vp + 40);
        if (sub)
        {
            using Fn16 = void(__thiscall*)(uint32*, uint32);
            reinterpret_cast<Fn16>(vtable[4])(inner, *reinterpret_cast<uint32*>(sub + 16));
        }
        *reinterpret_cast<uint8*>(vp + 52) = 1;
    }

    Core::Hook::CallMethod<void>(0x7B3850, &Core::StaticRef<uint32>(0x10F47E0));
    Core::Hook::Call<void>(0x40ACE0);
    Core::Hook::CallMethod<void>(0x833460, &Core::StaticRef<uint32>(0x11DC4B0), 0, 0, 1);

    const int32 result = Core::StaticRef<int32>(0x10F47F4);
    if (result)
    {
        *reinterpret_cast<uint8*>(result + 1376) = 1;
    }
    return result;
}

// 0x420F60
char __cdecl CGame::ReloadEpisode(char skipShutdown)
{
    Core::Hook::Call<void>(0x822D90, 1);
    NotifyActiveThreadOnEpisodeSwitch();
    return CallLoadEpisode(ms_nLastEpisode, 0, skipShutdown, 0);
}

// 0x420FB0
char __cdecl CGame::RestartEpisode()
{
    NotifyActiveThreadOnEpisodeSwitch();
    return CallLoadEpisode(ms_nActiveEpisode, 0, 0, 0);
}

// 0x4217D0
uint32 __cdecl CGame::ProcessTimeStep()
{
    Core::Hook::CallMethod<void>(0x7B4030, &Core::StaticRef<uint32>(0x10F47E0));

    const int32 d0 = Core::StaticRef<int32>(0x10D0170);
    const int32 d1 = Core::StaticRef<int32>(0x10E1984);
    const int32 state = Core::StaticRef<int32>(0xF0E4A0);
    if ((d0 == 1 || d1 != -1 || state == 18) &&
        (state == 2 || state == 7) &&
        Core::StaticRef<uint8>(0x10D00D8))
    {
        Core::StaticRef<int32>(0xF0E49C) =
            Core::StaticRef<int32>(0xF21C9C) + Core::StaticRef<int32>(0xF21C98);
        Core::StaticRef<uint8>(0x10D017B) = Core::StaticRef<uint8>(0x121DFA8);
    }

    if (!ms_bGamePaused)
    {
        return Core::Hook::Call<uint32>(0x7E0F60);
    }

    Core::Hook::Call<void>(0x7E0B10);
    Core::Hook::CallMethod<void>(0x959F00, &Core::StaticRef<uint32>(0x15BFF28));

    const uint32* timing = reinterpret_cast<const uint32*>(Core::Rebase(0x15BFF28));
    const uint32 delta = timing[timing[3] + 5] - timing[timing[3] + 7];
    uint32& maxDelta = Core::StaticRef<uint32>(0x15C0774);
    if (delta > maxDelta)
    {
        maxDelta = delta;
    }
    return delta;
}
