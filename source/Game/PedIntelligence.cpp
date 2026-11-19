#include "StdInc.h"
#include "Game/PedIntelligence.h"
#include "Game/Entity.h"
#include "Game/Ped.h"
#include "Core/Hook.h"
#include "Core/Matrix.h"
#include "Core/Vector.h"
#include <cmath>

// =========================================================================
// CPedIntelligence - Slice 1 (0x9C22C0 .. 0x9C2FFF) - 42 functions
//
// Method declarations for CPedIntelligence (add to PedIntelligence.h):
//   void  __thiscall SetUnkD0(uint32 val);                                                      // 0x9C22C0
//   void  __thiscall SetUnkD4(uint32 val);                                                      // 0x9C22D0
//   void  __thiscall SetUnkDC(uint32 val);                                                      // 0x9C22E0
//   void  __thiscall SetUnkE0(float val);                                                       // 0x9C22F0
//   void  __thiscall SetUnkE4(float val);                                                       // 0x9C2310
//   float __thiscall ComputeUnk9C2330(CEntity* target, float unusedArg);                        // 0x9C2330
//   void* __thiscall UpdateAndGetUnk1C4();                                                      // 0x9C2450
//   bool  __thiscall CheckUnk9C24A0(int32 id);                                                  // 0x9C24A0
//   void  __thiscall ClearUnk25C();                                                             // 0x9C24B0
//   bool  __thiscall CheckAndUpdateUnk25C();                                                    // 0x9C24E0
//   void* __thiscall GetFromUnk25C();                                                           // 0x9C2520
//   void  __thiscall SetUnk260(uint32 val);                                                     // 0x9C2540
//   void  __thiscall SetTaskSlot0(void* task, uint8 arg2);                                      // 0x9C2580
//   void  __thiscall SetTaskSlot1(void* task, uint8 arg2);                                      // 0x9C25A0
//   void  __thiscall SetTaskSlot2(void* task, uint8 arg2);                                      // 0x9C25C0
//   void* __thiscall FindTaskInPrimarySlots(int32 taskType);                                    // 0x9C25E0
//   void* __thiscall GetTask431();                                                              // 0x9C2620
//   void* __thiscall GetTask433();                                                              // 0x9C2630
//   void* __thiscall GetPrimaryTaskIf1016();                                                    // 0x9C2640
//   bool  __thiscall HasEventF1();                                                              // 0x9C2680
//   bool  __thiscall HasEventFE();                                                              // 0x9C26C0
//   bool  __thiscall CheckEvent41EAndVehicle(int32* outVal, bool* outFlag);                     // 0x9C2720
//   bool  __thiscall HasEvent120Or11F();                                                        // 0x9C28B0
//   void  __thiscall ClearUnk84AndSub20();                                                      // 0x9C2930
//   void  __thiscall ProcessTasks9C2960(float arg);                                             // 0x9C2960
//   void* __thiscall GetEvent2DEOrVehicle();                                                    // 0x9C29E0
//   bool  __thiscall HasAnyCombatTask();                                                        // 0x9C2A60
//   bool  __thiscall ProcessCombatTask(void* arg);                                              // 0x9C2AA0
//   float __thiscall GetMoveSpeed();                                                            // 0x9C2B10
//   void  __thiscall SetUnkF0F1(uint8 valF0, uint8 valF1);                                      // 0x9C2B80
//   bool  __thiscall HasAnyUnk2D0();                                                            // 0x9C2BB0
//   bool  __thiscall HasUnk2D0(void* ptr);                                                      // 0x9C2BD0
//   void  __thiscall UpdateUnk2D0Nearby();                                                      // 0x9C2C00
//   static void __cdecl ResetUnk152EBE0();                                                      // 0x9C2D00
//   void  __thiscall AddSubUnk2C8(float delta);                                                 // 0x9C2D10
//   void  __thiscall SetUnk2C8(float val);                                                      // 0x9C2D50
//   void  __thiscall SetUnk2C4_2CC(bool flag, float val);                                       // 0x9C2D90
//   void* __thiscall InitSub0();                                                                // 0x9C2DD0
//   void  __thiscall ClearSub0_24();                                                            // 0x9C2E10
//   void* __thiscall GetTask141_144Unk20(uint32* outVal);                                       // 0x9C2E30
//   bool  __thiscall IsTargetInViewCone(const CVector* targetPos, float minDist, float maxScore, float maxDist, float minScore); // 0x9C2E80
//   void* __thiscall FindTaskInAllSlots(int32 taskType);                                        // 0x9C2FD0
//
// Discovered CPedIntelligence field notes (offsets relative to CPedIntelligence*):
//   +0x00..+0x28 : sub-object cleared by InitSub0 / ClearSub0_24 (+0x24 ptr, +0x28 flags)
//   +0x40        : CPed* m_pPed (owner ped)
//   +0x44        : CTaskManager m_taskMgr (slots at +0x44.., slot 2 at +0x4C, secondary at +0x70)
//   +0x84        : sub-object cleared by ClearUnk84AndSub20
//   +0xD0..+0xE4 : uint32/float fields (+0xD0, +0xD4, +0xDC, +0xE0, +0xE4)
//   +0xF0, +0xF1 : uint8 fields
//   +0x1AC..+0x1C4: sub-object +0x1AC, counter +0x1BC, max +0x1C0, result +0x1C4
//   +0x25C       : void* sub-object (has vtbl[0] deleting dtor, +0x240 uint32)
//   +0x260       : uint32
//   +0x2C4..+0x2CC: float +0x2C4, int8 +0x2C8 (clamped 0..100), uint32 flags +0x2CC
//   +0x2D0..+0x2DB: void*[3]
//   +0x2E0       : event list head (node: +0x04 type, +0x08 flags, +0x0C next)
// =========================================================================

namespace {
    struct PedIntelEventNode
    {
        void* vtable;
        int32 type;
        uint32 flags;
        PedIntelEventNode* next;
    };

    inline bool HasPedIntelEvent(const uint8* self, int32 eventType)
    {
        const auto* node = *reinterpret_cast<PedIntelEventNode* const*>(self + 0x2E0);
        if (!node) {
            return false;
        }
        uint32 prevPrio = (node->flags >> 1) & 7u;
        while (node) {
            const uint32 prio = (node->flags >> 1) & 7u;
            if (prevPrio < prio && prevPrio >= 2u) {
                return false;
            }
            if (node->type == eventType) {
                return true;
            }
            prevPrio = prio;
            node = node->next;
        }
        return false;
    }

    inline bool HasPedGroupEvent(const uint8* ped, int32 eventType)
    {
        uint8* pedGroup = *reinterpret_cast<uint8* const*>(ped + 0x6C);
        if (!pedGroup) {
            return false;
        }
        return Core::Hook::CallMethod<bool>(0x4A5120, pedGroup + 0x808, eventType);
    }

    inline uint8* GetPedPosPtr(uint8* entity)
    {
        uint8* mat = *reinterpret_cast<uint8**>(entity + 0x20);
        return mat ? (mat + 0x30) : (entity + 0x10);
    }
}

static void InjectPedIntelligenceSlice1Hooks()
{
    Core::Hook::Install(0x9C22C0, &CPedIntelligence::SetUnkD0);
    Core::Hook::Install(0x9C22D0, &CPedIntelligence::SetUnkD4);
    Core::Hook::Install(0x9C22E0, &CPedIntelligence::SetUnkDC);
    Core::Hook::Install(0x9C22F0, &CPedIntelligence::SetUnkE0);
    Core::Hook::Install(0x9C2310, &CPedIntelligence::SetUnkE4);
    Core::Hook::Install(0x9C2330, &CPedIntelligence::ComputeUnk9C2330);
    Core::Hook::Install(0x9C2450, &CPedIntelligence::UpdateAndGetUnk1C4);
    Core::Hook::Install(0x9C24A0, &CPedIntelligence::CheckUnk9C24A0);
    Core::Hook::Install(0x9C24B0, &CPedIntelligence::ClearUnk25C);
    Core::Hook::Install(0x9C24E0, &CPedIntelligence::CheckAndUpdateUnk25C);
    Core::Hook::Install(0x9C2520, &CPedIntelligence::GetFromUnk25C);
    Core::Hook::Install(0x9C2540, &CPedIntelligence::SetUnk260);
    Core::Hook::Install(0x9C2580, &CPedIntelligence::SetTaskSlot0);
    Core::Hook::Install(0x9C25A0, &CPedIntelligence::SetTaskSlot1);
    Core::Hook::Install(0x9C25C0, &CPedIntelligence::SetTaskSlot2);
    Core::Hook::Install(0x9C25E0, &CPedIntelligence::FindTaskInPrimarySlots);
    Core::Hook::Install(0x9C2620, &CPedIntelligence::GetTask431);
    Core::Hook::Install(0x9C2630, &CPedIntelligence::GetTask433);
    Core::Hook::Install(0x9C2640, &CPedIntelligence::GetPrimaryTaskIf1016);
    Core::Hook::Install(0x9C2680, &CPedIntelligence::HasEventF1);
    Core::Hook::Install(0x9C26C0, &CPedIntelligence::HasEventFE);
    Core::Hook::Install(0x9C2720, &CPedIntelligence::CheckEvent41EAndVehicle);
    Core::Hook::Install(0x9C28B0, &CPedIntelligence::HasEvent120Or11F);
    Core::Hook::Install(0x9C2930, &CPedIntelligence::ClearUnk84AndSub20);
    Core::Hook::Install(0x9C2960, &CPedIntelligence::ProcessTasks9C2960);
    Core::Hook::Install(0x9C29E0, &CPedIntelligence::GetEvent2DEOrVehicle);
    Core::Hook::Install(0x9C2A60, &CPedIntelligence::HasAnyCombatTask);
    Core::Hook::Install(0x9C2AA0, &CPedIntelligence::ProcessCombatTask);
    Core::Hook::Install(0x9C2B10, &CPedIntelligence::GetMoveSpeed);
    Core::Hook::Install(0x9C2B80, &CPedIntelligence::SetUnkF0F1);
    Core::Hook::Install(0x9C2BB0, &CPedIntelligence::HasAnyUnk2D0);
    Core::Hook::Install(0x9C2BD0, &CPedIntelligence::HasUnk2D0);
    Core::Hook::Install(0x9C2C00, &CPedIntelligence::UpdateUnk2D0Nearby);
    Core::Hook::Install(0x9C2D00, &CPedIntelligence::ResetUnk152EBE0);
    Core::Hook::Install(0x9C2D10, &CPedIntelligence::AddSubUnk2C8);
    Core::Hook::Install(0x9C2D50, &CPedIntelligence::SetUnk2C8);
    Core::Hook::Install(0x9C2D90, &CPedIntelligence::SetUnk2C4_2CC);
    Core::Hook::Install(0x9C2DD0, &CPedIntelligenceSub10::InitSub0);
    Core::Hook::Install(0x9C2E10, &CPedIntelligenceSub10::ClearSub0_24);
    Core::Hook::Install(0x9C2E30, &CPedIntelligence::GetTask141_144Unk20);
    Core::Hook::Install(0x9C2E80, &CPedIntelligence::IsTargetInViewCone);
    Core::Hook::Install(0x9C2FD0, &CPedIntelligence::FindTaskInAllSlots);
}

void __thiscall CPedIntelligence::SetUnkD0(uint32 val)
{
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xD0) = val;
}

void __thiscall CPedIntelligence::SetUnkD4(uint32 val)
{
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xD4) = val;
}

void __thiscall CPedIntelligence::SetUnkDC(uint32 val)
{
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xDC) = val;
}

void __thiscall CPedIntelligence::SetUnkE0(float val)
{
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(this) + 0xE0) = val;
}

void __thiscall CPedIntelligence::SetUnkE4(float val)
{
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(this) + 0xE4) = val;
}

