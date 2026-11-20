#include "StdInc.h"
#include "Game/Vehicle.h"
#include "Game/ModelInfo.h"
#include "Game/Ped.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cassert>
#include <cmath>
#include <cstring>

struct CVehicleSubObjE24
{
    CVehicleSubObjE24* __thiscall Ctor();
};

struct CVehicleStateSnapshot
{
    void __thiscall Save(CVehicle* pVeh);
    void __thiscall Restore(CVehicle* pVeh);
};

struct CVehicleLink
{
    void*     m_pAttached; // +0x00
    void*     m_pPending;  // +0x04
    CVehicle* m_pOwner;    // +0x08
    int32     m_nBoneId;   // +0x0C
    uint8     m_bFlag;     // +0x10
    uint8     pad11;       // +0x11
    int16     m_nTimer;    // +0x12

    int32 __thiscall GetBoneIndex();
    void  __thiscall UpdateAudio(bool bUsePending);
    void  __thiscall Set(void* pEntity, bool bSet, bool bDoUpdate);
    bool  __thiscall Attach(void* pEntity);
    bool  __thiscall Detach(void* pEntity);
    void  __thiscall Process();
};

void CVehicle::InjectHooks()
{
    Core::Hook::Install(0x9C7C40, &CVehicle::Ctor);
    Core::Hook::Install(0x9C6D20, &CVehicle::GetPassenger);
    Core::Hook::Install(0x9C6D70, &CVehicle::SeatToPassengerIndex);
    Core::Hook::Install(0x9C6DB0, &CVehicle::PassengerIndexToSeat);
    Core::Hook::Install(0x9C52C0, &CVehicle::Unk_9C52C0);
    Core::Hook::Install(0x9C5360, &CVehicle::Unk_9C5360);
    Core::Hook::Install(0x9C5370, &CVehicle::Unk_9C5370);
    Core::Hook::Install(0x9C53C0, &CVehicle::Unk_9C53C0);
    Core::Hook::Install(0x9C5430, &CVehicle::Unk_9C5430);
    Core::Hook::Install(0x9C6DF0, &CVehicle::Unk_9C6DF0);
    Core::Hook::Install(0x9C6E40, &CVehicle::GetPedSeat);
    Core::Hook::Install(0x9C6E90, &CVehicle::Unk_9C6E90);
    Core::Hook::Install(0x9C6EF0, &CVehicle::Unk_9C6EF0);
    Core::Hook::Install(0x9C6F20, &CVehicle::Unk_9C6F20);
    Core::Hook::Install(0x9C6F90, &CVehicle::Unk_9C6F90);
    Core::Hook::Install(0x9C6FD0, &CVehicle::IsPlayerInVehicle);
    Core::Hook::Install(0x9C5440, &CVehicle::Unk_9C5440);
    Core::Hook::Install(0x9C5480, &CVehicle::Unk_9C5480);
    Core::Hook::Install(0x9C54C0, &CVehicle::Unk_9C54C0);
    Core::Hook::Install(0x9C5510, &CVehicle::Unk_9C5510);
    Core::Hook::Install(0x9C5570, &CVehicle::Unk_9C5570);
    Core::Hook::Install(0x9C55E0, &CVehicle::Unk_9C55E0);
    Core::Hook::Install(0x9C5630, &CVehicle::Unk_9C5630);
    Core::Hook::Install(0x9C5650, &CVehicle::Unk_9C5650);
    Core::Hook::Install(0x9C56E0, &CVehicle::Unk_9C56E0);
    Core::Hook::Install(0x9C5760, &CVehicle::Unk_9C5760);
    Core::Hook::Install(0x9C6AF0, &CVehicle::Unk_9C6AF0);
    Core::Hook::Install(0x9C6B20, &CVehicle::Unk_9C6B20);
    Core::Hook::Install(0x9C6B40, &CVehicle::Unk_9C6B40);
    Core::Hook::Install(0x9C6B70, &CVehicle::Unk_9C6B70);
    Core::Hook::Install(0x9C6BA0, &CVehicle::Unk_9C6BA0);
    Core::Hook::Install(0x9C6BC0, &CVehicle::Unk_9C6BC0);
    Core::Hook::Install(0x9C6C00, &CVehicle::Unk_9C6C00);
    Core::Hook::Install(0x9C6CF0, &CVehicle::GetPassengerCount);
    Core::Hook::Install(0x9C7010, &CVehicle::Unk_9C7010);
    Core::Hook::Install(0x9C7170, &CVehicle::Unk_9C7170);
    Core::Hook::Install(0x9C71C0, &CVehicle::Unk_9C71C0);
    Core::Hook::Install(0x9C7210, &CVehicle::Unk_9C7210);
    Core::Hook::Install(0x9C72B0, &CVehicle::Unk_9C72B0);
    Core::Hook::Install(0x9C72D0, &CVehicle::Unk_9C72D0);
    Core::Hook::Install(0x9C7380, &CVehicle::Unk_9C7380);
    Core::Hook::Install(0x9C7560, &CVehicle::Unk_9C7560);
    Core::Hook::Install(0x9C7580, &CVehicle::Unk_9C7580);
    Core::Hook::Install(0x9C75A0, &CVehicleStateSnapshot::Save);
    Core::Hook::Install(0x9C7670, &CVehicleStateSnapshot::Restore);
    Core::Hook::Install(0x9C7710, &CVehicle::Unk_9C7710);
    Core::Hook::Install(0x9C7960, &CVehicleLink::GetBoneIndex);
    Core::Hook::Install(0x9C7970, &CVehicleLink::UpdateAudio);
    Core::Hook::Install(0x9C79F0, &CVehicleLink::Set);
    Core::Hook::Install(0x9C7A40, &CVehicleLink::Attach);
    Core::Hook::Install(0x9C7AA0, &CVehicleLink::Detach);
    Core::Hook::Install(0x9C7B00, &CVehicleLink::Process);
    Core::Hook::Install(0x9C77F0, &CVehicleSubObjE24::Ctor);
    Core::Hook::Install(0x9C7BF0, &CVehicle::InitSystem);
    Core::Hook::Install(0x9C80B0, &CVehicle::Unk_9C80B0);
    Core::Hook::Install(0x9C80C0, &CVehicle::Unk_9C80C0);
    Core::Hook::Install(0x9C80D0, &CVehicle::Unk_9C80D0);
    Core::Hook::Install(0x9C83A0, &CVehicle::Unk_9C83A0);
    Core::Hook::Install(0x9C8700, &CVehicle::Unk_9C8700);
    Core::Hook::Install(0x9C8900, &CVehicle::Unk_9C8900);
    Core::Hook::Install(0x9C8A70, &CVehicle::Unk_9C8A70);
    Core::Hook::Install(0x9C8C70, &CVehicle::Unk_9C8C70);
    Core::Hook::Install(0x9C8D80, &CVehicle::Unk_9C8D80);
    Core::Hook::Install(0x9C8DC0, &CVehicle::Unk_9C8DC0);
    Core::Hook::Install(0x9C8F30, &CVehicle::Unk_9C8F30);
    Core::Hook::Install(0x9C9020, &CVehicle::Unk_9C9020);
    Core::Hook::Install(0x9C91A0, &CVehicle::Unk_9C91A0);
    Core::Hook::Install(0x9C9210, &CVehicle::Unk_9C9210);
    Core::Hook::Install(0x9C93B0, &CVehicle::Unk_9C93B0);
    Core::Hook::Install(0x9C9BA0, &CVehicle::Unk_9C9BA0);
    Core::Hook::Install(0x9C9E60, &CVehicle::Unk_9C9E60);
    Core::Hook::Install(0x9CA080, &CVehicle::Unk_9CA080);
    Core::Hook::Install(0x9CA250, &CVehicle::Unk_9CA250);
    Core::Hook::Install(0x9CA440, &CVehicle::Unk_9CA440);
    Core::Hook::Install(0x9CA600, &CVehicle::Unk_9CA600);
    Core::Hook::Install(0x9CA620, &CVehicle::Unk_9CA620);
    Core::Hook::Install(0x9CA650, &CVehicle::Unk_9CA650);
    Core::Hook::Install(0x9CA710, &CVehicle::Unk_9CA710);
    Core::Hook::Install(0x9CA8E0, &CVehicle::Unk_9CA8E0);
    Core::Hook::Install(0x9CA9B0, &CVehicle::Unk_9CA9B0);
    Core::Hook::Install(0x9CA9E0, &CVehicle::Unk_9CA9E0);
    Core::Hook::Install(0x9CAA30, &CVehicle::Unk_9CAA30);
    Core::Hook::Install(0x9CC990, &CVehicle::Unk_9CC990);
    Core::Hook::Install(0x9D4A50, &CVehicle::Unk_9D4A50);
    Core::Hook::Install(0x9D5ED0, &CVehicle::ScalarDeletingDtor);
    Core::Hook::Install(0x9D5F00, &CVehicle::Unk_9D5F00);
    Core::Hook::Install(0x9D5FC0, &CVehicle::Unk_9D5FC0);
    Core::Hook::Install(0x9D6170, &CVehicle::Unk_9D6170);

    Core::Log::Info("CVehicle hooks installed (88 functions)");
}

