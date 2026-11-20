#include "StdInc.h"
#include "Game/PlayerInfo.h"
#include "Game/Ped.h"
#include "Game/Vehicle.h"
#include "Core/Vector.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <algorithm>
#include <cmath>

namespace
{
    struct PlayerVehSearchCtx
    {
        CPed*     ped;       // +0x00
        CVehicle* bestVeh;   // +0x04
        float     bestScore; // +0x08
        uint32    pad0C;     // +0x0C
        CVector   pos;       // +0x10..+0x1B
        uint32    pad1C;     // +0x1C
        bool      bFlag;     // +0x20
    };

    float GetEntityVelSq(CEntity* entity)
    {
        CVector vel{};
        using GetVelFn = CVector*(__thiscall*)(CEntity*, CVector*);
        const CVector* v = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(entity))[0xEC / 4])(entity, &vel);
        return v->x * v->x + v->y * v->y + v->z * v->z;
    }

    bool IsVehWheelOnGround(CVehicle* veh, int32 wheelId)
    {
        uint8* wheel = Core::Hook::CallMethod<uint8*>(0x9C5510, veh, wheelId);
        return wheel && (*reinterpret_cast<const uint32*>(wheel + 0x164) & 1u) != 0;
    }

    bool IsVehWheelOffGround(CVehicle* veh, int32 wheelId)
    {
        uint8* wheel = Core::Hook::CallMethod<uint8*>(0x9C5510, veh, wheelId);
        return wheel != nullptr && (*reinterpret_cast<const uint32*>(wheel + 0x164) & 1u) == 0;
    }

    bool IsVehWheelPairOffGround(CVehicle* veh, int32 primaryWheel, int32 secondaryWheel)
    {
        return IsVehWheelOffGround(veh, primaryWheel) || IsVehWheelOffGround(veh, secondaryWheel);
    }

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
    Core::Hook::Install(0x878AF0, &CPlayerInfo::Clear);
    Core::Hook::Install(0x8795B0, &CPlayerInfo::Process);
    Core::Hook::Install(0x878750, &CPlayerInfo::UpdateAsyncProbe);
    Core::Hook::Install(0x878F70, &CPlayerInfo::ProcessWorldBounds);
    Core::Hook::Install(0x879220, &CPlayerInfo::SetPlayerControl);
    Core::Hook::Install(0x8786F0, &CPlayerInfo::SetPlayerPed);
    Core::Hook::Install(0x878720, &CPlayerInfo::SetRemoteVehicle);
    Core::Hook::Install(0x878850, &CPlayerInfo::GetPos);
    Core::Hook::Install(0x878900, &CPlayerInfo::IsWasted);
    Core::Hook::Install(0x878910, &CPlayerInfo::IsBusted);
    Core::Hook::Install(0x878920, &CPlayerInfo::SetWasted);
    Core::Hook::Install(0x878950, &CPlayerInfo::SetBusted);
    Core::Hook::Install(0x878990, &CPlayerInfo::HasFlagsFBFF);
    Core::Hook::Install(0x8789B0, &CPlayerInfo::HasFlagsCFFE);
    Core::Hook::Install(0x8789D0, &CPlayerInfo::HasFlagsEBFF);
    Core::Hook::Install(0x878A00, &CPlayerInfo::CanControlPlayer);
    Core::Hook::Install(0x878A40, &CPlayerInfo::GetMoney);
    Core::Hook::Install(0x878A50, &CPlayerInfo::SetMoney);
    Core::Hook::Install(0x878A80, &CPlayerInfo::AddMoney);
    Core::Hook::Install(0x878AA0, &CPlayerInfo::GetDisplayMoney);
    Core::Hook::Install(0x878AB0, &CPlayerInfo::SetDisplayMoney);
    Core::Hook::Install(0x8786D0, &CPlayerInfo::IsUnk16394E4Ready);
    Core::Hook::Install(0x878D30, &CPlayerInfo::ScoreVehicleCandidate);
    Core::Hook::Install(0x87A090, &CPlayerInfo::VehicleSearchCallback);
    Core::Hook::Install(0x87A450, &CPlayerInfo::HeliSearchCallback);
    Core::Hook::Install(0x87A840, &CPlayerInfo::FindVehicleToEnter);
    Core::Hook::Install(0x87AA70, &CPlayerInfo::FindHeliToEnter);
    Core::Hook::Install(0x817DC0, &CPlayerInfo::GetPlayerIndexFromPed);
    Core::Hook::Install(0x817F20, &HookedGetPlayerInfo);
    Core::Hook::Install(0x817F40, &HookedFindPlayerPedFast);
    Core::Hook::Install(0x818050, &HookedFindPlayerCoors);
    Core::Hook::Install(0x878890, &HookedFindPlayerVehicle);
    Core::Hook::Install(0x8788D0, &HookedFindPlayerPed);

    Core::Log::Info("CPlayerInfo hooks installed (34 functions)");
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


// 0x8786D0
bool __cdecl CPlayerInfo::IsUnk16394E4Ready()
{
    return Core::StaticRef<float>(0x16394E4) >= Core::StaticRef<const float>(0xDCB2A8);
}

// 0x878750
void __thiscall CPlayerInfo::UpdateAsyncProbe()
{
    if (m_pUnk5B4)
    {
        const int32 status = Core::Hook::Call<int32>(0xBCA4F0, m_pUnk5B4, 0);
        if (status == 2)
            return;
        if (status != 3)
            m_bUnk5AC = (status == 0) ? 1 : 0;
        m_pUnk5B4 = nullptr;
        return;
    }

    if ((m_pPed->m_nFlags & 0x08000000u) != 0)
    {
        m_nUnk5B0 = 0x2710;
        m_bUnk5AC = 1;
        return;
    }

    if (!IsUnk16394E4Ready())
        return;

    m_nUnk5B0 -= Core::Hook::Call<int32>(0x448330);
    if (m_nUnk5B0 > 0)
        return;

    m_nUnk5B0 = 0x2710;
    const CVector& pedPos = m_pPed->m_pMatrix->pos;
    CVector targetPos = pedPos;
    targetPos.z += Core::StaticRef<const float>(0xD50978);
    m_pUnk5B4 = Core::Hook::Call<void*>(0xBCAD70, &pedPos, &targetPos, 1, 9, 0, 6, 0);
}

