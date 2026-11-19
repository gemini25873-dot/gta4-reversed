#include "StdInc.h"
#include "Game/PedData.h"
#include "Game/Ped.h"
#include "Core/Hook.h"
#include <cmath>

void CPedData::InjectHooks()
{
    // Slice 1 (0xA66720..0xA66C6F)
    Core::Hook::Install(0xA66720, &CPedData::Ctor);
    Core::Hook::Install(0xA66830, &CPedData::Dtor);
    Core::Hook::Install(0xA66860, &CPedData::UpdateLodFlags);
    Core::Hook::Install(0xA66B20, &CPedData::HasEntry3C);
    Core::Hook::Install(0xA66B40, &CPedData::GetEntry3C);
    Core::Hook::Install(0xA66B50, &CPedData::HasSlotEntry);
    Core::Hook::Install(0xA66B70, &CPedData::GetSlotPtr);
    Core::Hook::Install(0xA66B80, &CPedData::GetSlotModelIndex);
    Core::Hook::Install(0xA66BB0, &CPedData::FindSlotIndex);
    Core::Hook::Install(0xA66BE0, &CPedData::GetActiveSlotOrder);
    Core::Hook::Install(0xA66C20, &CPedData::IsEntry3C);
    Core::Hook::Install(0xA66C40, &CPedData::HasSlotPtr);

    // Slice 2 (0xA66C70..0xA6701F)
    Core::Hook::Install(0xA66C70, &CPedDataSub8::HasMember);
    Core::Hook::Install(0xA66CA0, &CPedDataSub8::CountAllSlots);
    Core::Hook::Install(0xA66CF0, &CPedDataSub8::CountRegularSlots);
    Core::Hook::Install(0xA66D10, &CPedData::SetFlag56Bit0);
    Core::Hook::Install(0xA66D20, &CPedData::ClearFlag56Bit0AndSetBit2);
    Core::Hook::Install(0xA66D40, &CPedData::CreateDefaultSub50);
    Core::Hook::Install(0xA66D50, &CPedData::SetSub50ByType);
    Core::Hook::Install(0xA66D80, &CPedData::CheckSub55Match);
    Core::Hook::Install(0xA66DC0, &CPedData::SetSub50);
    Core::Hook::Install(0xA66DF0, &CPedData::NotifySub55IfActive);
    Core::Hook::Install(0xA66E40, &CPedData::HasMemberUsingTarget);
    Core::Hook::Install(0xA66EB0, &CPedData::IncrementGlobalCounter);
    Core::Hook::Install(0xA66ED0, &CPedData::GetPedGroupByPed);
    Core::Hook::Install(0xA66EF0, &CPedData::GetPoolIndex);
    Core::Hook::Install(0xA66F20, &CPedData::GetPedSub4DA);
    Core::Hook::Install(0xA66F40, &CPedData::ArePedsInSameGroup);
    Core::Hook::Install(0xA66F70, &CPedData::ResetGroupsByUnk58);
}

// 0xA66720
CPedData* __thiscall CPedData::Ctor()
{
    uint8* self = reinterpret_cast<uint8*>(this);

    Core::Hook::CallMethod<void>(0x8DA800, self);
    Core::Hook::CallMethod<void>(0x9E31C0, self + 0x80);
    Core::Hook::CallMethod<void>(0x9720C0, self + 0x110);

    *reinterpret_cast<float*>(self + 0x144) = Core::StaticRef<float>(0xD50398);

    *reinterpret_cast<uint32*>(self + 0x12C) = 0;
    *reinterpret_cast<uint32*>(self + 0x130) = 0;
    *reinterpret_cast<uint32*>(self + 0x134) = 0;
    *reinterpret_cast<uint32*>(self + 0x138) = 0;
    *reinterpret_cast<uint32*>(self + 0x13C) = 0;
    *reinterpret_cast<uint8*>(self + 0x140) = 0;
    *reinterpret_cast<uint8*>(self + 0x141) = 0;
    *reinterpret_cast<uint8*>(self + 0x142) = 0;
    *reinterpret_cast<uint8*>(self + 0x143) = 0;
    *reinterpret_cast<uint32*>(self + 0x148) = 0;
    *reinterpret_cast<uint32*>(self + 0x14C) = 0;
    *reinterpret_cast<uint32*>(self + 0x150) = 0;
    *reinterpret_cast<uint8*>(self + 0x154) = 0;
    *reinterpret_cast<uint8*>(self + 0x155) = 0;

    *reinterpret_cast<uint32*>(self + 0x170) = 0;
    *reinterpret_cast<int32*>(self + 0x174) = -1;
    *reinterpret_cast<uint32*>(self + 0x178) = 0;
    *reinterpret_cast<uint8*>(self + 0x17C) = 0;
    *reinterpret_cast<float*>(self + 0x180) = 0.0f;

    Core::Hook::CallMethod<void>(0xA66630, self);

    const uint16 randVal = static_cast<uint16>(Core::Hook::Call<int32>(0xD0D21A));
    const float scaled = static_cast<float>(randVal) * Core::StaticRef<float>(0xEBE1D8) * Core::StaticRef<float>(0xD61C08);
    const int32 delay = 500 - static_cast<int32>(scaled);

    *reinterpret_cast<uint32*>(self + 0x14C) = Core::StaticRef<uint32>(0x11DDE74);
    *reinterpret_cast<int32*>(self + 0x150) = delay;
    *reinterpret_cast<uint8*>(self + 0x154) = 1;

    Core::Hook::CallMethod<void>(0xA66070, self, 0);
    Core::Hook::CallMethod<void>(0xA661E0, self, 0);

    *reinterpret_cast<uint32*>(self + 0x158) = 0;
    return this;
}