CVehicle* __thiscall CVehicle::Ctor(uint8 createdBy)
{
    CPhysical::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD82164));

    Core::Hook::CallMethod<void>(0xAEBB10, m_vehSubObj260);
    Core::Hook::CallMethod<void>(0xCBB490, m_vehAudioEntity);

    Core::Hook::CallMethod<void>(0xCF0E80, m_audPlaceableTracker);
    *reinterpret_cast<void**>(m_audPlaceableTracker) = reinterpret_cast<void*>(Core::Rebase(0xD69690));
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 8) = 0;

    Core::Hook::CallMethod<void>(0x9C77F0, m_vehSubObjE24);

    m_nVehUnkFF4 = -1;
    for (int32 i = 0; i < 8; ++i)
        Core::Hook::CallMethod<void>(0x9C51B0, m_vehSubObjFFC + i * 0x14);

    Core::Hook::CallMethod<void>(0xC7E510, m_vehSubObj10E0);
    Core::Hook::CallMethod<void>(0x8B0E30, m_vehSubObj1120);

    m_nVehUnk12E0 = 0;

    uint32 flags2 = (m_nFlags2 & ~0x340u) | 0x80u;
    if ((flags2 & 0x7C00u) != 0xC00u)
        flags2 = (flags2 & ~0x7800u) | 0x400u;
    m_nFlags2 = flags2;

    m_nFlags2 |= 0x10u;
    m_nCreatedBy = createdBy;
    m_nVehUnkF9A = 0;
    m_fVehUnk10D0 = 0.0f;
    m_fVehUnk10D4 = 0.0f;
    m_fVehUnk10D8 = 0.0f;
    m_fVehUnk10DC = 0.0f;
    m_fVehUnk10C8 = 0.0f;
    m_fVehUnk10CC = 0.0f;

    m_nVehFlagsF64 = (m_nVehFlagsF64 & 0xC0u) | 0x80u;
    m_nVehFlagsF65 = 2;
    m_nVehFlagsF66 = 4;
    m_nVehFlagsF67 = 0;

    const float randNorm = static_cast<float>(Core::Hook::Call<int32>(0xD0D21A)) * Core::StaticRef<float>(0xEBE17C);
    const uint8 flagBit = (0.0f > randNorm) ? 1u : 0u;
    m_nVehFlagsF69 |= 1u;
    m_nVehFlagsF68 = (flagBit & 1u) | 0x80u;
    m_nVehFlagsF69 &= 0x8Fu;
    m_nVehFlagsF69 &= 0xBFu;
    m_nVehFlagsF6E &= 0x32u;
    m_nVehFlagsF69 &= 0x71u;
    m_nVehFlagsF64 &= 0xBFu;
    m_nFlags2 |= 0x4000000u;

    m_nVehUnk250 = 0;
    m_nVehFlagsF6A = 0x90u;
    m_nVehFlagsF6B = (m_nVehFlagsF6B & 0x20u) | 0x11u;
    m_nVehFlagsF6C = 0;
    m_nVehFlagsF6D = 0x20u;
    m_nVehFlagsF6F = 8u;
    m_nVehFlagsF70 = 4u;
    m_nVehFlagsF71 = (m_nVehFlagsF71 & 0xD2u) | 0x12u;

    Core::Hook::Call<void>(0x83B3E0, this, 0);

    m_nVehUnk1110 = 0;
    m_nVehUnk1114 = 0;
    m_nVehUnk12F0 = 0;
    m_nVehUnk12F4 = 0;
    m_nVehUnk12F8 = 0;
    m_nVehUnk12FC = 0;

    Core::Hook::CallMethod<void>(0xA47930, this, Core::StaticRef<float>(0xDAF864));

    m_nMaxPassengers = 8;
    m_pDriver = nullptr;
    for (uint8 i = 0; i < m_nMaxPassengers; ++i)
        m_pPassengers[i] = nullptr;

    m_nVehUnk1112 &= 0x0Cu;
    m_nVehUnk10C4 = -1;
    m_nVehUnk130E = 0;
    m_nVehUnk1310 = 0;
    m_nVehUnk1314 = 0;
    m_nVehUnk1308 = 0;
    m_nVehUnk130C = 0;
    m_nVehUnk133C = 0;
    m_nVehUnk1340 = 0;
    m_nVehUnk1341 = 0;

    m_nPhysFlags118 |= 0x10u;
    m_nVehFlagsF68 &= 0xF7u;

    const float defaultF = Core::StaticRef<float>(0xDCAFF0);
    const float oneF = Core::StaticRef<float>(0xD50398);
    m_fVehUnk1304 = defaultF;
    m_fVehUnk131C = defaultF;
    m_fVehUnk1324 = defaultF;
    m_fVehUnk1328 = defaultF;
    m_fVehUnk132C = oneF;

    m_nVehUnk12E4 = Core::StaticRef<uint32>(0x11DDE74);
    m_nVehUnk12E8 = 0;
    m_nVehUnk12EC = 0;
    m_nVehUnk1300 = 0;
    m_nVehUnkF98 = 0;
    m_nVehUnk1320 = 1;
    m_fVehUnk1318 = 0.0f;
    m_nVehUnk1335 = 0;
    m_nVehUnk1336 = 0;
    m_nVehUnk1342 = 0;

    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 0x0C) = 0;
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 0x10) = 0;
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 0x14) = 0;
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 0x18) = 0;

    m_nVehUnkFD0 = 0;
    m_nVehUnkFD4 = 0;
    m_nVehUnkFD8 = 0;
    m_nVehUnkFDC = 0;

    const int32 idx = Core::StaticRef<int32>(0x10F9AC0);
    m_fVehUnkF84 = *reinterpret_cast<float*>(Core::Rebase(0x11F5D34) + idx * 0x210);
    m_nVehUnkF88 = 0;

    m_nVehUnk1344 = 0;
    m_nVehUnkF9C = -1;
    m_nVehUnk10C1 = 0;
    m_nVehUnk10C2 = 0;
    m_nVehUnk1343 = 0;
    m_nVehUnk1337 = 0;
    m_fVehUnk1348 = 0.0f;
    m_nVehUnk134C = 0;
    m_nVehUnk1112 &= 0xF3u;

    m_fVehUnk1118 = static_cast<float>(Core::Hook::Call<int32>(0xD0D21A) % 15);
    m_nVehUnkF74 = Core::StaticRef<uint32>(0x11DDE74);
    m_nVehUnkFCC = 0;
    m_nVehUnkFE0 = 0;
    m_nVehUnk1338 = 0;
    m_nVehUnkF78 = -1;
    m_nVehUnkFF8 = 0;
    m_fVehUnkFC4 = 0.0f;
    m_fVehUnk1330 = oneF;
    m_nVehUnkF7C = 0;
    m_nVehUnkF80 = 0;
    m_nVehUnk110C = 0;

    for (int32 i = 0; i < 8; ++i)
        Core::Hook::CallMethod<void>(0x89BBB0, m_vehSubObjFFC + i * 0x14, this);

    m_nVehUnk109C = 0;
    m_fVehUnk10A8 = 0.0f;
    m_fVehUnk10A4 = 0.0f;
    m_fVehUnk10A0 = 0.0f;

    m_fVehUnk10B8 = 0.0f;
    m_fVehUnk10B4 = 0.0f;
    m_fVehUnk10B0 = 0.0f;
    *reinterpret_cast<CVehicle**>(m_vehSubObj1120) = this;
    Core::Hook::CallMethod<void>(0x8B3580, m_vehSubObj1120);

    m_nVehUnkFEC = 0;
    return this;
}

CPed* __thiscall CVehicle::GetPassenger(int32 seat)
{
    switch (seat)
    {
    case 0: return m_pDriver;
    case 1: return m_pPassengers[1];
    case 2: return m_pPassengers[0];
    case 3: return m_pPassengers[2];
    default: return nullptr;
    }
}

int32 __stdcall CVehicle::SeatToPassengerIndex(int32 seat)
{
    switch (seat)
    {
    case 1: return 1;
    case 2: return 0;
    case 3: return 2;
    default: return -1;
    }
}

int32 __stdcall CVehicle::PassengerIndexToSeat(int32 index)
{
    switch (index)
    {
    case 0: return 2;
    case 1: return 1;
    case 2: return 3;
    default: return 0;
    }
}

void __thiscall CVehicle::Unk_9C52C0()
{
    Core::Hook::CallMethod<void>(0x8B2850, m_vehSubObj1120 + 0x10);
    Core::Hook::CallMethod<void>(0x9E22A0, this);
}

bool __thiscall CVehicle::Unk_9C5360()
{
    return (m_nVehFlagsF6D & 0x10) != 0;
}

bool __thiscall CVehicle::Unk_9C5370(bool updateTimer)
{
    using Fn = bool(__thiscall*)(CVehicle*);
    if (!reinterpret_cast<Fn>(m_vftable[0x12C / 4])(this))
        return false;

    if (updateTimer)
        m_nVehUnk12F4 = Core::StaticRef<uint32>(0x11DDE74);

    m_nVehFlagsF6D |= 0x10u;
    m_nFlags &= ~8u;
    Core::Hook::Call<void>(0xA27B30, this, 0);
    return true;
}

bool __thiscall CVehicle::Unk_9C53C0(float* outProgress)
{
    *outProgress = Core::StaticRef<float>(0xD50398);
    if (Core::StaticRef<uint8>(0x11DC82A) == 0)
        return false;

    uint32 start = m_nVehUnk12F4;
    uint32 cur = Core::StaticRef<uint32>(0x11DDE74);
    uint32 dur = Core::StaticRef<uint32>(0xF1D984);
    if (cur > start + dur || cur < start)
        return false;

    *outProgress = static_cast<float>(static_cast<double>(static_cast<int32>(cur - start)) / static_cast<double>(dur));
    return true;
}

bool __thiscall CVehicle::Unk_9C5430()
{
    return (m_nVehFlagsF6D & 0x40) != 0;
}

int32 __stdcall CVehicle::Unk_9C6DF0(int32 seat)
{
    switch (seat)
    {
    case 1: return 0;
    case 2: return 2;
    case 3: return 1;
    case 4: return 3;
    default: return -1;
    }
}

int32 __thiscall CVehicle::GetPedSeat(CPed* pPed)
{
    if (pPed == m_pDriver)
        return 0;
    if (pPed == m_pPassengers[0])
        return 2;
    if (pPed == m_pPassengers[1])
        return 1;
    if (pPed == m_pPassengers[2])
        return 3;
    return -1;
}

int32 __thiscall CVehicle::Unk_9C6E90(int32 targetId)
{
    const int32 count = Unk_9C6B20();
    if (count <= 0)
        return 1;

    for (int32 i = 0; i < count; ++i)
    {
        void* entry = Unk_9C6AF0(i);
        if (Core::Hook::CallMethod<int32>(0x8EFD40, entry, 0) == targetId)
            return Core::Hook::CallMethod<int32>(0xBE6220, entry, this);
    }
    return 1;
}

int32 __thiscall CVehicle::Unk_9C6EF0(int32 a1)
{
    int32 val = *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0x1354);
    if (val != 1)
        return a1 + 1;
    if (a1 == 0)
        return 1;
    return (a1 == 2) ? 2 : 1;
}

const char* __thiscall CVehicle::Unk_9C6F20(int32 id)
{
    struct Entry
    {
        const char* name;
        int32 id;
    };

    int32 vehType = *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0x1354);
    void* mgr = Core::StaticRef<void*>(0x15F5514);
    void** mgrVtbl = *reinterpret_cast<void***>(mgr);

    using GetTableFn = Entry*(__thiscall*)(void*, int32);
    Entry* table = reinterpret_cast<GetTableFn>(mgrVtbl[3])(mgr, vehType);

    if (id >= 89 && vehType != 0)
        reinterpret_cast<void(__thiscall*)(void*, int32)>(mgrVtbl[4])(mgr, vehType);

    for (int32 i = 0; table[i].name != nullptr; ++i)
    {
        if (table[i].id == id)
            return table[i].name;
    }

    return Core::StaticRef<const char*>(0xF33A98);
}

void __thiscall CVehicle::Unk_9C6F90(float t)
{
    int32 idx = Core::StaticRef<int32>(0x10F9AC0);
    float baseVal = *reinterpret_cast<float*>(Core::Rebase(0x11F5D34) + idx * 0x210);
    m_fVehUnkF84 = (Core::StaticRef<float>(0xEBE004) - baseVal) * t + baseVal;
}

bool __thiscall CVehicle::IsPlayerInVehicle()
{
    CPed* pPlayer = Core::Hook::Call<CPed*>(0x817F40);
    if (!pPlayer)
        return false;
    if (pPlayer == m_pDriver)
        return true;

    for (uint32 i = 0; i < 8; ++i)
    {
        if (pPlayer == m_pPassengers[i])
            return true;
    }
    return false;
}

bool __thiscall CVehicle::Unk_9C5440()
{
    if (m_pDriver != nullptr && m_pDriver->m_nDummyConvertState == 2)
        return true;

    for (int32 i = 0; i < 8; ++i)
    {
        if (m_pPassengers[i] != nullptr && m_pPassengers[i]->m_nDummyConvertState == 2)
            return true;
    }
    return false;
}

bool __thiscall CVehicle::Unk_9C5480()
{
    if ((m_nVehFlagsF6D & 0x40) != 0)
        return true;
    if (m_pDriver != nullptr)
        return true;

    for (int32 i = 0; i < 8; ++i)
    {
        if (m_pPassengers[i] != nullptr)
            return true;
    }
    return false;
}

bool __thiscall CVehicle::Unk_9C54C0()
{
    if ((m_nVehFlagsF6D & 0x40) != 0)
        return true;
    if (m_pDriver != nullptr && m_pDriver->m_bIsInjured == 0)
        return true;

    for (int32 i = 0; i < 8; ++i)
    {
        if (m_pPassengers[i] != nullptr && m_pPassengers[i]->m_bIsInjured == 0)
            return true;
    }
    return false;
}

void* __thiscall CVehicle::Unk_9C5510(int32 key)
{
    int32 count = static_cast<int32>(m_nVehUnkFD4);
    if (count <= 0)
        return nullptr;

    uint8* arr = reinterpret_cast<uint8*>(m_nVehUnkFD0);
    for (int32 i = 0; i < count; ++i)
    {
        if (*reinterpret_cast<int32*>(arr + i * 0x170) == key)
            return arr + i * 0x170;
    }
    return nullptr;
}

int32 __thiscall CVehicle::Unk_9C5570()
{
    void* pSub = Core::Hook::CallMethod<void*>(0x9E7080, this);
    bool flag = false;
    if (!pSub)
    {
        flag = true;
    }
    else
    {
        void* p18 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(pSub) + 0x18);
        if (p18 != nullptr)
        {
            uint32 v = *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(p18) + 0x18);
            if (v == 0 || v == 2)
                flag = true;
        }
    }

    int32 count = static_cast<int32>(m_nVehUnkFD4);
    int32 total = 0;
    if (count > 0)
    {
        uint8* p = reinterpret_cast<uint8*>(m_nVehUnkFD0) + 0x164;
        for (int32 i = 0; i < count; ++i)
        {
            uint32 f = *reinterpret_cast<uint32*>(p);
            if ((f & 1u) != 0 || (flag && ((f >> 1) & 1u) != 0))
                ++total;
            p += 0x170;
        }
    }
    return total;
}

void* __thiscall CVehicle::Unk_9C55E0(int32 key)
{
    int32 count = static_cast<int32>(m_nVehUnkFDC);
    if (count <= 0)
        return nullptr;

    uint8* arr = reinterpret_cast<uint8*>(m_nVehUnkFD8);
    for (int32 i = 0; i < count; ++i)
    {
        if (*reinterpret_cast<int32*>(arr + i * 0x34) == key)
            return arr + i * 0x34;
    }
    return nullptr;
}

void __cdecl CVehicle::Unk_9C5630(int32 modelIdx, float* outA, float* outB, uint8 a4)
{
    CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[modelIdx];
    Core::Hook::CallMethod<void>(0xBE63A0, modelInfo, outA, outB, a4);
}

void __thiscall CVehicle::Unk_9C5650()
{
    uint16 val = static_cast<uint16>(m_nVehUnk130E);
    if (val != 0)
    {
        float dt = Core::StaticRef<float>(0x11DDE7C);
        uint32 dec = static_cast<uint32>(static_cast<int64>(dt * Core::StaticRef<float>(0xD82154)));
        if (static_cast<uint32>(val) > dec)
            m_nVehUnk130E = static_cast<int16>(val - dec);
        else
            m_nVehUnk130E = 0;

        if (static_cast<uint16>(m_nVehUnk130E) == 0)
        {
            using Fn = void(__thiscall*)(CVehicle*, uint32, int32, int32, int32, int32);
            reinterpret_cast<Fn>(m_vftable[0x180 / 4])(this, m_nVehUnk1310, 0, 1, 0, 0x33);
        }
    }
}

