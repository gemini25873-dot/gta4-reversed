#include "StdInc.h"
#include "Game/Garages.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CGarages::InjectHooks()
{
    Core::Hook::Install(0x8FF480, &CGarage::PlayerArrestedOrDied);
    Core::Hook::Install(0x8FF4D0, &CGarage::IsPointInsideGarage);
    Core::Hook::Install(0x8FEF00, &CSaveGarage::CopyGarageIntoSaveGarage);
    Core::Hook::Install(0x8FEF90, &CSaveGarage::CopyGarageOutOfSaveGarage);
    Core::Hook::Install(0x901210, &CGarages::Init);
    Core::Hook::Install(0x8FF020, &CGarages::Shutdown);
    Core::Hook::Install(0x8FF080, &CGarages::Reset);
    Core::Hook::Install(0x8FF470, &CGarages::CameraShouldBeOutside);
    Core::Hook::Install(0x8FF600, &CGarages::TidyUpGaragesAfterSave);
    Core::Hook::Install(0x8FF630, &CGarages::IsPointInAnyHideOutGarage);
    Core::Hook::Install(0x8FF670, &CGarages::IsPointInAnyGarage);
    Core::Hook::Install(0x900A90, &CGarages::PlayerArrestedOrDied);
    Core::Hook::Install(0x900FE0, &CGarages::CloseHideOutGaragesBeforeSave);

    Core::Log::Info("CGarages hooks installed (13 functions)");
}

// 0x8FF480
void __thiscall CGarage::PlayerArrestedOrDied()
{
    switch (m_nType)
    {
    case 1:
    case 3:
    case 4:
    case 5:
        if (static_cast<uint8>(m_nState - 1) <= 2u)
        {
            m_nState = 2;
        }
        break;
    case 2:
        if (m_nState == 0 || m_nState == 2 || m_nState == 3)
        {
            m_nState = 3;
        }
        break;
    default:
        break;
    }
}

// 0x8FF4D0
bool __thiscall CGarage::IsPointInsideGarage(const CVector* point) const
{
    if (point->z < m_fBaseZ || point->z > m_fCeilingZ)
    {
        return false;
    }

    const float dx = point->x - m_fBaseX;
    const float dy = point->y - m_fBaseY;

    const float proj1 = m_fDelta1X * dx + m_fDelta1Y * dy;
    if (proj1 < 0.0f || proj1 > m_fDelta1Length)
    {
        return false;
    }

    const float proj2 = m_fDelta2X * dx + m_fDelta2Y * dy;
    if (proj2 < 0.0f || proj2 > m_fDelta2Length)
    {
        return false;
    }

    return true;
}

// 0x8FEF00
void __thiscall CSaveGarage::CopyGarageIntoSaveGarage(const CGarage* garage)
{
    m_nType = garage->m_nType;
    m_nState = garage->m_nState;
    m_nFlags = garage->m_nFlags;
    m_fBaseX = garage->m_fBaseX;
    m_fBaseY = garage->m_fBaseY;
    m_fBaseZ = garage->m_fBaseZ;
    m_fDelta1X = garage->m_fDelta1X;
    m_fDelta1Y = garage->m_fDelta1Y;
    m_fDelta2X = garage->m_fDelta2X;
    m_fDelta2Y = garage->m_fDelta2Y;
    m_fCeilingZ = garage->m_fCeilingZ;
    m_fDelta1Length = garage->m_fDelta1Length;
    m_fDelta2Length = garage->m_fDelta2Length;
    m_fMinX = garage->m_fMinX;
    m_fMaxX = garage->m_fMaxX;
    m_fMinY = garage->m_fMinY;
    m_fMaxY = garage->m_fMaxY;
    m_nTimeOfNextEvent = garage->m_nTimeOfNextEvent;
    m_nNameHash = garage->m_nNameHash;
    m_nOriginalType = garage->m_nOriginalType;
    m_nSafehouseIndex = garage->m_nSafehouseIndex;
}