// 0x878920
void __thiscall CPlayerInfo::SetWasted()
{
    if (m_nPlayerState == 2)
    {
        m_nPlayerState = 3;
        Core::Hook::Call<void>(0x962B90, 0x105, 1.0f);
        Core::Hook::Call<void>(0x822D90, 1);
    }
}

// 0x878950
void __thiscall CPlayerInfo::SetBusted()
{
    if (m_nPlayerState == 2)
    {
        m_nPlayerState = 4;
        m_nUnk558 = 0;
        Core::Hook::Call<void>(0x962B90, 0x1A3, 1.0f);
        Core::Hook::Call<void>(0x822D90, 1);
    }
}

// 0x8789D0
bool __thiscall CPlayerInfo::HasFlagsEBFF() const
{
    if (Core::Hook::Call<bool>(0x462F90))
        return (m_nFlags & 0xFFFFEBFFu) != 0;
    return m_nFlags != 0;
}

// 0x878A00
bool __thiscall CPlayerInfo::CanControlPlayer() const
{
    if (!Core::Hook::Call<bool>(0x462F90))
        return true;

    if (m_nUnk56C)
    {
        void* obj = reinterpret_cast<void*>(m_nUnk56C);
        if (Core::Hook::CallMethod<bool>(0x49F3F0, obj) && Core::Hook::CallMethod<bool>(0x4A1100, obj))
            return true;
    }
    return false;
}

// 0x878A80
void __thiscall CPlayerInfo::AddMoney(int32 delta)
{
    SetMoney(GetMoney() + delta);
}

// 0x878AF0
void __thiscall CPlayerInfo::Clear()
{
    Core::Hook::CallMethod<void>(0x75D180, m_gamerInfo);
    m_pPed = nullptr;
    if (m_pRemoteVehicle)
        Core::Hook::CallMethod<void>(0x9E6580, m_pRemoteVehicle, &m_pRemoteVehicle);
    m_pRemoteVehicle = nullptr;

    SetMoney(0);
    SetDisplayMoney(0);

    m_nUnk4B0 = 0;
    m_nPlayerState = 2;
    m_nUnk534 = 0;
    m_nUnk4E0 = 0;
    m_nUnk4E1 = 0;
    m_nUnk4E2 = 0;
    Core::StaticRef<uint32>(0x1724200) = 0;
    m_fUnk4CC = 0.0f;
    m_nUnk4D9 = 0;
    m_nUnk4D8 = 0;
    m_nUnk4D0 = 0;
    m_nUnk4D4 = Core::StaticRef<uint32>(0x11DDE74);
    m_nUnk4B4 = 0;
    m_nUnk4B8 = 3;
    m_nUnk4C8 = 0;
    m_nUnk4E8 = 0;
    m_nUnk4EC = 0;
    m_nUnk4F0 = 0;
    m_nUnk4F8 = 0;
    m_nUnk4F4 = 0;
    Core::StaticRef<int32>(0x1724210) = 0x64;
    m_nMaxArmour = 100;

    m_nUnk4FC = 0;
    m_fUnk500 = 0.0f;
    m_nUnk504 = 0;
    m_nUnk508 = 0;
    m_fUnk50C = 0.0f;
    m_nUnk510 = 0;
    m_fUnk514 = 0.0f;
    m_nUnk518 = 0;
    m_nUnk51C = 0;
    m_fUnk520 = 0.0f;
    m_nUnk524 = 0;
    m_fUnk528 = 0.0f;
    m_nUnk52C = 0;
    m_fUnk530 = 0.0f;
    m_nUnk546 = 0;
    m_nUnk547 = 0;
    m_nUnk548 = 0;
    m_nMaxHealth = 200;
    m_nUnk54E = 0;
    m_nUnk54F = 0;
    m_bUnk5AC = 0;
    m_nUnk5B0 = 0;

    if (m_pUnk5B4)
        Core::Hook::Call<void>(0xBCA130, m_pUnk5B4);
    m_pUnk5B4 = nullptr;

    m_nUnk550 = 1;
    m_nUnk551 = 0;
    m_nUnk554 = (m_nUnk554 & 0xFFFFFFF3u) | 3u;
    m_nUnk538 = 0;
    m_nUnk53C = 0;
    m_nUnk540 = 0;
    m_nUnk544 = 0;

    Core::StaticRef<void*>(0x1724214) = &m_nUnk4B0;
    m_nUnk55A = 1;
    m_nUnk558 = 0;
    m_nUnk55C = 0;
    m_nUnk560 = 0;
    m_nUnk588 = 0;

    using InitUnkFn = void(__stdcall*)(void*, void*, uint32, void*);
    (*Core::StaticRef<InitUnkFn>(0xD4D4B8))(
        Core::StaticRef<void*>(0x1724234),
        Core::StaticRef<void*>(0x1724240),
        0,
        reinterpret_cast<void*>(Core::Rebase(0x1724200)));

    m_nFlags = 0;
    m_nUnk56C = 0;
    m_nUnk570 = 0;
    m_nUnk578 = 0;
    m_nUnk57C = 0;
    m_nUnk580 = 0;
    m_nUnk581 = 0;
    m_nUnk574 = -1;
}