bool __stdcall CVehicle::Unk_9C56E0(const uint8* pObj)
{
    if (!pObj)
        return false;

    bool flag = false;
    if (Core::StaticRef<int32>(0x10F8044) == 2)
    {
        CPed* pPlayer = Core::Hook::Call<CPed*>(0x817F40);
        if (pPlayer != nullptr && *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(pPlayer) + 0xEF4) > 0)
            flag = true;
    }

    if (static_cast<uint8>(pObj[0x295E] ^ pObj[0x295C]) > 0x7Fu)
        return true;

    if (Core::StaticRef<uint8>(0x1724286) != 0 || flag)
        return false;

    return static_cast<uint8>(pObj[0x296E] ^ pObj[0x296C]) > 0x7Fu;
}

void __thiscall CVehicle::Unk_9C5760(CPed* pPed)
{
    if (!pPed)
        return;

    int32 idx = 0;
    if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(this) + 0x1350) == 3)
    {
        for (idx = 0; idx < 8; ++idx)
        {
            if (m_pPassengers[idx] == pPed)
                break;
        }
        if (idx >= 8)
            return;
    }
    else
    {
        int32 maxPass = static_cast<int32>(m_nMaxPassengers);
        if (maxPass <= 0)
            return;

        for (idx = 0; idx < maxPass; ++idx)
        {
            if (m_pPassengers[idx] == pPed)
                break;
        }
        if (idx >= maxPass)
            return;
    }

    CPed** slot = &m_pPassengers[idx];
    if (*slot != nullptr)
        (*slot)->CleanUpOldReference(reinterpret_cast<CEntity**>(slot));
    *slot = nullptr;
}

void* __thiscall CVehicle::Unk_9C6AF0(int32 idx)
{
    uint8* modelInfo = reinterpret_cast<uint8*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    return modelInfo + 12 * idx + 0x350;
}

int32 __thiscall CVehicle::Unk_9C6B20()
{
    const uint8* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    return *reinterpret_cast<const int32*>(modelInfo + 0x3BC);
}

bool __thiscall CVehicle::Unk_9C6B40()
{
    uint32 count = m_nVehUnkFF8;
    if (count == 0)
        return false;

    for (uint32 i = 0; i < count; ++i)
    {
        if (Core::Hook::CallMethod<bool>(0x9C5280, m_vehSubObjFFC + i * 0x14))
            return true;
    }
    return false;
}

int32 __thiscall CVehicle::Unk_9C6B70(int32 key)
{
    uint32 count = m_nVehUnkFF8;
    if (count == 0)
        return -1;

    for (uint32 i = 0; i < count; ++i)
    {
        if (*reinterpret_cast<const int32*>(m_vehSubObjFFC + i * 0x14 + 0xC) == key)
            return static_cast<int32>(i);
    }
    return -1;
}

void* __thiscall CVehicle::Unk_9C6BA0(int32 idx)
{
    return m_vehSubObjFFC + idx * 0x14;
}

void* __thiscall CVehicle::Unk_9C6BC0(int32 val)
{
    uint8* entry = m_vehSubObjFFC + m_nVehUnkFF8 * 0x14;
    Core::Hook::CallMethod<void>(0x9C5200, entry, val);
    ++m_nVehUnkFF8;
    return entry;
}

void __cdecl CVehicle::Unk_9C6C00()
{
    if (Core::StaticRef<uint8>(0x11E917C) == 0)
        return;

    void* pCfg = reinterpret_cast<void*>(Core::Rebase(0x11E916C));
    Core::Hook::CallMethod<void>(0x88A3E0, reinterpret_cast<void*>(Core::Rebase(0x16192D0)), pCfg, reinterpret_cast<const char*>(Core::Rebase(0xD82084)), reinterpret_cast<void*>(Core::Rebase(0xD81D1F)));
    Core::Hook::CallMethod<void>(0x88A3E0, reinterpret_cast<void*>(Core::Rebase(0x1619290)), pCfg, reinterpret_cast<const char*>(Core::Rebase(0xD82094)), reinterpret_cast<void*>(Core::Rebase(0xD81E69)));
    Core::Hook::CallMethod<void>(0x88A3E0, reinterpret_cast<void*>(Core::Rebase(0x16192B0)), pCfg, reinterpret_cast<const char*>(Core::Rebase(0xD820A0)), reinterpret_cast<void*>(Core::Rebase(0xD81E6A)));
    Core::Hook::CallMethod<void>(0x88A3E0, reinterpret_cast<void*>(Core::Rebase(0x1619270)), pCfg, reinterpret_cast<const char*>(Core::Rebase(0xD820AC)), reinterpret_cast<void*>(Core::Rebase(0xD81E6B)));
    Core::Hook::CallMethod<void>(0x88A3E0, reinterpret_cast<void*>(Core::Rebase(0x16192F0)), pCfg, reinterpret_cast<const char*>(Core::Rebase(0xD820BC)), reinterpret_cast<void*>(Core::Rebase(0xD81E75)));

    Core::StaticRef<float>(0x1619120) = Core::Hook::CallMethod<float>(0x88A720, pCfg, "car.headlight.angle", 0.0f);
    Core::Hook::CallMethod<void>(0x88A750, reinterpret_cast<void*>(Core::Rebase(0x16191C8)), pCfg);
    Core::StaticRef<float>(0xF33A44) = Core::Hook::CallMethod<float>(0x88A720, pCfg, "car.lod.distance.high", 0.0f);
    Core::StaticRef<float>(0xF33A48) = Core::Hook::CallMethod<float>(0x88A720, pCfg, "car.lod.distance.low", 0.0f);
    Core::Hook::Call<void>(0x88C880);
}

int32 __thiscall CVehicle::GetPassengerCount()
{
    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    int32 count = 0;
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
        {
            if (m_pPassengers[i] != nullptr)
                ++count;
        }
    }
    return count;
}

void __thiscall CVehicle::Unk_9C7010(bool setFlag)
{
    auto updatePedFlag = [setFlag](CPed* pPed)
    {
        if (pPed != nullptr)
        {
            if (setFlag)
                pPed->m_nPropFlags |= 0x100u;
            else
                pPed->m_nPropFlags &= ~0x100u;
        }
    };

    updatePedFlag(m_pDriver);
    for (int32 i = 0; i < 8; ++i)
        updatePedFlag(m_pPassengers[i]);
}

void __thiscall CVehicle::Unk_9C7170(const float* pVec3)
{
    m_fVehUnk10A0 += pVec3[0];
    m_fVehUnk10A4 += pVec3[1];
    m_fVehUnk10A8 += pVec3[2];
}

void __thiscall CVehicle::Unk_9C71C0(const float* pVec3)
{
    m_fVehUnk10B0 += pVec3[0];
    m_fVehUnk10B4 += pVec3[1];
    m_fVehUnk10B8 += pVec3[2];
}

void __thiscall CVehicle::Unk_9C7210(const float* pA, const float* pB)
{
    m_fVehUnk10B0 += pA[2] * pB[1] - pA[1] * pB[2];
    m_fVehUnk10B4 += pA[0] * pB[2] - pA[2] * pB[0];
    m_fVehUnk10B8 += pA[1] * pB[0] - pA[0] * pB[1];
}

bool __thiscall CVehicle::Unk_9C72B0()
{
    uint8* pSub = Core::Hook::Call<uint8*>(0xA26FC0, this);
    return pSub != nullptr && pSub[0x26] == 0xF;
}

void __thiscall CVehicle::Unk_9C72D0()
{
    float vec[4] = {};
    using GetVelFn = void(__thiscall*)(CVehicle*, float*);
    reinterpret_cast<GetVelFn>(m_vftable[0xEC / 4])(this, vec);

    if (m_nVehUnk134C != 0 && (vec[1] * vec[1] + vec[0] * vec[0]) > Core::StaticRef<float>(0xDCB2A0))
    {
        m_nVehUnk134C = 0;
        m_fVehUnk1348 += Core::StaticRef<float>(0x11DDE7C);
        if (m_fVehUnk1348 >= Core::StaticRef<float>(0xF33AA8))
        {
            uint32 arg1 = reinterpret_cast<uint32>(&m_pMatrix->pos);
            Core::Hook::Call<void>(0x987730, 23, arg1, this, 0, 0, 0.0f);
        }
    }
    else
    {
        m_fVehUnk1348 = 0.0f;
    }
}

void __thiscall CVehicle::Unk_9C7380()
{
    if ((m_nVehFlagsF67 & 8) == 0)
        return;

    auto applyToPed = [this](CPed* pPed)
    {
        if (!pPed)
            return;

        uint8* p224 = static_cast<uint8*>(pPed->m_pPedIntelligence);
        uint32* node = *reinterpret_cast<uint32**>(p224 + 0x2E0);
        if (node != nullptr)
        {
            uint32 prevPri = (node[2] >> 1) & 7u;
            do
            {
                uint32 pri = (node[2] >> 1) & 7u;
                if (prevPri < pri)
                    break;
                prevPri = pri;
                if (node[1] == 0x32D)
                    return;
                node = reinterpret_cast<uint32*>(node[3]);
            } while (node);
        }

        uint8 eventBuf[0x24] = {};
        Core::Hook::CallMethod<void>(0xAB9A00, eventBuf, this, Core::StaticRef<uint32>(0x11DDE74), 0x3A);

        uint8 dmgInfo[0x14] = {};
        float dmg = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xD81E7C);
        Core::Hook::CallMethod<void>(0xAB9470, dmgInfo, this, dmg, 0x3A, 0, 0);

        uint8 resp[0x18] = {};
        Core::Hook::CallMethod<void>(0xABB170, dmgInfo, pPed, resp);

        if ((resp[4] & 0xC) != 0)
            Core::Hook::CallMethod<void>(0xAA80D0, p224 + 0x84, eventBuf, 0, 1);

        Core::Hook::CallMethod<void>(0xC28820, dmgInfo);
        Core::Hook::CallMethod<void>(0xAB9A80, eventBuf);
    };

    applyToPed(m_pDriver);

    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
            applyToPed(m_pPassengers[i]);
    }
}

int32 __thiscall CVehicle::Unk_9C7560()
{
    uint8* p34 = static_cast<uint8*>(m_pDrawHandler);
    if (!p34)
        return -1;
    uint8* p4 = *reinterpret_cast<uint8**>(p34 + 4);
    return p4 ? *reinterpret_cast<int32*>(p4 + 0xD8) : -1;
}

void __thiscall CVehicle::Unk_9C7580(int32 val)
{
    uint8* p34 = static_cast<uint8*>(m_pDrawHandler);
    if (!p34)
        return;
    uint8* p4 = *reinterpret_cast<uint8**>(p34 + 4);
    if (p4)
        *reinterpret_cast<int32*>(p4 + 0xD8) = val;
}

void __thiscall CVehicleStateSnapshot::Save(CVehicle* pVeh)
{
    uint8* base = reinterpret_cast<uint8*>(this);
    const float* mat = reinterpret_cast<const float*>(pVeh->m_pMatrix);
    float* dstMat = reinterpret_cast<float*>(base);
    for (int32 row = 0; row < 4; ++row)
    {
        dstMat[row * 4 + 0] = mat[row * 4 + 0];
        dstMat[row * 4 + 1] = mat[row * 4 + 1];
        dstMat[row * 4 + 2] = mat[row * 4 + 2];
    }

    std::memcpy(base + 0x41, reinterpret_cast<const uint8*>(pVeh) + 0xFE4, 4);
    *reinterpret_cast<int16*>(base + 0x46) = pVeh->m_nVehUnkF98;
    base[0x48] = pVeh->m_nMaxPassengers;
    *reinterpret_cast<float*>(base + 0x4C) = pVeh->m_fVehUnk10D8;
    *reinterpret_cast<float*>(base + 0x50) = pVeh->m_fVehUnk10C8;
    *reinterpret_cast<float*>(base + 0x54) = pVeh->m_fVehUnk10CC;
    std::memcpy(base + 0x58, &pVeh->m_nVehFlagsF64, 0xE);
}

