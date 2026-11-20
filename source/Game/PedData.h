#pragma once

class CPed;

struct CPedDataSlot
{
    void* m_pPtr = nullptr; // +0x00
    int32 m_nId = -1;       // +0x04
};

struct CPedDataSub8
{
    uint32       m_nUnk00 = 0;      // +0x00
    CPedDataSlot m_slots[7]{};      // +0x04..+0x3B
    CPedDataSlot m_leaderSlot{};    // +0x3C..+0x43
    float        m_fUnk44 = 0.0f;   // +0x44

    bool __thiscall HasMember(void* ptr);
    int32 __thiscall CountAllSlots();
    int32 __thiscall CountRegularSlots();
};

VALIDATE_SIZE(CPedDataSlot, 0x08);
VALIDATE_SIZE(CPedDataSub8, 0x48);

class CPedData
{
public:
    static void InjectHooks();

    // Slice 1 (0xA66720..0xA66C6F)
    CPedData* __thiscall Ctor();
    void __thiscall Dtor();
    uint8 __thiscall UpdateLodFlags(CPed* ped);
    int32 __thiscall HasEntry3C() const;
    void* __thiscall GetEntry3C() const;
    int32 __thiscall HasSlotEntry(int32 index) const;
    void* __thiscall GetSlotPtr(int32 index) const;
    int32 __thiscall GetSlotModelIndex(int32 index) const;
    int32 __thiscall FindSlotIndex(const void* ptr) const;
    int32 __thiscall GetActiveSlotOrder(const void* ptr) const;
    int32 __thiscall IsEntry3C(const void* ptr) const;
    bool __thiscall HasSlotPtr(const void* ptr) const;

    // Slice 2 (0xA66C70..0xA6701F)
    void __thiscall SetFlag56Bit0();
    void __thiscall ClearFlag56Bit0AndSetBit2();
    static void* __cdecl CreateDefaultSub50();
    bool __thiscall SetSub50ByType(int32 type);
    bool __thiscall CheckSub55Match();
    bool __thiscall SetSub50(void* obj);
    void __thiscall NotifySub55IfActive();
    bool __thiscall HasMemberUsingTarget(void* target);
    static void __cdecl IncrementGlobalCounter();
    static void* __cdecl GetPedGroupByPed(const void* ped);
    static int32 __cdecl GetPoolIndex(const void* ptr);
    static int32 __cdecl GetPedSub4DA(const void* ped);
    static bool __cdecl ArePedsInSameGroup(void* ped1, void* ped2);
    static void __cdecl ResetGroupsByUnk58(uint32 val);
};