float __thiscall CPedIntelligence::ComputeUnk9C2330(CEntity* target, float /*unusedArg*/)
{
    auto* targetBytes = reinterpret_cast<uint8*>(target);
    const float fallback = Core::StaticRef<float>(0xF317EC);
    if ((*reinterpret_cast<uint32*>(targetBytes + 0x28) & 0x3C0u) != 0xC0u) {
        return fallback;
    }
    if (targetBytes[0x219] == 0) {
        return fallback;
    }

    float baseScore = Core::StaticRef<float>(0xD50398);
    if (baseScore > fallback) {
        return fallback;
    }

    auto* self = reinterpret_cast<uint8*>(this);
    auto* ped = *reinterpret_cast<uint8**>(self + 0x40);
    const auto* pedPos = reinterpret_cast<const CVector*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
    const auto* targetPos = reinterpret_cast<const CVector*>(GetPedPosPtr(targetBytes));

    const float dx = targetPos->x - pedPos->x;
    const float dy = targetPos->y - pedPos->y;
    const float dz = targetPos->z - pedPos->z;
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz) - Core::StaticRef<float>(0xDCE03C);

    const float score1 = baseScore - (dist / Core::StaticRef<float>(0xF317F0)) * fallback * fallback;
    if (score1 > 0.0f) {
        return score1;
    }

    const float score2 = score1 * score1 - (dist / Core::StaticRef<float>(0xF317F4)) * fallback * fallback;
    if (score2 > 0.0f) {
        return score2 * Core::StaticRef<float>(0xEBDED4);
    }
    return 0.0f;
}

void* __thiscall CPedIntelligence::UpdateAndGetUnk1C4()
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto* counterPtr = reinterpret_cast<int32*>(self + 0x1BC);
    const int32 maxCount = *reinterpret_cast<int32*>(self + 0x1C0);

    int32 counter = *counterPtr;
    const bool wasZero = (counter == 0);
    counter += 1;
    *counterPtr = counter;
    if (counter > maxCount) {
        *counterPtr = 0;
    }
    if (wasZero) {
        *counterPtr = 0;
        void* ped = *reinterpret_cast<void**>(self + 0x40);
        Core::Hook::CallMethod<void>(0xA56FD0, self + 0x1AC, ped);
    }
    return self + 0x1C4;
}

bool __thiscall CPedIntelligence::CheckUnk9C24A0(int32 id)
{
    using GetSubFn = uint8*(__thiscall*)(CPedIntelligence*);
    void** vtable = *reinterpret_cast<void***>(this);
    uint8* sub = reinterpret_cast<GetSubFn>(vtable[7])(this);
    return Core::Hook::CallMethod<bool>(0xA67E80, sub + 0x20, id);
}

void __thiscall CPedIntelligence::ClearUnk25C()
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto** ptr = reinterpret_cast<void**>(self + 0x25C);
    if (*ptr) {
        using DeletingDtorFn = void(__thiscall*)(void*, uint8);
        void** vtable = *reinterpret_cast<void***>(*ptr);
        reinterpret_cast<DeletingDtorFn>(vtable[0])(*ptr, 1);
        *ptr = nullptr;
    }
}

bool __thiscall CPedIntelligence::CheckAndUpdateUnk25C()
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto** ptr = reinterpret_cast<void**>(self + 0x25C);
    if (!*ptr) {
        return false;
    }
    if (!Core::Hook::CallMethod<bool>(0xC899C0, *ptr)) {
        ClearUnk25C();
    }
    return true;
}

void* __thiscall CPedIntelligence::GetFromUnk25C()
{
    void* obj = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x25C);
    if (!obj) {
        return nullptr;
    }
    return Core::Hook::CallMethod<void*>(0xC87AE0, obj);
}

void __thiscall CPedIntelligence::SetUnk260(uint32 val)
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto* obj = *reinterpret_cast<uint8**>(self + 0x25C);
    auto* unk260 = reinterpret_cast<uint32*>(self + 0x260);
    if (obj) {
        if (*reinterpret_cast<uint32*>(obj + 0x240) == *unk260) {
            *unk260 = val;
            Core::Hook::CallMethod<void>(0xC88050, obj);
        }
        return;
    }
    *unk260 = val;
}

void __thiscall CPedIntelligence::SetTaskSlot0(void* task, uint8 arg2)
{
    Core::Hook::CallMethod<void>(0x9E58B0, reinterpret_cast<uint8*>(this) + 0x44, task, 0, arg2);
}

void __thiscall CPedIntelligence::SetTaskSlot1(void* task, uint8 arg2)
{
    Core::Hook::CallMethod<void>(0x9E58B0, reinterpret_cast<uint8*>(this) + 0x44, task, 1, arg2);
}

void __thiscall CPedIntelligence::SetTaskSlot2(void* task, uint8 arg2)
{
    Core::Hook::CallMethod<void>(0x9E58B0, reinterpret_cast<uint8*>(this) + 0x44, task, 2, arg2);
}

void* __thiscall CPedIntelligence::FindTaskInPrimarySlots(int32 taskType)
{
    uint8* taskMgr = reinterpret_cast<uint8*>(this) + 0x44;
    for (int32 slot = 0; slot < 3; ++slot) {
        if (void* task = Core::Hook::CallMethod<void*>(0x9E56A0, taskMgr, slot, taskType)) {
            return task;
        }
    }
    return nullptr;
}

void* __thiscall CPedIntelligence::GetTask431()
{
    return Core::Hook::CallMethod<void*>(0x426850, reinterpret_cast<uint8*>(this) + 0x44, 0x1AF);
}

void* __thiscall CPedIntelligence::GetTask433()
{
    return Core::Hook::CallMethod<void*>(0x426850, reinterpret_cast<uint8*>(this) + 0x44, 0x1B1);
}

void* __thiscall CPedIntelligence::GetPrimaryTaskIf1016()
{
    uint8* taskMgr = reinterpret_cast<uint8*>(this) + 0x44;
    void* task = Core::Hook::CallMethod<void*>(0x9E56E0, taskMgr, 0);
    if (!task) {
        return nullptr;
    }
    using GetTypeFn = int32(__thiscall*)(void*);
    void** vtable = *reinterpret_cast<void***>(task);
    if (reinterpret_cast<GetTypeFn>(vtable[3])(task) == 0x3F8) {
        return Core::Hook::CallMethod<void*>(0x9E56E0, taskMgr, 0);
    }
    return nullptr;
}

bool __thiscall CPedIntelligence::HasEventF1()
{
    return HasPedIntelEvent(reinterpret_cast<const uint8*>(this), 0xF1);
}

bool __thiscall CPedIntelligence::HasEventFE()
{
    const auto* self = reinterpret_cast<const uint8*>(this);
    if (HasPedIntelEvent(self, 0xFE)) {
        return true;
    }
    return HasPedGroupEvent(*reinterpret_cast<const uint8* const*>(self + 0x40), 0xFE);
}

bool __thiscall CPedIntelligence::CheckEvent41EAndVehicle(int32* outVal, bool* outFlag)
{
    *outVal = -1;
    *outFlag = false;

    auto* self = reinterpret_cast<uint8*>(this);
    if (HasPedIntelEvent(self, 0x41E)) {
        uint8* task = Core::Hook::CallMethod<uint8*>(0x426850, self + 0x44, 0x41E);
        if (!task) {
            return true;
        }
        *outVal = *reinterpret_cast<int32*>(task + 0x38);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        if (*reinterpret_cast<void**>(ped + 0xD78) != nullptr) {
            const uint8 flagArg = (task[0x35] >> 1) & 1u;
            const uint8 b1 = Core::Hook::CallMethod<uint8>(0xA78090, task, ped);
            const uint8 b2 = Core::Hook::CallMethod<uint8>(0xA78030, task, ped);
            const uint8 b3 = Core::Hook::CallMethod<uint8>(0xA78060, task, ped);
            float outBuf1[4] = {};
            float outBuf2[4] = {};
            if (Core::Hook::Call<bool>(0xA7D630, ped, outBuf2, outBuf1, 8, b3, b2, flagArg, b1, 0)) {
                *outFlag = true;
                return true;
            }
        }
        *outFlag = false;
        return true;
    }

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* vehicle = *reinterpret_cast<uint8**>(ped + 0xD78);
    if (!vehicle) {
        return false;
    }
    if (!Core::Hook::CallMethod<bool>(0x426910, self + 0x2E0, 0x414, 0)) {
        return false;
    }
    if ((*reinterpret_cast<uint8*>(vehicle) & 7u) == 1u) {
        return false;
    }
    float vehPos[4] = {};
    if (!Core::Hook::CallMethod<bool>(0xC697A0, vehicle, vehPos, nullptr)) {
        return false;
    }
    const auto* pedPos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
    const float dx = pedPos[0] - vehPos[0];
    const float dy = pedPos[1] - vehPos[1];
    const float maxDist = Core::StaticRef<float>(0xF31804);
    return (maxDist * maxDist) > (dx * dx + dy * dy);
}

bool __thiscall CPedIntelligence::HasEvent120Or11F()
{
    auto* self = reinterpret_cast<uint8*>(this);
    if (HasPedIntelEvent(self, 0x120)) {
        return true;
    }
    if (Core::Hook::CallMethod<bool>(0x426910, self + 0x2E0, 0x11F, 0)) {
        return true;
    }
    return HasPedGroupEvent(*reinterpret_cast<const uint8* const*>(self + 0x40), 0x11F);
}

void __thiscall CPedIntelligence::ClearUnk84AndSub20()
{
    auto* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0xAA7AE0, self + 0x84, 1);

    using GetSubFn = void*(__thiscall*)(CPedIntelligence*);
    void** vtable = *reinterpret_cast<void***>(this);
    void* sub = reinterpret_cast<GetSubFn>(vtable[8])(this);
    Core::Hook::CallMethod<void>(0xA68900, sub);
}

void __thiscall CPedIntelligence::ProcessTasks9C2960(float arg)
{
    auto* self = reinterpret_cast<uint8*>(this);
    void** primary = reinterpret_cast<void**>(self + 0x44);
    void* primaryTask = nullptr;
    for (int32 i = 0; i < 5; ++i) {
        if (primary[i]) {
            primaryTask = primary[i];
            break;
        }
    }

    using ProcessTaskFn = bool(__thiscall*)(void*, void*, float);
    void* ped = *reinterpret_cast<void**>(self + 0x40);
    if (primaryTask) {
        void** vtable = *reinterpret_cast<void***>(primaryTask);
        if (reinterpret_cast<ProcessTaskFn>(vtable[13])(primaryTask, ped, arg)) {
            return;
        }
    }

    auto** secondary = reinterpret_cast<uint8**>(self + 0x70);
    uint8* secondaryTask = nullptr;
    for (int32 i = 0; i < 3; ++i) {
        if (secondary[i] && (secondary[i][0x0C] & 1u) == 0) {
            secondaryTask = secondary[i];
            break;
        }
    }
    if (secondaryTask) {
        void** vtable = *reinterpret_cast<void***>(secondaryTask);
        reinterpret_cast<ProcessTaskFn>(vtable[13])(secondaryTask, ped, arg);
    }
}

void* __thiscall CPedIntelligence::GetEvent2DEOrVehicle()
{
    auto* self = reinterpret_cast<uint8*>(this);
    if (HasPedIntelEvent(self, 0x2DE)) {
        return Core::Hook::CallMethod<void*>(0x9A4850, self + 0x2E0, 0x2DE, 5);
    }
    if (Core::Hook::CallMethod<bool>(0x426910, self + 0x2E0, 0x2C5, 0)) {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        return *reinterpret_cast<void**>(ped + 0xB40);
    }
    return nullptr;
}

bool __thiscall CPedIntelligence::HasAnyCombatTask()
{
    uint8* taskMgr = reinterpret_cast<uint8*>(this) + 0x44;
    return Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3AE) != nullptr
        || Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3A9) != nullptr
        || Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3B5) != nullptr;
}

bool __thiscall CPedIntelligence::ProcessCombatTask(void* arg)
{
    auto* self = reinterpret_cast<uint8*>(this);
    uint8* taskMgr = self + 0x44;
    void* ped = *reinterpret_cast<void**>(self + 0x40);

    if (void* task = Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3AE)) {
        return Core::Hook::CallMethod<bool>(0x9F21E0, task, ped, arg);
    }
    if (void* task = Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3A9)) {
        return Core::Hook::CallMethod<bool>(0x9F21E0, task, ped, arg);
    }
    if (void* task = Core::Hook::CallMethod<void*>(0x426850, taskMgr, 0x3B5)) {
        return Core::Hook::CallMethod<bool>(0xA0EDF0, task, ped, arg);
    }
    return false;
}