void __thiscall CVehicleStateSnapshot::Restore(CVehicle* pVeh)
{
    const uint8* base = reinterpret_cast<const uint8*>(this);
    using Fn1 = void(__thiscall*)(CVehicle*, void*, int32, int32);
    reinterpret_cast<Fn1>(pVeh->m_vftable[1])(pVeh, this, 0, 0);

    std::memcpy(reinterpret_cast<uint8*>(pVeh) + 0xFE4, base + 0x41, 4);
    Core::Hook::CallMethod<void>(0x9C52E0, pVeh);

    pVeh->m_nVehUnkF98 = *reinterpret_cast<const int16*>(base + 0x46);
    pVeh->m_nMaxPassengers = base[0x48];
    pVeh->m_fVehUnk10D8 = *reinterpret_cast<const float*>(base + 0x4C);
    pVeh->m_fVehUnk10C8 = *reinterpret_cast<const float*>(base + 0x50);
    pVeh->m_fVehUnk10CC = *reinterpret_cast<const float*>(base + 0x54);
    std::memcpy(&pVeh->m_nVehFlagsF64, base + 0x58, 0xE);
}

void* __thiscall CVehicle::Unk_9C7710(int32 boneIdx)
{
    void* v40 = reinterpret_cast<void*(__thiscall*)(CVehicle*)>(m_vftable[40])(this);
    uint8* pObj = v40
        ? reinterpret_cast<uint8*(__thiscall*)(void*)>((*reinterpret_cast<void***>(v40))[56])(v40)
        : *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x100);

    if (pObj != nullptr)
    {
        uint8* p78 = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x78);
        if (p78 != nullptr && (p78[0] & 1) != 0)
            Core::Hook::CallMethod<void>(0x509FF0, reinterpret_cast<void*>(Core::Rebase(0x16397D0)), p78 + 0x1A04, 1);

        return *reinterpret_cast<uint8**>(pObj + 0xC) + 0x50 * boneIdx;
    }

    float one = Core::StaticRef<float>(0xD50398);
    Core::StaticRef<float>(0x1639460) = one;
    Core::StaticRef<float>(0x1639474) = one;
    Core::StaticRef<float>(0x1639488) = one;
    Core::StaticRef<float>(0x1639464) = 0.0f;
    Core::StaticRef<float>(0x1639468) = 0.0f;
    Core::StaticRef<float>(0x1639470) = 0.0f;
    Core::StaticRef<float>(0x1639478) = 0.0f;
    Core::StaticRef<float>(0x1639480) = 0.0f;
    Core::StaticRef<float>(0x1639484) = 0.0f;
    Core::StaticRef<float>(0x1639490) = 0.0f;
    Core::StaticRef<float>(0x1639494) = 0.0f;
    Core::StaticRef<float>(0x1639498) = 0.0f;
    return reinterpret_cast<void*>(Core::Rebase(0x1639460));
}

int32 __thiscall CVehicleLink::GetBoneIndex()
{
    return m_pOwner->Unk_9C6B70(m_nBoneId);
}

void __thiscall CVehicleLink::UpdateAudio(bool bUsePending)
{
    CVehicle* pA = m_pOwner;
    CEntity* pB = static_cast<CEntity*>(bUsePending ? m_pPending : m_pAttached);
    uint8 argFlag = bUsePending ? 1 : 0;

    if (pA != nullptr && pB != nullptr)
    {
        void* p6Ca = reinterpret_cast<void*>(pA->m_nUnk6C);
        void* p6Cb = reinterpret_cast<void*>(pB->m_nUnk6C);
        if (p6Ca != nullptr && p6Cb != nullptr)
        {
            int32 bone = GetBoneIndex();
            int32 idB = Core::Hook::CallMethod<int32>(0x49F370, p6Cb);
            int32 idA = Core::Hook::CallMethod<int32>(0x49F370, p6Ca);
            Core::Hook::CallMethod<void>(0x470980, reinterpret_cast<void*>(Core::Rebase(0x18CF5C8)), idA, idB, bone, argFlag, -1);
            m_nTimer = 200;
            m_bFlag = 1;
        }
    }
}

void __thiscall CVehicleLink::Set(void* pEntity, bool bSet, bool bDoUpdate)
{
    if (bDoUpdate)
    {
        if (bSet)
            Core::Hook::CallMethod<void>(0x9C5210, this, pEntity);
        else if (Core::Hook::CallMethod<bool>(0x9C5240, this, pEntity))
            Core::Hook::CallMethod<void>(0x9C5210, this, nullptr);
    }
    m_nTimer = 0;
    m_bFlag = 0;
}

bool __thiscall CVehicleLink::Attach(void* pEntity)
{
    CEntity* pEnt = static_cast<CEntity*>(pEntity);
    uint8* p6C = reinterpret_cast<uint8*>(pEnt->m_nUnk6C);
    if (!p6C || p6C[0xE] == 0)
    {
        m_pPending = pEntity;
        pEnt->RegisterReference(reinterpret_cast<CEntity**>(&m_pPending));
    }

    uint8* pOwner6C = reinterpret_cast<uint8*>(m_pOwner->m_nUnk6C);
    if (pOwner6C != nullptr && (pOwner6C[0xE] != 0 || Core::Hook::CallMethod<bool>(0x49F3F0, pOwner6C)))
        return false;

    m_pAttached = pEntity;
    pEnt->RegisterReference(reinterpret_cast<CEntity**>(&m_pAttached));
    return true;
}

bool __thiscall CVehicleLink::Detach(void* pEntity)
{
    if (m_pPending == pEntity)
    {
        if (m_pPending)
            static_cast<CEntity*>(m_pPending)->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pPending));
        m_pPending = nullptr;
    }

    uint8* pOwner6C = reinterpret_cast<uint8*>(m_pOwner->m_nUnk6C);
    if (pOwner6C != nullptr && (pOwner6C[0xE] != 0 || Core::Hook::CallMethod<bool>(0x49F3F0, pOwner6C)))
        return false;

    if (m_pAttached)
        static_cast<CEntity*>(m_pAttached)->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pAttached));
    m_pAttached = nullptr;
    return true;
}

void __thiscall CVehicleLink::Process()
{
    uint8* pOwner6C = reinterpret_cast<uint8*>(m_pOwner->m_nUnk6C);
    bool canModify = (!pOwner6C || pOwner6C[0xE] == 0) && (!pOwner6C || !Core::Hook::CallMethod<bool>(0x49F3F0, pOwner6C));

    if (m_nTimer > 0)
    {
        m_nTimer -= static_cast<int16>(static_cast<int32>(Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xDAF864)));
        if (m_nTimer <= 0)
        {
            m_nTimer = 0;
            m_bFlag = 0;
        }
    }

    if (m_pPending != nullptr)
    {
        CEntity* pPendEnt = static_cast<CEntity*>(m_pPending);
        uint8* pPend6C = reinterpret_cast<uint8*>(pPendEnt->m_nUnk6C);
        if ((!pPend6C || pPend6C[0xE] == 0) && m_pAttached == nullptr)
        {
            if (canModify)
            {
                m_pAttached = m_pPending;
                pPendEnt->RegisterReference(reinterpret_cast<CEntity**>(&m_pAttached));
                m_bFlag = 0;
            }
            else if (m_bFlag == 0)
            {
                UpdateAudio(true);
            }
        }

        if (m_pPending)
            static_cast<CEntity*>(m_pPending)->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pPending));
        m_pPending = nullptr;
    }
    else if (m_pAttached != nullptr)
    {
        CEntity* pAttEnt = static_cast<CEntity*>(m_pAttached);
        uint8* pAtt6C = reinterpret_cast<uint8*>(pAttEnt->m_nUnk6C);
        if (!pAtt6C || pAtt6C[0xE] == 0)
        {
            if (canModify)
            {
                pAttEnt->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_pAttached));
                m_pAttached = nullptr;
            }
            else if (m_bFlag == 0)
            {
                UpdateAudio(false);
            }
        }
    }
}

CVehicleSubObjE24* __thiscall CVehicleSubObjE24::Ctor()
{
    uint8* base = reinterpret_cast<uint8*>(this);
    for (int32 i = 0; i < 14; ++i)
        reinterpret_cast<int32*>(base)[i] = -1;

    Core::Hook::CallMethod<void>(0xA34470, base + 0x74);
    Core::Hook::CallMethod<void>(0xA34470, base + 0xA0);
    Core::Hook::CallMethod<void>(0xA34470, base + 0xD0);

    *reinterpret_cast<int32*>(base + 0xFC) = -1;
    Core::Hook::CallMethod<void>(0xA33360, base);

    Core::Hook::CallMethod<void>(0xA338F0, base + 0x74);
    Core::Hook::CallMethod<void>(0xA338F0, base + 0xA0);
    Core::Hook::CallMethod<void>(0xA338F0, base + 0xD0);

    base[0xCC] = 0;
    base[0x124] &= 7u;
    base[0x125] &= ~1u;
    *reinterpret_cast<uint32*>(base + 0x64) = Core::StaticRef<uint32>(0x11DDE74);
    *reinterpret_cast<uint32*>(base + 0x68) = 0;
    base[0x11A] = 0;
    *reinterpret_cast<float*>(base + 0x11C) = Core::StaticRef<float>(0xD50398);
    base[0x13C] = 0xFF;
    base[0x119] = 0;
    *reinterpret_cast<uint32*>(base + 0x104) = 0;
    base[0x118] = 0;
    base[0x121] = 0;
    base[0x122] = 0;
    base[0x126] = 0x14;
    base[0x13D] = 0;
    base[0x127] = static_cast<uint8>((Core::Hook::Call<int32>(0xD0D21A) & 7) + 4);
    *reinterpret_cast<uint32*>(base + 0x138) = 0;
    *reinterpret_cast<float*>(base + 0x12C) = 0.0f;
    *reinterpret_cast<uint32*>(base + 0x108) = 0;
    base[0x128] = 0;
    *reinterpret_cast<uint32*>(base + 0x10C) = 0;
    *reinterpret_cast<uint32*>(base + 0x110) = 0;
    base[0x100] = 3;
    *reinterpret_cast<int32*>(base + 0xFC) = -1;
    *reinterpret_cast<uint32*>(base + 0x6C) = 0;
    *reinterpret_cast<float*>(base + 0x130) = 1.0f;
    *reinterpret_cast<float*>(base + 0x134) = 0.0f;
    *reinterpret_cast<uint32*>(base + 0x70) = 0;
    *reinterpret_cast<uint32*>(base + 0x114) = 0;
    base[0x123] = 0;
    return this;
}

void __cdecl CVehicle::InitSystem()
{
    Core::Hook::Call<void>(0x9718C0);
    CVehicle::Unk_9C6C00();
    Core::StaticRef<uint32>(0x1619258) = Core::Hook::Call<uint32>(0x5B1C30, "headlights", 0);
    Core::StaticRef<uint32>(0x161925C) = Core::Hook::Call<uint32>(0x5B1C30, "carbottom", 0);
    Core::StaticRef<uint32>(0x1619260) = Core::Hook::Call<uint32>(0x5B1C30, "bikebottom", 0);
}

int32 __thiscall CVehicle::Unk_9C80B0(int32, int32, int32, int32, int32)
{
    return 0;
}

int32 __thiscall CVehicle::Unk_9C80C0()
{
    return 10;
}

int32 __thiscall CVehicle::Unk_9C80D0()
{
    CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    int32* pCC = reinterpret_cast<int32*>(modelInfo->m_pVehicleStruct);

    auto zeroBoneMat = [this](int32 bone)
    {
        float* m = Core::Hook::CallMethod<float*>(0x4D9100, this, bone);
        m[0] = m[1] = m[2] = 0.0f;
        m[4] = m[5] = m[6] = 0.0f;
        m[8] = m[9] = m[10] = 0.0f;
        m[12] = m[13] = m[14] = 0.0f;
    };

    uint32 maskE20 = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(this) + 0xE20);
    if (maskE20 != 0)
    {
        for (int32 i = 0x59; i <= 0x62; ++i)
        {
            if (((1u << (i - 0x58)) & maskE20) != 0)
                zeroBoneMat(pCC[i]);
        }
    }

    uint8* self = reinterpret_cast<uint8*>(this);
    if (self[0x11EE] != 0)
    {
        for (int32 k = 0; k <= 12; ++k)
        {
            if (self[0x11E0 + k] != 0)
                zeroBoneMat(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(pCC) + 0xC8 + k * 4));
        }
        self[0x11EE] = 0;
    }

    if ((m_nVehFlagsF6E & 0x20) != 0 && self[0x11EF] != 0)
    {
        for (int32 k = 0; k <= 7; ++k)
        {
            if (self[0x11F0 + k] != 0)
                zeroBoneMat(*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(pCC) + 0x104 + k * 4));
        }
        self[0x11EF] = 0;
    }

    if (Core::StaticRef<uint8>(0x15BFF22) == 0)
    {
        float scale = *reinterpret_cast<float*>(self + 0x111C);
        if (scale >= 0.0f)
        {
            self[0x5B] = static_cast<uint8>(static_cast<int32>(static_cast<float>(self[0x5B]) * scale));
            *reinterpret_cast<float*>(self + 0x111C) = Core::StaticRef<float>(0xEBDED4);
        }
    }

    modelInfo->m_nIDEFlags |= 0x2000000u;

    bool hasPlayer = IsPlayerInVehicle();
    if (hasPlayer)
        m_nFlags2 |= 0x10000000u;
    else
        m_nFlags2 &= ~0x10000000u;

    bool bit29 = hasPlayer && ((Core::StaticRef<uint8>(0x11DC582) & 2) != 0 || Core::StaticRef<uint8>(0x1111D88) != 0);
    if (bit29)
        m_nFlags2 |= 0x20000000u;
    else
        m_nFlags2 &= ~0x20000000u;

    bool bit28 = (m_nFlags2 & 0x10000000u) != 0;
    auto propagateBit28 = [bit28](CPed* pPed)
    {
        if (pPed != nullptr)
        {
            if (bit28)
                pPed->m_nFlags2 |= 0x10000000u;
            else
                pPed->m_nFlags2 &= ~0x10000000u;
        }
    };

    propagateBit28(m_pDriver);
    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
            propagateBit28(m_pPassengers[i]);
    }

    if (Core::StaticRef<int32>(0x10D0170) == 1 ||
        Core::StaticRef<int32>(0x10E1984) != Core::StaticRef<int32>(0xE3447C) ||
        Core::StaticRef<int32>(0xF0E4A0) == 0x12)
    {
        return 0;
    }

    return Core::Hook::CallMethod<int32>(0x9E1480, this);
}