// 0x878D30
void __cdecl CPlayerInfo::ScoreVehicleCandidate(CVehicle* veh, CPed* ped, const CVector* vehPos, float dist, float* outBestScore, CVehicle** outBestVeh, const CVector* searchPos, bool bFlag)
{
    float maxDist = Core::StaticRef<const float>(0xF1F45C);
    if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1354) == 2)
    {
        using GetExtraDistFn = float(__thiscall*)(CVehicle*);
        maxDist += reinterpret_cast<GetExtraDistFn>((*reinterpret_cast<void***>(veh))[0x58 / 4])(veh);
    }

    const float targetAngle = Core::Hook::Call<float>(0xA4E5D0, searchPos->x, searchPos->y);
    const CVector& pedPos = ped->m_pMatrix->pos;
    const float dirAngle = Core::Hook::Call<float>(0xA4E5D0, vehPos->x - pedPos.x, vehPos->y - pedPos.y);
    float angleDiff = targetAngle - dirAngle;

    const float twoPi = Core::StaticRef<const float>(0xEBDEE4);
    const float pi = Core::StaticRef<const float>(0xEBDFF0);
    const float negPi = Core::StaticRef<const float>(0xEBDFFC);

    while (angleDiff > pi)
        angleDiff -= twoPi;
    while (angleDiff < negPi)
        angleDiff += twoPi;

    if (angleDiff < 0.0f)
        angleDiff = Core::StaticRef<const float>(0xEBDED0) - angleDiff;

    if (bFlag && angleDiff > Core::StaticRef<const float>(0xD85DA8))
        return;

    const float angleScore = Core::StaticRef<const float>(0xD50398) - angleDiff * Core::StaticRef<const float>(0xD646A0);
    const float distScore = (maxDist - dist) / maxDist;
    float totalScore;
    if (bFlag)
        totalScore = angleScore * Core::StaticRef<const float>(0xD64640) + distScore;
    else
        totalScore = distScore * Core::StaticRef<const float>(0xD64640) + angleScore;

    const float doorRating = Core::Hook::CallMethod<float>(0x8AE610, reinterpret_cast<uint8*>(veh) + 0x1120);
    if (doorRating <= 0.0f || *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(veh) + 0x1128) <= 0.0f)
        totalScore *= Core::StaticRef<const float>(0xD6464C);

    if (Core::StaticRef<uint8>(0x1618055) != 0)
    {
        char buf[512];
        Core::Hook::Call<int32>(0xD0C8E4, buf, reinterpret_cast<const char*>(Core::Rebase(0xD64650)),
            static_cast<double>(distScore + angleScore),
            static_cast<double>(angleScore),
            static_cast<double>(distScore));
    }

    if (totalScore >= *outBestScore)
    {
        *outBestScore = totalScore;
        *outBestVeh = veh;
    }
}

// 0x878F70
void __thiscall CPlayerInfo::ProcessWorldBounds()
{
    if (Core::Hook::Call<bool>(0x462F90))
    {
        CVector minBound{};
        CVector maxBound{};
        Core::Hook::Call<void>(0x4961A0, &minBound, &maxBound);

        CPed* ped = m_pPed;
        float margin = Core::StaticRef<const float>(0xD8F340);
        float innerMargin = Core::StaticRef<const float>(0xD7E2BC);

        if ((ped->m_nPedFlags2 & 4u) != 0 && ped->m_pMyVehicle)
        {
            CVehicle* veh = ped->m_pMyVehicle;
            if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1354) == 4)
            {
                margin = Core::StaticRef<const float>(0xEBE5A4);
                innerMargin = Core::StaticRef<const float>(0xD8F340);
            }
            else
            {
                margin = Core::StaticRef<const float>(0xD8D70C);
                innerMargin = Core::StaticRef<const float>(0xDCEA38);
            }
        }

        const CVector& pos = ped->m_pMatrix->pos;
        const bool outsideWarn =
            (minBound.x + margin > pos.x) || (pos.x > maxBound.x - margin) ||
            (minBound.y + margin > pos.y) || (pos.y > maxBound.y - margin) ||
            (minBound.z + margin > pos.z) || (pos.z > maxBound.z - margin);

        if (outsideWarn)
        {
            uint8& shownFlag = Core::StaticRef<uint8>(0x11E6F5C);
            if (shownFlag == 0)
            {
                const char* msg = Core::Hook::CallMethod<const char*>(0x7B54C0, reinterpret_cast<void*>(Core::Rebase(0x10F4CE8)), reinterpret_cast<const char*>(Core::Rebase(0xD64674)));
                Core::Hook::Call<void>(0x85DA20, msg, Core::StaticRef<int32>(0x10F4F44), 0, -1, 4000, 10000, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0);
                shownFlag = 1;
            }

            const CVector& curPos = m_pPed->m_pMatrix->pos;
            const bool outsideKill =
                (minBound.x + innerMargin > curPos.x) || (curPos.x >= maxBound.x - innerMargin) ||
                (minBound.y + innerMargin > curPos.y) || (curPos.y >= maxBound.y - innerMargin) ||
                (minBound.z + innerMargin > curPos.z) || (curPos.z >= maxBound.z - innerMargin);

            if (outsideKill)
            {
                CPed* curPed = m_pPed;
                const uint8* portalTracker = (curPed->m_nPedFlags2 & 4u) != 0 && curPed->m_pMyVehicle
                    ? *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(curPed->m_pMyVehicle) + 0x6C)
                    : nullptr;
                if ((curPed->m_nPedFlags2 & 4u) != 0 && (!portalTracker || portalTracker[0xE] == 0))
                {
                    CVehicle* veh = curPed->m_pMyVehicle;
                    using KillVehFn = void(__thiscall*)(CVehicle*, int32, int32, int32, int32, int32);
                    reinterpret_cast<KillVehFn>((*reinterpret_cast<void***>(veh))[0x180 / 4])(veh, 0, 0, 1, 0, 0x33);
                }
                else
                {
                    using SetHealthFn = void(__thiscall*)(CPed*, float, int32);
                    reinterpret_cast<SetHealthFn>((*reinterpret_cast<void***>(curPed))[0xF4 / 4])(curPed, 0.0f, 0);
                }
            }
        }
    }
    else
    {
        Core::StaticRef<uint8>(0x11E6F5C) = 0;
    }

    const CVector& pos = m_pPed->m_pMatrix->pos;
    const float distSq = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
    if (distSq > Core::StaticRef<const float>(0xD6466C))
    {
        using GetHealthFn = float(__thiscall*)(CPed*, int32);
        using SetHealthFn = void(__thiscall*)(CPed*, float, int32);
        const float curHealth = reinterpret_cast<GetHealthFn>((*reinterpret_cast<void***>(m_pPed))[0xFC / 4])(m_pPed, 0);
        const float newHealth = curHealth - Core::StaticRef<float>(0x11DDE7C);
        reinterpret_cast<SetHealthFn>((*reinterpret_cast<void***>(m_pPed))[0xF4 / 4])(m_pPed, newHealth, 0);
    }
}