float __thiscall CPedIntelligence::GetMoveSpeed()
{
    auto* self = reinterpret_cast<uint8*>(this);
    void* task = Core::Hook::CallMethod<void*>(0x9E5730, self + 0x44);
    if (!task) {
        return 0.0f;
    }

    using GetTypeFn = int32(__thiscall*)(void*);
    void** vtable = *reinterpret_cast<void***>(task);
    if (reinterpret_cast<GetTypeFn>(vtable[3])(task) == 2) {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        auto* moveBlend = *reinterpret_cast<const float**>(ped + 0xA90);
        const float vx = moveBlend[1];
        const float vy = moveBlend[2];
        return std::sqrt(vx * vx + vy * vy);
    }

    using GetSubFn = void*(__thiscall*)(void*);
    using GetSpeedFn = float(__thiscall*)(void*);
    void* sub = reinterpret_cast<GetSubFn>(vtable[12])(task);
    if (!sub) {
        return 0.0f;
    }
    void** subVtable = *reinterpret_cast<void***>(sub);
    return reinterpret_cast<GetSpeedFn>(subVtable[2])(sub);
}

void __thiscall CPedIntelligence::SetUnkF0F1(uint8 valF0, uint8 valF1)
{
    auto* self = reinterpret_cast<uint8*>(this);
    if (valF0 == 0x20) {
        return;
    }
    if (valF0 == 0 || static_cast<int32>(valF1) > static_cast<int32>(self[0xF1])) {
        self[0xF0] = valF0;
        self[0xF1] = valF1;
    }
}

bool __thiscall CPedIntelligence::HasAnyUnk2D0()
{
    auto* arr = reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x2D0);
    for (int32 i = 0; i < 3; ++i) {
        if (arr[i] != nullptr) {
            return true;
        }
    }
    return false;
}

bool __thiscall CPedIntelligence::HasUnk2D0(void* ptr)
{
    auto* arr = reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x2D0);
    for (int32 i = 0; i < 3; ++i) {
        if (arr[i] == ptr) {
            return true;
        }
    }
    return false;
}

void __thiscall CPedIntelligence::UpdateUnk2D0Nearby()
{
    auto* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    if ((*reinterpret_cast<uint32*>(ped + 0x29C) & 0x300000u) != 0) {
        return;
    }
    if (!HasAnyUnk2D0()) {
        return;
    }
    if (Core::Hook::CallMethod<bool>(0x94E7D0, ped + 0xBC0)) {
        return;
    }
    if (!Core::Hook::CallMethod<bool>(0x9E6BD0, ped, 0)) {
        return;
    }

    const uint32 r = Core::Hook::Call<uint32>(0xD0D21A) & 0xFFFFu;
    const int32 chance = static_cast<int32>(
        static_cast<float>(r) * Core::StaticRef<float>(0xEBE1D8) * Core::StaticRef<float>(0xD8F340)
    );
    if (chance != 0x32) {
        return;
    }

    int32 count = -1;
    const float radius = Core::StaticRef<float>(0xD50960);
    const void* pos = *reinterpret_cast<uint8**>(ped + 0x20) + 0x30;
    auto** results = reinterpret_cast<void**>(Core::Rebase(0x1618120));
    Core::Hook::Call<void>(0x81CA00, pos, radius, 0, &count, 0x400, results, 0, 1, 1, 1, 0);

    int32 hits = 0;
    for (int32 i = 0; i < count; ++i) {
        if (HasUnk2D0(results[i])) {
            ++hits;
        }
    }
    if (hits != 0) {
        Core::Hook::Call<int32>(0xD0D21A);
        Core::Hook::Call<int32>(0xD0D21A);
    }
}

void __cdecl CPedIntelligence::ResetUnk152EBE0()
{
    Core::Hook::Call<void>(0x91B9C0);
}

void __thiscall CPedIntelligence::AddSubUnk2C8(float delta)
{
    auto* valPtr = reinterpret_cast<int8*>(reinterpret_cast<uint8*>(this) + 0x2C8);
    const int8 deltaByte = static_cast<int8>(static_cast<int32>(delta));
    int32 next = static_cast<int32>(*valPtr) - static_cast<int32>(deltaByte);
    if (next <= 0) {
        next = 0;
    } else if (next >= 100) {
        next = 100;
    }
    *valPtr = static_cast<int8>(next);
}

void __thiscall CPedIntelligence::SetUnk2C8(float val)
{
    auto* valPtr = reinterpret_cast<int8*>(reinterpret_cast<uint8*>(this) + 0x2C8);
    const int8 valByte = static_cast<int8>(static_cast<int32>(val));
    int32 clamped = static_cast<int32>(valByte);
    if (clamped <= 0) {
        clamped = 0;
    } else if (clamped >= 100) {
        clamped = 100;
    }
    *valPtr = static_cast<int8>(clamped);
}

void __thiscall CPedIntelligence::SetUnk2C4_2CC(bool flag, float val)
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto* flags = reinterpret_cast<uint32*>(self + 0x2CC);
    *flags ^= (static_cast<uint32>(flag ? 1 : 0) ^ *flags) & 1u;
    *reinterpret_cast<float*>(self + 0x2C4) = val;
    SetUnk2C8(Core::StaticRef<float>(0xD8F340));
}

void* __thiscall CPedIntelligenceSub10::InitSub0()
{
    auto* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<float*>(self + 0x00) = 0.0f;
    *reinterpret_cast<float*>(self + 0x04) = 0.0f;
    *reinterpret_cast<float*>(self + 0x08) = 0.0f;
    *reinterpret_cast<float*>(self + 0x10) = 0.0f;
    *reinterpret_cast<float*>(self + 0x14) = 0.0f;
    *reinterpret_cast<float*>(self + 0x18) = 0.0f;
    *reinterpret_cast<uint32*>(self + 0x28) &= ~3u;
    *reinterpret_cast<float*>(self + 0x20) = 0.0f;
    *reinterpret_cast<void**>(self + 0x24) = nullptr;
    return self;
}

void __thiscall CPedIntelligenceSub10::ClearSub0_24()
{
    auto* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<uint32*>(self + 0x28) &= ~1u;
    auto** ptr = reinterpret_cast<void**>(self + 0x24);
    if (*ptr) {
        Core::Hook::CallMethod<void>(0x9E6580, *ptr, ptr);
        *ptr = nullptr;
    }
}

void* __thiscall CPedIntelligence::GetTask141_144Unk20(uint32* outVal)
{
    uint8* task = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x4C);
    if (!task) {
        return nullptr;
    }
    using GetTypeFn = int32(__thiscall*)(void*);
    void** vtable = *reinterpret_cast<void***>(task);
    const int32 type = reinterpret_cast<GetTypeFn>(vtable[3])(task);
    if (type != 0x142 && type != 0x143 && type != 0x144 && type != 0x141) {
        return nullptr;
    }
    using GetValFn = uint32(__thiscall*)(void*);
    *outVal = reinterpret_cast<GetValFn>(vtable[21])(task);
    return task + 0x20;
}

bool __thiscall CPedIntelligence::IsTargetInViewCone(const CVector* targetPos, float minDist, float maxScore, float maxDist, float minScore)
{
    auto* self = reinterpret_cast<uint8*>(this);
    if (maxDist < 0.0f) {
        maxDist = *reinterpret_cast<float*>(self + 0xE4);
    }

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    const auto* mat = *reinterpret_cast<const CMatrix* const*>(ped + 0x20);
    const float dx = targetPos->x - mat->pos.x;
    const float dy = targetPos->y - mat->pos.y;
    const float dz = targetPos->z - mat->pos.z;
    const float distSq = dx * dx + dy * dy + dz * dz;
    const float dist = std::sqrt(distSq);
    if (maxDist <= dist) {
        return false;
    }

    const float minClamp = Core::StaticRef<float>(0xD58488);
    float clamped = minDist;
    if (minClamp > clamped) {
        clamped = minClamp;
    } else if (clamped > maxDist) {
        clamped = maxDist;
    }

    if (clamped > dist) {
        const float one = Core::StaticRef<float>(0xD50398);
        minScore = (one - (dist / clamped)) * (maxScore - minScore) + minScore;
    }

    const float invDist = Core::Hook::Call<float>(0x5BB1F0, distSq);
    const float dot = mat->up.z * (dz * invDist)
                    + mat->up.y * (dy * invDist)
                    + mat->up.x * (dx * invDist);
    return dot > minScore;
}

void* __thiscall CPedIntelligence::FindTaskInAllSlots(int32 taskType)
{
    uint8* taskMgr = reinterpret_cast<uint8*>(this) + 0x44;
    static constexpr int32 kSlots[] = { 4, 3, 1, 2, 0, 0 };
    for (int32 slot : kSlots) {
        if (void* task = Core::Hook::CallMethod<void*>(0x9E5660, taskMgr, slot, taskType)) {
            return task;
        }
    }
    return FindTaskInPrimarySlots(taskType);
}


/*
    Slice 2 (0x9C3000..0x9C41FF) method declarations for PedIntelligence.h:

    struct CPedIntelligenceSub10
    {
        CVector4 m_vecMin;      // +0x00..+0x0F
        CVector4 m_vecMax;      // +0x10..+0x1F
        float    m_fRadius;     // +0x20
        void*    m_pEntity;     // +0x24
        uint32   m_nFlags;      // +0x28

        void __thiscall Dtor();                                                                                             // 0x9C3660
        bool __thiscall GetBounds(CVector4* outMin, CVector4* outMax, float* outRadius);                                    // 0x9C3670
        bool __thiscall IsPointInside(const CVector4* point, uint32 unused);                                                // 0x9C3930
        void __thiscall SetBox(const CVector4* minPos, const CVector4* maxPos, float width, void* entity, bool isLocal);    // 0x9C3CD0
        void __thiscall SetSphere(const CVector4* center, float radius, void* entity);                                      // 0x9C3DB0
        void __thiscall GetCenterAndRadiusSq(CVector4* outCenter, float* outRadiusSq);                                      // 0x9C3E20
    };
    VALIDATE_SIZE(CPedIntelligenceSub10, 0x2C);

    // CPedIntelligence methods (Slice 2: 0x9C3000..0x9C41FF)
    void* __thiscall GetActiveTask518();                                    // 0x9C3040
    bool __thiscall IsClimbingOrVaulting();                                 // 0x9C3090
    void* __thiscall GetActiveTaskFE();                                     // 0x9C30C0
    void* __thiscall GetActiveTask120();                                    // 0x9C3110
    void __thiscall ProcessFirst();                                         // 0x9C3160
    void __thiscall ProcessSecond();                                        // 0x9C31D0
    void __thiscall UpdateStuckTimer();                                     // 0x9C3260
    void __thiscall SetVec290(const CVector4* vec);                         // 0x9C34D0
    void __thiscall UpdatePosCheck2B0(const CVector4* vec);                 // 0x9C3510
    int32 __thiscall GetAlertnessLevel();                                   // 0x9C3640
    CPedIntelligence* __thiscall Ctor(void* ped);                           // 0x9C3F40
    void __thiscall Dtor();                                                 // 0x9C4160
    void __thiscall SetDecisionMakerType(int32 type);                       // 0x9C41F0
*/