// 0xA66830
void __thiscall CPedData::Dtor()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0xA666E0, self, nullptr);
    Core::Hook::CallMethod<void>(0xC28820, self + 0x110);
    Core::Hook::CallMethod<void>(0xC28820, self + 0x80);
    Core::Hook::CallMethod<void>(0xC28820, self);
}

// 0xA66860
void __thiscall CPedData::UpdateLodFlags(CPed* ped)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    const uint8* pedBytes = reinterpret_cast<const uint8*>(ped);

    bool bPedUnk219 = false;
    bool bInVehicle = false;
    bool bDummyConvert2 = false;
    bool bTask419 = false;
    bool bVehType1 = false;
    bool bPlayerDriver = false;
    bool bPlayerPassenger = false;

    const uint32 entityFlags = *reinterpret_cast<const uint32*>(pedBytes + 0x28);
    if ((entityFlags & 0x3C0u) == 0xC0u) {
        bDummyConvert2 = (*reinterpret_cast<const uint8*>(pedBytes + 0xA70) == 2);
        const uint32 pedFlags2 = *reinterpret_cast<const uint32*>(pedBytes + 0x26C);
        bPedUnk219 = (*reinterpret_cast<const uint8*>(pedBytes + 0x219) != 0);
        const uint32 shifted = pedFlags2 >> 2;
        bInVehicle = ((shifted & 1u) != 0);
        if (bInVehicle) {
            uint8* pedIntel = *reinterpret_cast<uint8* const*>(pedBytes + 0x224);
            if (pedIntel != nullptr) {
                uint8* taskMgr = pedIntel + 0x2E0;
                if (taskMgr != nullptr) {
                    bTask419 = (Core::Hook::CallMethod<uint8>(0x426910, taskMgr, int32(0x419), int8(0)) != 0);
                }
            }

            if ((shifted & 1u) != 0) {
                uint8* myVeh = *reinterpret_cast<uint8* const*>(pedBytes + 0xB40);
                if (myVeh != nullptr && *reinterpret_cast<const int32*>(myVeh + 0x1354) == 1) {
                    bVehType1 = true;
                }
            }

            uint8* playerVeh = Core::Hook::Call<uint8*>(0x817F80, 0);
            if (playerVeh != nullptr) {
                if (*reinterpret_cast<CPed* const*>(playerVeh + 0xFA0) == ped) {
                    bPlayerDriver = true;
                } else if (Core::Hook::CallMethod<uint8>(0x9C5D30, playerVeh, ped) != 0) {
                    bPlayerPassenger = true;
                }
            }
        }
    }

    const uint8* mat = *reinterpret_cast<const uint8* const*>(pedBytes + 0x20);
    const float* pos = (mat != nullptr)
        ? reinterpret_cast<const float*>(mat + 0x30)
        : reinterpret_cast<const float*>(pedBytes + 0x10);

    const float dx = pos[0] - Core::StaticRef<float>(0x11DC4E0);
    const float dy = pos[1] - Core::StaticRef<float>(0x11DC4E4);
    const float dz = pos[2] - Core::StaticRef<float>(0x11DC4E8);
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    const float baseMult = Core::StaticRef<float>(0xF12F70);
    float scale = 0.0f;
    if (bInVehicle) {
        if (bPedUnk219) {
            scale = baseMult * Core::StaticRef<float>(0xF36D94);
        } else {
            const float vehMult = Core::StaticRef<float>(0xF36D90);
            scale = baseMult * vehMult;
            if (bVehType1 && Core::StaticRef<int32>(0x10F8044) == 1) {
                scale = (vehMult + Core::StaticRef<float>(0xD5829C)) * Core::StaticRef<float>(0xF12F6C);
            }
        }
    } else {
        scale = baseMult * Core::StaticRef<float>(0xF36D8C);
    }

    if (bTask419) {
        scale = baseMult * Core::StaticRef<float>(0xF36D98);
    }

    const bool flag6220F0 = (Core::Hook::Call<uint8>(0x6220F0) != 0);
    const int32 lodSetting = flag6220F0
        ? Core::StaticRef<int32>(0xFDCEA4)
        : Core::StaticRef<int32>(0xFDCEAC);
    float lodDistMult = static_cast<float>(lodSetting) * Core::StaticRef<float>(0xD8CCE8);

    if ((Core::StaticRef<int32>(0x10D0170) == 1
         || Core::StaticRef<int32>(0x10E1984) != Core::StaticRef<int32>(0xE3447C)
         || Core::StaticRef<int32>(0xF0E4A0) == 0x12)
        && Core::StaticRef<int32>(0x10D01A4) != 0)
    {
        lodDistMult = Core::StaticRef<float>(0xDCB2A4);
    }

    bool bit3 = (dist <= lodDistMult * scale);
    const uint8 globalLodFlag = Core::StaticRef<uint8>(0x10F4C32);
    if (bPlayerDriver || (bPlayerPassenger && bVehType1) || globalLodFlag == 0) {
        bit3 = true;
    }

    bool bit5 = (dist <= (baseMult * Core::StaticRef<float>(0xF36D9C)) * lodDistMult);
    if (globalLodFlag == 0) {
        bit5 = true;
    }

    bool bit4 = (dist > (baseMult * Core::StaticRef<float>(0xF36DA0)) * lodDistMult);
    if (bDummyConvert2) {
        bit4 = false;
    }

    uint8 packed = (bit5 ? 0x20u : 0u) | (bit4 ? 0x10u : 0u) | (bit3 ? 0x08u : 0u);
    self[0x72] = (self[0x72] & 0xC7u) | packed;
}