// 0x879220
void __thiscall CPlayerInfo::SetPlayerControl(bool bCutsceneOrScript, bool bDisable, float fPosZ, bool bSetPedTask, bool bClearArea)
{
    if (bCutsceneOrScript)
    {
        if (bDisable)
            m_nFlags |= 0x800u;
        else
            m_nFlags &= ~0x800u;
    }
    else
    {
        if (bDisable)
            m_nFlags |= 0x20u;
        else
            m_nFlags &= ~0x20u;
    }

    if ((m_nFlags & 0x820u) == 0)
    {
        m_Wanted.m_nFlags &= ~4u;
        *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pPed) + 0x118) &= ~0x400u;
        Core::Hook::CallMethod<void>(0x8786A0, m_pPed, 1);
        if (bClearArea)
            Core::Hook::Call<void>(0x79EBE0, 0);
    }
    else
    {
        m_Wanted.m_nFlags |= 4u;
        Core::Hook::Call<void>(0x81A260);
        Core::Hook::Call<void>(0x7C3A10);
        *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pPed) + 0x118) |= 0x400u;
        if (m_pPed->m_pPlayerInfo)
            *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(&m_pPed->m_pPlayerInfo->m_Wanted) + 0x3D0) &= ~0x10u;
        Core::Hook::CallMethod<void>(0x9BF070, m_pPed);

        float& wantedUnk3B4 = *reinterpret_cast<float*>(reinterpret_cast<uint8*>(&m_Wanted) + 0x3B4);
        if (wantedUnk3B4 < 0.0f)
            wantedUnk3B4 = 0.0f;

        const uint8* portalTracker = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(m_pPed) + 0x6C);
        if ((!portalTracker || portalTracker[0xE] == 0) && bSetPedTask)
            Core::Hook::CallMethod<void>(0x9C4400, m_pPed->m_pPedIntelligence, 1, 0);

        Core::Hook::CallMethod<void>(0xA292B0, reinterpret_cast<void*>(Core::Rebase(0x164B030)), m_pPed);

        if (bClearArea)
        {
            CVector pos{};
            GetPos(&pos);
            Core::Hook::CallMethod<void>(0xA29130, reinterpret_cast<void*>(Core::Rebase(0x164B030)), &pos, fPosZ);

            GetPos(&pos);
            Core::Hook::Call<void>(0x961520, -1, &pos, Core::StaticRef<const float>(0xD54634));
            Core::Hook::Call<void>(0x99E910);

            GetPos(&pos);
            Core::Hook::Call<void>(0x81A280, &pos, fPosZ);
        }

        Core::Hook::CallMethod<void>(0x9BEE70, m_pPed);
    }

    const uint8* portalTracker = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(m_pPed) + 0x6C);
    if (!portalTracker || portalTracker[0xE] == 0)
        Core::Hook::Call<void>(0x99C280, 4, static_cast<uint32>(bDisable));
}

