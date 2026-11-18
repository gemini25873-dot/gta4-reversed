#pragma once
#include "Core/Vector.h"

class CObject;

class CPickup
{
public:
    uint8    m_pad00[0x04]{};           // +0x00..+0x03
    CObject* m_pObject = nullptr;       // +0x04
    void*    m_pUnk08 = nullptr;        // +0x08
    uint8    m_pad0C[0x04]{};           // +0x0C..+0x0F
    int32    m_nBlipIndex = -1;         // +0x10
    uint8    m_pad14[0x04]{};           // +0x14..+0x17
    CVector  m_vecPos{};                // +0x18..+0x23
    uint8    m_pad24[0x1E]{};           // +0x24..+0x41
    uint16   m_nReferenceIndex = 0;     // +0x42
    uint8    m_nType = 0;               // +0x44
    uint8    m_pad45[0x0B]{};           // +0x45..+0x4F

    void  __thiscall DestroyObject();
    uint8 __thiscall Remove();
};

VALIDATE_SIZE(CPickup, 0x50);
VALIDATE_OFFSET(CPickup, m_pObject, 0x04);
VALIDATE_OFFSET(CPickup, m_pUnk08, 0x08);
VALIDATE_OFFSET(CPickup, m_nBlipIndex, 0x10);
VALIDATE_OFFSET(CPickup, m_vecPos, 0x18);
VALIDATE_OFFSET(CPickup, m_nReferenceIndex, 0x42);
VALIDATE_OFFSET(CPickup, m_nType, 0x44);

class CPickups
{
public:
    static constexpr int32 MAX_NUM_PICKUPS = 1500;

    static inline int32& ms_nNumType23Pickups = Core::StaticRef<int32>(0x1549110);

    static CPickup* GetArray()
    {
        return reinterpret_cast<CPickup*>(Core::Rebase(0x1549150));
    }

    static void InjectHooks();

    static int32 __cdecl Init();
    static uint8 __cdecl RemoveAllPickups();
    static int32 __cdecl GetActualPickupIndex(int32 uniqueIndex);
    static int32 __cdecl GetNewUniquePickupIndex(int32 index);
    static uint8 __cdecl RemoveMoneyAndWeaponDropsInArea(const CVector* pos, float radius);
    static uint8 __cdecl RemoveTemporaryPickups();
    static uint8 __cdecl RemovePickupsOfType(int32 type);
    static uint8 __cdecl RemovePickup(int32 uniqueIndex);
};