// 0xA66B20
int32 __thiscall CPedData::HasEntry3C() const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    if (*reinterpret_cast<const void* const*>(self + 0x3C) != nullptr
        || *reinterpret_cast<const int32*>(self + 0x40) >= 0)
    {
        return 1;
    }
    return 0;
}

// 0xA66B40
void* __thiscall CPedData::GetEntry3C() const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    return *reinterpret_cast<void* const*>(self + 0x3C);
}

// 0xA66B50
int32 __thiscall CPedData::HasSlotEntry(int32 index) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint8* slot = self + 0x04 + index * 8;
    if (*reinterpret_cast<const void* const*>(slot) != nullptr
        || *reinterpret_cast<const int32*>(slot + 0x04) >= 0)
    {
        return 1;
    }
    return 0;
}

// 0xA66B70
void* __thiscall CPedData::GetSlotPtr(int32 index) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    return *reinterpret_cast<void* const*>(self + 0x04 + index * 8);
}

// 0xA66B80
int32 __thiscall CPedData::GetSlotModelIndex(int32 index) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint8* slot = self + 0x04 + index * 8;
    const uint8* obj = *reinterpret_cast<const uint8* const*>(slot);
    if (obj != nullptr) {
        void* sub6C = *reinterpret_cast<void* const*>(obj + 0x6C);
        if (sub6C != nullptr) {
            return static_cast<int32>(Core::Hook::CallMethod<int16>(0x49F370, sub6C));
        }
    }
    return *reinterpret_cast<const int32*>(slot + 0x04);
}

// 0xA66BB0
int32 __thiscall CPedData::FindSlotIndex(const void* ptr) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    for (int32 i = 0; i < 7; ++i) {
        if (*reinterpret_cast<const void* const*>(self + 0x04 + i * 8) == ptr) {
            return i;
        }
    }
    return -1;
}