// 0x8795B0
void __thiscall CPlayerInfo::Process()
{
    if (m_pPed)
    {
        const int32 localIdx = ms_nLocalPlayerIndex;
        CPlayerInfo* localPlayer = (localIdx == -1) ? nullptr : ms_apPlayers[localIdx];
        if (localPlayer == this)
        {
            const CMatrix* mat = m_pPed->m_pMatrix;
            Core::StaticRef<float>(0x11E6F60) = mat->pos.x;
            Core::StaticRef<float>(0x11E6F64) = mat->pos.y;
            Core::StaticRef<float>(0x11E6F68) = mat->pos.z;
            Core::StaticRef<float>(0x11E6F6C) = mat->pad3;
        }
    }

    if (Core::StaticRef<int32>(0x10D0170) == 1)
        return;
    if (Core::StaticRef<int32>(0x10E1984) != Core::StaticRef<int32>(0xE3447C))
        return;
    if (Core::StaticRef<int32>(0xF0E4A0) == 0x12)
        return;

    uint8* taskMgr = Core::Hook::CallMethod<uint8*>(0x9BE5D0, m_pPed);
    m_nUnk4D0 = Core::StaticRef<uint32>(0x11dde74);

    if (reinterpret_cast<uint8*>(m_pPed->m_pPedIntelligence) + 0x2E0 != nullptr &&
        Core::Hook::CallMethod<bool>(0x426910, reinterpret_cast<uint8*>(m_pPed->m_pPedIntelligence) + 0x2E0, 0x41E, 0))
    {
        const float val = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<const float>(0xDAF864);
        Core::Hook::Call<void>(0x962B90, 0x33, val);
    }
    else if ((m_pPed->m_nPedFlags2 & 4u) != 0 && m_pPed->m_pMyVehicle)
    {
        CVehicle* veh = m_pPed->m_pMyVehicle;
        const int32 vehType = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1350);
        if (vehType == 0)
        {
            if (Core::Hook::CallMethod<int32>(0x9C5570, veh) < 3)
                m_nUnk504 += Core::Hook::Call<int32>(0x448330);
            else
                m_nUnk504 = 0;

            const bool frontOff = IsVehWheelPairOffGround(veh, 9, 13);
            const bool rearOff = IsVehWheelPairOffGround(veh, 10, 14);

            if (!frontOff && rearOff && *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(veh) + 0x170) == 0.0f)
            {
                m_nUnk4FC += Core::Hook::Call<uint32>(0x448330);
                m_fUnk500 += Core::Hook::CallMethod<float>(0x803420, veh);
                const uint32 halfStep = Core::Hook::Call<uint32>(0x448330) >> 1;
                m_nUnk518 = (m_nUnk518 > halfStep) ? (m_nUnk518 - halfStep) : 0;
                m_nUnk508 = 0;
                m_nUnk510 = 0;
            }
            else if (frontOff && !rearOff && *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(veh) + 0x170) == 0.0f)
            {
                m_nUnk4FC += Core::Hook::Call<uint32>(0x448330);
                m_fUnk500 += Core::Hook::CallMethod<float>(0x803420, veh);
                const uint32 halfStep = Core::Hook::Call<uint32>(0x448330) >> 1;
                m_nUnk518 = (m_nUnk518 > halfStep) ? (m_nUnk518 - halfStep) : 0;
                m_nUnk508 = 0;
                m_nUnk510 = 0;
            }
            else
            {
                if ((frontOff || rearOff) && m_nUnk4FC > 0 && m_nUnk518 < 0x1F4u)
                {
                    m_nUnk518 += Core::Hook::Call<uint32>(0x448330);
                }
                else
                {
                    if (m_nUnk4FC >= 0x3E8u)
                    {
                        m_fUnk520 = m_fUnk500;
                        m_nUnk51C = m_nUnk4FC;
                        Core::Hook::Call<void>(0x9655C0, 0x71, static_cast<float>(m_nUnk4FC));
                        Core::Hook::Call<void>(0x9655C0, 0x52, m_fUnk500);
                    }
                    m_nUnk4FC = 0;
                    m_fUnk500 = 0.0f;
                    m_nUnk518 = 0;
                }
                m_nUnk508 = 0;
                m_nUnk510 = 0;
            }
        }
        else if (vehType == 1)
        {
            if (!IsVehWheelOnGround(veh, 9) && m_nUnk510 == 0)
            {
                if (IsVehWheelOnGround(veh, 13) &&
                    (*reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(veh) + 0x170) <= Core::StaticRef<const float>(0xD82338) ||
                     GetEntityVelSq(veh) > Core::StaticRef<const float>(0xDC9290)))
                {
                    m_nUnk508 += Core::Hook::Call<uint32>(0x448330);
                    m_fUnk50C += Core::Hook::CallMethod<float>(0x803420, veh);
                    const uint32 fifthStep = Core::Hook::Call<uint32>(0x448330) / 5u;
                    m_nUnk518 = (m_nUnk518 > fifthStep) ? (m_nUnk518 - fifthStep) : 0;
                }
                else if (m_nUnk508 > 0 && m_nUnk518 < 0x1F4u)
                {
                    m_nUnk518 += Core::Hook::Call<uint32>(0x448330);
                }
                else
                {
                    if (m_nUnk508 >= 0xBB8u)
                    {
                        m_fUnk528 = m_fUnk50C;
                        m_nUnk524 = m_nUnk508;
                        Core::Hook::Call<void>(0x9655C0, 0x70, static_cast<float>(m_nUnk508));
                        Core::Hook::Call<void>(0x9655C0, 0x54, m_fUnk50C);
                    }
                    m_nUnk508 = 0;
                    m_fUnk50C = 0.0f;
                    m_nUnk518 = 0;
                }
                m_nUnk4FC = 0;
                m_nUnk504 = 0;
            }
            else if (m_nUnk508 > 0)
            {
                if (m_nUnk508 >= 0xBB8u)
                {
                    m_fUnk528 = m_fUnk50C;
                    m_nUnk524 = m_nUnk508;
                    Core::Hook::Call<void>(0x9655C0, 0x70, static_cast<float>(m_nUnk508));
                    Core::Hook::Call<void>(0x9655C0, 0x54, m_fUnk50C);
                }
                m_nUnk508 = 0;
                m_fUnk50C = 0.0f;
                m_nUnk518 = 0;
                m_nUnk4FC = 0;
                m_nUnk504 = 0;
            }
            else if (!IsVehWheelOnGround(veh, 13) && IsVehWheelOnGround(veh, 9) &&
                     (*reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(veh) + 0x170) <= Core::StaticRef<const float>(0xD82338) ||
                      GetEntityVelSq(veh) > Core::StaticRef<const float>(0xDC9290)))
            {
                m_nUnk510 += Core::Hook::Call<uint32>(0x448330);
                m_fUnk514 += Core::Hook::CallMethod<float>(0x803420, veh);
                const uint32 fifthStep = Core::Hook::Call<uint32>(0x448330) / 5u;
                m_nUnk518 = (m_nUnk518 > fifthStep) ? (m_nUnk518 - fifthStep) : 0;
                m_nUnk4FC = 0;
                m_nUnk504 = 0;
            }
            else
            {
                if (!IsVehWheelOnGround(veh, 13) && m_nUnk510 > 0 && m_nUnk518 < 0x1F4u)
                {
                    m_nUnk518 += Core::Hook::Call<uint32>(0x448330);
                }
                else
                {
                    if (m_nUnk510 >= 0x3E8u)
                    {
                        m_fUnk530 = m_fUnk514;
                        m_nUnk52C = m_nUnk510;
                        Core::Hook::Call<void>(0x9655C0, 0x6F, static_cast<float>(m_nUnk510));
                        Core::Hook::Call<void>(0x9655C0, 0x53, m_fUnk514);
                    }
                    m_fUnk514 = 0.0f;
                    m_nUnk510 = 0;
                    m_nUnk518 = 0;
                }
                m_nUnk4FC = 0;
                m_nUnk504 = 0;
            }
        }
        else
        {
            m_nUnk508 = 0;
            m_nUnk510 = 0;
            m_nUnk518 = 0;
            m_nUnk4FC = 0;
            m_nUnk504 = 0;
        }
    }
    else
    {
        m_nUnk508 = 0;
        m_nUnk510 = 0;
        m_nUnk518 = 0;
        m_nUnk4FC = 0;
        m_nUnk504 = 0;
    }

    ProcessWorldBounds();

    const int32 money = GetMoney();
    int32 displayMoney = GetDisplayMoney();
    if (displayMoney != money)
    {
        const int32 diff = money - displayMoney;
        const int32 absDiff = (diff < 0) ? -diff : diff;
        int32 step;
        if (absDiff > 100000)
            step = 12345;
        else if (absDiff > 10000)
            step = 1234;
        else if (absDiff > 1000)
            step = 123;
        else
            step = (absDiff > 50) ? 42 : 1;

        if (diff < 0)
        {
            Core::Hook::Call<void>(0x965510, 0x6C, static_cast<float>(step));
            displayMoney -= step;
        }
        else
        {
            Core::Hook::Call<void>(0x962B90, 0x6C, static_cast<float>(step));
            displayMoney += step;
            if ((Core::StaticRef<uint32>(0x11DDF38) % 3u) == 0)
                Core::Hook::CallMethod<void>(0x7B9130, reinterpret_cast<void*>(Core::Rebase(0x10F85B0)), reinterpret_cast<const char*>(Core::Rebase(0xD64620)));
        }

        SetDisplayMoney(displayMoney);
        if (displayMoney - Core::StaticRef<int32>(0x15E3604) >= 0x7A120 &&
            !Core::Hook::Call<bool>(0x467E30, 0x19) &&
            !Core::Hook::Call<bool>(0x462F90))
        {
            Core::Hook::Call<void>(0x467D30, 0x19);
        }
    }

    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(m_pPed) + 0xA94) = Core::StaticRef<const float>(0xD8D70C);
    m_nMaxHealth = static_cast<uint16>(static_cast<int32>(*reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(m_pPed) + 0xA94)));

    if ((m_pPed->m_nPedFlags2 & 4u) != 0)
        Core::Hook::Call<double>(0x9633E0, 0x1A6);

    CPed* localPed = FindPlayerPedFast();
    CVehicle* localVeh = Core::Hook::Call<CVehicle*>(0x817F80);

    if (m_nUnk4E0 != 0)
    {
        if (Core::StaticRef<uint32>(0x11DDE74) - m_nUnk4E4 > 0x7D0u)
        {
            m_nUnk4E0 = 0;
            if (localVeh && (localVeh->m_nFlags2 & 0x7C00u) != 0xC00u)
                localVeh->m_nFlags2 &= ~0x7C00u;
        }
    }

    if ((Core::StaticRef<uint32>(0x11DDF38) & 0x1Fu) == 0)
    {
        if (localVeh && (m_pPed->m_nPedFlags2 & 4u) != 0 &&
            localVeh->m_pMatrix->at.z < 0.0f &&
            std::sqrt(GetEntityVelSq(localVeh)) < Core::StaticRef<const float>(0xDCE034) &&
            (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(localVeh) + 0x1350) == 0 ||
             *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(FindPlayerVehicle(nullptr)) + 0x1350) == 2) &&
            (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(localVeh) + 0x118) & 1u) == 0)
        {
            if (localVeh->m_pMatrix->at.z < Core::StaticRef<const float>(0xDCC5DC))
                m_nUnk4F4 += 2;
            else
                m_nUnk4F4 += 1;
        }
        else
        {
            m_nUnk4F4 = 0;
        }
    }

    if (localVeh)
    {
        Core::Hook::Call<void>(0x966060, localVeh);
    }
    else if (GetEntityVelSq(localPed) > Core::StaticRef<const float>(0xDECD60))
    {
        if (Core::Hook::CallMethod<bool>(0x9C3090, m_pPed->m_pPedIntelligence))
        {
            const uint32 id0 = Core::Hook::CallMethod<uint32>(0x839030, taskMgr);
            const uint32 id1 = Core::Hook::CallMethod<uint32>(0x839060, taskMgr);
            const uint8* unk = Core::Hook::CallMethod<const uint8*>(0x8393E0, taskMgr);
            if (Core::Hook::Call<int32>(0x834730, id0) != 0 ||
                Core::Hook::Call<int32>(0x834730, id1) != 0 ||
                static_cast<uint8>(unk[6] ^ unk[4]) > 0x7Fu)
            {
                const float dist = Core::Hook::CallMethod<float>(0x803420, localPed);
                Core::Hook::Call<void>(0x962B90, 0x51, dist);
                const float t = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<const float>(0xDAF864);
                Core::Hook::Call<void>(0x962B90, 0x4B, t);
            }
        }
        else if (Core::Hook::CallMethod<bool>(0x9C2680, m_pPed->m_pPedIntelligence))
        {
            const CVector& pos = localPed->m_pMatrix->pos;
            const float waterZ = Core::Hook::Call<float>(0x994490, pos.x, pos.y, pos.z, 0, 0, 4);
            const float submerge = pos.z - waterZ;
            if (submerge > Core::StaticRef<const float>(0xD64640))
                Core::Hook::Call<void>(0x9655C0, 0x59, submerge);
        }
        else if (Core::Hook::CallMethod<float>(0x803420, localPed) > Core::StaticRef<const float>(0xEBE284))
        {
            const float dist = Core::Hook::CallMethod<float>(0x803420, localPed);
            Core::Hook::Call<void>(0x962B90, 0x50, dist);
            const float t = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<const float>(0xDAF864);
            Core::Hook::Call<void>(0x962B90, 0x46, t);
        }
    }

    Core::Hook::CallMethod<void>(0xC40580, this);
    UpdateAsyncProbe();
}