static void InjectPedIntelligenceSlice2Hooks()
{
    Core::Hook::Install(0x9C3040, &CPedIntelligence::GetActiveTask518);
    Core::Hook::Install(0x9C3090, &CPedIntelligence::IsClimbingOrVaulting);
    Core::Hook::Install(0x9C30C0, &CPedIntelligence::GetActiveTaskFE);
    Core::Hook::Install(0x9C3110, &CPedIntelligence::GetActiveTask120);
    Core::Hook::Install(0x9C3160, &CPedIntelligence::ProcessFirst);
    Core::Hook::Install(0x9C31D0, &CPedIntelligence::ProcessSecond);
    Core::Hook::Install(0x9C3260, &CPedIntelligence::UpdateStuckTimer);
    Core::Hook::Install(0x9C34D0, &CPedIntelligence::SetVec290);
    Core::Hook::Install(0x9C3510, &CPedIntelligence::UpdatePosCheck2B0);
    Core::Hook::Install(0x9C3640, &CPedIntelligence::GetAlertnessLevel);
    Core::Hook::Install(0x9C3660, &CPedIntelligenceSub10::Dtor);
    Core::Hook::Install(0x9C3670, &CPedIntelligenceSub10::GetBounds);
    Core::Hook::Install(0x9C3930, &CPedIntelligenceSub10::IsPointInside);
    Core::Hook::Install(0x9C3CD0, &CPedIntelligenceSub10::SetBox);
    Core::Hook::Install(0x9C3DB0, &CPedIntelligenceSub10::SetSphere);
    Core::Hook::Install(0x9C3E20, &CPedIntelligenceSub10::GetCenterAndRadiusSq);
    Core::Hook::Install(0x9C3F40, &CPedIntelligence::Ctor);
    Core::Hook::Install(0x9C4160, &CPedIntelligence::Dtor);
    Core::Hook::Install(0x9C41F0, &CPedIntelligence::SetDecisionMakerType);
}

// 0x9C3040
void* __thiscall CPedIntelligence::GetActiveTask518()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void** tasks = reinterpret_cast<void**>(self + 0x44);

    void* task = nullptr;
    for (int32 i = 0; i < 5; ++i)
    {
        if (tasks[i] != nullptr)
        {
            task = tasks[i];
            break;
        }
    }

    if (task == nullptr)
        return nullptr;

    void* leaf = task;
    while (void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(leaf) + 0x08))
        leaf = sub;

    if (leaf == nullptr)
        return nullptr;

    const int32 type = reinterpret_cast<int32(__thiscall*)(void*)>((*reinterpret_cast<void***>(leaf))[3])(leaf);
    return (type == 0x518) ? leaf : nullptr;
}

// 0x9C3090
bool __thiscall CPedIntelligence::IsClimbingOrVaulting()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);

    if (sub6C != nullptr && sub6C[0x0E] != 0)
        return Core::Hook::CallMethod<bool>(0x426910, self + 0x2E0, 0x518, static_cast<uint8>(0));

    return ((*reinterpret_cast<uint32*>(ped + 0x2A0) >> 11) & 1u) != 0;
}

// 0x9C30C0
void* __thiscall CPedIntelligence::GetActiveTaskFE()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void** tasks = reinterpret_cast<void**>(self + 0x44);

    void* task = nullptr;
    for (int32 i = 0; i < 5; ++i)
    {
        if (tasks[i] != nullptr)
        {
            task = tasks[i];
            break;
        }
    }

    if (task == nullptr)
        return nullptr;

    void* leaf = task;
    while (void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(leaf) + 0x08))
        leaf = sub;

    if (leaf == nullptr)
        return nullptr;

    const int32 type = reinterpret_cast<int32(__thiscall*)(void*)>((*reinterpret_cast<void***>(leaf))[3])(leaf);
    return (type == 0xFE) ? leaf : nullptr;
}

// 0x9C3110
void* __thiscall CPedIntelligence::GetActiveTask120()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void** tasks = reinterpret_cast<void**>(self + 0x44);

    void* task = nullptr;
    for (int32 i = 0; i < 5; ++i)
    {
        if (tasks[i] != nullptr)
        {
            task = tasks[i];
            break;
        }
    }

    if (task == nullptr)
        return nullptr;

    void* leaf = task;
    while (void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(leaf) + 0x08))
        leaf = sub;

    if (leaf == nullptr)
        return nullptr;

    const int32 type = reinterpret_cast<int32(__thiscall*)(void*)>((*reinterpret_cast<void***>(leaf))[3])(leaf);
    return (type == 0x120) ? leaf : nullptr;
}

// 0x9C3160
void __thiscall CPedIntelligence::ProcessFirst()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    Core::Hook::CallMethod<void>(0x89BFA0, ped + 0x27C, ped);

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);
    if (sub6C != nullptr && sub6C[0x0E] != 0)
        return;

    void** tasks = reinterpret_cast<void**>(self + 0x44);
    void* task = nullptr;
    for (int32 i = 0; i < 5; ++i)
    {
        if (tasks[i] != nullptr)
        {
            task = tasks[i];
            break;
        }
    }

    if (task == nullptr)
        return;

    void* leaf = task;
    while (void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(leaf) + 0x08))
        leaf = sub;

    if (leaf != nullptr)
    {
        if (reinterpret_cast<bool(__thiscall*)(void*)>((*reinterpret_cast<void***>(leaf))[2])(leaf))
            reinterpret_cast<void(__thiscall*)(void*, void*)>((*reinterpret_cast<void***>(leaf))[18])(leaf, *reinterpret_cast<void**>(self + 0x40));
    }
}

// 0x9C31D0
void __thiscall CPedIntelligence::ProcessSecond()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);

    if (sub6C == nullptr || sub6C[0x0E] == 0)
    {
        void** tasks = reinterpret_cast<void**>(self + 0x44);
        void* task = nullptr;
        for (int32 i = 0; i < 5; ++i)
        {
            if (tasks[i] != nullptr)
            {
                task = tasks[i];
                break;
            }
        }

        if (task != nullptr)
        {
            void* leaf = task;
            while (void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(leaf) + 0x08))
                leaf = sub;

            if (leaf != nullptr)
            {
                if (reinterpret_cast<bool(__thiscall*)(void*)>((*reinterpret_cast<void***>(leaf))[2])(leaf))
                    reinterpret_cast<void(__thiscall*)(void*, void*)>((*reinterpret_cast<void***>(leaf))[19])(leaf, *reinterpret_cast<void**>(self + 0x40));
            }
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if ((ped[0x26C] & 0x04) != 0)
    {
        uint8* veh = *reinterpret_cast<uint8**>(ped + 0xB40);
        if (veh != nullptr && *reinterpret_cast<int32*>(veh + 0x1350) == 1)
            Core::Hook::CallMethod<void>(0xAAAF20, veh, ped);
    }
}

// 0x9C3260
void __thiscall CPedIntelligence::UpdateStuckTimer()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* intel = *reinterpret_cast<uint8**>(ped + 0x224);
    const uint32 moveState = *reinterpret_cast<uint32*>(intel + 0x2E8) & 7u;

    int32 taskType = -1;
    uint8 outFlag = 0;
    const bool hasTask = Core::Hook::CallMethod<bool>(0x9C2720, self, &taskType, &outFlag);
    const bool isSpecialTask = hasTask && (taskType == 0x0F || taskType == 0x0E || taskType == 0x02);

    if (moveState == 2 || moveState == 3 || moveState == 4)
    {
        if (!isSpecialTask)
        {
            uint8* scanner = *reinterpret_cast<uint8**>((*reinterpret_cast<uint8**>(self + 0x40)) + 0x78);
            uint8* ev = Core::Hook::CallMethod<uint8*>(0x9EC060, scanner, 0, 4);
            while (ev != nullptr)
            {
                if (*reinterpret_cast<float*>(ev + 0x58) >= 0.9f)
                {
                    const uint32 flags = *reinterpret_cast<uint32*>(ev + 0x04);
                    if (((flags >> 21) & 1u) == 0)
                    {
                        uint8* evEntity = (*reinterpret_cast<uint16*>(ev + 0x44) == 1) ? *reinterpret_cast<uint8**>(ev + 0x40) : nullptr;
                        if (evEntity != nullptr && (evEntity[0x06] & 0x10) != 0)
                        {
                            const uint32 lowNibble = flags & 0x0Fu;
                            if (lowNibble == 0 || Core::Hook::Call<bool>(0xA4BBA0, lowNibble, 0))
                            {
                                *reinterpret_cast<uint32*>(self + 0x264) = 0;
                                return;
                            }
                        }
                    }

                    if ((flags & ((1u << 9) | (1u << 10) | (1u << 11) | (1u << 12))) != 0)
                    {
                        *reinterpret_cast<uint32*>(self + 0x264) = 0;
                        return;
                    }
                }

                scanner = *reinterpret_cast<uint8**>((*reinterpret_cast<uint8**>(self + 0x40)) + 0x78);
                ev = Core::Hook::CallMethod<uint8*>(0x9EC0F0, scanner, 0, 4);
            }
        }
    }
    else
    {
        if (!isSpecialTask)
        {
            uint32& timer268 = *reinterpret_cast<uint32*>(self + 0x268);
            timer268 += 1;
            if (timer268 == 0xFFFFFFFFu)
                timer268 = 4;
            *reinterpret_cast<uint32*>(self + 0x264) = 0;
            return;
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if (*reinterpret_cast<uint32*>(ped + 0x16C) != 0)
    {
        if (*reinterpret_cast<uint32*>(self + 0x268) > 4)
        {
            const float* pos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
            float* dst = reinterpret_cast<float*>(self + 0x270);
            dst[0] = pos[0];
            dst[1] = pos[1];
            dst[2] = pos[2];
            dst[3] = pos[3];
        }
        *reinterpret_cast<uint32*>(self + 0x268) = 0;
    }
    else
    {
        uint32& timer268 = *reinterpret_cast<uint32*>(self + 0x268);
        timer268 += 1;
        if (timer268 == 0xFFFFFFFFu)
            timer268 = 4;
    }

    if (*reinterpret_cast<uint32*>(self + 0x268) > 4)
    {
        *reinterpret_cast<uint32*>(self + 0x264) = 0;
        return;
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    const float* pos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
    const float dx = pos[0] - *reinterpret_cast<float*>(self + 0x270);
    const float dy = pos[1] - *reinterpret_cast<float*>(self + 0x274);
    const float distSq = dx * dx + dy * dy;
    const float thresh = (ped[0x29C] & 0x04) ? 0.125f : 0.0625f;

    if (distSq < thresh)
    {
        *reinterpret_cast<uint32*>(self + 0x264) += 1;
    }
    else
    {
        *reinterpret_cast<uint32*>(self + 0x264) = 0;
        float* dst = reinterpret_cast<float*>(self + 0x270);
        dst[0] = pos[0];
        dst[1] = pos[1];
        dst[2] = pos[2];
        dst[3] = pos[3];
    }
}

// 0x9C34D0
void __thiscall CPedIntelligence::SetVec290(const CVector4* vec)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<CVector4*>(self + 0x290) = *vec;
    *reinterpret_cast<uint32*>(self + 0x2A0) = Core::StaticRef<uint32>(0x11DDE74);
}

// 0x9C3510
void __thiscall CPedIntelligence::UpdatePosCheck2B0(const CVector4* vec)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    CVector4* stored = reinterpret_cast<CVector4*>(self + 0x2B0);

    if (std::fabs(vec->x - stored->x) < 0.1f &&
        std::fabs(vec->y - stored->y) < 0.1f &&
        std::fabs(vec->z - stored->z) < 2.0f)
    {
        int32& count = *reinterpret_cast<int32*>(self + 0x2C0);
        count += 1;
        if (count >= 3)
        {
            Core::Hook::Call<void>(0x939570, vec);
            stored->x = 0.0f;
            stored->y = 0.0f;
            stored->z = 0.0f;
            count = 0;
        }
    }
    else
    {
        *stored = *vec;
        *reinterpret_cast<int32*>(self + 0x2C0) = 1;
    }
}

// 0x9C3640
int32 __thiscall CPedIntelligence::GetAlertnessLevel()
{
    const float val = Core::Hook::CallMethod<float>(0x9C2B10, this);
    return Core::Hook::Call<int32>(0xA28660, val);
}

// 0x9C3660
void __thiscall CPedIntelligenceSub10::Dtor()
{
    Core::Hook::CallMethod<void>(0x9C2E10, this);
}