// 0xA66BE0
int32 __thiscall CPedData::GetActiveSlotOrder(const void* ptr) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    int32 count = 0;
    for (int32 i = 0; i < 7; ++i) {
        const uint8* slot = self + 0x04 + i * 8;
        const void* entryPtr = *reinterpret_cast<const void* const*>(slot);
        if (entryPtr == ptr) {
            return count;
        }
        if (entryPtr != nullptr || *reinterpret_cast<const int32*>(slot + 0x04) >= 0) {
            ++count;
        }
    }
    return -1;
}

// 0xA66C20
int32 __thiscall CPedData::IsEntry3C(const void* ptr) const
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    if (ptr != nullptr && *reinterpret_cast<const void* const*>(self + 0x3C) == ptr) {
        return 1;
    }
    return 0;
}

// 0xA66C40
bool __thiscall CPedData::HasSlotPtr(const void* ptr) const
{
    if (ptr == nullptr) {
        return false;
    }
    const uint8* self = reinterpret_cast<const uint8*>(this);
    for (int32 i = 0; i < 7; ++i) {
        if (*reinterpret_cast<const void* const*>(self + 0x04 + i * 8) == ptr) {
            return true;
        }
    }
    return false;
}

// 0xA66C70
bool __thiscall CPedDataSub8::HasMember(void* ptr)
{
    if (Core::Hook::CallMethod<bool>(0xA66C40, this, ptr))
        return true;
    if (Core::Hook::CallMethod<bool>(0xA66C20, this, ptr))
        return true;
    return false;
}

// 0xA66CA0
int32 __thiscall CPedDataSub8::CountAllSlots()
{
    int32 count = 0;
    for (int32 i = 0; i < 7; ++i)
    {
        if (m_slots[i].m_pPtr != nullptr || m_slots[i].m_nId >= 0)
            ++count;
    }
    if (m_leaderSlot.m_pPtr != nullptr || m_leaderSlot.m_nId >= 0)
        ++count;
    return count;
}

// 0xA66CF0
int32 __thiscall CPedDataSub8::CountRegularSlots()
{
    int32 count = 0;
    for (int32 i = 0; i < 7; ++i)
    {
        if (m_slots[i].m_pPtr != nullptr || m_slots[i].m_nId >= 0)
            ++count;
    }
    return count;
}

// 0xA66D10
void __thiscall CPedData::SetFlag56Bit0()
{
    *reinterpret_cast<uint16*>(reinterpret_cast<uint8*>(this) + 0x56) |= 0x1;
}

// 0xA66D20
void __thiscall CPedData::ClearFlag56Bit0AndSetBit2()
{
    uint16& flags = *reinterpret_cast<uint16*>(reinterpret_cast<uint8*>(this) + 0x56);
    flags = (flags & 0xFFFEu) | 0x4u;
}

// 0xA66D40
void* __cdecl CPedData::CreateDefaultSub50()
{
    return Core::Hook::Call<void*>(0xCCCF90, 0);
}

// 0xA66D50
bool __thiscall CPedData::SetSub50ByType(int32 type)
{
    void*& sub50 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x50);
    if (sub50 != nullptr)
    {
        using DtorFn = void(__thiscall*)(void*, int32);
        reinterpret_cast<DtorFn>((*reinterpret_cast<void***>(sub50))[0])(sub50, 1);
    }

    void* created = Core::Hook::Call<void*>(0xCCCF90, type);
    sub50 = created;
    return Core::Hook::CallMethod<bool>(0xCCB440, created, this);
}

// 0xA66D80
bool __thiscall CPedData::CheckSub55Match()
{
    if (!Core::Hook::Call<bool>(0x462F90))
        return true;

    void* mgr = Core::StaticRef<void*>(0x18A8300);
    if (Core::Hook::CallMethod<void*>(0x493120, mgr) == nullptr)
        return true;

    void* obj = Core::Hook::CallMethod<void*>(0x493120, Core::StaticRef<void*>(0x18A8300));
    const uint8 val55 = *(reinterpret_cast<const uint8*>(this) + 0x55);
    return Core::Hook::CallMethod<bool>(0x4A3A70, obj, static_cast<uint32>(val55));
}