// 0x87A090
bool __cdecl CPlayerInfo::VehicleSearchCallback(CVehicle* veh, void* rawCtx)
{
    auto* ctx = static_cast<PlayerVehSearchCtx*>(rawCtx);
    if ((veh->m_nFlags & 1u) == 0 || (veh->m_nFlags2 & 0x3C0u) != 0x80u)
        return true;

    const int32 vehType = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1350);
    const bool isBike = (vehType == 2);

    bool skip = false;
    if ((veh->m_nVehFlagsF67 & 8u) != 0)
        skip = true;
    else if (vehType == 0 && Core::Hook::CallMethod<int32>(0x9C5570, veh) == 0 &&
             (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(veh) + 0x118) & 1u) != 0)
        skip = true;
    else if (vehType == 1 && veh->m_pMatrix->at.z > Core::StaticRef<const float>(0xEBE228) &&
             Core::Hook::CallMethod<int32>(0x9C5570, veh) == 0 &&
             (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(veh) + 0x118) & 1u) != 0)
        skip = true;
    else if (!Core::Hook::CallMethod<bool>(0x8AEFE0, reinterpret_cast<uint8*>(veh) + 0x1120, 1))
        skip = true;
    else if ((veh->m_nFlags2 & 0x7C00u) == 0xC00u)
        skip = true;
    else if (isBike && !Core::Hook::CallMethod<bool>(0x9CA650, veh, ctx->ped))
        skip = true;

    if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1354) == 3)
    {
        if (veh->m_nModelIndex == Core::StaticRef<int32>(0x123097C))
            return true;

        const CVector& vehPos = veh->m_pMatrix->pos;
        const CVector& pedPos = ctx->ped->m_pMatrix->pos;
        const CVector& right = veh->m_pMatrix->right;
        const float dot = right.x * (pedPos.x - vehPos.x) + right.y * (pedPos.y - vehPos.y) + right.z * (pedPos.z - vehPos.z);
        const bool flag1535 = (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(veh) + 0x1535) & 0x10u) != 0;
        if (flag1535 ? (0.0f > dot) : (dot > 0.0f))
            return true;
    }

    if (skip)
        return true;

    if (!veh->m_pMatrix)
    {
        Core::Hook::CallMethod<void>(0x9EA460, veh);
        Core::Hook::CallMethod<void>(0x9E9440, &veh->m_vecPos, veh->m_pMatrix);
    }

    if (veh->m_pMatrix->at.z <= Core::StaticRef<const float>(0xD796E4))
    {
        if (vehType != 1)
            return true;
        const CMatrix* mat = Core::Hook::CallMethod<const CMatrix*>(0x447980, veh);
        if (mat->at.z <= Core::StaticRef<const float>(0xDCC5DC))
            return true;
    }

    CVector boundCentre{};
    using GetBoundCentreFn = CVector*(__thiscall*)(CVehicle*, CVector*);
    reinterpret_cast<GetBoundCentreFn>((*reinterpret_cast<void***>(veh))[0x54 / 4])(veh, &boundCentre);

    const CVector& basePos = veh->GetPosition();
    using GetUnk1A4Fn = float(__thiscall*)(CVehicle*);
    const float zOff = reinterpret_cast<GetUnk1A4Fn>((*reinterpret_cast<void***>(veh))[0x1A4 / 4])(veh);

    const CVector& pedPos = ctx->ped->m_pMatrix->pos;
    const float dz = std::fabs(pedPos.z - (basePos.z - zOff + Core::StaticRef<const float>(0xD50398)));
    const bool standingOnVeh = (ctx->ped->m_pGroundEntity == veh);

    if (dz < Core::StaticRef<const float>(0xD64640))
    {
        if (isBike && !standingOnVeh && (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(ctx->ped) + 0x118) & 1u) == 0)
            return true;
    }
    else
    {
        if (!isBike)
            return true;
        if (!standingOnVeh && (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(ctx->ped) + 0x118) & 1u) == 0)
            return true;
    }

    if ((veh->m_nVehFlagsF6A & 0x80u) == 0)
        return true;

    CVector entryPos{};
    int32 unkDoor = 0;
    if (!Core::Hook::Call<bool>(0xAA7480, ctx->ped, veh, &entryPos, &unkDoor))
        return true;

    const float dx = pedPos.x - entryPos.x;
    const float dy = pedPos.y - entryPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    float maxDist = Core::StaticRef<const float>(0xF1F45C);
    if (isBike)
    {
        using GetExtraDistFn = float(__thiscall*)(CVehicle*);
        maxDist += reinterpret_cast<GetExtraDistFn>((*reinterpret_cast<void***>(veh))[0x58 / 4])(veh);
    }

    if (dist > maxDist)
    {
        const float z = pedPos.z;
        const float r = Core::StaticRef<const float>(0xD6D870);
        uint8 sphere[0x74]{};
        Core::Hook::CallMethod<void>(0x8ECD30, sphere, veh, z, r, 0, 0, 0, 0, 0, 0);
        if (Core::Hook::CallMethod<bool>(0x8E7F80, sphere, &pedPos))
            dist = maxDist;
    }

    if (maxDist >= dist)
    {
        ScoreVehicleCandidate(veh, ctx->ped, &entryPos, dist, &ctx->bestScore, &ctx->bestVeh, &ctx->pos, ctx->bFlag);
    }
    return true;
}