// 0x9C3670
bool __thiscall CPedIntelligenceSub10::GetBounds(CVector4* outMin, CVector4* outMax, float* outRadius)
{
    if ((m_nFlags & 1u) == 0)
        return false;

    if (m_pEntity != nullptr)
    {
        uint8* entity = reinterpret_cast<uint8*>(m_pEntity);
        if ((m_nFlags & 2u) != 0)
        {
            if (*reinterpret_cast<void**>(entity + 0x20) == nullptr)
            {
                Core::Hook::CallMethod<void>(0x9EA460, entity);
                Core::Hook::CallMethod<void>(0x9E9440, entity + 0x10, *reinterpret_cast<void**>(entity + 0x20));
            }

            const float* m = *reinterpret_cast<const float**>(entity + 0x20);
            outMin->x = m[0] * m_vecMin.x + m[4] * m_vecMin.y + m[8]  * m_vecMin.z;
            outMin->y = m[1] * m_vecMin.x + m[5] * m_vecMin.y + m[9]  * m_vecMin.z;
            outMin->z = m[2] * m_vecMin.x + m[6] * m_vecMin.y + m[10] * m_vecMin.z;

            outMax->x = m[0] * m_vecMax.x + m[4] * m_vecMax.y + m[8]  * m_vecMax.z;
            outMax->y = m[1] * m_vecMax.x + m[5] * m_vecMax.y + m[9]  * m_vecMax.z;
            outMax->z = m[2] * m_vecMax.x + m[6] * m_vecMax.y + m[10] * m_vecMax.z;
        }
        else
        {
            *outMin = m_vecMin;
            *outMax = m_vecMax;
        }

        const uint8* mat = *reinterpret_cast<const uint8* const*>(entity + 0x20);
        const float* pos = mat ? reinterpret_cast<const float*>(mat + 0x30) : reinterpret_cast<const float*>(entity + 0x10);
        outMin->x += pos[0];
        outMin->y += pos[1];
        outMin->z += pos[2];

        outMax->x += pos[0];
        outMax->y += pos[1];
        outMax->z += pos[2];

        *outRadius = m_fRadius;
        return true;
    }

    *outMin = m_vecMin;
    *outMax = m_vecMax;
    *outRadius = m_fRadius;
    return true;
}

// 0x9C3930
bool __thiscall CPedIntelligenceSub10::IsPointInside(const CVector4* point, uint32 /*unused*/)
{
    if ((m_nFlags & 1u) == 0)
        return false;

    if ((m_nFlags & 4u) != 0)
    {
        CVector4 minV{}, maxV{};
        float radius = 0.0f;
        GetBounds(&minV, &maxV, &radius);

        const float dx = minV.x - point->x;
        const float dy = minV.y - point->y;
        const float dz = minV.z - point->z;
        return (radius * radius) > (dx * dx + dy * dy + dz * dz);
    }

    CVector4 minV{}, maxV{};
    float width = 0.0f;
    GetBounds(&minV, &maxV, &width);

    float angle = Core::Hook::Call<float>(0xA4E320, minV.x, minV.y, maxV.x, maxV.y) + 1.57079637f;
    constexpr float kTwoPi = 6.28318548f;
    while (angle < 0.0f)
        angle += kTwoPi;
    while (angle > kTwoPi)
        angle -= kTwoPi;

    const float halfW = width * 0.5f;
    const float sinA = static_cast<float>(std::sin(static_cast<double>(angle)));
    const float cosA = static_cast<float>(std::cos(static_cast<double>(angle)));

    const float c1x = minV.x + sinA * halfW;
    const float c1y = minV.y - cosA * halfW;

    const float segX = maxV.x - minV.x;
    const float segY = maxV.y - minV.y;
    const float segLenSq = segX * segX + segY * segY;
    const float segLen = std::sqrt(segLenSq);

    const float perpX = minV.x - c1x;
    const float perpY = minV.y - c1y;
    const float perpLenSq = perpX * perpX + perpY * perpY;
    const float halfLen = std::sqrt(perpLenSq);

    const float ptDx = point->x - minV.x;
    const float ptDy = point->y - minV.y;

    const float invSegLen = (segLenSq > 0.0f) ? (1.0f / std::sqrt(segLenSq)) : 0.0f;
    const float segDirX = segX * invSegLen;
    const float segDirY = segY * invSegLen;
    const float projAlong = ptDx * segDirX + ptDy * segDirY;
    if (projAlong < 0.0f || projAlong > segLen)
        return false;

    const float invPerpLen = (perpLenSq > 0.0f) ? (1.0f / std::sqrt(perpLenSq)) : 0.0f;
    const float perpDirX = perpX * invPerpLen;
    const float perpDirY = perpY * invPerpLen;
    const float projPerp = std::fabs(ptDx * perpDirX + ptDy * perpDirY);
    if (projPerp > halfLen)
        return false;

    if (point->z < minV.z || point->z > maxV.z)
        return false;

    return true;
}

// 0x9C3CD0
void __thiscall CPedIntelligenceSub10::SetBox(const CVector4* minPos, const CVector4* maxPos, float width, void* entity, bool isLocal)
{
    Core::Hook::CallMethod<void>(0x9C2E10, this);

    if (entity != nullptr)
    {
        m_pEntity = entity;
        Core::Hook::CallMethod<void>(0x9E6550, entity, &m_pEntity);
    }

    m_nFlags = (m_nFlags & ~0x02u) | ((isLocal ? 1u : 0u) << 1) | 0x01u;
    m_vecMin = *minPos;
    m_vecMax = *maxPos;
    m_fRadius = width;

    if (m_vecMin.z > m_vecMax.z)
    {
        const CVector4 tmp = m_vecMin;
        m_vecMin = m_vecMax;
        m_vecMax = tmp;
    }

    m_nFlags &= ~0x04u;
}

// 0x9C3DB0
void __thiscall CPedIntelligenceSub10::SetSphere(const CVector4* center, float radius, void* entity)
{
    Core::Hook::CallMethod<void>(0x9C2E10, this);

    if (entity != nullptr)
    {
        m_pEntity = entity;
        Core::Hook::CallMethod<void>(0x9E6550, entity, &m_pEntity);
    }

    m_nFlags |= 0x01u;
    m_vecMin = *center;
    m_fRadius = radius;
    m_vecMax.x = 0.0f;
    m_vecMax.y = 0.0f;
    m_vecMax.z = 0.0f;
    m_nFlags |= 0x04u;
}

// 0x9C3E20
void __thiscall CPedIntelligenceSub10::GetCenterAndRadiusSq(CVector4* outCenter, float* outRadiusSq)
{
    CVector4 minV{}, maxV{};
    float radius = 0.0f;
    GetBounds(&minV, &maxV, &radius);

    if ((m_nFlags & 0x04u) != 0)
    {
        *outCenter = minV;
        *outRadiusSq = radius * radius;
        return;
    }

    outCenter->x = (minV.x + maxV.x) * 0.5f;
    outCenter->y = (minV.y + maxV.y) * 0.5f;
    outCenter->z = (minV.z + maxV.z) * 0.5f;
    outCenter->w = maxV.w;

    const float dx = minV.x - outCenter->x;
    const float dy = minV.y - outCenter->y;
    const float dz = minV.z - outCenter->z;
    const float hr = radius * 0.5f;
    *outRadiusSq = hr * hr + dz * dz + dy * dy + dx * dx;
}

// 0x9C3F40
CPedIntelligence* __thiscall CPedIntelligence::Ctor(void* ped)
{
    uint8* self = reinterpret_cast<uint8*>(this);

    *reinterpret_cast<uintptr_t*>(self + 0x00) = Core::Rebase(0xD81CE8);
    Core::Hook::CallMethod<void>(0x9C2DD0, self + 0x10);

    *reinterpret_cast<void**>(self + 0x40) = ped;
    Core::Hook::CallMethod<void>(0x9E55A0, self + 0x44, ped);
    Core::Hook::CallMethod<void>(0xAA7A90, self + 0x84, ped);

    *reinterpret_cast<float*>(self + 0xE0) = 15.0f;
    *reinterpret_cast<float*>(self + 0xE4) = 15.0f;
    *reinterpret_cast<int32*>(self + 0xD0) = -1;
    *reinterpret_cast<uint32*>(self + 0xD4) = 0;
    *reinterpret_cast<uint32*>(self + 0xD8) = 12;
    *reinterpret_cast<uint32*>(self + 0xDC) = 7;
    *reinterpret_cast<uint32*>(self + 0xE8) = 3;
    *reinterpret_cast<float*>(self + 0xEC) = 10.0f;
    *reinterpret_cast<uint8*>(self + 0xF0) = 0;
    *reinterpret_cast<uint8*>(self + 0xF1) = 0;

    Core::Hook::CallMethod<void>(0xA52920, self + 0xF4);
    Core::Hook::CallMethod<void>(0xA52960, self + 0x150);
    Core::Hook::CallMethod<void>(0xA50CA0, self + 0x1AC, 5);
    *reinterpret_cast<uintptr_t*>(self + 0x1AC) = Core::Rebase(0xD81CD8);

    *reinterpret_cast<uint8*>(self + 0x208) = 0;
    *reinterpret_cast<uint32*>(self + 0x20C) = Core::StaticRef<uint32>(0x11DDE74);
    *reinterpret_cast<uint32*>(self + 0x210) = 0;
    *reinterpret_cast<uint8*>(self + 0x214) = 1;
    *reinterpret_cast<uint8*>(self + 0x215) = 0;
    *reinterpret_cast<uint8*>(self + 0x21E) = 0;
    *reinterpret_cast<uint8*>(self + 0x220) = 0;

    Core::Hook::CallMethod<void>(0xA50C80, self + 0x230);
    Core::Hook::CallMethod<void>(0x89DEB0, self + 0x250, 0x0B);
    Core::Hook::Call<void>(0x89DED0, self + 0x250);

    *reinterpret_cast<float*>(self + 0x288) = 9999.0f;
    *reinterpret_cast<uint32*>(self + 0x25C) = 0;
    *reinterpret_cast<uintptr_t*>(self + 0x260) = Core::Rebase(0xC88280);
    *reinterpret_cast<uint32*>(self + 0x264) = 0;
    *reinterpret_cast<uint32*>(self + 0x268) = 0;
    *reinterpret_cast<uint32*>(self + 0x280) = 0;
    *reinterpret_cast<uint32*>(self + 0x284) = 0;
    *reinterpret_cast<uint32*>(self + 0x28C) = Core::StaticRef<uint32>(0x11DDE74);

    *reinterpret_cast<float*>(self + 0x290) = 9999.0f;
    *reinterpret_cast<float*>(self + 0x294) = 9999.0f;
    *reinterpret_cast<float*>(self + 0x298) = 9999.0f;
    *reinterpret_cast<uint32*>(self + 0x2A0) = 0;

    *reinterpret_cast<float*>(self + 0x2B0) = 9999.0f;
    *reinterpret_cast<float*>(self + 0x2B4) = 9999.0f;
    *reinterpret_cast<float*>(self + 0x2B8) = 9999.0f;

    *reinterpret_cast<uint32*>(self + 0x2CC) &= ~1u;
    *reinterpret_cast<uint32*>(self + 0x2C0) = 0;
    *reinterpret_cast<float*>(self + 0x2C4) = 0.0f;
    *reinterpret_cast<uint8*>(self + 0x2C8) = 0;

    Core::Hook::CallMethod<void>(0x9A44A0, self + 0x2E0);

    uint8* pedData = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(ped) + 0x21C);
    const uint32 pedType = *reinterpret_cast<uint32*>(pedData + 0x12C);
    if (pedType - 3u <= 0x0Bu)
    {
        *reinterpret_cast<float*>(self + 0xE4) = 40.0f;
        *reinterpret_cast<float*>(self + 0xE0) = 40.0f;
    }

    *reinterpret_cast<uint32*>(self + 0x2D0) = 0;
    *reinterpret_cast<uint32*>(self + 0x2D4) = 0;
    *reinterpret_cast<uint32*>(self + 0x2D8) = 0;

    return this;
}