void* __thiscall CVehicle::Unk_9C83A0(int32 a2, int32 a3, int32 a4, int32 a5)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if ((m_nFlags2 & 0x20000000u) != 0 && a2 == 1 && a3 != 0x40)
        return nullptr;

    int32 modelIdx = m_nModelIndex;
    CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[modelIdx]);
    int32 effModel = (a5 != -1) ? a5 : modelIdx;
    void* savedPtr = nullptr;

    if ((m_nVehFlagsF70 & 0x40) != 0)
    {
        int32 altModel = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(modelInfo) + 0x34C);
        if (Core::Hook::Call<bool>(0x832DD0, altModel, CModelInfo::ms_nStreamingModuleId))
        {
            effModel = altModel;
            Core::Hook::CallMethod<void>(0x7B7130, reinterpret_cast<void*>(Core::Rebase(0x10F5F80)), altModel);
            uint8* p34 = static_cast<uint8*>(m_pDrawHandler);
            savedPtr = p34 ? *reinterpret_cast<void**>(p34 + 4) : nullptr;
            if (p34)
                *reinterpret_cast<uint32*>(p34 + 4) = m_nVehUnkFEC;
        }
    }

    auto makeDC = [](uintptr_t fnAddr)
    {
        uint32* dc = Core::Hook::Call<uint32*>(0x7BDD80, 12, 0);
        if (dc)
        {
            dc[0] = Core::Rebase(0xD50380);
            uint32& seq = Core::StaticRef<uint32>(0xF1023C);
            dc[1] = (dc[1] & ~0x3FFFu) | (seq++ & 0x3FFFu);
            dc[0] = Core::Rebase(0xD503A0);
            dc[2] = Core::Rebase(fnAddr);
            Core::Hook::CallMethod<void>(0x404E90, dc);
        }
    };

    uint8* pRet = nullptr;
    if ((m_nFlags2 & 0x10000000u) != 0)
    {
        makeDC(0xB41300);
        pRet = Core::Hook::CallMethod<uint8*>(0x9E15D0, this, a2, a3, a4, effModel);
        makeDC(0xB41330);
    }
    else
    {
        pRet = Core::Hook::CallMethod<uint8*>(0x9E15D0, this, a2, a3, a4, effModel);
    }

    if (pRet != nullptr)
    {
        if (self[0x11C0] != 0 && Core::StaticRef<uint8>(0x1135B85) == 0)
            pRet[0x59] |= 0x40u;

        if (m_nVehUnkFD4 != 0)
        {
            pRet[0x59] |= 1u;
            float arrA[6] = {};
            float arrB[6] = {};
            bool any = false;
            int32 count = static_cast<int32>(m_nVehUnkFD4);
            for (int32 i = 0; i < count; ++i)
            {
                uint8* item = reinterpret_cast<uint8*>(m_nVehUnkFD0) + i * 0x170;
                float val = *reinterpret_cast<const float*>(item + 0x160);
                if (val < Core::StaticRef<float>(0xDAF864))
                {
                    float w = 1.0f;
                    if (val > 0.0f)
                    {
                        float ratio = *reinterpret_cast<const float*>(item + 0x70) / *reinterpret_cast<const float*>(item + 0x20);
                        if (ratio > 1.0f)
                            ratio = 1.0f;
                        w = (1.0f - val * Core::StaticRef<float>(0xDCB2AC)) * ratio;
                        if (w < 0.0f)
                            w = 0.0f;
                    }
                    int32 slot = *reinterpret_cast<const int32*>(item);
                    assert(slot >= 0 && slot < 6);
                    arrB[slot] = w;
                    arrA[slot] = Core::Hook::CallMethod<float>(0xB49520, item);
                    any = true;
                }
            }
            if (any)
                Core::Hook::CallMethod<void>(0x7BFE60, pRet, arrB, arrA);

            if (IsPlayerInVehicle())
                pRet[0x59] |= 8u;

            if (*reinterpret_cast<const int32*>(self + 0x1354) == 4)
            {
                uint8* pCC = reinterpret_cast<uint8*>(modelInfo->m_pVehicleStruct);
                if (*reinterpret_cast<const int16*>(pCC + 0x206) > -1 || *reinterpret_cast<const int16*>(pCC + 0x208) > -1)
                    pRet[0x59] |= (*reinterpret_cast<const float*>(self + 0x1F24) > Core::StaticRef<float>(0xD85DB0)) ? 0x30u : 0x10u;
            }
            else if (Core::Hook::Call<bool>(0x462E60))
            {
                pRet[0x59] |= 8u;
            }
        }
    }

    if (a2 == 0 && a3 != 1 && a3 != 2)
        m_nVehUnk12E4 = Core::StaticRef<uint32>(0x11DDE74);

    if (savedPtr)
    {
        uint8* p34 = static_cast<uint8*>(m_pDrawHandler);
        if (p34)
            *reinterpret_cast<void**>(p34 + 4) = savedPtr;
    }

    if (a3 == 8)
        ++Core::StaticRef<int32>(0x1619244);

    return pRet;
}

bool __thiscall CVehicle::Unk_9C8700(int32 boneIdx, float* outVec4)
{
    outVec4[0] = 0.0f;
    outVec4[1] = 0.0f;
    outVec4[2] = 0.0f;

    CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    int32 id = static_cast<int32>(modelInfo->m_pVehicleStruct->m_nBones[boneIdx]);
    if (id == -1)
        return false;

    struct BoneNode
    {
        uint8    pad00[0x10];
        BoneNode* pParent;
        uint8    pad14[0x0C];
        CVector  vecPos;
        float    f2C;
        uint8    pad30[0x10];
        uint8    quat[0x10];
    };

    void* v40 = reinterpret_cast<void*(__thiscall*)(CVehicle*)>(m_vftable[40])(this);
    uint8* pObj = v40
        ? reinterpret_cast<uint8*(__thiscall*)(void*)>((*reinterpret_cast<void***>(v40))[56])(v40)
        : *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x100);

    uint8* base = **reinterpret_cast<uint8***>(pObj + 4) + id * 0xE0;
    outVec4[0] = *reinterpret_cast<const float*>(base + 0x20);
    outVec4[1] = *reinterpret_cast<const float*>(base + 0x24);
    outVec4[2] = *reinterpret_cast<const float*>(base + 0x28);
    outVec4[3] = *reinterpret_cast<const float*>(base + 0x2C);

    for (BoneNode* node = *reinterpret_cast<BoneNode**>(**reinterpret_cast<uint8***>(pObj + 4) + id * 0xE0 + 0x10);
         node != nullptr;
         node = node->pParent)
    {
        CMatrix mat;
        Core::Hook::CallMethod<void>(0x5BE050, &mat, node->quat);
        CVector v(outVec4[0], outVec4[1], outVec4[2]);
        outVec4[0] = mat.right.x * v.x + mat.up.x * v.y + mat.at.x * v.z + node->vecPos.x;
        outVec4[1] = mat.right.y * v.x + mat.up.y * v.y + mat.at.y * v.z + node->vecPos.y;
        outVec4[2] = mat.right.z * v.x + mat.up.z * v.y + mat.at.z * v.z + node->vecPos.z;
    }

    return true;
}

void __thiscall CVehicle::Unk_9C8900(int32 boneId, float* outMat4x4)
{
    const float* m = static_cast<const float*>(Unk_9C7710(boneId));
    for (int32 row = 0; row < 4; ++row)
    {
        outMat4x4[row * 4 + 0] = m[row * 4 + 0];
        outMat4x4[row * 4 + 1] = m[row * 4 + 1];
        outMat4x4[row * 4 + 2] = m[row * 4 + 2];
    }

    void* v40 = reinterpret_cast<void*(__thiscall*)(CVehicle*)>(m_vftable[40])(this);
    uint8* pObj = v40
        ? reinterpret_cast<uint8*(__thiscall*)(void*)>((*reinterpret_cast<void***>(v40))[56])(v40)
        : *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x100);

    for (uint8* node = *reinterpret_cast<uint8**>(**reinterpret_cast<uint8***>(pObj + 4) + boneId * 0xE0 + 0x10);
         node != nullptr;
         node = *reinterpret_cast<uint8**>(node + 0x10))
    {
        const float* pm = static_cast<const float*>(Unk_9C7710(static_cast<int32>(*reinterpret_cast<const uint16*>(node + 0x16))));
        float tmp[16] = {};
        for (int32 row = 0; row < 4; ++row)
        {
            tmp[row * 4 + 0] = pm[row * 4 + 0];
            tmp[row * 4 + 1] = pm[row * 4 + 1];
            tmp[row * 4 + 2] = pm[row * 4 + 2];
        }
        Core::Hook::CallMethod<void>(0x513430, outMat4x4, tmp);
    }

    Core::Hook::CallMethod<void>(0x513430, outMat4x4, m_pMatrix);
}

void __thiscall CVehicle::Unk_9C8A70()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* pE14 = *reinterpret_cast<uint8**>(self + 0xE14);
    uint8* pC = *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(pE14 + 4) + 0xC);
    uint32* tbl = *reinterpret_cast<uint32**>(pC + 0x94);
    if (!tbl)
        return;

    uint16 cnt92 = *reinterpret_cast<const uint16*>(pC + 0x92);
    for (int32 i = 0; i < static_cast<int32>(cnt92); ++i)
    {
        uint8* sub = Core::Hook::CallMethod<uint8*>(0x51DDD0, pE14);
        if ((*reinterpret_cast<uint8***>(sub + 0xD4))[i][0xC] != 0)
            tbl[i * 2 + 1] = 0x69F8;
    }

    int32 cntFD4 = static_cast<int32>(m_nVehUnkFD4);
    for (int32 i = 0; i < cntFD4; ++i)
    {
        int16 idx = *reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(m_nVehUnkFD0) + i * 0x170 + 4);
        if (idx > -1)
            tbl[idx * 2 + 1] = 0x3FE;
    }

    auto applyBone = [pE14, tbl](int32 boneId, uint32 val)
    {
        if (boneId != -1)
        {
            void* sub = Core::Hook::CallMethod<void*>(0x51DDD0, pE14);
            int32 res = Core::Hook::Call<int32>(0xA4B480, sub, boneId);
            if (res != -1)
                tbl[res * 2 + 1] = val;
        }
    };

    CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    uint8* pCC = reinterpret_cast<uint8*>(modelInfo->m_pVehicleStruct);

    for (uint32 off = 0x164; off <= 0x188; off += 4)
        applyBone(*reinterpret_cast<const int32*>(pCC + off), 0x7BFE);

    int32 vehSubType = *reinterpret_cast<const int32*>(self + 0x1354);
    if (vehSubType == 0)
    {
        for (uint32 off = 0x124; off <= 0x140; off += 4)
            applyBone(*reinterpret_cast<const int32*>(pCC + off), 0x7BFE);
    }
    if (vehSubType == 4)
    {
        int32 b = *reinterpret_cast<const int32*>(pCC + 0xA0);
        if (b > -1)
            applyBone(b, 0x7BFE);

        int8 c0 = *reinterpret_cast<const int8*>(self + 0x1F40);
        if (c0 > 0)
            tbl[c0 * 2 + 1] = 0x3DE;

        int8 c1 = *reinterpret_cast<const int8*>(self + 0x1F41);
        if (c1 > 0)
            tbl[c1 * 2 + 1] = 0x3DE;
    }
}