// 0x87A450
bool __cdecl CPlayerInfo::HeliSearchCallback(CVehicle* veh, void* rawCtx)
{
    auto* ctx = static_cast<PlayerVehSearchCtx*>(rawCtx);
    if ((veh->m_nFlags & 1u) == 0 || (veh->m_nFlags2 & 0x3C0u) != 0x80u)
        return true;

    if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(veh) + 0x1354) == 3)
    {
        const CVector& vehPos = veh->m_pMatrix->pos;
        const CVector& pedPos = ctx->ped->m_pMatrix->pos;
        const CVector& right = veh->m_pMatrix->right;
        const float dot = right.x * (pedPos.x - vehPos.x) + right.y * (pedPos.y - vehPos.y) + right.z * (pedPos.z - vehPos.z);
        const bool flag1535 = (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(veh) + 0x1535) & 0x10u) != 0;
        if (flag1535)
        {
            if (0.0f > dot)
                return true;
        }
        else if (dot > 0.0f)
        {
            return true;
        }
    }

    CVector boundCentre{};
    using GetBoundCentreFn = CVector*(__thiscall*)(CVehicle*, CVector*);
    reinterpret_cast<GetBoundCentreFn>((*reinterpret_cast<void***>(veh))[0x54 / 4])(veh, &boundCentre);

    const CVector& basePos = veh->GetPosition();
    using GetUnk1A4Fn = float(__thiscall*)(CVehicle*);
    const float zOff = reinterpret_cast<GetUnk1A4Fn>((*reinterpret_cast<void***>(veh))[0x1A4 / 4])(veh);

    const CVector& pedPos = ctx->ped->m_pMatrix->pos;
    const float dz = std::fabs(pedPos.z - (basePos.z - zOff + Core::StaticRef<const float>(0xD50398)));
    if (dz >= Core::StaticRef<const float>(0xD64640))
        return true;

    if ((veh->m_nVehFlagsF6A & 0x80u) == 0)
        return true;

    const float* modelBounds = Core::StaticRef<const float*>(0x15F73B0 + veh->m_nModelIndex * 4);
    const CMatrix* mat = Core::Hook::CallMethod<const CMatrix*>(0x447980, veh);
    const CVector delta{
        pedPos.x - basePos.x,
        pedPos.y - basePos.y,
        pedPos.z - basePos.z
    };

    float dist = 0.0f;
    const float d0 = std::fabs(mat->right.x * delta.x + mat->right.y * delta.y + mat->right.z * delta.z) - modelBounds[0x30 / 4];
    if (d0 > 0.0f)
        dist += d0;
    const float d1 = std::fabs(mat->up.x * delta.x + mat->up.y * delta.y + mat->up.z * delta.z) - modelBounds[0x34 / 4];
    if (d1 > 0.0f)
        dist += d1;
    const float d2 = std::fabs(mat->at.x * delta.x + mat->at.y * delta.y + mat->at.z * delta.z) - modelBounds[0x38 / 4];
    if (d2 > 0.0f)
        dist += d2;

    if (dist <= Core::StaticRef<const float>(0xF1F45C))
    {
        ScoreVehicleCandidate(veh, ctx->ped, &basePos, dist, &ctx->bestScore, &ctx->bestVeh, &ctx->pos, ctx->bFlag);
    }
    return true;
}