// 0x9C4160
void __thiscall CPedIntelligence::Dtor()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<uintptr_t*>(self + 0x00) = Core::Rebase(0xD81CE8);

    void* obj25C = *reinterpret_cast<void**>(self + 0x25C);
    if (obj25C != nullptr)
    {
        reinterpret_cast<void(__thiscall*)(void*, int32)>((*reinterpret_cast<void***>(obj25C))[0])(obj25C, 1);
        *reinterpret_cast<void**>(self + 0x25C) = nullptr;
    }

    Core::Hook::CallMethod<void>(0x9A69C0, self + 0x2E0);
    Core::Hook::Call<void>(0x89DF30, self + 0x250);
    Core::Hook::CallMethod<void>(0x8A5690, self + 0x250);

    *reinterpret_cast<uintptr_t*>(self + 0x1AC) = Core::Rebase(0xD81CD8);
    Core::Hook::CallMethod<void>(0xA52900, self + 0x1AC);
    Core::Hook::CallMethod<void>(0xA52980, self + 0x150);
    Core::Hook::CallMethod<void>(0xA52940, self + 0xF4);
    Core::Hook::CallMethod<void>(0xAA80C0, self + 0x84);
    Core::Hook::CallMethod<void>(0x9E55E0, self + 0x44);
    Core::Hook::CallMethod<void>(0x9C3660, self + 0x10);
}

// 0x9C41F0
void __thiscall CPedIntelligence::SetDecisionMakerType(int32 type)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int32*>(self + 0xD8) = type;

    auto getDecisionMakerMgr = []() -> void*
    {
        void* mgr = Core::StaticRef<void*>(0x16C3454);
        if (mgr == nullptr)
        {
            void* mem = Core::Hook::Call<void*>(0x4011D0, 0x20020);
            mgr = mem ? Core::Hook::CallMethod<void*>(0x804720, mem) : nullptr;
            Core::StaticRef<void*>(0x16C3454) = mgr;
        }
        return mgr;
    };

    bool setSpecial = false;
    if (Core::StaticRef<int32>(0x10F8044) == 3 && Core::Hook::Call<bool>(0x462F90))
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        uint8* info = reinterpret_cast<uint8*(__thiscall*)(uint8*)>((*reinterpret_cast<void***>(ped))[0xD0 / 4])(ped);
        if (Core::Hook::Call<bool>(0x88ECB0, *reinterpret_cast<uint32*>(info + 0x12C)))
        {
            *reinterpret_cast<uint8*>(*reinterpret_cast<uint8**>(self + 0x40) + 0x389) = 0x14;
            setSpecial = true;
        }
    }

    if (!setSpecial)
    {
        void* mgr = getDecisionMakerMgr();
        uint8* entry = Core::Hook::CallMethod<uint8*>(0xB5B580, mgr, *reinterpret_cast<int32*>(self + 0xD8));
        int32 val = static_cast<int32>(entry[0x8FB]);
        if (val < 0)
            val = 0;
        else if (val > 100)
            val = 100;

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        *reinterpret_cast<uint8*>(ped + 0x2B0 + 0xD9) = static_cast<uint8>(val);
    }

    void* mgr = getDecisionMakerMgr();
    uint8* entry = Core::Hook::CallMethod<uint8*>(0xB5B580, mgr, *reinterpret_cast<int32*>(self + 0xD8));
    const float senseRange = static_cast<float>(entry[0x8FA]);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    Core::Hook::CallMethod<void>(0x9C2310, *reinterpret_cast<void**>(ped + 0x224), senseRange);
}


// ============================================================================
// CPedIntelligence Slice 3 (0x9C4200..0x9C515F)
// Method declarations for CPedIntelligence:
//
//   void* __thiscall GetOrCreateSub25C(bool createIfMissing);                // 0x9C4300 (ret 0x4)
//   void* __thiscall UpdateSub25C();                                         // 0x9C4350 (ret)
//   void  __thiscall SetSub25CTarget(void* target, uint32 arg2);             // 0x9C43A0 (ret 0x8)
//   void  __thiscall Reset(bool bResetEvents, bool bUpdateTasks);            // 0x9C4400 (ret 0x8)
//   void  __thiscall Process();                                              // 0x9C4590 (ret)
//   void  __thiscall UpdateTaskMotionFlags();                                // 0x9C4910 (ret)
//   void  __thiscall ProcessTasks();                                         // 0x9C4940 (ret)
//   static void __cdecl ShutdownPoolAndClearEvents();                        // 0x9C4B10 (ret)
//   void  __thiscall AdjustSub2C8(float scale);                              // 0x9C4B60 (ret 0x4)
//   void* __thiscall CreateDefaultTask200(uint32 unused);                    // 0x9C4C50 (ret 0x4)
//   bool  __thiscall CheckRelationshipBit(int32 relType, int32 otherGroup);  // 0x9C4C70 (ret 0x8, ECX = CPed*)
//   void* __thiscall ScalarDeletingDtor(uint8 flags);                        // 0x9C4D10 (ret 0x4)
//   bool  __thiscall IsAnySeenPedHatedInRange();                             // 0x9C4D40 (ret)
//   void  __thiscall ProcessDeath(bool bCreateDeadTask);                     // 0x9C4DF0 (ret 0x4)
//   bool  __thiscall IsFriendlyWithPed(void* otherPed);                      // 0x9C4EF0 (ret 0x4)
//   bool  __thiscall IsPedThreatening(void* otherPed, bool bCheckDislike);   // 0x9C4FC0 (ret 0x8)
//   static bool __cdecl ArePedsFriendly(void* pedA, void* pedB);           // 0x9C5120 (ret)
// ============================================================================

static void InjectPedIntelligenceSlice3Hooks()
{
    Core::Hook::Install(0x9C4300, &CPedIntelligence::GetOrCreateSub25C);
    Core::Hook::Install(0x9C4350, &CPedIntelligence::UpdateSub25C);
    Core::Hook::Install(0x9C43A0, &CPedIntelligence::SetSub25CTarget);
    Core::Hook::Install(0x9C4400, &CPedIntelligence::Reset);
    Core::Hook::Install(0x9C4590, &CPedIntelligence::Process);
    Core::Hook::Install(0x9C4910, &CPedIntelligence::UpdateTaskMotionFlags);
    Core::Hook::Install(0x9C4940, &CPedIntelligence::ProcessTasks);
    Core::Hook::Install(0x9C4B10, &CPedIntelligence::ShutdownPoolAndClearEvents);
    Core::Hook::Install(0x9C4B60, &CPedIntelligence::AdjustSub2C8);
    Core::Hook::Install(0x9C4C50, &CPedIntelligence::CreateDefaultTask200);
    Core::Hook::Install(0x9C4C70, &CPedIntelligence::CheckRelationshipBit);
    Core::Hook::Install(0x9C4D10, &CPedIntelligence::ScalarDeletingDtor);
    Core::Hook::Install(0x9C4D40, &CPedIntelligence::IsAnySeenPedHatedInRange);
    Core::Hook::Install(0x9C4DF0, &CPedIntelligence::ProcessDeath);
    Core::Hook::Install(0x9C4EF0, &CPedIntelligence::IsFriendlyWithPed);
    Core::Hook::Install(0x9C4FC0, &CPedIntelligence::IsPedThreatening);
    Core::Hook::Install(0x9C5120, &CPedIntelligence::ArePedsFriendly);
}

// 0x9C4300
void* __thiscall CPedIntelligence::GetOrCreateSub25C(bool createIfMissing)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void** sub25C = reinterpret_cast<void**>(self + 0x25C);
    if (*sub25C == nullptr && createIfMissing)
    {
        void* pool = Core::StaticRef<void*>(0x1711134);
        void* slot = Core::Hook::CallMethod<void*>(0x96D520, pool);
        *sub25C = slot
            ? Core::Hook::CallMethod<void*>(0xC89000, slot, *reinterpret_cast<void**>(self + 0x40))
            : nullptr;
    }
    return *sub25C;
}

// 0x9C4350
void* __thiscall CPedIntelligence::UpdateSub25C()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void** sub25C = reinterpret_cast<void**>(self + 0x25C);
    if (*sub25C == nullptr)
    {
        void* pool = Core::StaticRef<void*>(0x1711134);
        void* slot = Core::Hook::CallMethod<void*>(0x96D520, pool);
        *sub25C = slot
            ? Core::Hook::CallMethod<void*>(0xC89000, slot, *reinterpret_cast<void**>(self + 0x40))
            : nullptr;
    }
    if (*sub25C == nullptr)
        return nullptr;

    return Core::Hook::CallMethod<void*>(0xC8A060, *sub25C);
}

// 0x9C43A0
void __thiscall CPedIntelligence::SetSub25CTarget(void* target, uint32 arg2)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (*reinterpret_cast<void**>(self + 0x40) == target)
        return;

    void** sub25C = reinterpret_cast<void**>(self + 0x25C);
    if (*sub25C == nullptr)
    {
        void* pool = Core::StaticRef<void*>(0x1711134);
        void* slot = Core::Hook::CallMethod<void*>(0x96D520, pool);
        *sub25C = slot
            ? Core::Hook::CallMethod<void*>(0xC89000, slot, *reinterpret_cast<void**>(self + 0x40))
            : nullptr;
    }

    if (*sub25C != nullptr)
    {
        using SetTargetFn = void(__thiscall*)(void*, void*, uint32);
        reinterpret_cast<SetTargetFn>((*reinterpret_cast<void***>(*sub25C))[1])(*sub25C, target, arg2);
    }
}

// 0x9C4400
void __thiscall CPedIntelligence::Reset(bool bResetEvents, bool bUpdateTasks)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (bResetEvents)
    {
        Core::Hook::CallMethod<void>(0x9C24B0, self);

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        uint8* eventGroup = self + 0x84;

        if ((*reinterpret_cast<uint8*>(ped + 0x26C) & 0x4) != 0 && *reinterpret_cast<void**>(ped + 0xB40) != nullptr)
        {
            if (!Core::Hook::CallMethod<bool>(0xAA7FB0, eventGroup, 0x2C5))
            {
                void* pool = Core::StaticRef<void*>(0x164B01C);
                void* slot = Core::Hook::CallMethod<void*>(0x439CB0, pool);
                void* ev = slot
                    ? Core::Hook::CallMethod<void*>(0x9B78A0, slot, *reinterpret_cast<void**>(*reinterpret_cast<uint8**>(self + 0x40) + 0xB40), 0)
                    : nullptr;

                uint8 evWrapper[0x18]{};
                Core::Hook::CallMethod<void>(0x918C90, evWrapper, 3, ev, 0);
                Core::Hook::CallMethod<void>(0xAA80D0, eventGroup, evWrapper, 0, 1);
                Core::Hook::CallMethod<void>(0x918CC0, evWrapper);
            }
        }
        else if (!Core::Hook::CallMethod<bool>(0xAA7FB0, eventGroup, 0xCB))
        {
            void* pool = Core::StaticRef<void*>(0x164B01C);
            void* slot = Core::Hook::CallMethod<void*>(0x439CB0, pool);
            void* ev = slot
                ? Core::Hook::CallMethod<void*>(0xA1D730, slot, 0, 0, 0, Core::StaticRef<float>(0xD5829C))
                : nullptr;

            uint8 evWrapper[0x18]{};
            Core::Hook::CallMethod<void>(0x918C90, evWrapper, 3, ev, 0);
            Core::Hook::CallMethod<void>(0xAA80D0, eventGroup, evWrapper, 0, 1);
            Core::Hook::CallMethod<void>(0x918CC0, evWrapper);
        }

        using GetDefaultTaskFn = void*(__thiscall*)(void*);
        void* defaultTask = reinterpret_cast<GetDefaultTaskFn>((*reinterpret_cast<void***>(self))[8])(self);
        Core::Hook::CallMethod<void>(0xA6DDB0, defaultTask);
        Core::Hook::CallMethod<void>(0x9E6430, self + 0x44);
        Core::Hook::Call<void>(0xBBDC20);
    }

    if (bUpdateTasks)
    {
        uint8* taskMgr = self + 0x44;
        for (int32 i = 0; i < 6; ++i)
        {
            if (i == 3)
                continue;

            uint8* task = Core::Hook::CallMethod<uint8*>(0x9E56E0, taskMgr, i);
            if (task == nullptr)
                continue;

            using MakeAbortableFn = bool(__thiscall*)(void*, void*, int32, void*);
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
            if ((*reinterpret_cast<uint8*>(task + 0xC) & 1) != 0)
            {
                Core::Hook::CallMethod<void>(0x9E5AC0, taskMgr, nullptr, i);
                continue;
            }

            if (reinterpret_cast<MakeAbortableFn>((*reinterpret_cast<void***>(task))[5])(task, ped, 1, nullptr))
            {
                *reinterpret_cast<uint32*>(task + 0xC) |= 2;
                Core::Hook::CallMethod<void>(0x9E5AC0, taskMgr, nullptr, i);
            }
            else if ((*reinterpret_cast<uint8*>(task + 0xC) & 1) == 0)
            {
                ped = *reinterpret_cast<uint8**>(self + 0x40);
                if (reinterpret_cast<MakeAbortableFn>((*reinterpret_cast<void***>(task))[5])(task, ped, 0, nullptr))
                    *reinterpret_cast<uint32*>(task + 0xC) |= 2;
            }
        }
    }
}