bool __thiscall CVehicle::Unk_9C8C70(bool a2, bool a3, int32, int32, bool a6, bool a7, bool a8)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (a3)
    {
        uint8* p6C = reinterpret_cast<uint8*>(m_nUnk6C);
        if (p6C && (p6C[0xE] != 0 || !reinterpret_cast<bool(__thiscall*)(void*)>((*reinterpret_cast<void***>(p6C))[24])(p6C)))
            return false;
    }

    if (*reinterpret_cast<const int32*>(self + 0x1354) == 3)
        return false;

    if (Core::Hook::Call<bool>(0x83B940, this) && a6)
        return false;

    if (m_pDriver && ((m_pDriver->m_nDummyConvertState == 2 && a2) || (m_pDriver->m_nPedUnk219 != 0 && !a8)))
        return false;

    for (uint32 i = 0; i < 8; ++i)
    {
        if (m_pPassengers[i] && ((m_pPassengers[i]->m_nDummyConvertState == 2 && a2) || m_pPassengers[i]->m_nPedUnk219 != 0))
            return false;
    }

    if (!a7 && !Core::Hook::Call<bool>(0x439C60, this))
        return false;

    switch (m_nCreatedBy)
    {
    case 2: return !a2;
    case 4: return false;
    default: return true;
    }
}

bool __thiscall CVehicle::Unk_9C8D80()
{
    if (Core::Hook::CallMethod<bool>(0x9C61E0, this))
        return false;
    if (*reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(this) + 0xE34) == 0xFFFF)
        return false;

    uint32 tmp = 0;
    if (Core::Hook::Call<bool>(0x96A350, this, &tmp))
        return false;

    return (m_nVehFlagsF6D & 0x20) != 0;
}

bool __thiscall CVehicle::Unk_9C8DC0(bool a2, bool a3, int32 a4)
{
    if (Core::StaticRef<uint8>(0xF33AB0) != 0 && !a3)
    {
        uint8 v16[0x600];
        Core::Hook::CallStdcall<void>(0x401020, v16, 0x60, 0x10, reinterpret_cast<void*>(Core::Rebase(0x425360)));

        uint8 v14[0x40];
        Core::Hook::CallMethod<void>(0x441ED0, v14, m_pMatrix);
        *reinterpret_cast<float*>(v14 + 0x38) += Core::StaticRef<float>(0xF33AAC);

        int32 v13[2] = { reinterpret_cast<int32>(this), a4 };
        int32 cnt = Core::Hook::Call<int32>(0x99BE70, v14, this, -1, v13, 2, v16, 0x10, 0xA8);
        for (int32 i = 0; i < cnt; ++i)
        {
            CEntity* p = Core::Hook::Call<CEntity*>(0xA72220, *reinterpret_cast<const uint32*>(v16 + i * 0x60));
            if (p && (p->m_nFlags & 1u) != 0)
                return false;
        }
    }

    if (a2)
        m_nVehUnk12F4 = m_nVehUnk12F8 = Core::StaticRef<uint32>(0x11DDE74);

    uint8* pRes = Core::Hook::CallMethod<uint8*>(0x928000, Core::StaticRef<void*>(0x1544AD0), this, 0);
    if (pRes)
    {
        reinterpret_cast<void(__thiscall*)(CVehicle*)>(m_vftable[54])(this);
        int32 h = m_nInteriorHash;
        int32 idx = m_nInteriorIndex;
        if (h != -1 && idx != -1 && idx != 0)
        {
            void* pPool = Core::Hook::CallMethod<void*>(0xA4C560, Core::StaticRef<void*>(0x165D2E4), h);
            *reinterpret_cast<uint32*>(pRes + 0x10) = pPool ? Core::Hook::CallMethod<uint32>(0xA3CA40, pPool, idx) : 0;
        }
        else
        {
            *reinterpret_cast<uint32*>(pRes + 0x10) = 0;
        }
    }

    m_nVehFlagsF6D &= ~0x10u;
    return true;
}

bool __thiscall CVehicle::Unk_9C8F30()
{
    if (Core::StaticRef<uint8>(0xF1D926) == 0)
        return false;
    if (m_nCreatedBy == 2 || m_nCreatedBy == 3)
        return false;

    const uint8* self = reinterpret_cast<const uint8*>(this);
    if (*reinterpret_cast<const int32*>(self + 0x1350) == 1)
        return false;

    int32 vt = *reinterpret_cast<const int32*>(self + 0x1354);
    if (vt == 4 || vt == 5)
        return false;

    int32 model = m_nModelIndex;
    if (model == Core::StaticRef<int32>(0x1230778) ||
        model == Core::StaticRef<int32>(0x12308EC) ||
        model == Core::StaticRef<int32>(0x1230310))
    {
        return false;
    }

    if (!Core::Hook::CallMethod<bool>(0x8AEFE0, m_vehSubObj1120, 0))
        return false;

    void* p817 = Core::Hook::Call<void*>(0x817FD0);
    bool flag = p817 && Core::Hook::CallMethod<int32>(0x920FF0, p817) > 0;

    for (int32 i = 0; i <= 3; ++i)
    {
        CPed* pPed = GetPassenger(i);
        if (pPed)
        {
            if (pPed->m_bIsInjured != 0)
                return false;
            if (*reinterpret_cast<const int32*>(static_cast<const uint8*>(pPed->m_pPedData) + 0x12C) == 2 && flag)
                return false;
            CPed* player = Core::Hook::Call<CPed*>(0x817F40);
            if (player && Core::Hook::CallMethod<CPed*>(0x9A69E0, static_cast<uint8*>(pPed->m_pPedIntelligence) + 0x2E0) == player)
                return false;
        }
    }
    return true;
}

bool __thiscall CVehicle::Unk_9C9020()
{
    if ((m_nVehFlagsF6D & 0x40) != 0)
        return true;

    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* p6C = reinterpret_cast<uint8*>(m_nUnk6C);
    if (p6C && (p6C[0xE] != 0 || Core::Hook::CallMethod<bool>(0x49F3F0, p6C)))
        return false;

    if (!reinterpret_cast<bool(__thiscall*)(CVehicle*)>(m_vftable[79])(this))
        return false;

    int32 tmp = -1;
    if (Core::Hook::Call<bool>(0x96A350, this, &tmp))
        return false;

    if (!Unk_9C5480() || Unk_9C5440())
        return false;

    if (m_pDriver && m_pDriver->m_nPedUnk219 != 0)
        return false;

    if ((m_nVehFlagsF6F & 0x20) != 0)
        return false;

    if (m_pDriver)
    {
        void* p = Core::Hook::Call<void*>(0xA26FC0, this);
        if (!p)
            return false;
        self[0xEF0] = 1;
        Core::Hook::CallMethod<void>(0xA33830, self + 0xEF4, p);
    }

    auto checkPed = [](CPed* p) -> bool
    {
        return p && (Core::Hook::CallMethod<bool>(0x4443C0, p) ||
                     (p->m_nUnk6C && Core::Hook::CallMethod<bool>(0x49F3F0, reinterpret_cast<void*>(p->m_nUnk6C))));
    };

    if (checkPed(m_pDriver))
        return false;

    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
        {
            if (checkPed(m_pPassengers[i]))
                return false;
        }
    }

    if (m_pDriver)
        Core::Hook::CallMethod<void>(0x89D8F0, m_pDriver, 0);

    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
        {
            if (m_pPassengers[i])
                reinterpret_cast<void(__thiscall*)(CPed*)>((*reinterpret_cast<void***>(m_pPassengers[i]))[38])(m_pPassengers[i]);
        }
    }

    m_nVehFlagsF6D |= 0x40u;
    return true;
}

void __thiscall CVehicle::Unk_9C91A0()
{
    if (Unk_9C5440())
        return;

    if (m_pDriver)
        Core::Hook::CallMethod<void>(0x89D8F0, m_pDriver, 0);

    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
        {
            if (m_pPassengers[i])
                reinterpret_cast<void(__thiscall*)(CPed*)>((*reinterpret_cast<void***>(m_pPassengers[i]))[38])(m_pPassengers[i]);
        }
    }

    if (m_nCreatedBy != 2)
        m_nFlags |= 0x400u;
}

bool __thiscall CVehicle::Unk_9C9210()
{
    if ((m_nVehFlagsF6D & 0x40) == 0)
        return true;

    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* p6C = reinterpret_cast<uint8*>(m_nUnk6C);
    if (p6C && (p6C[0xE] != 0 || Core::Hook::CallMethod<bool>(0x49F3F0, p6C)))
    {
        if (!p6C || p6C[0xE] == 0)
            Core::Hook::Call<void>(0xC28820, reinterpret_cast<void*>(Core::Rebase(0x191F104)), 2, "PRETEND_OCCUPANTS_FAIL", "PENDING_OWNER_CHANGE");
        return false;
    }

    if (Core::Hook::Call<bool>(0x462F90))
    {
        Core::StaticRef<uint8>(0xFC7EDB) = 0;
        bool ok = Core::Hook::CallMethod<bool>(0x475170, reinterpret_cast<void*>(Core::Rebase(0x18EDB70)), 1, 0);
        Core::StaticRef<uint8>(0xFC7EDB) = 1;
        if (!ok)
            return false;
    }

    if (Core::Hook::CallMethod<int32>(0x447270, Core::StaticRef<void*>(0x18A82AC)) < 5)
        return false;

    if (!Core::Hook::CallMethod<bool>(0x8F2210, reinterpret_cast<void*>(Core::Rebase(0x1354040))))
        return false;

    if (Core::Hook::Call<bool>(0x905A80, m_nModelIndex, 0x18) &&
        !Core::Hook::CallMethod<bool>(0x8F19A0, reinterpret_cast<void*>(Core::Rebase(0x1354040))))
    {
        return false;
    }

    Core::StaticRef<uint8>(0x11DC829) = 1;
    bool ok = false;
    if ((m_nVehFlagsF6F & 0x20) != 0)
    {
        Core::StaticRef<uint8>(0xFC7EDB) = 0;
        ok = Core::Hook::Call<bool>(0x83B500, this, 0, 0);
        Core::StaticRef<uint8>(0xFC7EDB) = 1;
    }
    else if (Core::Hook::Call<bool>(0x462F90))
    {
        Core::StaticRef<uint8>(0xFC7EDB) = 0;
        ok = Core::Hook::CallMethod<void*>(0x9C5B50, this, 1) != nullptr;
        Core::StaticRef<uint8>(0xFC7EDB) = 1;
    }
    else
    {
        ok = Core::Hook::Call<bool>(0x83B260, this, 0, 0x63, 1);
    }

    Core::StaticRef<uint8>(0x11DC829) = 0;
    if (!ok)
        return false;

    for (int32 i = 0; i <= 3; ++i)
    {
        CPed* p = GetPassenger(i);
        if (p)
            p->m_nUnk63 = 0;
    }

    if (m_pDriver)
    {
        void* p = Core::Hook::Call<void*>(0xA26FC0, this);
        if (p && self[0xEF0] != 0)
            Core::Hook::CallMethod<void>(0xA33830, p, self + 0xEF4);
    }

    self[0xEF0] = 0;
    Core::Hook::CallMethod<void>(0xA338F0, self + 0xEF4);
    m_nVehFlagsF6D &= ~0x40u;
    return true;
}

void __thiscall CVehicle::Unk_9C93B0(void** outArr, int32* pCount, int32 maxCount, void* pExtra)
{
    Core::Hook::CallStdcall<void>(0x9E7140, outArr, pCount, maxCount, pExtra);

    using Fn46 = void(__thiscall*)(CPed*, void**, int32*, int32, void*);
    if (m_pDriver && *pCount < maxCount)
    {
        outArr[(*pCount)++] = m_pDriver;
        reinterpret_cast<Fn46>((*reinterpret_cast<void***>(m_pDriver))[46])(m_pDriver, outArr, pCount, maxCount, nullptr);
    }

    for (int32 i = 0; i <= 3; ++i)
    {
        CPed* p = GetPassenger(i);
        if (p && p != m_pDriver && *pCount < maxCount)
        {
            outArr[(*pCount)++] = p;
            reinterpret_cast<Fn46>((*reinterpret_cast<void***>(p))[46])(p, outArr, pCount, maxCount, nullptr);
        }
    }
}