// 0x87A840
CVehicle* __cdecl CPlayerInfo::FindVehicleToEnter(CPed* ped, const CVector* pos, bool bFlag)
{
    if ((ped->m_nPedUnk268 & 0x2000u) != 0)
        return nullptr;

    CPlayerInfo* info = ped->m_pPlayerInfo;
    if (info->m_pRemoteVehicle)
    {
        auto* remoteVeh = reinterpret_cast<CVehicle*>(info->m_pRemoteVehicle);
        if (!Core::Hook::CallMethod<bool>(0x9C6380, remoteVeh))
            return nullptr;
        return remoteVeh;
    }

    if (ped->m_pGroundEntity &&
        (ped->m_pGroundEntity->m_nFlags2 & 0x3C0u) == 0x80u &&
        *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(ped->m_pGroundEntity) + 0x1354) == 2 &&
        Core::Hook::CallMethod<bool>(0x9CA650, ped->m_pGroundEntity, ped))
    {
        return reinterpret_cast<CVehicle*>(ped->m_pGroundEntity);
    }

    const CVector& pedPos = ped->m_pMatrix->pos;
    const float r = Core::StaticRef<const float>(0xF1F45C);
    const float minLimit = Core::StaticRef<const float>(0xEBDED8);
    const float maxLimit = Core::StaticRef<const float>(0xEBDECC);

    float minX = std::max(minLimit, pedPos.x - r);
    float minY = std::max(minLimit, pedPos.y - r);
    float minZ = std::max(minLimit, pedPos.z - r);
    float maxX = std::min(maxLimit, pedPos.x + r);
    float maxY = std::min(maxLimit, pedPos.y + r);
    float maxZ = std::min(maxLimit, pedPos.z + r);

    float aabb[8] = {
        minX, minY, minZ, 0.0f,
        maxX, maxY, maxZ, 0.0f
    };

    PlayerVehSearchCtx ctx{};
    ctx.ped = ped;
    ctx.bestVeh = nullptr;
    ctx.bestScore = 0.0f;
    ctx.pos = *pos;
    ctx.bFlag = bFlag;

    Core::Hook::Call<void>(0x818C40, aabb, reinterpret_cast<void*>(Core::Rebase(0x87A090)), &ctx, 4, 0xD);

    if (ctx.bestVeh && Core::Hook::CallMethod<bool>(0x9C6380, ctx.bestVeh))
        return ctx.bestVeh;
    return nullptr;
}

// 0x87AA70
CVehicle* __cdecl CPlayerInfo::FindHeliToEnter(CPed* ped, const CVector* pos, bool bFlag)
{
    PlayerVehSearchCtx ctx{};
    ctx.ped = ped;
    ctx.bestVeh = nullptr;
    ctx.bestScore = 0.0f;
    ctx.pos = *pos;
    ctx.bFlag = bFlag;

    struct PoolHeader
    {
        uint8* objects;
        uint8* flags;
        uint32 size;
        uint32 elemSize;
    };
    const PoolHeader* pool = Core::StaticRef<PoolHeader*>(0x1619240);
    const int32 targetModel = Core::StaticRef<int32>(0x1230790);

    for (uint32 i = pool->size; i > 0;)
    {
        --i;
        if ((pool->flags[i] & 0x80u) == 0)
        {
            auto* veh = reinterpret_cast<CVehicle*>(pool->objects + i * pool->elemSize);
            if (veh && veh->m_nModelIndex == targetModel)
                HeliSearchCallback(veh, &ctx);
        }
    }

    if (ctx.bestVeh && Core::Hook::CallMethod<bool>(0x9C6380, ctx.bestVeh))
        return ctx.bestVeh;
    return nullptr;
}
