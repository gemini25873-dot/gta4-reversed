#include "StdInc.h"
#include "Game/PlayerInfo.h"
#include "Game/Ped.h"
#include "Game/Vehicle.h"
#include "Core/Vector.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    CPlayerInfo* (__cdecl* const kGetPlayerInfo)(uint32)       = &CPlayerInfo::GetPlayerInfo;
    CPed*        (__cdecl* const kFindPlayerPedFast)()          = &CPlayerInfo::FindPlayerPedFast;
    CVector*     (__cdecl* const kFindPlayerCoors)(CVector*)    = &CPlayerInfo::FindPlayerCoors;
    CVehicle*    (__cdecl* const kFindPlayerVehicle)(CPlayerInfo*) = &CPlayerInfo::FindPlayerVehicle;
    CPed*        (__cdecl* const kFindPlayerPed)(CPlayerInfo*)  = &CPlayerInfo::FindPlayerPed;

    // LTCG leaf callers in GTAIV.exe (e.g. 0x818090) assume these helpers preserve ECX and EDX.
    void __declspec(naked) HookedFindPlayerPedFast()
    {
        __asm {
            push ecx
            push edx
            mov eax, kFindPlayerPedFast
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetPlayerInfo()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetPlayerInfo
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedFindPlayerCoors()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kFindPlayerCoors
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedFindPlayerVehicle()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kFindPlayerVehicle
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedFindPlayerPed()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kFindPlayerPed
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }
}

void CPlayerInfo::InjectHooks()
{
    Core::Hook::Install(0x87AB70, &CPlayerInfo::Ctor);
    Core::Hook::Install(0x8786F0, &CPlayerInfo::SetPlayerPed);
    Core::Hook::Install(0x878720, &CPlayerInfo::SetRemoteVehicle);
    Core::Hook::Install(0x878850, &CPlayerInfo::GetPos);
    Core::Hook::Install(0x878900, &CPlayerInfo::IsWasted);
    Core::Hook::Install(0x878910, &CPlayerInfo::IsBusted);
    Core::Hook::Install(0x878990, &CPlayerInfo::HasFlagsFBFF);
    Core::Hook::Install(0x8789B0, &CPlayerInfo::HasFlagsCFFE);
    Core::Hook::Install(0x878A40, &CPlayerInfo::GetMoney);
    Core::Hook::Install(0x878A50, &CPlayerInfo::SetMoney);
    Core::Hook::Install(0x878AA0, &CPlayerInfo::GetDisplayMoney);
    Core::Hook::Install(0x878AB0, &CPlayerInfo::SetDisplayMoney);
    Core::Hook::Install(0x817DC0, &CPlayerInfo::GetPlayerIndexFromPed);
    Core::Hook::Install(0x817F20, &HookedGetPlayerInfo);
    Core::Hook::Install(0x817F40, &HookedFindPlayerPedFast);
    Core::Hook::Install(0x818050, &HookedFindPlayerCoors);
    Core::Hook::Install(0x878890, &HookedFindPlayerVehicle);
    Core::Hook::Install(0x8788D0, &HookedFindPlayerPed);

    Core::Log::Info("CPlayerInfo hooks installed (18 functions)");
}

// 0x87AB70
CPlayerInfo* __thiscall CPlayerInfo::Ctor()
{
    Core::Hook::CallMethod<void>(0x75D120, m_gamerInfo);
    Core::Hook::CallMethod<void>(0x87AB30, &m_Wanted);

    m_nUnk570 = 0;
    m_nUnk574 = -1;
    m_nUnk578 = 0;
    m_nUnk57C = 0;
    m_nUnk580 = 0;
    m_nUnk581 = 0;
    Core::Hook::CallMethod<void>(0x447290, &m_nMoneyEncrypted);
    Core::Hook::CallMethod<void>(0x447290, &m_nDisplayMoneyEncrypted);
    m_nPlayerIndex = -1;
    m_nUnk584 = 0;
    m_pUnk5B4 = nullptr;
    m_pRemoteVehicle = nullptr;
    Core::Hook::CallMethod<void>(0x878AF0, this);

    for (float& f : m_fUnk598)
        f = 1.0f;

    return this;
}

// 0x8786F0
void __thiscall CPlayerInfo::SetPlayerPed(CPed* ped)
{
    if (m_pPed)
        Core::Hook::CallMethod<void>(0x9E6580, m_pPed, &m_pPed);
    m_pPed = ped;
    if (ped)
        Core::Hook::CallMethod<void>(0x9E6550, ped, &m_pPed);
}

