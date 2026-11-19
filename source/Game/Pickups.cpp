#include "StdInc.h"
#include "Game/Pickups.h"
#include "Core/Hook.h"
#include "Core/Log.h"

#include <cmath>

void CPickups::InjectHooks()
{
    Core::Hook::Install(0x933220, &CPickup::DestroyObject);
    Core::Hook::Install(0x9350C0, &CPickup::Remove);
    Core::Hook::Install(0x9357E0, &CPickups::Init);
    Core::Hook::Install(0x935800, &CPickups::RemoveAllPickups);
    Core::Hook::Install(0x9337F0, &CPickups::GetActualPickupIndex);
    Core::Hook::Install(0x934A20, &CPickups::GetNewUniquePickupIndex);
    Core::Hook::Install(0x9352E0, &CPickups::RemoveMoneyAndWeaponDropsInArea);
    Core::Hook::Install(0x9355C0, &CPickups::RemoveTemporaryPickups);
    Core::Hook::Install(0x9355F0, &CPickups::RemovePickupsOfType);
    Core::Hook::Install(0x935820, &CPickups::RemovePickup);

    Core::Log::Info("CPickups hooks installed (10 functions)");
}

// 0x933220
void __thiscall CPickup::DestroyObject()
{
    if (!m_pObject)
    {
        return;
    }

    const int16 modelIdx = *reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(m_pObject) + 46);
    if (modelIdx == Core::StaticRef<int32>(0x1230214) || modelIdx == Core::StaticRef<int32>(0x1230220))
    {
        Core::Hook::CallMethod<void>(0x9E2220, m_pObject);
    }

    if (m_pObject)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pObject, &m_pObject);
    }

    Core::Hook::CallMethod<void>(0x8DFBC0, m_pObject);
    void** vtable = *reinterpret_cast<void***>(m_pObject);
    using Fn48 = void(__thiscall*)(CObject*, uint32);
    reinterpret_cast<Fn48>(vtable[12])(m_pObject, 0);

    m_pObject = nullptr;
    m_pUnk08 = nullptr;
}

// 0x9350C0
uint8 __thiscall CPickup::Remove()
{
    Core::Log::InfoOnce("pickup_remove", "CPickup::Remove executed");
    const int32 index = static_cast<int32>(this - CPickups::GetArray());

    if (Core::Hook::Call<char>(0x80E600, m_nBlipIndex))
    {
        Core::Hook::Call<void>(0x80E620, m_nBlipIndex, 1);
        m_nBlipIndex = -1;
    }

    DestroyObject();
    uint8 result = Core::Hook::CallMethod<uint8>(0x933CE0, this, 0);

    if (m_nType != 23)
    {
        result = Core::Hook::Call<uint8>(0x462F90);
        if (result)
        {
            void* netMgr = Core::StaticRef<void*>(0x18A8300);
            int32 mgr = Core::Hook::CallMethod<int32>(0x4930F0, netMgr);
            if (mgr && index >= 1400)
            {
                mgr = Core::Hook::CallMethod<int32>(0x4930F0, netMgr);
                void** vtable = *reinterpret_cast<void***>(mgr);
                using Fn52 = uint8(__thiscall*)(int32, int32);
                result = reinterpret_cast<Fn52>(vtable[13])(mgr, index - 1400);
            }
        }
    }

    return result;
}

// 0x9357E0
int32 __cdecl CPickups::Init()
{
    Core::Log::InfoOnce("pickups_init", "CPickups::Init executed");
    return Core::Hook::CallMethod<int32>(
        0xB3CC50,
        &Core::StaticRef<uint32>(0x1692078),
        "Pickups",
        reinterpret_cast<void*>(Core::Rebase(0x934E30)),
        reinterpret_cast<void*>(Core::Rebase(0x935630)));
}

// 0x935800
uint8 __cdecl CPickups::RemoveAllPickups()
{
    CPickup* pickups = GetArray();
    uint8 result = 0;
    for (int32 i = 0; i < MAX_NUM_PICKUPS; ++i)
    {
        if (pickups[i].m_nType != 0)
        {
            result = pickups[i].Remove();
        }
    }
    return result;
}

// 0x9337F0
int32 __cdecl CPickups::GetActualPickupIndex(int32 uniqueIndex)
{
    if (uniqueIndex == -1)
    {
        return -1;
    }

    const uint16 slot = static_cast<uint16>(uniqueIndex);
    const uint16 ref = static_cast<uint16>(static_cast<uint32>(uniqueIndex) >> 16);
    if (ref != GetArray()[slot].m_nReferenceIndex)
    {
        return -1;
    }
    return static_cast<int32>(slot);
}

// 0x934A20
int32 __cdecl CPickups::GetNewUniquePickupIndex(int32 index)
{
    CPickup& pickup = GetArray()[index];
    if (pickup.m_nReferenceIndex >= 0xFFFEu)
    {
        pickup.m_nReferenceIndex = 1;
    }
    else
    {
        ++pickup.m_nReferenceIndex;
    }
    return index | (static_cast<int32>(pickup.m_nReferenceIndex) << 16);
}

// 0x9352E0
uint8 __cdecl CPickups::RemoveMoneyAndWeaponDropsInArea(const CVector* pos, float radius)
{
    CPickup* pickups = GetArray();
    uint8 result = 0;
    for (int32 i = 0; i < MAX_NUM_PICKUPS; ++i)
    {
        const uint8 type = pickups[i].m_nType;
        result = type;
        if (type == 4 || type == 5 || type == 8)
        {
            const float dx = pickups[i].m_vecPos.x - pos->x;
            const float dy = pickups[i].m_vecPos.y - pos->y;
            const float dz = pickups[i].m_vecPos.z - pos->z;
            if (radius > std::sqrt(dx * dx + dy * dy + dz * dz))
            {
                result = pickups[i].Remove();
            }
        }
    }
    return result;
}

// 0x9355C0
uint8 __cdecl CPickups::RemoveTemporaryPickups()
{
    CPickup* pickups = GetArray();
    uint8 result = 0;
    for (int32 i = 0; i < MAX_NUM_PICKUPS; ++i)
    {
        const uint8 type = pickups[i].m_nType;
        result = type;
        if (type == 22 || type == 23)
        {
            result = pickups[i].Remove();
        }
    }
    return result;
}

// 0x9355F0
uint8 __cdecl CPickups::RemovePickupsOfType(int32 type)
{
    CPickup* pickups = GetArray();
    uint8 result = 0;
    for (int32 i = 0; i < MAX_NUM_PICKUPS; ++i)
    {
        result = pickups[i].m_nType;
        if (static_cast<int32>(result) == type)
        {
            result = pickups[i].Remove();
        }
    }
    if (type == 23)
    {
        ms_nNumType23Pickups = 0;
    }
    return result;
}

// 0x935820
uint8 __cdecl CPickups::RemovePickup(int32 uniqueIndex)
{
    const int32 index = GetActualPickupIndex(uniqueIndex);
    if (index == -1)
    {
        return 0xFF;
    }

    CPickup& pickup = GetArray()[index];
    if (Core::Hook::Call<char>(0x462F90) && pickup.m_nType != 23)
    {
        Core::Hook::Call<void>(0x463000);
    }
    return pickup.Remove();
}