// 0x9C4590
void __thiscall CPedIntelligence::Process()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x9C3260, self);

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    Core::Hook::CallMethod<void>(0xBC9870, *reinterpret_cast<void**>(ped + 0xA90), 0);

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* eventGroup = self + 0x84;
    if (!Core::Hook::CallMethod<bool>(0xA52620, self + 0x230, ped, eventGroup))
    {
        ped = *reinterpret_cast<uint8**>(self + 0x40);
        Core::Hook::CallMethod<void>(0xA55FA0, self + 0x220, ped, eventGroup);
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    void* ac0 = *reinterpret_cast<void**>(ped + 0xAC0);
    void* ent38 = *reinterpret_cast<void**>(ped + 0x38);
    if (ac0 != nullptr
        && (ent38 == nullptr || ent38 != *reinterpret_cast<void**>(ped + 0x7C4))
        && Core::Hook::Call<bool>(0xA72240))
    {
        ped = *reinterpret_cast<uint8**>(self + 0x40);
        if (*reinterpret_cast<uint32*>(ped + 0x7C8) != 0
            && Core::Hook::Call<bool>(0xC5ED00, ped, 0xC, *reinterpret_cast<void**>(ped + 0xAC0), 0.0f))
        {
            ped = *reinterpret_cast<uint8**>(self + 0x40);
            alignas(16) float velBuf[4]{};
            using GetVelFn = float*(__thiscall*)(void*, float*);
            float* vel = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(ped))[59])(ped, velBuf);
            const float speedSq = vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2];
            const float thresh = Core::StaticRef<float>(0xF3186C);
            if (speedSq > thresh * thresh)
            {
                uint8* speech = *reinterpret_cast<uint8**>(self + 0x40) + 0x580;
                void* str = Core::Hook::Call<void*>(0x5B1C30, reinterpret_cast<const char*>(Core::Rebase(0xD81C70)), 0);
                Core::Hook::CallMethod<void>(
                    0xA5AA70, speech,
                    reinterpret_cast<const char*>(Core::Rebase(0xD81C7C)),
                    1, 1, str, -1, 0, 0, 1.0f, 0, 0);
            }

            void* pool = Core::StaticRef<void*>(0x164B01C);
            void* slot = Core::Hook::CallMethod<void*>(0x439CB0, pool);
            void* ev = slot
                ? Core::Hook::CallMethod<void*>(
                    0xC61B20, slot, 0x7D0, 0x7530,
                    *reinterpret_cast<void**>(*reinterpret_cast<uint8**>(self + 0x40) + 0xAC0),
                    4, 0, 0.0f, 0, 0)
                : nullptr;

            if (ev != nullptr)
                Core::Hook::CallMethod<void>(0xC5AC10, ev, *reinterpret_cast<void**>(self + 0x40));

            uint8 evWrapper[0x18]{};
            Core::Hook::CallMethod<void>(0x91A380, evWrapper, 0x7530, ev, 0);
            Core::Hook::CallMethod<void>(0xAA80D0, eventGroup, evWrapper, 0, 1);
            Core::Hook::CallMethod<void>(0x91A3F0, evWrapper);
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if (*reinterpret_cast<float*>(ped + 0x170) > 0.0f)
    {
        uint8* ground = *reinterpret_cast<uint8**>(ped + 0x16C);
        if (ground != nullptr && (*reinterpret_cast<uint32*>(ground + 0x28) & 0x3C0) != 0xC0)
        {
            uint8* mat = *reinterpret_cast<uint8**>(ped + 0x20);
            const float dot =
                *reinterpret_cast<float*>(ped + 0x188) * *reinterpret_cast<float*>(mat + 0x18) +
                *reinterpret_cast<float*>(ped + 0x184) * *reinterpret_cast<float*>(mat + 0x14) +
                *reinterpret_cast<float*>(ped + 0x180) * *reinterpret_cast<float*>(mat + 0x10);
            if (Core::StaticRef<float>(0xDCC5DC) > dot)
                *reinterpret_cast<uint32*>(ped + 0x29C) |= 0x40;
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if (*reinterpret_cast<float*>(ped + 0x170) > 0.0f)
    {
        uint8* ground = *reinterpret_cast<uint8**>(ped + 0x16C);
        if (ground != nullptr
            && (*reinterpret_cast<uint32*>(ground + 0x28) & 0x3C0) == 0x100
            && *reinterpret_cast<uint8*>(ground + 0x22B) > 0
            && *reinterpret_cast<int8*>(ground + 0x249) != 0)
        {
            uint8* gMat = *reinterpret_cast<uint8**>(ground + 0x20);
            const float* gPos = gMat ? reinterpret_cast<const float*>(gMat + 0x30) : reinterpret_cast<const float*>(ground + 0x10);
            const float* pPos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
            const float dx = pPos[0] - gPos[0];
            const float dy = pPos[1] - gPos[1];
            const float dz = pPos[2] - gPos[2];

            const float* gRight = Core::Hook::CallMethod<const float*>(0x447980, ground);
            const float dot1 = gRight[6] * dz + gRight[5] * dy + gRight[4] * dx;
            if (dot1 > Core::StaticRef<float>(0xDECD60) && *reinterpret_cast<int8*>(ground + 0x249) < 0)
            {
                *reinterpret_cast<uint8*>(*reinterpret_cast<uint8**>(self + 0x40) + 0x299) = 0xA;
            }
            else
            {
                const float* gRight2 = Core::Hook::CallMethod<const float*>(
                    0x447980, *reinterpret_cast<void**>(*reinterpret_cast<uint8**>(self + 0x40) + 0x16C));
                const float dot2 = gRight2[6] * dz + gRight2[5] * dy + gRight2[4] * dx;
                if (0.0f > dot2 && *reinterpret_cast<int8*>(ground + 0x249) > 0)
                    *reinterpret_cast<uint8*>(*reinterpret_cast<uint8**>(self + 0x40) + 0x299) = 0xA;
            }
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if ((*reinterpret_cast<uint32*>(ped + 0x26C) & 0x2000) == 0)
        *reinterpret_cast<float*>(self + 0x288) = Core::StaticRef<float>(0xD81CE0);
}

// 0x9C4910
void __thiscall CPedIntelligence::UpdateTaskMotionFlags()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x9E57D0, self + 0x44, self + 0x2E0);
    const uint32 flags = Core::Hook::CallMethod<uint32>(0x9C3640, self);
    uint32* dst = reinterpret_cast<uint32*>(self + 0x2E8);
    *dst ^= (*dst ^ flags) & 7;
}

// 0x9C4940
void __thiscall CPedIntelligence::ProcessTasks()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0xA56F00, self + 0xF4, *reinterpret_cast<void**>(self + 0x40));
    Core::Hook::CallMethod<void>(0xA56F90, self + 0x150, *reinterpret_cast<void**>(self + 0x40));

    int32 counter = *reinterpret_cast<int32*>(self + 0x1BC);
    const bool wasZero = (counter == 0);
    ++counter;
    *reinterpret_cast<int32*>(self + 0x1BC) = counter;
    if (counter > *reinterpret_cast<int32*>(self + 0x1C0))
        *reinterpret_cast<int32*>(self + 0x1BC) = 0;
    if (wasZero)
        *reinterpret_cast<int32*>(self + 0x1BC) = 0;

    using GetSubFn = void*(__thiscall*)(void*);
    void* sub = reinterpret_cast<GetSubFn>((*reinterpret_cast<void***>(self))[2])(self);
    Core::Hook::CallMethod<void>(0xA57E90, sub, *reinterpret_cast<void**>(self + 0x40));

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* intel = *reinterpret_cast<uint8**>(ped + 0x224);
    if (*(intel + 0x2DC) != 0 && *(ped + 0xA70) == 1)
        Core::Hook::CallMethod<void>(0xA51D50, self + 0x250, ped);

    intel = *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(self + 0x40) + 0x224);
    *(intel + 0x2DC) = 0;

    using GetDefaultTaskFn = void*(__thiscall*)(void*);
    void* defaultTask = reinterpret_cast<GetDefaultTaskFn>((*reinterpret_cast<void***>(self))[8])(self);
    Core::Hook::CallMethod<void>(0xA6DDB0, defaultTask);
    Core::Hook::CallMethod<void>(0x9E6430, self + 0x44);
    Core::Hook::CallMethod<void>(0x9C4910, self);
    Core::Hook::CallMethod<void>(0x9C2C00, self);
    Core::Hook::CallMethod<void>(0x9C24E0, self);

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    void* d78 = *reinterpret_cast<void**>(ped + 0xD78);
    if (d78 != nullptr && (*reinterpret_cast<uint32*>(ped + 0x26C) & 0x800) == 0)
    {
        alignas(16) float pos[4]{};
        Core::Hook::CallMethod<void>(0xC697A0, d78, pos, 0);

        ped = *reinterpret_cast<uint8**>(self + 0x40);
        const float* matPos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
        const float dx = pos[0] - matPos[0];
        const float dy = pos[1] - matPos[1];
        const float dz = pos[2] - matPos[2];
        if (dx * dx + dy * dy + dz * dz > Core::StaticRef<float>(0xD8523C))
            Core::Hook::CallMethod<void>(0x89DC70, ped);
    }

    const float dtScaled = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xD50978);
    Core::Hook::CallMethod<void>(0x9C2D10, self, dtScaled);

    float f2C4 = *reinterpret_cast<float*>(self + 0x2C4);
    if (f2C4 > 0.0f)
    {
        f2C4 -= Core::StaticRef<float>(0x11DDE7C);
        if (f2C4 <= 0.0f)
            f2C4 = 0.0f;
        *reinterpret_cast<float*>(self + 0x2C4) = f2C4;

        const float val = (*reinterpret_cast<uint8*>(self + 0x2CC) & 1)
            ? Core::StaticRef<float>(0xD8F340)
            : 0.0f;
        Core::Hook::CallMethod<void>(0x9C2D50, self, val);
    }
}

// 0x9C4B10
void __cdecl CPedIntelligence::ShutdownPoolAndClearEvents()
{
    void* mgr = Core::Hook::Call<void*>(0xAA83F0);
    Core::Hook::CallMethod<void>(0xAA8010, mgr);

    void* pool = Core::StaticRef<void*>(0x16C3454);
    if (pool == nullptr)
    {
        void* mem = Core::Hook::Call<void*>(0x4011D0, 0x20020);
        pool = mem ? Core::Hook::CallMethod<void*>(0x804720, mem) : nullptr;
        Core::StaticRef<void*>(0x16C3454) = pool;
    }

    if (pool != nullptr)
        Core::Hook::Call<void>(0x5B1C10, pool);

    Core::StaticRef<void*>(0x16C3454) = nullptr;
    Core::Hook::Call<void>(0x91B9C0);
}