// 0xA66DC0
bool __thiscall CPedData::SetSub50(void* obj)
{
    void*& sub50 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x50);
    if (sub50 != nullptr)
    {
        using DtorFn = void(__thiscall*)(void*, int32);
        reinterpret_cast<DtorFn>((*reinterpret_cast<void***>(sub50))[0])(sub50, 1);
    }

    sub50 = obj;
    return Core::Hook::CallMethod<bool>(0xCCB440, obj, this);
}

// 0xA66DF0
void __thiscall CPedData::NotifySub55IfActive()
{
    if (!Core::Hook::Call<bool>(0x462F90))
        return;

    const uint8* self = reinterpret_cast<const uint8*>(this);
    if ((self[0x56] & 0x4) == 0)
        return;

    if (!CheckSub55Match())
        return;

    if (Core::Hook::CallMethod<void*>(0x493120, Core::StaticRef<void*>(0x18A8300)) == nullptr)
        return;

    void* obj = Core::Hook::CallMethod<void*>(0x493120, Core::StaticRef<void*>(0x18A8300));
    using NotifyFn = void(__thiscall*)(void*, uint32);
    reinterpret_cast<NotifyFn>((*reinterpret_cast<void***>(obj))[0x34 / 4])(obj, static_cast<uint32>(self[0x55]));
}

// 0xA66E40
bool __thiscall CPedData::HasMemberUsingTarget(void* target)
{
    uint8* sub8 = reinterpret_cast<uint8*>(this) + 0x08;
    for (int32 i = 0; i < 8; ++i)
    {
        uint8* ped = Core::Hook::CallMethod<uint8*>(0xA66B70, sub8, i);
        if (ped == nullptr)
            continue;

        void* veh = *reinterpret_cast<void* const*>(ped + 0xB40);
        if (veh != nullptr && (ped[0x26C] & 0x4) != 0 && veh == target)
            return true;

        void* intel = *reinterpret_cast<void* const*>(ped + 0x224);
        uint8* task = Core::Hook::CallMethod<uint8*>(0x9C2FD0, intel, 0x2DE);
        if (task != nullptr && *reinterpret_cast<void* const*>(task + 0x18) == target)
            return true;
    }
    return false;
}

// 0xA66EB0
void __cdecl CPedData::IncrementGlobalCounter()
{
    if (Core::StaticRef<uint8>(0x1662168) != 0)
        ++Core::StaticRef<uint32>(0x166216C);
}

// 0xA66ED0
void* __cdecl CPedData::GetPedGroupByPed(const void* ped)
{
    const uint8 idx = reinterpret_cast<const uint8*>(ped)[0xD42];
    if (idx == 0xFF)
        return nullptr;
    return reinterpret_cast<void*>(Core::Rebase(0x1662170) + static_cast<uint32>(idx) * 0x5C);
}

// 0xA66EF0
int32 __cdecl CPedData::GetPoolIndex(const void* ptr)
{
    const uintptr_t base = Core::Rebase(0x1662170);
    const uintptr_t target = reinterpret_cast<uintptr_t>(ptr);
    for (int32 i = 0; i < 64; ++i)
    {
        if (base + static_cast<uintptr_t>(i) * 0x5C == target)
            return i;
    }
    return -1;
}

// 0xA66F20
int32 __cdecl CPedData::GetPedSub4DA(const void* ped)
{
    const uint8* sub = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(ped) + 0x228);
    return static_cast<int32>(*reinterpret_cast<const int8*>(sub + 0x4DA));
}

// 0xA66F40
bool __cdecl CPedData::ArePedsInSameGroup(void* ped1, void* ped2)
{
    if (Core::Hook::CallMethod<void*>(0x89DD80, ped1) == nullptr)
        return false;
    void* g1 = Core::Hook::CallMethod<void*>(0x89DD80, ped1);
    void* g2 = Core::Hook::CallMethod<void*>(0x89DD80, ped2);
    return g1 == g2;
}

// 0xA66F70
void __cdecl CPedData::ResetGroupsByUnk58(uint32 val)
{
    const uintptr_t base = Core::Rebase(0x1662170);
    const uintptr_t end  = Core::Rebase(0x1663870);
    for (uintptr_t cur = base; cur < end; cur += 0x5C)
    {
        if (*reinterpret_cast<const uint32*>(cur + 0x58) == val)
        {
            void* defaultObj = Core::Hook::Call<void*>(0xA66D40);
            Core::Hook::CallMethod<bool>(0xA66DC0, reinterpret_cast<void*>(cur), defaultObj);
        }
    }
}