// 0x878720
void __thiscall CPlayerInfo::SetRemoteVehicle(CEntity* veh)
{
    if (m_pRemoteVehicle)
        Core::Hook::CallMethod<void>(0x9E6580, m_pRemoteVehicle, &m_pRemoteVehicle);
    m_pRemoteVehicle = veh;
    if (veh)
        Core::Hook::CallMethod<void>(0x9E6550, veh, &m_pRemoteVehicle);
}

// 0x878850
CVector* __thiscall CPlayerInfo::GetPos(CVector* outPos) const
{
    const CEntity* target = m_pPed;
    if ((m_pPed->m_nPedFlags2 & 4u) != 0 && m_pPed->m_pMyVehicle)
        target = m_pPed->m_pMyVehicle;

    const CVector& pos = target->m_pMatrix->pos;
    outPos->x = pos.x;
    outPos->y = pos.y;
    outPos->z = pos.z;
    return outPos;
}

// 0x878900
bool __thiscall CPlayerInfo::IsWasted() const
{
    return m_nPlayerState == 3;
}

// 0x878910
bool __thiscall CPlayerInfo::IsBusted() const
{
    return m_nPlayerState == 4;
}

// 0x878990
bool __thiscall CPlayerInfo::HasFlagsFBFF() const
{
    return (m_nFlags & 0xFFFFFBFFu) != 0;
}

// 0x8789B0
bool __thiscall CPlayerInfo::HasFlagsCFFE() const
{
    return (m_nFlags & 0xFFFFCFFEu) != 0;
}

// 0x878A40
int32 __thiscall CPlayerInfo::GetMoney()
{
    return Core::Hook::CallMethod<int32>(0x447370, &m_nMoneyEncrypted);
}

// 0x878A50
void __thiscall CPlayerInfo::SetMoney(int32 amount)
{
    int32 clamped = (amount < 0) ? 0 : amount;
    if (clamped >= 999999999)
        clamped = 999999999;
    Core::Hook::CallMethod<void>(0x447350, &m_nMoneyEncrypted, clamped);
}

// 0x878AA0
int32 __thiscall CPlayerInfo::GetDisplayMoney()
{
    return Core::Hook::CallMethod<int32>(0x447370, &m_nDisplayMoneyEncrypted);
}

// 0x878AB0
void __thiscall CPlayerInfo::SetDisplayMoney(int32 amount)
{
    int32 clamped = (amount < 0) ? 0 : amount;
    if (clamped >= 999999999)
        clamped = 999999999;
    Core::Hook::CallMethod<void>(0x447350, &m_nDisplayMoneyEncrypted, clamped);
}

// 0x817DC0
int32 __cdecl CPlayerInfo::GetPlayerIndexFromPed(const CPed* ped)
{
    for (int32 i = 0; i < 32; ++i)
    {
        if (ms_apPlayers[i] && ms_apPlayers[i]->m_pPed == ped)
            return i;
    }
    return -1;
}

// 0x817F20
CPlayerInfo* __cdecl CPlayerInfo::GetPlayerInfo(uint32 index)
{
    return (index > 0x1Fu) ? nullptr : ms_apPlayers[index];
}

// 0x817F40
CPed* __cdecl CPlayerInfo::FindPlayerPedFast()
{
    if (ms_nLocalPlayerIndex == -1)
        return nullptr;
    CPlayerInfo* info = ms_apPlayers[ms_nLocalPlayerIndex];
    return info ? info->m_pPed : nullptr;
}

// 0x818050
CVector* __cdecl CPlayerInfo::FindPlayerCoors(CVector* outPos)
{
    CPed* ped = FindPlayerPedFast();
    if (ped)
    {
        const CVector& pos = ped->m_pMatrix->pos;
        outPos->x = pos.x;
        outPos->y = pos.y;
        outPos->z = pos.z;
    }
    else
    {
        outPos->x = 0.0f;
        outPos->y = 0.0f;
        outPos->z = 0.0f;
    }
    return outPos;
}

// 0x878890
CVehicle* __cdecl CPlayerInfo::FindPlayerVehicle(CPlayerInfo* info)
{
    if (!info)
    {
        if (ms_nLocalPlayerIndex == -1)
            return nullptr;
        info = ms_apPlayers[ms_nLocalPlayerIndex];
        if (!info)
            return nullptr;
    }

    CPed* ped = info->m_pPed;
    if (ped && (ped->m_nPedFlags2 & 4u) != 0)
        return ped->m_pMyVehicle;
    return nullptr;
}

// 0x8788D0
CPed* __cdecl CPlayerInfo::FindPlayerPed(CPlayerInfo* info)
{
    if (!info)
    {
        if (ms_nLocalPlayerIndex == -1)
            return nullptr;
        info = ms_apPlayers[ms_nLocalPlayerIndex];
        if (!info)
            return nullptr;
    }
    return info->m_pPed;
}
