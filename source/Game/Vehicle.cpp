#include "StdInc.h"
#include "Game/Vehicle.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CVehicle::InjectHooks()
{
    Core::Hook::Install(0x9C7C40, &CVehicle::Ctor);
    Core::Hook::Install(0x9C6D20, &CVehicle::GetPassenger);
    Core::Hook::Install(0x9C6D70, &CVehicle::SeatToPassengerIndex);
    Core::Hook::Install(0x9C6DB0, &CVehicle::PassengerIndexToSeat);

    Core::Log::Info("CVehicle hooks installed (4 functions)");
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