// 0x8FEF90
CGarage* __thiscall CSaveGarage::CopyGarageOutOfSaveGarage(CGarage* garage) const
{
    garage->m_nType = m_nType;
    garage->m_nState = m_nState;
    garage->m_nFlags = m_nFlags;
    garage->m_fBaseX = m_fBaseX;
    garage->m_fBaseY = m_fBaseY;
    garage->m_fBaseZ = m_fBaseZ;
    garage->m_fDelta1X = m_fDelta1X;
    garage->m_fDelta1Y = m_fDelta1Y;
    garage->m_fDelta2X = m_fDelta2X;
    garage->m_fDelta2Y = m_fDelta2Y;
    garage->m_fCeilingZ = m_fCeilingZ;
    garage->m_fDelta1Length = m_fDelta1Length;
    garage->m_fDelta2Length = m_fDelta2Length;
    garage->m_fMinX = m_fMinX;
    garage->m_fMaxX = m_fMaxX;
    garage->m_fMinY = m_fMinY;
    garage->m_fMaxY = m_fMaxY;
    garage->m_nTimeOfNextEvent = m_nTimeOfNextEvent;
    garage->m_pCarToCollect = nullptr;
    garage->m_nNameHash = m_nNameHash;
    garage->m_nOriginalType = m_nOriginalType;
    garage->m_nSafehouseIndex = m_nSafehouseIndex;
    return garage;
}

// 0x901210
int32 __cdecl CGarages::Init()
{
    return Core::Hook::CallMethod<int32>(
        0xB3CC50,
        &Core::StaticRef<uint32>(0x1692078),
        "Garages",
        reinterpret_cast<void*>(Core::Rebase(0x901110)),
        reinterpret_cast<void*>(Core::Rebase(0x901030)));
}

// 0x8FF020
void __cdecl CGarages::Shutdown()
{
    ms_nNumGarages = 0;
    CGarage* garages = GetArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        Core::Hook::CallMethod<void>(0xCE8280, garages[i].m_unk5C);
    }
}

// 0x8FF080
void __cdecl CGarages::Reset()
{
    CGarage* garages = GetArray();
    for (uint32 i = 0; i < ms_nNumGarages; ++i)
    {
        garages[i].m_nType = garages[i].m_nOriginalType;
        garages[i].m_nUnk50 = 0;
        garages[i].m_fUnk54 = 0.0f;
        garages[i].m_nUnk58 = 0;
        Core::Hook::CallMethod<void>(0xC8AAF0, garages[i].m_unk5C);
    }

    ms_nMessageEndTime = 0;
    ms_nMessageStartTime = 0;
    ms_bBombsAreFree = 0;
    ms_bRespraysAreFree = 0;
    ms_bNoResprays = 0;
    ms_bHasResprayHappened = 0;
    ms_nLastTimeHelpMessage = 0;
    ms_nLastGaragePlayerWasIn = -1;

    CStoredCar* storedCars = GetStoredCarsArray();
    for (int32 i = 0; i < MAX_NUM_STORED_CARS; ++i)
    {
        Core::Hook::CallMethod<void>(0xC8A5E0, &storedCars[i]);
    }
}

// 0x8FF470
int32 __cdecl CGarages::CameraShouldBeOutside()
{
    return ms_bCamShouldBeOutside;
}

// 0x8FF600
void __cdecl CGarages::TidyUpGaragesAfterSave()
{
    CGarage* garages = GetArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        if (garages[i].m_nType == 5 && (garages[i].m_nFlags & 0x40) != 0)
        {
            garages[i].m_nState = 1;
        }
    }
}

// 0x8FF630
bool __cdecl CGarages::IsPointInAnyHideOutGarage(const CVector* point)
{
    CGarage* garages = GetArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        if (garages[i].m_nType == 5 && garages[i].IsPointInsideGarage(point))
        {
            return true;
        }
    }
    return false;
}

// 0x8FF670
bool __cdecl CGarages::IsPointInAnyGarage(const CVector* point)
{
    CGarage* garages = GetArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        if (garages[i].m_nType != 0 && garages[i].IsPointInsideGarage(point))
        {
            return true;
        }
    }
    return false;
}

// 0x900A90
void __cdecl CGarages::PlayerArrestedOrDied()
{
    CGarage* garages = GetArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        if (garages[i].m_nType != 0)
        {
            garages[i].PlayerArrestedOrDied();
        }
    }
    ms_nMessageEndTime = 0;
    ms_nMessageStartTime = 0;
}

// 0x900FE0
void __cdecl CGarages::CloseHideOutGaragesBeforeSave(bool unk)
{
    CGarage* garages = GetArray();
    CStoredCar* storedCars = GetStoredCarsArray();
    for (int32 i = 0; i < MAX_NUM_GARAGES; ++i)
    {
        if (garages[i].m_nType == 5)
        {
            if (garages[i].m_nState != 0)
            {
                Core::Hook::CallMethod<void>(
                    0x900DF0,
                    &garages[i],
                    &storedCars[garages[i].m_nSafehouseIndex * 2],
                    static_cast<uint8>(unk ? 1 : 0));
                garages[i].m_nFlags |= 0x40;
                garages[i].m_nState = 0;
            }
            else
            {
                garages[i].m_nFlags &= ~0x40u;
            }
        }
    }
}
