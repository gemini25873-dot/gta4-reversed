#include "StdInc.h"
#include "Game/WeaponInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    void RecordSimpleEvent(int32 eventId)
    {
        uint8 buf[52];
        Core::Hook::CallMethod<void*>(0xBCAF00, buf, eventId);
        Core::Hook::Call<int32>(0x81DB10, buf, 52);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }

    void RecordNamedEvent(int32 eventId, const char* name, bool flushAfter = false)
    {
        uint8 buf[52];
        Core::Hook::CallMethod<void*>(0xBCAF00, buf, eventId);
        const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
        Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
        Core::Hook::Call<int32>(0x81DB10, buf, 52);
        if (flushAfter)
            Core::Hook::Call<int32>(0x81DA10);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }
}

void CWeaponInfo::InjectHooks()
{
    // 0x81DB10 skipped: inside sub_75D510 (__thiscall, 2 args) call,
    // ECX/this argument is ambiguous in IDA decompile and byte_11D51D0 size is unknown.
    Core::Hook::Install(0x81DBE0, &CWeaponInfo::RecordEventByIndex);
    Core::Hook::Install(0x81DC60, &CWeaponInfo::RecordEvent9);
    Core::Hook::Install(0x81DCA0, &CWeaponInfo::RecordEvent10);
    Core::Hook::Install(0x81DCE0, &CWeaponInfo::RecordEvent4);
    Core::Hook::Install(0x81DD40, &CWeaponInfo::RecordEvent5);
    Core::Hook::Install(0x81DDA0, &CWeaponInfo::RecordEvent6);
    Core::Hook::Install(0x81DE10, &CWeaponInfo::RecordEvent23);
    Core::Hook::Install(0x81DE70, &CWeaponInfo::RecordEvent13);
    Core::Hook::Install(0x81DEB0, &CWeaponInfo::RecordEvent14);
    Core::Hook::Install(0x81DEF0, &CWeaponInfo::RecordEvent15);
    Core::Hook::Install(0x81DF30, &CWeaponInfo::RecordEvent17);
    Core::Hook::Install(0x81DF90, &CWeaponInfo::RecordEvent18);
    Core::Hook::Install(0x81DFF0, &CWeaponInfo::RecordEvent19);

    Core::Log::Info("CWeaponInfo hooks installed (13 functions, 0x81DB10 skipped)");
}

void __cdecl CWeaponInfo::RecordEventByIndex(int32 index)
{
    const char* name = reinterpret_cast<const char**>(Core::Rebase(0xF27A88))[index];
    if (!name)
        name = "UNKNOWN";

    uint8 buf[52];
    Core::Hook::CallMethod<void*>(0xBCAF00, buf, 11);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    Core::Hook::Call<int32>(0x81DB10, buf, 52);
    m_dwLastEventTime = Core::Hook::Call<uint32>(0x5AFB10) | 1u;
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent9()
{
    RecordSimpleEvent(9);
}

void __cdecl CWeaponInfo::RecordEvent10()
{
    RecordSimpleEvent(10);
}

void __cdecl CWeaponInfo::RecordEvent4(const char* name)
{
    RecordNamedEvent(4, name);
}

void __cdecl CWeaponInfo::RecordEvent5(const char* name)
{
    RecordNamedEvent(5, name);
}

void __cdecl CWeaponInfo::RecordEvent6(const char* name)
{
    RecordNamedEvent(6, name, true);
}

void __cdecl CWeaponInfo::RecordEvent23(const char* name)
{
    RecordNamedEvent(23, name);
}

void __cdecl CWeaponInfo::RecordEvent13()
{
    RecordSimpleEvent(13);
}

void __cdecl CWeaponInfo::RecordEvent14()
{
    RecordSimpleEvent(14);
}

void __cdecl CWeaponInfo::RecordEvent15()
{
    RecordSimpleEvent(15);
}

void __cdecl CWeaponInfo::RecordEvent17(const char* name)
{
    RecordNamedEvent(17, name);
}

void __cdecl CWeaponInfo::RecordEvent18(const char* name)
{
    RecordNamedEvent(18, name);
}

void __cdecl CWeaponInfo::RecordEvent19(const char* name)
{
    RecordNamedEvent(19, name);
}