bool __thiscall CVehicle::Unk_9C9BA0()
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint8* p38 = static_cast<const uint8*>(m_pPhysicsInst);
    if (!p38 || *reinterpret_cast<const uint16*>(p38 + 8) == 0xFFFF || (m_nFlags & 0x1Cu) != 0)
        return true;

    bool ok = true;
    if ((m_nVehFlagsF67 & 0x40) != 0)
    {
        ok = false;
    }
    else
    {
        int32 s = *reinterpret_cast<const int32*>(self + 0x144);
        if (s == 1)
        {
            ok = false;
        }
        else if (s == 2)
        {
            if (m_pMatrix->pos.z > Core::StaticRef<float>(0xD81E78))
                ok = false;
        }
        else if (reinterpret_cast<bool(__thiscall*)(CVehicle*)>(m_vftable[85])(this) ||
                 (*reinterpret_cast<const uint32*>(self + 0x1F4) & 8u) != 0)
        {
            ok = false;
        }
    }

    int32 cntFDC = static_cast<int32>(m_nVehUnkFDC);
    for (int32 i = 0; i < cntFDC; ++i)
    {
        if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(m_nVehUnkFD8) + i * 0x34 + 0x13) & 1u) != 0)
            ok = false;
    }

    int32 cntFD4 = static_cast<int32>(m_nVehUnkFD4);
    for (int32 i = 0; i < cntFD4; ++i)
    {
        float v = *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(m_nVehUnkFD0) + i * 0x170 + 0x160);
        if (v > Core::StaticRef<float>(0xD82150) && v < Core::StaticRef<float>(0xDAF864))
            ok = false;
    }

    if (ok && m_pDriver && m_pDriver->m_nPedUnk218 == 0 && m_pDriver->m_nPedUnk219 != 0)
    {
        void* v24 = Core::Hook::CallMethod<void*>(0x9BE5D0, m_pDriver);
        if (Core::Hook::CallMethod<int32>(0x9C5570, this) != static_cast<int32>(m_nVehUnkFD4))
        {
            if (Core::Hook::CallMethod<double>(0x839110, v24) != 0.0 ||
                Core::Hook::Call<float>(0x8347C0, Core::Hook::CallMethod<void*>(0x839330, v24)) != 0.0f)
            {
                ok = false;
            }
        }
    }

    uint8* mgr = Core::StaticRef<uint8*>(0x1666880);
    uint32 flags = *reinterpret_cast<const uint32*>(*reinterpret_cast<const uint8* const*>(mgr + 0x70) + *reinterpret_cast<const uint16*>(p38 + 8) * 8 + 4);
    if ((flags & 3u) == 1)
    {
        if (!ok)
        {
            Core::Hook::CallMethod<void>(0xA47660, this);
            return false;
        }
        return true;
    }

    void* v27 = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (v27)
    {
        void* v28 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(v27) + 0x18);
        if (v28)
        {
            uint32 val = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(v28) + 0x18);
            if ((val == 0 || val == 2) && !ok)
                Core::Hook::CallMethod<void>(0x69AE50, v28, 1);
        }
    }

    return false;
}

void __thiscall CVehicle::Unk_9C9E60(uint8* pArg, bool bFlag)
{
    float vec[4] = {};
    float* v = reinterpret_cast<float*(__thiscall*)(CVehicle*, float*)>(m_vftable[59])(this, vec);
    const CVector& up = m_pMatrix->up;
    float dot = v[2] * up.z + v[1] * up.y + v[0] * up.x;

    float f1 = Core::Hook::Call<float>(0x8348B0, pArg + 0x2918);
    float f2 = Core::Hook::Call<float>(0x8348B0, pArg + 0x2928);
    float diff = f1 - f2;

    if (bFlag)
    {
        m_fVehUnk10CC = 1.0f;
        m_fVehUnk10C8 = 0.0f;
        return;
    }

    if (((m_nVehFlagsF64 & 8) == 0 && (m_nVehFlagsF67 & 8) != 0) ||
        (m_pDriver && Core::Hook::CallMethod<bool>(0x426910, static_cast<uint8*>(m_pDriver->m_pPedIntelligence) + 0x2E0, 0x145, 0)))
    {
        m_fVehUnk10C8 = 0.0f;
        m_fVehUnk10CC = 0.0f;
        return;
    }

    float absDot = (dot >= 0.0f) ? dot : (Core::StaticRef<float>(0xEBDED0) - dot);
    float thr = Core::StaticRef<float>(0xDCC5F8);
    if (absDot < Core::StaticRef<float>(0xDCE03C))
    {
        if (f1 > thr && f2 > thr)
        {
            m_fVehUnk10C8 = f1;
            m_fVehUnk10CC = f2;
            return;
        }
        m_fVehUnk10CC = 0.0f;
        m_fVehUnk10C8 = diff;
        return;
    }

    if (f1 > thr && f2 > thr)
    {
        m_fVehUnk10C8 = f1;
        m_fVehUnk10CC = f2;
        return;
    }

    if (dot >= 0.0f)
    {
        if (diff < 0.0f)
        {
            m_fVehUnk10C8 = 0.0f;
            m_fVehUnk10CC = Core::StaticRef<float>(0xEBDED0) - diff;
            return;
        }
        m_fVehUnk10CC = 0.0f;
        m_fVehUnk10C8 = diff;
        return;
    }

    if (diff < 0.0f || (m_fVehUnk10C8 > Core::StaticRef<float>(0xDCE03C) && dot > Core::StaticRef<float>(0xEBE374)))
    {
        m_fVehUnk10CC = 0.0f;
        m_fVehUnk10C8 = diff;
        return;
    }

    m_fVehUnk10C8 = 0.0f;
    m_fVehUnk10CC = diff;
}

void CVehicle::ApplyPassengerImpulse(CPed* pPed)
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint8* p6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (p6C && p6C[0xE] != 0)
        return;

    const uint8* pPed6C = reinterpret_cast<const uint8*>(pPed->m_nUnk6C);
    if (pPed6C && pPed6C[0xE] != 0)
        return;

    float vec1[3];
    float vec2[3];
    if (*reinterpret_cast<const int32*>(self + 0x1350) == 1)
    {
        float s1 = Core::StaticRef<float>(0xEBE0D8);
        vec1[0] = m_pMatrix->up.x * s1;
        vec1[1] = m_pMatrix->up.y * s1;
        vec1[2] = m_pMatrix->up.z * s1;

        void* pInfo = Core::Hook::CallMethod<void*>(0x9E86F0, pPed);
        float s2 = static_cast<float>(reinterpret_cast<double(__thiscall*)(void*)>((*reinterpret_cast<void***>(pInfo))[9])(pInfo)) * Core::StaticRef<float>(0xEBDF88);
        vec2[0] = s2 * m_pMatrix->at.x;
        vec2[1] = s2 * m_pMatrix->at.y;
        vec2[2] = s2 * m_pMatrix->at.z;
    }
    else
    {
        float dx = pPed->m_pMatrix->pos.x - m_pMatrix->pos.x;
        float dy = pPed->m_pMatrix->pos.y - m_pMatrix->pos.y;
        if (static_cast<float>(reinterpret_cast<double(__thiscall*)(CVehicle*)>(m_vftable[22])(this)) <= (dx * dx + dy * dy))
            return;

        vec2[0] = dx;
        vec2[1] = dy;
        vec2[2] = 0.0f;

        void* pInfo = Core::Hook::CallMethod<void*>(0x9E86F0, pPed);
        vec1[0] = 0.0f;
        vec1[1] = 0.0f;
        vec1[2] = static_cast<float>(reinterpret_cast<double(__thiscall*)(void*)>((*reinterpret_cast<void***>(pInfo))[9])(pInfo)) * Core::StaticRef<float>(0xEBE0E0);
    }

    Core::Hook::CallMethod<void>(0xA47B80, this, vec1, vec2, 0);
}

bool __thiscall CVehicle::Unk_9CA080(CPed* pPed)
{
    bool added = false;
    int32 maxPass = static_cast<int32>(m_nMaxPassengers);
    if (maxPass > 0)
    {
        for (int32 i = 0; i < maxPass; ++i)
        {
            if (!added && m_pPassengers[i] == nullptr)
            {
                m_pPassengers[i] = pPed;
                pPed->RegisterReference(reinterpret_cast<CEntity**>(&m_pPassengers[i]));
                added = true;
                break;
            }
        }
    }

    ApplyPassengerImpulse(pPed);
    return added;
}

bool __thiscall CVehicle::Unk_9CA250(CPed* pPed, uint32 seatIdx)
{
    if (reinterpret_cast<bool(__thiscall*)(CVehicle*)>(m_vftable[74])(this) &&
        pPed->m_nPedUnk218 == 0 &&
        pPed->m_nPedUnk219 != 0 &&
        !reinterpret_cast<bool(__thiscall*)(CVehicle*, int32, int32, int32)>(m_vftable[77])(this, 1, 0, 0))
    {
        return false;
    }

    bool added = false;
    if (seatIdx < static_cast<uint32>(m_nMaxPassengers) && m_pPassengers[seatIdx] == nullptr)
    {
        m_pPassengers[seatIdx] = pPed;
        pPed->RegisterReference(reinterpret_cast<CEntity**>(&m_pPassengers[seatIdx]));
        added = true;
    }

    ApplyPassengerImpulse(pPed);
    return added;
}

void __thiscall CVehicle::Unk_9CA440(void* a2, int32 a3)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if ((m_nVehFlagsF6D & 0x40) != 0 && !reinterpret_cast<bool(__thiscall*)(CVehicle*)>(m_vftable[83])(this))
    {
        self[0xEF0] = 0;
        Core::Hook::CallMethod<void>(0xA338F0, self + 0xEF4);
        m_nVehFlagsF6D &= ~0x40u;
    }

    for (int32 i = 0; i <= 3; ++i)
    {
        CPed* pPed = GetPassenger(i);
        if (!pPed)
            continue;

        uint8* p6C = reinterpret_cast<uint8*>(pPed->m_nUnk6C);
        if (p6C && p6C[0xE] != 0)
            continue;

        if (*reinterpret_cast<const int32*>(self + 0x1354) == 4 ||
            reinterpret_cast<bool(__thiscall*)(CPed*)>(pPed->m_vftable[74])(pPed) ||
            pPed->m_nDummyConvertState == 2 ||
            static_cast<float>(reinterpret_cast<double(__thiscall*)(CPed*)>(pPed->m_vftable[63])(pPed)) > Core::StaticRef<float>(0xD8D70C) ||
            Core::Hook::CallMethod<bool>(0x426910, static_cast<uint8*>(pPed->m_pPedIntelligence) + 0x2E0, 0x2E2, 0) ||
            Core::Hook::Call<double>(0x438960, 0.0f, 1.0f) >= Core::StaticRef<float>(0xD50964))
        {
            uint8 dmgInfo[0x1C] = {};
            uint8 eventBuf[0x24] = {};
            uint8 resp[0x18] = {};
            Core::Hook::CallMethod<void>(0xAB9470, dmgInfo, a2, Core::StaticRef<float>(0xD81CE0), a3, 0, 0);
            Core::Hook::CallMethod<void>(0xAB9A00, eventBuf, a2, Core::StaticRef<uint32>(0x11DDE74), a3);
            Core::Hook::CallMethod<void>(0xABB170, dmgInfo, pPed, resp);
            if (Core::Hook::CallMethod<bool>(0x9164C0, eventBuf, pPed))
                Core::Hook::CallMethod<void>(0xAA80D0, static_cast<uint8*>(pPed->m_pPedIntelligence) + 0x84, eventBuf, 0, 1);
            Core::Hook::CallMethod<void>(0xAB9A80, eventBuf);
            Core::Hook::CallMethod<void>(0xC28820, dmgInfo);
        }
        else
        {
            uint8 fireEvt[0x58] = {};
            Core::Hook::CallMethod<void>(0x919200, fireEvt, pPed->m_pMyVehicle);
            if (Core::Hook::CallMethod<bool>(0x9164C0, fireEvt, pPed))
                Core::Hook::CallMethod<void>(0xAA80D0, static_cast<uint8*>(pPed->m_pPedIntelligence) + 0x84, fireEvt, 0, 1);
            Core::Hook::CallMethod<void>(0x919250, fireEvt);
        }
    }
}

bool __thiscall CVehicle::Unk_9CA600()
{
    return m_pMatrix->at.z <= Core::StaticRef<float>(0xEBE708);
}

bool __thiscall CVehicle::Unk_9CA620()
{
    float v = m_pMatrix->right.z;
    return v >= Core::StaticRef<float>(0xDCA2A8) || v <= Core::StaticRef<float>(0xEBE21C);
}

bool __thiscall CVehicle::Unk_9CA650(CPed* pPed)
{
    uint8* p6C = reinterpret_cast<uint8*>(pPed->m_nUnk6C);
    if (p6C && p6C[0xE] != 0)
        return true;

    int32 st = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(this) + 0x1320);
    if (Core::Hook::Call<bool>(0x462F90) && pPed->m_nPedUnk219 != 0 && st >= 9)
    {
        if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(pPed->m_pPlayerInfo) + 0x574) != st - 9)
            return false;
    }

    if (st == 2 || st == 5 || st == 7 || st == 8)
        return false;

    if (st == 4)
    {
        uint8* p = Core::Hook::CallMethod<uint8*>(0x89DD80, pPed);
        if (pPed->m_nPedUnk219 == 0 && p)
        {
            CPed* pSub = Core::Hook::CallMethod<CPed*>(0xA66B40, p + 8);
            if (pSub && pSub->m_nPedUnk219 != 0)
                return true;
        }
        return false;
    }

    return pPed->m_nPedUnk219 == 0 || st != 3;
}