// 0x9C4B60
void __thiscall CPedIntelligence::AdjustSub2C8(float scale)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    const int32 id = *reinterpret_cast<int32*>(self + 0xD8);

    void* pool = Core::StaticRef<void*>(0x16C3454);
    if (pool == nullptr)
    {
        void* mem = Core::Hook::Call<void*>(0x4011D0, 0x20020);
        pool = mem ? Core::Hook::CallMethod<void*>(0x804720, mem) : nullptr;
        Core::StaticRef<void*>(0x16C3454) = pool;
    }

    uint8* info = Core::Hook::CallMethod<uint8*>(0xB5B580, pool, id);
    float mult = 0.0f;
    if ((*reinterpret_cast<uint32*>(info + 0x8F4) & 0x18000) == 0)
    {
        mult = Core::StaticRef<float>(0xD50398);
    }
    else
    {
        uint8* pedInfo = Core::Hook::CallMethod<uint8*>(0x8A6530, self);
        if ((*reinterpret_cast<uint32*>(pedInfo + 0x8F4) & 0x18000) == 0x18000)
        {
            mult = Core::StaticRef<float>(0xD50398);
        }
        else
        {
            pedInfo = Core::Hook::CallMethod<uint8*>(0x8A6530, self);
            if ((*reinterpret_cast<uint32*>(pedInfo + 0x8F4) & 0x18000) == 0x8000)
                mult = Core::StaticRef<float>(0xDCC5F4);
        }
    }

    const int32 delta = static_cast<int8>(static_cast<int32>(mult * scale));
    int32 cur = *reinterpret_cast<int8*>(self + 0x2C8) + delta;
    if (cur <= 0)
        cur = 0;
    else if (cur >= 100)
        cur = 100;

    *reinterpret_cast<int8*>(self + 0x2C8) = static_cast<int8>(cur);
}

// 0x9C4C50
void* __thiscall CPedIntelligence::CreateDefaultTask200(uint32 unused)
{
    (void)unused;
    void* pool = Core::StaticRef<void*>(0x1618014);
    void* slot = Core::Hook::CallMethod<void*>(0x439CB0, pool);
    return slot ? Core::Hook::CallMethod<void*>(0x9A02C0, slot) : nullptr;
}

// 0x9C4C70 (ECX = CPed*)
bool __thiscall CPedIntelligence::CheckRelationshipBit(int32 relType, int32 otherGroup)
{
    if (otherGroup < 0 || otherGroup >= 96)
        return false;

    uint8* self = reinterpret_cast<uint8*>(this);
    uint32 mask[3]{};
    mask[otherGroup >> 5] |= (1u << (otherGroup & 0x1F));
    const uint32* bits = reinterpret_cast<const uint32*>(self + 0xB44 + relType * 12);
    return ((bits[0] & mask[0]) | (bits[1] & mask[1]) | (bits[2] & mask[2])) != 0;
}

// 0x9C4D10
void* __thiscall CPedIntelligence::ScalarDeletingDtor(uint8 flags)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x9C4160, self);
    if (flags & 1)
    {
        void* pool = Core::StaticRef<void*>(0x18A82B0);
        Core::Hook::CallMethod<void>(0x448300, pool, self);
    }
    return this;
}

// 0x9C4D40
bool __thiscall CPedIntelligence::IsAnySeenPedHatedInRange()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    for (int32 i = 0; i < *reinterpret_cast<int32*>(self + 0xE8) && i < 16; ++i)
    {
        uint8* otherPed = *reinterpret_cast<uint8**>(self + 0x168 + i * 4);
        if (otherPed == nullptr)
            continue;

        const int32 otherGroup = *reinterpret_cast<int32*>(otherPed + 0xBB4);
        void* ped = *reinterpret_cast<void**>(self + 0x40);
        if (!Core::Hook::CallMethod<bool>(0x9C4C70, ped, 0, otherGroup))
            continue;

        const float* pedPos = reinterpret_cast<const float*>(
            *reinterpret_cast<uint8**>(*reinterpret_cast<uint8**>(self + 0x40) + 0x20) + 0x30);
        const float* otherPos = reinterpret_cast<const float*>(
            *reinterpret_cast<uint8**>(otherPed + 0x20) + 0x30);

        const float dx = pedPos[0] - otherPos[0];
        const float dy = pedPos[1] - otherPos[1];
        const float dz = pedPos[2] - otherPos[2];
        const float range = *reinterpret_cast<float*>(self + 0xEC);
        if (range * range > dx * dx + dy * dy + dz * dz)
            return true;
    }
    return false;
}

// 0x9C4DF0
void __thiscall CPedIntelligence::ProcessDeath(bool bCreateDeadTask)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x9C24B0, self);

    uint8* taskMgr = self + 0x44;
    void* task3 = Core::Hook::CallMethod<void*>(0x9E56E0, taskMgr, 3);
    void* clonedTask = nullptr;
    if (task3 != nullptr)
    {
        using GetTypeFn = int32(__thiscall*)(void*);
        if (reinterpret_cast<GetTypeFn>((*reinterpret_cast<void***>(task3))[3])(task3) == 0x131)
        {
            using CloneFn = void*(__thiscall*)(void*);
            clonedTask = reinterpret_cast<CloneFn>((*reinterpret_cast<void***>(task3))[1])(task3);
        }
    }

    Core::Hook::CallMethod<void>(0xAA7AE0, self + 0x84, 1);

    using GetDefaultTaskFn = void*(__thiscall*)(void*);
    void* defaultTask = reinterpret_cast<GetDefaultTaskFn>((*reinterpret_cast<void***>(self))[8])(self);
    Core::Hook::CallMethod<void>(0xA68900, defaultTask);
    Core::Hook::CallMethod<void>(0x9E5B80, taskMgr);
    Core::Hook::CallMethod<void>(0x89E270, *reinterpret_cast<void**>(self + 0x40), 0);
    Core::Hook::Call<void>(0xBBDC20);

    if (clonedTask != nullptr)
        Core::Hook::CallMethod<void>(0x9E5AC0, taskMgr, clonedTask, 3);

    if (bCreateDeadTask)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
        void* mgr22C = *reinterpret_cast<void**>(ped + 0x22C);
        using CreateDeadTaskFn = void*(__thiscall*)(void*, void*);
        void* task = reinterpret_cast<CreateDeadTaskFn>((*reinterpret_cast<void***>(mgr22C))[5])(mgr22C, ped);
        Core::Hook::CallMethod<void>(0x9E58B0, taskMgr, task, 4, 0);
    }

    void* pool = Core::StaticRef<void*>(0x164B01C);
    void* slot = Core::Hook::CallMethod<void*>(0x439CB0, pool);
    uint8* deadEv = nullptr;
    if (slot != nullptr)
    {
        deadEv = Core::Hook::CallMethod<uint8*>(0xA28B30, slot, 1);
        *reinterpret_cast<uintptr_t*>(deadEv + 0x00) = Core::Rebase(0xD6973C);
        *reinterpret_cast<uintptr_t*>(deadEv + 0x14) = Core::Rebase(0xD69700);
        *reinterpret_cast<uint32*>(deadEv + 0x20) = 0;
    }
    if (deadEv != nullptr)
        Core::Hook::CallMethod<void>(0x9E5E20, taskMgr, deadEv, 2);

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* p6C = *reinterpret_cast<uint8**>(ped + 0x6C);
    if (p6C == nullptr || *(p6C + 0xE) == 0)
        Core::Hook::CallMethod<void>(0x9C4910, self);
}

// 0x9C4EF0
bool __thiscall CPedIntelligence::IsFriendlyWithPed(void* otherPed)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* other = static_cast<uint8*>(otherPed);
    const int32 otherGroup = *reinterpret_cast<int32*>(other + 0xBB4);

    if (Core::Hook::CallMethod<bool>(0x9C4C70, ped, 0, otherGroup))
        return true;
    if (Core::Hook::CallMethod<bool>(0x9C4C70, ped, 1, otherGroup))
        return true;

    if (Core::Hook::CallMethod<void*>(0x89DD80, ped) != nullptr
        && Core::Hook::CallMethod<void*>(0x89DD80, ped) == Core::Hook::CallMethod<void*>(0x89DD80, other))
    {
        return true;
    }

    const int32 pedType = *reinterpret_cast<int32*>(*reinterpret_cast<uint8**>(ped + 0x21C) + 0x12C);
    if (pedType == 0x10 && *reinterpret_cast<int32*>(*reinterpret_cast<uint8**>(other + 0x21C) + 0x12C) == 0x10)
        return true;
    if (pedType == 0x11 && *reinterpret_cast<int32*>(*reinterpret_cast<uint8**>(other + 0x21C) + 0x12C) == 0x11)
        return true;
    if (pedType == 0x2 && *reinterpret_cast<int32*>(*reinterpret_cast<uint8**>(other + 0x21C) + 0x12C) == 0x2)
        return true;

    const int32 targetModel = Core::StaticRef<int32>(0x12303AC);
    if (*reinterpret_cast<int16*>(ped + 0x2E) == targetModel
        && *reinterpret_cast<int16*>(other + 0x2E) == targetModel)
    {
        return true;
    }

    return false;
}

// 0x9C4FC0
bool __thiscall CPedIntelligence::IsPedThreatening(void* otherPed, bool bCheckDislike)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x40);
    uint8* other = static_cast<uint8*>(otherPed);
    const int32 otherGroup = *reinterpret_cast<int32*>(other + 0xBB4);

    bool bThreat = (bCheckDislike && Core::Hook::CallMethod<bool>(0x9C4C70, ped, 3, otherGroup))
        || Core::Hook::CallMethod<bool>(0x9C4C70, ped, 5, otherGroup)
        || Core::Hook::CallMethod<bool>(0x9C4C70, ped, 4, otherGroup);

    uint8* grp = Core::Hook::CallMethod<uint8*>(0x89DD80, ped);
    if (grp != nullptr && !bThreat)
    {
        uint8* leader = Core::Hook::CallMethod<uint8*>(0xA66B40, grp + 0x8);
        if (leader != nullptr)
        {
            const int32 leaderGroup = *reinterpret_cast<int32*>(leader + 0xBB4);
            bThreat = (bCheckDislike && Core::Hook::CallMethod<bool>(0x9C4C70, other, 3, leaderGroup))
                || Core::Hook::CallMethod<bool>(0x9C4C70, other, 5, leaderGroup)
                || Core::Hook::CallMethod<bool>(0x9C4C70, other, 4, leaderGroup);
        }
    }

    if (Core::Hook::Call<bool>(0x462F90))
    {
        if (!bThreat)
        {
            ped = *reinterpret_cast<uint8**>(self + 0x40);
            if (*reinterpret_cast<int32*>(*reinterpret_cast<uint8**>(ped + 0x21C) + 0x12C) == 2
                && *(ped + 0xA70) == 1)
            {
                using HasWeaponFn = bool(__thiscall*)(void*);
                if (reinterpret_cast<HasWeaponFn>((*reinterpret_cast<void***>(other))[0x128 / 4])(other)
                    && Core::Hook::CallMethod<int32>(0x4389F0, other) > 0)
                {
                    bThreat = true;
                }
            }
        }
    }

    if (!bThreat)
        return false;

    ped = *reinterpret_cast<uint8**>(self + 0x40);
    if (Core::Hook::CallMethod<void*>(0x89DD80, ped) != nullptr
        && Core::Hook::CallMethod<void*>(0x89DD80, other) == Core::Hook::CallMethod<void*>(0x89DD80, *reinterpret_cast<void**>(self + 0x40)))
    {
        return false;
    }

    if (Core::Hook::CallMethod<bool>(0x9C4EF0, self, other))
        return false;

    return bThreat;
}

// 0x9C5120
bool __cdecl CPedIntelligence::ArePedsFriendly(void* pedA, void* pedB)
{
    void* intelA = *reinterpret_cast<void**>(static_cast<uint8*>(pedA) + 0x224);
    if (Core::Hook::CallMethod<bool>(0x9C4EF0, intelA, pedB))
        return true;

    void* intelB = *reinterpret_cast<void**>(static_cast<uint8*>(pedB) + 0x224);
    if (Core::Hook::CallMethod<bool>(0x9C4EF0, intelB, pedA))
        return true;

    return false;
}


void CPedIntelligence::InjectHooks()
{
    InjectPedIntelligenceSlice1Hooks();
    InjectPedIntelligenceSlice2Hooks();
    InjectPedIntelligenceSlice3Hooks();
}