bool __thiscall CVehicle::Unk_9CA710(bool bSkipVelCheck)
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    if (m_nModelIndex == Core::StaticRef<int32>(0x123097C))
        return false;

    if (*reinterpret_cast<const int32*>(self + 0x1350) == 2)
        return true;

    float upZ = m_pMatrix->at.z;
    float vec[4] = {};
    using GetVelFn = float*(__thiscall*)(CVehicle*, float*);

    if (upZ <= Core::StaticRef<float>(0xDC9290) && upZ >= Core::StaticRef<float>(0xEBE0D8))
    {
        float* v = reinterpret_cast<GetVelFn>(m_vftable[59])(this, vec);
        float absZ = (v[2] >= 0.0f) ? v[2] : (Core::StaticRef<float>(0xEBDED0) - v[2]);
        if (absZ > Core::StaticRef<float>(0xF33A50))
            return false;

        v = reinterpret_cast<GetVelFn>(m_vftable[59])(this, vec);
        if (std::sqrtf(v[0] * v[0] + v[1] * v[1]) > Core::StaticRef<float>(0xF33A4C))
            return false;

        float* a = Core::Hook::CallMethod<float*>(0xA478E0, this, vec);
        return (a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) <= Core::StaticRef<float>(0xD82320);
    }

    if (!bSkipVelCheck)
    {
        float* v = reinterpret_cast<GetVelFn>(m_vftable[59])(this, vec);
        if (std::sqrtf(v[0] * v[0] + v[1] * v[1]) > Core::StaticRef<float>(0xF33A4C))
            return false;

        v = reinterpret_cast<GetVelFn>(m_vftable[59])(this, vec);
        if (Core::Hook::Call<float>(0x424C00, v[2]) > Core::StaticRef<float>(0xF33A50))
            return false;
    }

    float* a = Core::Hook::CallMethod<float*>(0xA478E0, this, vec);
    return (a[1] * a[1] + a[2] * a[2] + a[0] * a[0]) <= Core::StaticRef<float>(0xD82320);
}

bool __thiscall CVehicle::Unk_9CA8E0(CPed* pPed)
{
    float vec[4] = {};
    float* v = reinterpret_cast<float*(__thiscall*)(CVehicle*, float*)>(m_vftable[59])(this, vec);
    float spdSq = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

    const uint8* self = reinterpret_cast<const uint8*>(this);
    int32 model = m_nModelIndex;
    if (model == Core::StaticRef<int32>(0x123097C))
        return false;

    float thr = (*reinterpret_cast<const int32*>(self + 0x1354) == 1)
        ? Core::StaticRef<float>(0xF33AA0)
        : Core::StaticRef<float>(0xD8F340);
    if (spdSq < thr)
        return false;

    CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[model]);
    if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(modelInfo) + 0xC4) == 0x57)
        return false;

    if ((modelInfo->m_nVehicleFlags & 0x20u) != 0)
    {
        int32 seat = GetPedSeat(pPed);
        if (seat == 1 || seat == 3)
            return false;
    }

    return true;
}

bool __thiscall CVehicle::Unk_9CA9B0()
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint8* p6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (p6C && p6C[0xE] != 0)
        return false;

    if (*reinterpret_cast<const int32*>(self + 0x1354) != 0 || m_nVehUnkF98 != -1)
        return false;

    m_nVehUnkF98 = 15000;
    return true;
}

bool __cdecl CVehicle::Unk_9CA9E0(int32 modelIdx)
{
    if (modelIdx == Core::StaticRef<int32>(0x1230898) ||
        modelIdx == Core::StaticRef<int32>(0x12309DC) ||
        modelIdx == Core::StaticRef<int32>(0x12302A4) ||
        modelIdx == Core::StaticRef<int32>(0x1230A0C) ||
        modelIdx == Core::StaticRef<int32>(0x12302F8) ||
        modelIdx == Core::StaticRef<int32>(0x12307E4) ||
        modelIdx == Core::StaticRef<int32>(0x12303A0))
    {
        return true;
    }

    return Core::Hook::Call<bool>(0x9C5F50, modelIdx);
}

void __thiscall CVehicle::Unk_9CAA30()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if ((m_nFlags2 & 0x7C00u) != 0xC00u)
    {
        const uint8* p6C = reinterpret_cast<const uint8*>(m_nUnk6C);
        if (!p6C || p6C[0xE] == 0)
        {
            float cur = static_cast<float>(reinterpret_cast<double(__thiscall*)(CVehicle*)>(m_vftable[63])(this));
            float limit = Core::StaticRef<float>(0xD582A0);
            float target = (cur > limit) ? cur : limit;
            reinterpret_cast<void(__thiscall*)(CVehicle*, float, int32)>(m_vftable[61])(this, target, 0);
        }
    }

    Core::Hook::CallMethod<void>(0xA292B0, reinterpret_cast<void*>(Core::Rebase(0x164B030)), this);

    float f10FC = *reinterpret_cast<const float*>(self + 0x10FC);
    float f1128 = *reinterpret_cast<const float*>(self + 0x1128);
    if ((f10FC < 0.0f && f10FC > Core::StaticRef<float>(0xD6A9A4)) ||
        (f1128 > Core::StaticRef<float>(0xEBE00C) && f1128 < 0.0f))
    {
        *reinterpret_cast<float*>(self + 0x1128) = 1.0f;
        *reinterpret_cast<float*>(self + 0x10FC) = Core::StaticRef<float>(0xD6A9A4);
    }

    int32 cnt = static_cast<int32>(m_nVehUnkFD4);
    for (int32 i = 0; i < cnt; ++i)
        Core::Hook::CallMethod<void>(0xB42E30, reinterpret_cast<uint8*>(m_nVehUnkFD0) + i * 0x170);
}

void __thiscall CVehicle::Unk_9CC990(CPed* pPed, int32)
{
    bool cond = Core::Hook::CallMethod<bool>(0x9C6270, this, pPed);
    if (!cond)
    {
        uint8* p6C = reinterpret_cast<uint8*>(pPed->m_nUnk6C);
        uint8* p2E0 = static_cast<uint8*>(pPed->m_pPedIntelligence) + 0x2E0;
        if (p6C && p6C[0xE] != 0 && p2E0 != nullptr)
            cond = Core::Hook::CallMethod<bool>(0x426910, p2E0, 0x2C5, 0);
    }

    if (cond)
    {
        CVehicleModelInfo* modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
        int32 id = 0x175;
        int32 v7 = Core::Hook::Call<int32>(0x8A6E40, *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(modelInfo) + 0xC4), &id, pPed, this, 0, 0, 0, 1);
        void* p78 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(pPed) + 0x78);
        if (Core::Hook::CallMethod<int32>(0x9EBE90, p78, v7, id) == 0)
            Core::Hook::CallMethod<void>(0x9EFEB0, p78, v7, id, Core::StaticRef<float>(0xDCB2A0), -1);
    }
}

bool __thiscall CVehicle::Unk_9D4A50()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x9D3600, this, 0);
    *reinterpret_cast<uint32*>(self + 0xE88) = Core::StaticRef<uint32>(0x11DDE74);
    Core::Hook::CallMethod<void>(0xA338F0, self + 0xEF4);
    m_nVehFlagsF6D |= 0x40u;
    self[0xF1C] = 0;
    self[0xF1B] = 10;
    self[0xF1A] = 1;
    self[0xEF0] = 1;
    return true;
}

CVehicle* __thiscall CVehicle::ScalarDeletingDtor(uint8 flags)
{
    Core::Hook::CallMethod<void>(0x9D43E0, this);
    if ((flags & 1) != 0)
        Core::Hook::CallMethod<void>(0x8BE6D0, Core::StaticRef<void*>(0x1619240), this);
    return this;
}

void __thiscall CVehicle::Unk_9D5F00()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[m_nModelIndex];
    if (modelInfo->m_pFragType != nullptr)
    {
        m_nFlags |= 0x4000000u;
        void* pPool = Core::Hook::CallMethod<void*>(0x96D520, Core::StaticRef<void*>(0x170665C));
        uint8* pEnt = pPool
            ? Core::Hook::CallMethod<uint8*>(0xC711F0, pPool, 0xE, modelInfo->m_pFragType, m_pMatrix, 0)
            : nullptr;
        *reinterpret_cast<void**>(self + 0xE14) = pEnt;
        Core::Hook::CallMethod<void>(0x686AD0, pEnt, 4, 1);
        *reinterpret_cast<uint32*>(pEnt + 0x60) |= 8u;
        Core::Hook::CallMethod<void>(0x9E6FA0, this, pEnt, 1);
        reinterpret_cast<void(__thiscall*)(uint8*, int32)>((*reinterpret_cast<void***>(pEnt))[40])(pEnt, 0);
        reinterpret_cast<void(__thiscall*)(uint8*)>((*reinterpret_cast<void***>(pEnt))[28])(pEnt);
    }

    uint32 flag = (Core::StaticRef<float>(0x16394E4) >= Core::StaticRef<float>(0xDCB2A8)) ? 0x100u : 0u;
    Core::Hook::CallMethod<void>(0x9D2A00, this, flag);
    Core::Hook::CallMethod<void>(0x9D4AC0, this);
}

void __thiscall CVehicle::Unk_9D5FC0()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* p6C = reinterpret_cast<uint8*>(m_nUnk6C);
    if (p6C && p6C[0xE] != 0)
    {
        if ((m_nFlags2 & 0x7C00u) == 0xC00u)
            return;
    }
    else
    {
        reinterpret_cast<void(__thiscall*)(CVehicle*, float)>(m_vftable[60])(this, Core::StaticRef<float>(0xDAF864));
    }

    Core::Hook::CallMethod<void>(0x8B3580, m_vehSubObj1120);
    Core::Hook::CallMethod<void>(0x8AEE60, m_vehSubObj1120);
    Core::Hook::CallMethod<void>(0x9D4AC0, this);
    reinterpret_cast<void(__thiscall*)(CVehicle*)>(m_vftable[72])(this);

    *reinterpret_cast<uint32*>(self + 0x1AC) = 0;
    *reinterpret_cast<uint32*>(self + 0x1B0) = 0;
    *reinterpret_cast<uint32*>(self + 0x1B4) = 0;
    *reinterpret_cast<uint32*>(self + 0x1B8) = 0;

    Core::Hook::CallMethod<void>(0x910470, reinterpret_cast<void*>(Core::Rebase(0x13DD760)), this, 0);
    Core::Hook::CallMethod<void>(0xAEAD50, m_vehSubObj260);

    uint8* p34 = static_cast<uint8*>(m_pDrawHandler);
    if (p34)
    {
        uint8* p4 = *reinterpret_cast<uint8**>(p34 + 4);
        if (p4)
            p4[0xCC] = 0;
    }

    if ((m_nFlags2 & 0x7C00u) == 0xC00u)
        *reinterpret_cast<uint32*>(self + 0x118) &= ~0x10000u;
}

void __thiscall CVehicle::Unk_9D6170(void* a2, float angle, int32)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (angle >= Core::StaticRef<float>(0xEBDFFC) && angle <= Core::StaticRef<float>(0xEBDEE4))
    {
        if (m_pMatrix)
        {
            float c = std::cosf(angle);
            float s = std::sinf(angle);
            m_pMatrix->right = { c, s, 0.0f };
            m_pMatrix->up = { -s, c, 0.0f };
            m_pMatrix->at = { 0.0f, 0.0f, 1.0f };
        }
        else
        {
            m_fPad1C = angle;
        }
    }

    reinterpret_cast<void(__thiscall*)(CVehicle*, void*, int32, int32)>(m_vftable[2])(this, a2, 1, 0);
    Core::Hook::CallMethod<void>(0x9E1F70, this);
    reinterpret_cast<void(__thiscall*)(CVehicle*, int32)>(m_vftable[45])(this, 1);
    reinterpret_cast<void(__thiscall*)(CVehicle*)>(m_vftable[100])(this);
    Core::Hook::CallMethod<void>(0xC7E570, m_vehSubObj10E0);
    Core::Hook::CallMethod<void>(0x8AEE60, m_vehSubObj1120);

    uint8* pSub = Core::Hook::CallMethod<uint8*>(0x9E7080, this);
    if (pSub)
    {
        reinterpret_cast<void(__thiscall*)(uint8*)>((*reinterpret_cast<void***>(pSub))[4])(pSub);
        void* p18 = *reinterpret_cast<void**>(pSub + 0x18);
        if (p18)
            Core::Hook::CallMethod<void>(0x69AE50, p18, 1);
    }

    *reinterpret_cast<uint32*>(self + 0x118) &= ~0x80000u;
    Core::Hook::CallMethod<void>(0x9D6090, this, 1);
    Core::Hook::CallMethod<void>(0x7C8C00, reinterpret_cast<void*>(Core::Rebase(0x110A890)), this);
}
