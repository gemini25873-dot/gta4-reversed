#include "StdInc.h"
#include "Game/PedMoveBlend.h"
#include "Game/PedIntelligence.h"
#include "Core/Hook.h"
#include <cmath>

namespace {
    struct MoveBlendSlot
    {
        int32 id;       // +0x00 (0x165D19C)
        int32 groupId;  // +0x04 (0x165D1A0)
        float weight;   // +0x08 (0x165D1A4)
        float f1;       // +0x0C (0x165D1A8)
        float f2;       // +0x10 (0x165D1AC)
        float f3;       // +0x14 (0x165D1B0)
        int32 zero;     // +0x18 (0x165D1B4)
        float rate;     // +0x1C (0x165D1B8)
    };
    VALIDATE_SIZE(MoveBlendSlot, 0x20);

    inline void PushMoveBlendSlot(int32 id, float weight, float rate)
    {
        int32& count = Core::StaticRef<int32>(0x165D198);
        if (count >= 9)
            return;

        auto* slots = &Core::StaticRef<MoveBlendSlot>(0x165D19C);
        slots[count].id = id;
        slots[count].groupId = -1;
        slots[count].weight = weight;
        slots[count].f1 = 0.0f;
        slots[count].f2 = 0.0f;
        slots[count].f3 = 0.0f;
        slots[count].zero = 0;
        slots[count].rate = rate;
        ++count;
    }

    inline void AddMoveBlendSlotCall(int32 id, float weight, float f1, float f2, float f3, int32 zero, int32 groupId, float rate)
    {
        Core::Hook::CallMethod<void>(0xA34680, &Core::StaticRef<uint8>(0x165D198), id, weight, f1, f2, f3, zero, groupId, rate);
    }

    inline float WrapPiDiff(float a)
    {
        const float twoPi = Core::StaticRef<float>(0xEBDED0);
        return (a < 0.0f) ? (twoPi - a) : a;
    }

    inline float SignOrZero(float v)
    {
        if (v < 0.0f) return Core::StaticRef<float>(0xEBDED4);
        if (v == 0.0f) return 0.0f;
        return Core::StaticRef<float>(0xD50398);
    }

    inline float StepMoveCoord(float cur, float target, float step)
    {
        if (target > cur) {
            if (cur != 0.0f && SignOrZero(target) != SignOrZero(cur)) {
                cur += Core::StaticRef<float>(0xF27D60) * step;
            } else {
                cur += step;
            }
            if (cur > target) cur = target;
        } else if (cur > target) {
            if (cur != 0.0f && SignOrZero(target) != SignOrZero(cur)) {
                cur -= Core::StaticRef<float>(0xF27D60) * step;
            } else {
                cur -= step;
            }
            if (target > cur) cur = target;
        }
        return cur;
    }
}

// =========================================================================
// Slice 1: 0xA35050 .. 0xA35E32
// =========================================================================

// 0xA35050
bool __cdecl CPedMoveBlend::IsAnimGroup53_59(int32 groupId)
{
    return groupId == 0x36 || groupId == 0x37 || groupId == 0x35 || groupId == 0x39;
}

// 0xA35070
void __thiscall CPedMoveBlend::ResetBlendFlagsAndSaveTargetHeading()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<uint32*>(self + 0x50) &= 0xFFF4FC7Fu;
    *reinterpret_cast<float*>(self + 0x58) = *reinterpret_cast<float*>(ped + 0xAB4);

    const bool bFlag = Core::Hook::CallMethod<bool>(0x89C780, ped);
    uint32 flags = *reinterpret_cast<uint32*>(self + 0x50);
    flags ^= ((static_cast<uint32>(bFlag) << 10) ^ flags) & 0x400u;

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    uint32 subFlags = (*reinterpret_cast<uint32*>(subA90 + 0x50) >> 1) & 0xFFu;
    flags ^= ((subFlags << 2) ^ flags) & 0x4u;

    *reinterpret_cast<uint32*>(self + 0x50) = flags;
    *reinterpret_cast<uint32*>(self + 0x14) = *reinterpret_cast<uint32*>(self + 0x4);
    *reinterpret_cast<uint32*>(self + 0x18) = *reinterpret_cast<uint32*>(self + 0x8);
}

// 0xA350D0
void __thiscall CPedMoveBlend::ApplyPedVelocityToTask()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    float sum = Core::StaticRef<float>(0x165D210)
        + Core::StaticRef<float>(0x165D1F0)
        + Core::StaticRef<float>(0x165D1D0)
        + Core::StaticRef<float>(0x165D1B0);

    float prev60 = *reinterpret_cast<float*>(self + 0x60);
    float delta = prev60 - *reinterpret_cast<float*>(self + 0x64);
    *reinterpret_cast<float*>(self + 0x64) = prev60;
    *reinterpret_cast<float*>(self + 0x60) = sum;

    void* taskObj = Core::Hook::CallMethod<void*>(0x9E7080, *reinterpret_cast<void**>(self + 0x24));
    if (taskObj) {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
        float* mat = *reinterpret_cast<float**>(ped + 0x20);
        float scale = Core::StaticRef<float>(0xF359BC) * delta;
        CVector vec(mat[0] * scale, mat[1] * scale, mat[2] * scale);
        using ApplyFn = void(__thiscall*)(void*, CVector*, uintptr_t, uint32);
        reinterpret_cast<ApplyFn>((*reinterpret_cast<void***>(taskObj))[0x70 / 4])(
            taskObj, &vec, Core::Rebase(0x18A8C50), 0);
    }
}

// 0xA35180
void __thiscall CPedMoveBlend::GetEntityPositionByIndex(CVector4* outPos, int32 /*unused*/)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    int32 idx = *reinterpret_cast<int32*>(self + 0x3C);
    uint8* ent = Core::Hook::Call<uint8*>(0xC54AD0, idx);
    if (!ent) {
        ent = Core::Hook::Call<uint8*>(0xC54F10, *reinterpret_cast<int32*>(self + 0x3C));
    }
    outPos->x = 0.0f;
    outPos->y = *reinterpret_cast<float*>(ent + 0x360);
    outPos->z = *reinterpret_cast<float*>(ent + 0x364);
    outPos->w = *reinterpret_cast<float*>(ent + 0x368);
}

// 0xA351D0
float __cdecl CPedMoveBlend::ComputeBlendSegment(const float* thresholds, float speed)
{
    if (thresholds[0] > speed)
        return 0.0f;

    for (int32 i = 0; i < 3; ++i) {
        float a = thresholds[i];
        float b = thresholds[i + 1];
        if (speed >= a && b >= speed) {
            if (a == b) {
                return static_cast<float>(i) + Core::StaticRef<float>(0xD50398);
            }
            return static_cast<float>(i) + (speed - a) / (b - a);
        }
    }
    return Core::StaticRef<float>(0xD885D4);
}

// 0xA35260
void* __thiscall CPedMoveBlend::HasActiveMoveAnimInSlot2(void* animPlayer)
{
    uint8* node = Core::Hook::CallMethod<uint8*>(0x9EC060, animPlayer, 0, 2);
    while (node) {
        uint32 id = *reinterpret_cast<uint32*>(node + 0xC);
        if (id >= 0x12u && id <= 0x1Bu)
            return node;
        node = Core::Hook::CallMethod<uint8*>(0x9EC0F0, animPlayer, 0, 2);
    }
    return nullptr;
}

// 0xA352C0
bool __thiscall CPedMoveBlend::HasActiveLocomotionAnimInSlot2(void* animPlayer)
{
    uint8* node = Core::Hook::CallMethod<uint8*>(0x9EC060, animPlayer, 0, 2);
    while (node) {
        uint32 id = *reinterpret_cast<uint32*>(node + 0xC);
        if (id == 0x26u || id == 0x27u || id == 0x30u || id == 0x31u || id == 0x35u || id == 0x36u)
            return true;
        node = Core::Hook::CallMethod<uint8*>(0x9EC0F0, animPlayer, 0, 2);
    }
    return false;
}

// 0xA35310
bool __thiscall CPedMoveBlend::IsMovingOrTurning(void* pedInfo)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (*reinterpret_cast<float*>(self + 0x4) != 0.0f)
        return true;
    if (*reinterpret_cast<float*>(self + 0x8) != 0.0f)
        return true;

    void* animPlayer = *reinterpret_cast<void**>(static_cast<uint8*>(pedInfo) + 0x78);
    using CheckFn = bool(__thiscall*)(void*, void*);
    if (reinterpret_cast<CheckFn>((*reinterpret_cast<void***>(this))[0x58 / 4])(this, animPlayer))
        return true;

    if (*reinterpret_cast<float*>(self + 0x54) != Core::StaticRef<float>(0xD885D4))
        return true;

    return false;
}

// 0xA35370
void __thiscall CPedMoveBlend::ClampMoveSpeeds()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (*reinterpret_cast<uint32*>(self + 0x50) & 0x2u) {
        float minV = Core::StaticRef<float>(0xEBE0FC);
        float maxV = Core::StaticRef<float>(0xDCB2A4);
        float& x = *reinterpret_cast<float*>(self + 0xC);
        if (x < minV) x = minV;
        else if (x > maxV) x = maxV;

        float& y = *reinterpret_cast<float*>(self + 0x10);
        if (y < minV) y = minV;
        else if (y > maxV) y = maxV;
    } else {
        *reinterpret_cast<float*>(self + 0xC) = 0.0f;
        float& y = *reinterpret_cast<float*>(self + 0x10);
        float maxV = Core::StaticRef<float>(0xD885D4);
        if (y < 0.0f) y = 0.0f;
        else if (y > maxV) y = maxV;
    }
}

// 0xA353F0
void __thiscall CPedMoveBlend::SetUnk6C(float val)
{
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(this) + 0x6C) = val;
}

// 0xA35400
float __cdecl CPedMoveBlend::GetDefaultFloat_F359A4()
{
    return Core::StaticRef<float>(0xF359A4);
}

// 0xA35410
CPedMoveBlend* __thiscall CPedMoveBlend::Ctor(void* arg)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0xBC9740, this, arg);
    *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD886C4);
    Core::Hook::CallMethod<void>(0xA348E0, this);
    return this;
}

// 0xA35430
void __cdecl CPedMoveBlend::StartIdleAnim(void* ped, uint32* animData)
{
    uint8* p = static_cast<uint8*>(ped);
    void* animPlayer = *reinterpret_cast<void**>(p + 0x78);
    if (!Core::Hook::CallMethod<void*>(0x9ED5A0, animPlayer, 0)) {
        uint32 unk = Core::Hook::CallMethod<uint32>(0x89C8C0, ped);
        Core::Hook::Call<void*>(0xA34CE0, animPlayer, animData[0], 0xF, Core::StaticRef<float>(0xDCB2A0), unk);
    }
}

// 0xA35470
void __cdecl CPedMoveBlend::StartIdleAnimFromPed(uint8* ped)
{
    uint8* subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    int32 idx = *reinterpret_cast<int32*>(subA90 + 0x3C);
    if (idx != -1) {
        uint32* animData = Core::Hook::Call<uint32*>(0xC54AD0, idx);
        if (!animData) {
            subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
            animData = Core::Hook::Call<uint32*>(0xC54F10, *reinterpret_cast<int32*>(subA90 + 0x3C));
        }
        StartIdleAnim(ped, animData);
    }
}

// 0xA354B0
bool __thiscall CPedMoveBlend::ProcessStopBlend(uint32* arg0, void* arg1, float arg2, bool arg3)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);

    if (!arg1) {
        arg1 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x15);
        if (!arg1) {
            arg1 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x16);
            if (!arg1) {
                arg1 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x13);
                if (!arg1) {
                    arg1 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x14);
                }
            }
        }
    }

    *reinterpret_cast<int32*>(self + 0x40) = -1;
    float f89d180 = Core::Hook::CallMethod<float>(0x89D180, *reinterpret_cast<void**>(self + 0x24));
    float rate = f89d180 / Core::StaticRef<float>(0xDB0870);

    if (arg3 || (*reinterpret_cast<uint32*>(self + 0x50) & 0x200u) || !Core::StaticRef<uint8>(0xF3594C)
        || !((*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(arg0) + 0x378) >> 8) & 1u))
    {
        *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x28)
            * Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xF35954) * rate;
        int32 id = static_cast<int32>(arg0[1]);
        if (id == -1) id = 0x31;
        Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, 1.0f, id);
        return true;
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if ((ped[0x1E2] & 0xFu) >= 2 && *reinterpret_cast<float*>(ped + 0xD5C) == Core::StaticRef<float>(0xDECD60)) {
        *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x28)
            * Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xF35954) * rate;
        int32 id = static_cast<int32>(arg0[1]);
        if (id == -1) id = 0x31;
        Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, 1.0f, id);
        return true;
    }

    if (ped[0x219]) {
        rate = Core::Hook::Call<float>(0x41EBC0, rate, Core::StaticRef<float>(0xF35958));
    }

    float diff = *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0);
    float wrapped = Core::Hook::Call<float>(0xA4E400, diff);
    float absWrapped = Core::Hook::Call<float>(0x424C00, wrapped) * Core::StaticRef<float>(0xD646A0);
    float clamped = (absWrapped > 1.0f) ? Core::StaticRef<float>(0xD50398) : absWrapped;
    float blendRate = clamped * Core::StaticRef<float>(0xD57060);

    uint8* anim10 = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x10);
    uint8* anim11 = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x11);

    if (wrapped != Core::StaticRef<float>(0xDECD60)
        && ((*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(arg0) + 0x378) >> 8) & 1u))
    {
        if (wrapped > Core::StaticRef<float>(0xF35950)) {
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x100000u;
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            void* intel = *reinterpret_cast<void**>(ped + 0x224);
            if (!Core::Hook::CallMethod<bool>(0x426910, static_cast<uint8*>(intel) + 0x2E0, 0x259, 0)) {
                *reinterpret_cast<uint32*>(ped + 0x29C) |= 0xC00000u;
            }
            if (!anim10) {
                anim10 = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x10, blendRate, arg0[1]);
                if (anim10) {
                    if (arg1) {
                        Core::Hook::CallMethod<void>(0xA4D6E0, anim10, Core::StaticRef<float>(0xD50964));
                    }
                    float rem = (Core::StaticRef<float>(0xD50398) - *reinterpret_cast<float*>(anim10 + 0x4C))
                        * Core::StaticRef<float>(0xD85DA8);
                    if (wrapped > rem) {
                        float d = wrapped - rem;
                        if (d > Core::StaticRef<float>(0xDECD60)) {
                            *reinterpret_cast<float*>(self + 0x68) = d / Core::Hook::CallMethod<float>(0xA4C450, anim10);
                        }
                    } else {
                        *reinterpret_cast<float*>(self + 0x68) = 0.0f;
                    }
                }
            } else if ((anim10[0x46] >> 2) & 1u) {
                *reinterpret_cast<float*>(self + 0x68) = wrapped / Core::Hook::CallMethod<float>(0xA4C450, anim10);
            }

            if (anim10) {
                if (arg1) {
                    *reinterpret_cast<uint32*>(anim10 + 0x4) |= 0x10000u;
                } else {
                    *reinterpret_cast<uint32*>(anim10 + 0x4) &= ~0x10000u;
                }
                *reinterpret_cast<float*>(anim10 + 0x54) = rate;
            }
            *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x68) * Core::StaticRef<float>(0x11DDE7C);
            return true;
        }

        if (Core::StaticRef<float>(0xEBDED0) - Core::StaticRef<float>(0xF35950) > wrapped) {
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x100000u;
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            *reinterpret_cast<uint32*>(ped + 0x29C) |= 0xC00000u;

            if (!anim11) {
                anim11 = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x11, blendRate, arg0[1]);
                if (anim11) {
                    if (arg1) {
                        Core::Hook::CallMethod<void>(0xA4D6E0, anim11, Core::StaticRef<float>(0xD50964));
                    }
                    float rem = (Core::StaticRef<float>(0xD50398) - *reinterpret_cast<float*>(anim11 + 0x4C))
                        * Core::StaticRef<float>(0xEBE050);
                    if (rem > wrapped) {
                        float d = wrapped - rem;
                        if (d < 0.0f) {
                            *reinterpret_cast<float*>(self + 0x68) = d / Core::Hook::CallMethod<float>(0xA4C450, anim11);
                        }
                    } else {
                        *reinterpret_cast<float*>(self + 0x68) = 0.0f;
                    }
                }
            } else if ((anim11[0x46] >> 2) & 1u) {
                *reinterpret_cast<float*>(self + 0x68) = wrapped / Core::Hook::CallMethod<float>(0xA4C450, anim11);
            }

            if (anim11) {
                if (arg1) {
                    *reinterpret_cast<uint32*>(anim11 + 0x4) |= 0x10000u;
                } else {
                    *reinterpret_cast<uint32*>(anim11 + 0x4) &= ~0x10000u;
                }
                *reinterpret_cast<float*>(anim11 + 0x54) = rate;
            }
            *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x68) * Core::StaticRef<float>(0x11DDE7C);
            return true;
        }

        if (Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x10)) {
            arg2 = Core::StaticRef<float>(0xDCB2A0);
        }
    }

    (void)anim10;
    uint32 unk = Core::Hook::CallMethod<uint32>(0x89C8C0, *reinterpret_cast<void**>(self + 0x24));
    Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, arg2, unk);
    *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x28)
        * Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xF35954) * rate;
    return true;
}

// 0xA35900
void* __thiscall CPedMoveBlend::ProcessStartTurnBlend(uint32* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    float diff = *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0);
    float wrapped = Core::Hook::Call<float>(0xA4E400, diff);

    float angle = wrapped;
    float minA = Core::StaticRef<float>(0xD88738);
    if (angle <= minA) {
        angle = minA;
    } else {
        float maxA = Core::StaticRef<float>(0xD88734);
        if (angle >= maxA) {
            angle = maxA;
        }
    }

    bool bIsCrouched = ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u) != 0;
    float desiredSpeed = *reinterpret_cast<float*>(self + 0x10);
    float speedThresh = bIsCrouched ? Core::StaticRef<float>(0xEBE08C) : Core::StaticRef<float>(0xDCB2A4);
    bool bHighSpeed = desiredSpeed >= speedThresh;

    if (bIsCrouched) {
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        if (ped[0x219] && Core::Hook::CallMethod<float>(0x9C0340, ped, 0) > 0.0f) {
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            uint8* ent2DC = *reinterpret_cast<uint8**>(ped + 0x2DC);
            bool bMatch = false;
            if (ent2DC) {
                int16 modelIdx = *reinterpret_cast<int16*>(ent2DC + 0x2E);
                using GetModelFn = int32(__thiscall*)(void*);
                int32 pedModel = reinterpret_cast<GetModelFn>((*reinterpret_cast<void***>(ped))[0x12C / 4])(ped);
                if (modelIdx == pedModel)
                    bMatch = true;
            }
            if (!bMatch && !Core::Hook::Call<bool>(0xA348A0, ped)) {
                bHighSpeed = true;
            }
        }
    }

    uint32 argFlags = arg0[0x378 / 4];
    void* animPlayer = *reinterpret_cast<void**>((*reinterpret_cast<uint8**>(self + 0x24)) + 0x78);

    if (!((argFlags >> 2) & 1u)
        || (*reinterpret_cast<uint32*>(self + 0x50) & 0x80000u)
        || Core::StaticRef<float>(0xDCB2A4) > *reinterpret_cast<float*>(self + 0x70))
    {
        bHighSpeed = false;
    }

    bool bAllowed = true;
    if (bIsCrouched) {
        bAllowed = bHighSpeed ? (Core::StaticRef<uint8>(0xF3594E) != 0) : (Core::StaticRef<uint8>(0xF3594D) != 0);
    } else {
        bAllowed = bHighSpeed ? (Core::StaticRef<uint8>(0xF3596C) != 0) : (Core::StaticRef<uint8>(0xF3594F) != 0);
    }
    if ((argFlags & 1u) && !((argFlags >> 1) & 1u)) {
        bAllowed = false;
    }

    float animRate = Core::StaticRef<float>(0x165D16C);
    static const int32 kSlotOrder[] = { 0x12, 0x13, 0x15, 0x14, 0x16, 0x17, 0x18, 0x1A, 0x19, 0x1B };
    uint8* activeAnim = nullptr;
    for (int32 slot : kSlotOrder) {
        activeAnim = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, slot);
        if (activeAnim)
            break;
    }

    if (activeAnim
        && Core::Hook::CallMethod<float>(0xA4DB70, activeAnim) >= 0.0f
        && ((*reinterpret_cast<uint32*>(activeAnim + 0x4) >> 4) & 1u)
        && !((activeAnim[0x46] >> 2) & 1u))
    {
        int32 id = *reinterpret_cast<int32*>(activeAnim + 0xC);
        *reinterpret_cast<float*>(activeAnim + 0x54) = animRate;
        if (id != 0x12 && id != 0x17) {
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x200000u;
        }
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        float heading = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4));
        if (*reinterpret_cast<uint32*>(self + 0x50) & 0x200000u) {
            float prev94 = *reinterpret_cast<float*>(self + 0x94);
            if (Core::Hook::Call<float>(0x6001C0, heading) == Core::Hook::Call<float>(0x6001C0, prev94)) {
                float sub = Core::Hook::Call<float>(0x5C90C0, prev94, heading);
                if (Core::Hook::Call<float>(0x424C00, sub) < Core::StaticRef<float>(0xD88730)) {
                    *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x90) * Core::StaticRef<float>(0x11DDE7C);
                    return activeAnim;
                }
            }
        }

        float turnAngle = *reinterpret_cast<float*>(self + 0x2C);
        float minT = Core::StaticRef<float>(0xEBDFFC);
        float maxT = Core::StaticRef<float>(0xEBDFF0);
        if (turnAngle < minT) turnAngle = minT;
        else if (turnAngle > maxT) turnAngle = maxT;
        *reinterpret_cast<float*>(self + 0x30) = turnAngle * Core::StaticRef<float>(0x11DDE7C);
        return activeAnim;
    }

    float absWrapped = (angle < 0.0f) ? (Core::StaticRef<float>(0xEBDED0) - angle) : angle;
    float targetBlend = 0.0f;
    int32 animSlot = 0;

    if (absWrapped >= Core::StaticRef<float>(0xDA63E4) && bAllowed) {
        if (absWrapped < Core::StaticRef<float>(0xD8872C)) {
            if (angle > 0.0f) {
                targetBlend = Core::StaticRef<float>(0xD85DA8);
                animSlot = (bHighSpeed ? 5 : 0) + 0x13;
            } else {
                targetBlend = Core::StaticRef<float>(0xEBE050);
                animSlot = (bHighSpeed ? 5 : 0) + 0x14;
            }
        } else {
            if (angle > 0.0f) {
                targetBlend = Core::StaticRef<float>(0xD88734);
                animSlot = (bHighSpeed ? 5 : 0) + 0x15;
            } else {
                targetBlend = Core::StaticRef<float>(0xD88738);
                animSlot = (bHighSpeed ? 5 : 0) + 0x16;
            }
        }
    } else {
        animSlot = (bHighSpeed ? 5 : 0) + 0x12;
    }

    using CheckV58Fn = bool(__thiscall*)(void*, void*);
    bool b58 = reinterpret_cast<CheckV58Fn>((*reinterpret_cast<void***>(this))[0x58 / 4])(this, animPlayer);
    float blendIn = b58 ? Core::StaticRef<float>(0xDCB2A0) : Core::StaticRef<float>(0xF35970);

    uint8* newAnim = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], animSlot, blendIn, arg0[1]);
    if (!newAnim)
        return nullptr;

    *reinterpret_cast<float*>(newAnim + 0x54) = animRate;
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<float*>(self + 0x94) = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4));

    if (targetBlend == 0.0f) {
        *reinterpret_cast<float*>(self + 0x90) = *reinterpret_cast<float*>(self + 0x28);
    } else {
        float denom = (Core::StaticRef<float>(0xEBE128) * Core::StaticRef<float>(0xDCEA38))
            * (*reinterpret_cast<float*>(ped + 0xAB8));
        float dur = Core::Hook::CallMethod<float>(0xA4C450, newAnim);
        float val = (angle - targetBlend) / dur;
        float negDenom = Core::StaticRef<float>(0xEBDED0) - denom;
        if (val < negDenom) val = negDenom;
        if (val > denom) val = denom;
        *reinterpret_cast<float*>(self + 0x90) = val;
    }

    *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x90) * Core::StaticRef<float>(0x11DDE7C);
    return newAnim;
}

// =========================================================================
// Slice 2: 0xA35E40 .. 0xA38608
// =========================================================================

// 0xA35E40
void __thiscall CPedMoveBlend::BuildBlendWeights(float* outWeights)
{
    float val = Core::StaticRef<float>(0x165D194);
    float frac = std::fmodf(val, 1.0f);
    float absFrac = (frac < 0.0f) ? (Core::StaticRef<float>(0xEBDED0) - frac) : frac;
    int32 baseIdx = static_cast<int32>(val);
    float eps = Core::StaticRef<float>(0xDAFA18);
    float one = Core::StaticRef<float>(0xD50398);
    float rate = Core::StaticRef<float>(0x1544AE0);

    int32* outIndices = reinterpret_cast<int32*>(outWeights);
    outIndices[0] = baseIdx;
    if (absFrac < eps) {
        outWeights[2] = one;
        outWeights[4] = rate;
    } else {
        outIndices[1] = baseIdx + 1;
        outWeights[2] = one - frac;
        outWeights[3] = frac;
        outWeights[4] = rate;
        outWeights[5] = rate;
    }
}

// 0xA35EE0
bool __thiscall CPedMoveBlend::CanTriggerQuickTurn(uint8* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!((*reinterpret_cast<uint32*>(arg0 + 0x378) >> 9) & 1u))
        return false;
    if (Core::Hook::Call<bool>(0x462F90))
        return false;
    if (Core::StaticRef<uint8>(0x18A82FC) != 0)
        return false;
    if (!((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u))
        return false;
    if (Core::StaticRef<uint32>(0x11DDE74) - *reinterpret_cast<uint32*>(self + 0x7C) < 0xBB8u)
        return false;

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    float diff = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));
    float absDiff = Core::Hook::Call<float>(0x424C00, diff);
    return absDiff >= Core::StaticRef<float>(0xF35978);
}

// 0xA35F70
void __thiscall CPedMoveBlend::UpdateSlopeAndObstacleCheck(uint8* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!Core::StaticRef<uint8>(0xF35986) || !((*reinterpret_cast<uint32*>(arg0 + 0x378) >> 11) & 1u)) {
        *reinterpret_cast<int16*>(self + 0xAA) = 0;
        return;
    }

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    if ((*reinterpret_cast<uint32*>(subA90 + 0x50) >> 1) & 1u) {
        *reinterpret_cast<int16*>(self + 0xAA) = 0;
        return;
    }

    float two = Core::StaticRef<float>(0xDCB2A4);
    bool bCrouched = ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u) != 0;
    if (bCrouched && *reinterpret_cast<float*>(self + 0x8) < two) {
        Core::StaticRef<uint8>(0x165D17B) = 0;
    }

    float thresh = Core::StaticRef<float>(0xF359CC);
    if (!(Core::StaticRef<uint32>(0x165D2C4) & 1u)) {
        Core::StaticRef<uint32>(0x165D2C4) |= 1u;
        float t = thresh * Core::StaticRef<float>(0xD82338);
        Core::StaticRef<float>(0x165D2C0) = t * t;
    }
    float distSqThresh = Core::StaticRef<float>(0x165D2C0);

    float* mat = *reinterpret_cast<float**>(ped + 0x20);
    float dx = mat[12] - *reinterpret_cast<float*>(self + 0x98);
    float dy = mat[13] - *reinterpret_cast<float*>(self + 0x9C);
    bool bMoved = false;

    if (!bCrouched || Core::StaticRef<uint8>(0x165D17B) == 0) {
        if (dx * dx + dy * dy > distSqThresh) {
            bool bCheck = (Core::StaticRef<uint8>(0x165D17A) != 0) || ((ped[0x14F] & 1u) != 0);
            bMoved = bCheck;
            if (bCheck && (*reinterpret_cast<uint8*>(self + 0xA8) & 1u)) {
                float dz = *reinterpret_cast<float*>(ped + 0xB20) - *reinterpret_cast<float*>(self + 0xA4);
                if (dz >= thresh) {
                    *reinterpret_cast<int16*>(self + 0xAA) = 1;
                } else if (Core::StaticRef<float>(0xEBDED0) - thresh >= dz) {
                    *reinterpret_cast<int16*>(self + 0xAA) = -1;
                } else {
                    *reinterpret_cast<int16*>(self + 0xAA) = 0;
                }
            } else {
                *reinterpret_cast<int16*>(self + 0xAA) = 0;
            }
            uint16 a8 = *reinterpret_cast<uint16*>(self + 0xA8);
            *reinterpret_cast<uint16*>(self + 0xA8) = (a8 & 0xFFFEu) | (bCheck ? 1u : 0u);
            *reinterpret_cast<float*>(self + 0x98) = mat[12];
            *reinterpret_cast<float*>(self + 0x9C) = mat[13];
            *reinterpret_cast<float*>(self + 0xA0) = mat[14];
            *reinterpret_cast<float*>(self + 0xA4) = *reinterpret_cast<float*>(ped + 0xB20);
        }
    }

    if (Core::StaticRef<uint8>(0xF35987) && !bMoved && bCrouched) {
        subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
        if (!((*reinterpret_cast<uint32*>(subA90 + 0x50) >> 1) & 1u)
            && *reinterpret_cast<float*>(self + 0x8) >= two)
        {
            uint32 now = Core::StaticRef<uint32>(0x11DDE74);
            if (now - Core::StaticRef<uint32>(0x165D180) >= 0xFAu) {
                Core::StaticRef<uint32>(0x165D180) = now;
                CVector4 velOut;
                using GetVelFn = CVector*(__thiscall*)(void*, CVector4*);
                CVector* vel = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(ped))[0xEC / 4])(ped, &velOut);
                float mult = Core::StaticRef<float>(0xD885EC);
                ped = *reinterpret_cast<uint8**>(self + 0x24);
                float* pMat = *reinterpret_cast<float**>(ped + 0x20);
                CVector probePos(
                    pMat[12] + vel->x * mult,
                    pMat[13] + vel->y * mult,
                    pMat[14] + vel->z * mult + Core::StaticRef<float>(0xD50398));

                alignas(16) uint8 probe[0x60]{};
                Core::Hook::CallMethod<void>(0x425360, probe);
                float zBot = probePos.z - Core::StaticRef<float>(0xDCB2A0);
                if (Core::Hook::Call<bool>(0x991E90, &probePos, zBot, ped, probe, 6, 0)
                    && ((*reinterpret_cast<uint32*>(probe + 0x48) >> 24) & 1u))
                {
                    Core::StaticRef<uint8>(0x165D17B) = 1;
                    float hitZ = *reinterpret_cast<float*>(probe + 0x18);
                    float* curMat = *reinterpret_cast<float**>((*reinterpret_cast<uint8**>(self + 0x24)) + 0x20);
                    float dz = hitZ - (curMat[14] - Core::StaticRef<float>(0xD50398));
                    if (dz > thresh) {
                        *reinterpret_cast<int16*>(self + 0xAA) = 1;
                    } else if (Core::StaticRef<float>(0xEBDED0) - thresh > dz) {
                        *reinterpret_cast<int16*>(self + 0xAA) = -1;
                    } else {
                        *reinterpret_cast<int16*>(self + 0xAA) = 0;
                    }
                } else {
                    *reinterpret_cast<int16*>(self + 0xAA) = 0;
                    Core::StaticRef<uint8>(0x165D17B) = 0;
                }
            }
        }
    }

    if (*reinterpret_cast<int16*>(self + 0xAA) != 0) {
        *reinterpret_cast<float*>(self + 0x78) = Core::StaticRef<float>(0xF359C8);
    }
}

// 0xA362E0
void __thiscall CPedMoveBlend::UpdateHeadingAndSpeed(uint8* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint32 flags26C = *reinterpret_cast<uint32*>(ped + 0x26C);

    if ((flags26C & 0x2000u) || (!(flags26C & 1u) && !(*reinterpret_cast<uint32*>(ped + 0x29C) & 4u))) {
        *reinterpret_cast<float*>(self + 0x8) = *reinterpret_cast<float*>(self + 0x10);
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        *reinterpret_cast<float*>(self + 0x2C) = 0.0f;
        *reinterpret_cast<float*>(self + 0x28) = 0.0f;
        return;
    }

    uint32 blendFlags = *reinterpret_cast<uint32*>(self + 0x50);
    bool bCrouched = ((blendFlags >> 13) & 1u) != 0;
    float accel = bCrouched ? Core::StaticRef<float>(0xF27D54) : Core::StaticRef<float>(0xF27D4C);
    float decel = bCrouched ? Core::StaticRef<float>(0xF27D58) : Core::StaticRef<float>(0xF27D50);
    float targetY = *reinterpret_cast<float*>(self + 0x10);

    if (!(blendFlags & 0x20u) && *reinterpret_cast<int32*>(self + 0x48) == -1) {
        float curY = *reinterpret_cast<float*>(self + 0x8);
        if (targetY > curY) {
            curY += Core::StaticRef<float>(0x11DDE7C) * accel;
            if (curY > targetY) curY = targetY;
            float maxSpd = *reinterpret_cast<float*>(arg0 + 0x350) + Core::StaticRef<float>(0xD50398);
            if (curY > maxSpd) curY = maxSpd;
            *reinterpret_cast<float*>(self + 0x8) = curY;
        } else if (curY > targetY) {
            curY -= Core::StaticRef<float>(0x11DDE7C) * decel;
            if (targetY > curY) curY = targetY;
            if (curY < 0.0f) curY = 0.0f;
            *reinterpret_cast<float*>(self + 0x8) = curY;
        } else {
            *reinterpret_cast<float*>(self + 0x8) = targetY;
        }
    }

    *reinterpret_cast<uint32*>(self + 0x50) = blendFlags & ~0x20u;
    *reinterpret_cast<float*>(ped + 0xAB0) = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB0));
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<float*>(ped + 0xAB4) = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4));
    ped = *reinterpret_cast<uint8**>(self + 0x24);

    float diff = *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0);
    *reinterpret_cast<float*>(self + 0x2C) = diff;
    float prevDiff = *reinterpret_cast<float*>(self + 0x58) - *reinterpret_cast<float*>(ped + 0xAB4);

    if ((Core::StaticRef<uint8>(0x165D174) && bCrouched) || (Core::StaticRef<uint8>(0x165D175) && !bCrouched)) {
        if (WrapPiDiff(prevDiff) > Core::StaticRef<float>(0xEBE478)) {
            if (Core::Hook::Call<float>(0x6001C0, diff) == Core::Hook::Call<float>(0x6001C0, prevDiff)) {
                *reinterpret_cast<float*>(self + 0x2C) = diff + prevDiff;
            }
        }
    }

    float& turnTarget = *reinterpret_cast<float*>(self + 0x2C);
    if (turnTarget > Core::StaticRef<float>(0xEBDFF0)) {
        turnTarget -= Core::StaticRef<float>(0xEBDEE4);
    } else if (Core::StaticRef<float>(0xEBDFFC) > turnTarget) {
        turnTarget += Core::StaticRef<float>(0xEBDEE4);
    }

    if (Core::StaticRef<float>(0xEBE3F0) > WrapPiDiff(turnTarget)) {
        *reinterpret_cast<float*>(self + 0x30) = turnTarget;
        turnTarget = 0.0f;
    }

    float turnRate = Core::StaticRef<float>(0xF27D6C);
    float turnAccel = Core::StaticRef<float>(0xF27D68);
    bool bTurnScaled = bCrouched
        ? (Core::StaticRef<uint8>(0x165D174) != 0) // wait: check 0xF3596D / 0xF3596E
        : (Core::StaticRef<uint8>(0xF3596E) != 0);
    bTurnScaled = (Core::StaticRef<uint8>(0xF3596D) && bCrouched) || (Core::StaticRef<uint8>(0xF3596E) && !bCrouched);
    bool bTurnSq = (Core::StaticRef<uint8>(0xF3596F) && bCrouched) || (Core::StaticRef<uint8>(0xF3597C) && !bCrouched);

    if (bTurnScaled) {
        float t = WrapPiDiff(turnTarget) * Core::StaticRef<float>(0xD646A0) + Core::StaticRef<float>(0xD885EC);
        float one = Core::StaticRef<float>(0xD50398);
        if (t < 0.0f) t = 0.0f;
        else if (t > one) t = one;
        float tOrig = t;
        if (bTurnSq) {
            t = t * t;
            if (t < 0.0f) t = 0.0f;
            else if (t > one) t = one;
        }
        if (Core::StaticRef<uint8>(0x165D2CB) && !bCrouched) {
            if (*reinterpret_cast<float*>(self + 0x8) > 0.0f && *reinterpret_cast<float*>(self + 0x20) > one) {
                float mult = Core::StaticRef<float>(0xD885EC);
                float v = tOrig;
                if (Core::StaticRef<uint8>(0x165D2CA)) {
                    v = v * mult + mult;
                }
                Core::StaticRef<float>(0xF359D4) = v;
                Core::StaticRef<uint8>(0x165D18C) = 1;
            }
        } else {
            turnAccel *= t;
            turnRate *= t;
        }
    }

    float div = bCrouched ? Core::StaticRef<float>(0xF359C0) : Core::StaticRef<float>(0xF359C4);
    turnTarget /= div;

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* veh = *reinterpret_cast<uint8**>(ped + 0xAC0);
    float maxTurn = *reinterpret_cast<float*>(ped + 0xAB8);
    if (veh && ((*reinterpret_cast<uint32*>(veh + 0x28) & 0x3C0u) == 0x80u)) {
        if ((Core::StaticRef<uint8>(0x165D2C9) || !bCrouched) && *reinterpret_cast<int32*>(veh + 0x1354) == 2) {
            maxTurn *= Core::StaticRef<float>(0xD5829C);
        }
    }
    maxTurn *= Core::StaticRef<float>(0xEBE128) * Core::StaticRef<float>(0xDCEA38);
    if (turnTarget > maxTurn) {
        turnTarget = maxTurn;
    } else {
        float negMax = Core::StaticRef<float>(0xEBDED0) - maxTurn;
        if (negMax > turnTarget)
            turnTarget = negMax;
    }

    if ((Core::StaticRef<uint8>(0x165D178) && bCrouched) || Core::StaticRef<uint8>(0x165D177)) {
        if (Core::StaticRef<uint8>(0x165D2C8) || (*reinterpret_cast<uint32*>(ped + 0x29C) & 0x800u)) {
            if (*reinterpret_cast<float*>(self + 0x8) >= Core::StaticRef<float>(0xDCB2A4)) {
                float cur28 = *reinterpret_cast<float*>(self + 0x28);
                if ((cur28 > 0.0f && turnTarget < 0.0f) || (cur28 < 0.0f && turnTarget > 0.0f)) {
                    *reinterpret_cast<float*>(self + 0x28) = turnTarget;
                }
            }
        }
    }

    float cur28 = *reinterpret_cast<float*>(self + 0x28);
    float dt = Core::StaticRef<float>(0x11DDE7C);
    if (turnTarget > cur28 && cur28 > Core::StaticRef<float>(0xEBE1E4)) {
        cur28 += dt * turnAccel;
        if (cur28 > turnTarget) cur28 = turnTarget;
        *reinterpret_cast<float*>(self + 0x28) = cur28;
    } else if (cur28 > turnTarget && Core::StaticRef<float>(0xDAFA18) > cur28) {
        cur28 -= dt * turnAccel;
        if (turnTarget > cur28) cur28 = turnTarget;
        *reinterpret_cast<float*>(self + 0x28) = cur28;
    } else if (turnTarget > cur28) {
        cur28 += dt * turnRate;
        if (cur28 > turnTarget) cur28 = turnTarget;
        *reinterpret_cast<float*>(self + 0x28) = cur28;
    } else {
        cur28 -= dt * turnRate;
        if (turnTarget > cur28) cur28 = turnTarget;
        *reinterpret_cast<float*>(self + 0x28) = cur28;
    }
}

// 0xA36820
void __thiscall CPedMoveBlend::UpdateHeadingOnly()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<float*>(ped + 0xAB0) = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB0));
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<float*>(ped + 0xAB4) = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4));
    ped = *reinterpret_cast<uint8**>(self + 0x24);

    float diff = *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0);
    if (diff > Core::StaticRef<float>(0xEBDFF0)) diff -= Core::StaticRef<float>(0xEBDEE4);
    else if (Core::StaticRef<float>(0xEBDFFC) > diff) diff += Core::StaticRef<float>(0xEBDEE4);

    if (Core::StaticRef<float>(0xEBE3F0) > WrapPiDiff(diff)) {
        *reinterpret_cast<float*>(self + 0x30) = diff;
        diff = 0.0f;
    }
    *reinterpret_cast<float*>(self + 0x2C) = diff;

    float maxTurn = *reinterpret_cast<float*>(ped + 0xAB8) * Core::StaticRef<float>(0xEBE128) * Core::StaticRef<float>(0xDCEA38);
    float rate1 = Core::StaticRef<float>(0xF27D70);
    float rate2 = Core::StaticRef<float>(0xF27D74);
    float div = Core::StaticRef<float>(0xD796E4);

    if (ped) {
        using HasWeaponFn = bool(__thiscall*)(void*);
        if (reinterpret_cast<HasWeaponFn>((*reinterpret_cast<void***>(ped))[0x128 / 4])(ped)) {
            bool bFlag = Core::Hook::Call<bool>(0x9B16D0, 1) || Core::Hook::Call<bool>(0x9ADB90);
            uint8* cam = Core::Hook::Call<uint8*>(0x7C3900);
            if (cam[0x328D]) {
                if (bFlag) {
                    float s = Core::StaticRef<float>(0xD50960);
                    div *= s;
                    rate1 *= s;
                    rate2 *= s;
                    maxTurn *= Core::StaticRef<float>(0xD7E2BC);
                    (void)div;
                    div = Core::StaticRef<float>(0xDAFA18);
                } else {
                    float s = Core::StaticRef<float>(0xDCB2A0);
                    rate1 *= s;
                    rate2 *= s;
                    maxTurn *= s;
                }
            }
        }
    }

    float& target = *reinterpret_cast<float*>(self + 0x2C);
    target /= div;
    if (target > maxTurn) target = maxTurn;
    else {
        float negMax = Core::StaticRef<float>(0xEBDED0) - maxTurn;
        if (negMax > target) target = negMax;
    }

    float cur = *reinterpret_cast<float*>(self + 0x28);
    float dt = Core::StaticRef<float>(0x11DDE7C);
    if (target > cur && cur > Core::StaticRef<float>(0xEBE1E4)) {
        cur += dt * rate1;
        if (cur > target) cur = target;
    } else if (cur > target && Core::StaticRef<float>(0xDAFA18) > cur) {
        cur -= dt * rate1;
        if (target > cur) cur = target;
    } else if (target > cur) {
        cur += dt * rate2;
        if (cur > target) cur = target;
    } else {
        cur -= dt * rate2;
        if (target > cur) cur = target;
    }
    *reinterpret_cast<float*>(self + 0x28) = cur;
}

// 0xA36AA0
bool __thiscall CPedMoveBlend::Blend4WayStrafe(uint32* arg0, float* arg1, float arg2, bool arg3)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    float blendRate = (*reinterpret_cast<uint8*>(self + 0x52) & 1u)
        ? Core::StaticRef<float>(0xDCB2A0) : Core::StaticRef<float>(0xD885EC);

    float ax = WrapPiDiff(arg1[0]);
    float ay = WrapPiDiff(arg1[1]);

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    float heading = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));

    bool bZeroInput = (arg2 >= ax && arg2 >= ay);
    float px = WrapPiDiff(*reinterpret_cast<float*>(self + 0x14));
    float py = WrapPiDiff(*reinterpret_cast<float*>(self + 0x18));
    bool bPrevZero = (arg2 >= px && arg2 >= py);

    bool bSkip = false;
    if (Core::StaticRef<uint32>(0x11DDE74) - *reinterpret_cast<uint32*>(self + 0x74) > 0xFAu) {
        bSkip = Core::Hook::CallMethod<bool>(0x89C780, ped);
    }

    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);
    if (bZeroInput) {
        uint32 flags = arg0[0x378 / 4];
        if (((flags >> 8) & 1u) && !bSkip) {
            float spd = *reinterpret_cast<float*>(self + 0x28);
            if (spd > Core::StaticRef<float>(0xD88600)) {
                float w = Core::StaticRef<float>(0xF3595C) * heading;
                if (w < 0.0f) w = Core::StaticRef<float>(0xEBDED0) - w;
                float minW = Core::StaticRef<float>(0xF35960);
                float maxW = Core::StaticRef<float>(0xF35964);
                if (w < minW) w = minW;
                else if (w > maxW) w = maxW;
                uint8* anim = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x10, Core::StaticRef<float>(0xDCB2A0), arg0[1]);
                if (anim) *reinterpret_cast<float*>(anim + 0x54) = w;
                *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
                return true;
            } else if (Core::StaticRef<float>(0xD8875C) > spd) {
                float w = Core::Hook::Call<float>(0x424C00, Core::StaticRef<float>(0xF3595C) * heading);
                float clamped = Core::Hook::Call<float>(0x404E50, w, Core::StaticRef<float>(0xF35960), Core::StaticRef<float>(0xF35964));
                uint8* anim = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x11, Core::StaticRef<float>(0xDCB2A0), arg0[1]);
                if (anim) *reinterpret_cast<float*>(anim + 0x54) = clamped;
                *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
                return true;
            } else {
                if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x10))
                    Core::Hook::CallMethod<void>(0xA4DBC0, a, Core::StaticRef<float>(0xEBDED4));
                if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x11))
                    Core::Hook::CallMethod<void>(0xA4DBC0, a, Core::StaticRef<float>(0xEBDED4));
            }
        }

        if ((arg0[0x378 / 4] >> 8) & 1u) {
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            *reinterpret_cast<float*>(ped + 0xAB0) = *reinterpret_cast<float*>(ped + 0xAB4);
        }

        void* a10 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x10);
        void* a11 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x11);
        if (!((arg0[0x378 / 4] >> 8) & 1u)) {
            if (a10) Core::Hook::CallMethod<void>(0xA4DBC0, a10, Core::StaticRef<float>(0xEBDED4));
            if (a11) Core::Hook::CallMethod<void>(0xA4DBC0, a11, Core::StaticRef<float>(0xEBDED4));
            a10 = nullptr;
            a11 = nullptr;
        }

        bool bHasGroup = ((arg0[0x378 / 4] >> 10) & 1u) != 0;
        uint32 g0 = bHasGroup ? arg0[0] : 0xFFFFFFFFu;
        uint32 g1 = bHasGroup ? arg0[1] : 0xFFFFFFFFu;

        if (!bSkip && !(self[0x52] & 1u) && (!bPrevZero || a10 || a11)) {
            if (g0 == 0xFFFFFFFFu && g1 == 0xFFFFFFFFu) {
                Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, Core::StaticRef<float>(0xD885FC), arg0[1]);
            } else {
                Core::Hook::Call<void*>(0xA34CE0, animPlayer, g0, 0x4B, blendRate, g1);
            }
        } else {
            uint8* a4B = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x4B);
            if (a4B) {
                if ((a4B[0x46] >> 2) & 1u) {
                    Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, Core::StaticRef<float>(0xD885FC), arg0[1]);
                } else if (g0 != 0xFFFFFFFFu || g1 != 0xFFFFFFFFu) {
                    Core::Hook::Call<void*>(0xA34CE0, animPlayer, g0, 0x4B, 1.0f, g1);
                }
            } else {
                Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, blendRate, arg0[1]);
            }
        }
        *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
        return true;
    }

    float mag = std::sqrtf(arg1[0] * arg1[0] + arg1[1] * arg1[1]);
    float maxSpd = *reinterpret_cast<float*>(self + 0x70);
    if (mag > maxSpd) mag = maxSpd;

    float one = Core::StaticRef<float>(0xD50398);
    float two = Core::StaticRef<float>(0xDCB2A4);
    float w0 = 0.0f, w1 = 0.0f, w2 = 0.0f, w3 = 0.0f;
    if (mag < one) {
        w0 = mag;
        w2 = one - mag;
    } else {
        w1 = (mag >= two) ? one : (mag - one);
        w0 = one - w1;
    }
    if (mag > two) {
        w3 = (mag >= Core::StaticRef<float>(0xD885D4)) ? one : (mag - two);
    }

    float dirAngle = -Core::Hook::Call<float>(0xA4E780, arg1[0], arg1[1], 0.0f, 0.0f);
    if (dirAngle < 0.0f) dirAngle += Core::StaticRef<float>(0xEBE6C4);

    float lo = 0.0f;
    float hi = Core::StaticRef<float>(0xD6B868);
    float curW = 0.0f;
    float rate = Core::StaticRef<float>(0x165D16C);
    const auto* tbl4 = &Core::StaticRef<int32>(0xD8874C);
    const auto* tbl4Sprint = &Core::StaticRef<int32>(0xD8873C);

    for (int32 i = 0; i < 4; ++i) {
        curW = 0.0f;
        bool bStop = Core::Hook::Call<bool>(0x9B3B00, 1) || Core::Hook::Call<bool>(0x9ADB90);
        if (w3 != 0.0f && !bStop) {
            float t = 0.0f;
            if (i == 0) {
                t = (hi - dirAngle) * w3 * Core::StaticRef<float>(0xD6B3D0) + Core::StaticRef<float>(0xDCC5FC);
            } else if (i == 3) {
                t = (dirAngle - Core::StaticRef<float>(0xEBE6C4) + hi) * w3 * Core::StaticRef<float>(0xD6B3D0) + Core::StaticRef<float>(0xDCC5FC);
            }
            if (t > one) t = one;
            else if (t <= 0.0f) t = 0.0f;
            curW = t;
            if (t != 0.0f) {
                w0 = 0.0f;
                w1 = one - t;
            }
        }

        if (dirAngle == lo) {
            if (w0 != 0.0f) PushMoveBlendSlot(tbl4[i], w0, rate);
            if (w1 != 0.0f && !arg3) AddMoveBlendSlotCall(tbl4Sprint[i], w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (curW != 0.0f && !arg3) AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            break;
        }
        if (dirAngle == hi) {
            int32 next = (i + 1) & 3;
            if (w0 != 0.0f) AddMoveBlendSlotCall(tbl4[next], w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (w1 != 0.0f && !arg3) AddMoveBlendSlotCall(tbl4Sprint[next], w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (curW != 0.0f && !arg3) AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            break;
        }
        if (dirAngle > lo && hi > dirAngle) {
            float frac = (hi == lo) ? one : ((dirAngle - lo) / (hi - lo));
            int32 next = (i + 1) & 3;
            if (w0 != 0.0f) {
                AddMoveBlendSlotCall(tbl4[i], (one - frac) * w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
                AddMoveBlendSlotCall(tbl4[next], frac * w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            if (w1 != 0.0f && !arg3) {
                AddMoveBlendSlotCall(tbl4Sprint[i], (one - frac) * w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
                AddMoveBlendSlotCall(tbl4Sprint[next], frac * w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            if (curW != 0.0f && !arg3) {
                AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            break;
        }
        lo = hi;
        hi += Core::StaticRef<float>(0xD6B868);
    }

    if (w2 != 0.0f) {
        PushMoveBlendSlot(0xF, w2, one);
    }
    return false;
}

// 0xA37540
bool __thiscall CPedMoveBlend::Blend8WayStrafe(uint32* arg0, float* arg1, float arg2, bool arg3)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    float blendRate = (*reinterpret_cast<uint8*>(self + 0x52) & 1u)
        ? Core::StaticRef<float>(0xDCB2A0) : Core::StaticRef<float>(0xD885EC);

    float ax = WrapPiDiff(arg1[0]);
    float ay = WrapPiDiff(arg1[1]);

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    float heading = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));

    bool bZeroInput = (arg2 >= ax && arg2 >= ay);
    float px = WrapPiDiff(*reinterpret_cast<float*>(self + 0x14));
    float py = WrapPiDiff(*reinterpret_cast<float*>(self + 0x18));
    bool bPrevZero = (arg2 >= px && arg2 >= py);

    bool bSkip = false;
    if (Core::StaticRef<uint32>(0x11DDE74) - *reinterpret_cast<uint32*>(self + 0x74) > 0xFAu) {
        bSkip = Core::Hook::CallMethod<bool>(0x89C780, ped);
    }

    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);
    if (bZeroInput) {
        uint32 flags = arg0[0x378 / 4];
        if (((flags >> 8) & 1u) && !bSkip) {
            float spd = *reinterpret_cast<float*>(self + 0x28);
            if (spd > Core::StaticRef<float>(0xD88600)) {
                float w = Core::StaticRef<float>(0xF3595C) * heading;
                if (w < 0.0f) w = Core::StaticRef<float>(0xEBDED0) - w;
                float minW = Core::StaticRef<float>(0xF35960);
                float maxW = Core::StaticRef<float>(0xF35964);
                if (w < minW) w = minW;
                else if (w > maxW) w = maxW;
                uint8* anim = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x10, Core::StaticRef<float>(0xDCB2A0), arg0[1]);
                if (anim) *reinterpret_cast<float*>(anim + 0x54) = w;
                *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
                return true;
            } else if (Core::StaticRef<float>(0xD8875C) > spd) {
                float w = Core::Hook::Call<float>(0x424C00, Core::StaticRef<float>(0xF3595C) * heading);
                float clamped = Core::Hook::Call<float>(0x404E50, w, Core::StaticRef<float>(0xF35960), Core::StaticRef<float>(0xF35964));
                uint8* anim = Core::Hook::Call<uint8*>(0xA34CE0, animPlayer, arg0[0], 0x11, Core::StaticRef<float>(0xDCB2A0), arg0[1]);
                if (anim) *reinterpret_cast<float*>(anim + 0x54) = clamped;
                *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
                return true;
            } else {
                if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x10))
                    Core::Hook::CallMethod<void>(0xA4DBC0, a, Core::StaticRef<float>(0xEBDED4));
                if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x11))
                    Core::Hook::CallMethod<void>(0xA4DBC0, a, Core::StaticRef<float>(0xEBDED4));
            }
        }

        if ((arg0[0x378 / 4] >> 8) & 1u) {
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            *reinterpret_cast<float*>(ped + 0xAB0) = *reinterpret_cast<float*>(ped + 0xAB4);
        }

        void* a10 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x10);
        void* a11 = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x11);
        if (!((arg0[0x378 / 4] >> 8) & 1u)) {
            if (a10) Core::Hook::CallMethod<void>(0xA4DBC0, a10, Core::StaticRef<float>(0xEBDED4));
            if (a11) Core::Hook::CallMethod<void>(0xA4DBC0, a11, Core::StaticRef<float>(0xEBDED4));
            a10 = nullptr;
            a11 = nullptr;
        }

        bool bHasGroup = ((arg0[0x378 / 4] >> 10) & 1u) != 0;
        uint32 g0 = bHasGroup ? arg0[0] : 0xFFFFFFFFu;
        uint32 g1 = bHasGroup ? arg0[1] : 0xFFFFFFFFu;

        if (!bSkip && !(self[0x52] & 1u) && (!bPrevZero || a10 || a11)) {
            if (g0 != 0xFFFFFFFFu || g1 != 0xFFFFFFFFu) {
                Core::Hook::Call<void*>(0xA34CE0, animPlayer, g0, 0x4B, blendRate, g1);
            }
        } else {
            uint8* a4B = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x4B);
            if (a4B) {
                if ((a4B[0x46] >> 2) & 1u) {
                    Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, Core::StaticRef<float>(0xD885FC), arg0[1]);
                } else if (g0 == 0xFFFFFFFFu && g1 == 0xFFFFFFFFu) {
                    Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, Core::StaticRef<float>(0xD885FC), arg0[1]);
                } else {
                    Core::Hook::Call<void*>(0xA34CE0, animPlayer, g0, 0x4B, 1.0f, g1);
                }
            } else {
                Core::Hook::Call<void*>(0xA34CE0, animPlayer, arg0[0], 0xF, blendRate, arg0[1]);
            }
        }
        *reinterpret_cast<float*>(self + 0x30) *= Core::StaticRef<float>(0xF35968);
        return true;
    }

    float mag = std::sqrtf(arg1[0] * arg1[0] + arg1[1] * arg1[1]);
    float maxSpd = *reinterpret_cast<float*>(self + 0x70);
    if (mag > maxSpd) mag = maxSpd;

    float one = Core::StaticRef<float>(0xD50398);
    float two = Core::StaticRef<float>(0xDCB2A4);
    float w0 = 0.0f, w1 = 0.0f, w2 = 0.0f, w3 = 0.0f;
    if (mag < one) {
        w0 = mag;
        w2 = one - mag;
    } else {
        w1 = (mag >= two) ? one : (mag - one);
        w0 = one - w1;
    }
    if (mag > two) {
        w3 = (mag >= Core::StaticRef<float>(0xD885D4)) ? one : (mag - two);
    }

    float dirAngle = -Core::Hook::Call<float>(0xA4E780, arg1[0], arg1[1], 0.0f, 0.0f);
    if (dirAngle < 0.0f) dirAngle += Core::StaticRef<float>(0xEBE6C4);

    float lo = 0.0f;
    float hi = Core::StaticRef<float>(0xD582B0);
    float curW = 0.0f;
    float rate = Core::StaticRef<float>(0x165D16C);
    const auto* tbl8 = &Core::StaticRef<int32>(0xD88780);
    const auto* tbl8Sprint = &Core::StaticRef<int32>(0xD88760);

    for (int32 i = 0; i < 8; ++i) {
        curW = 0.0f;
        bool bStop = Core::Hook::Call<bool>(0x9B3B00, 1) || Core::Hook::Call<bool>(0x9ADB90);
        if (w3 != 0.0f && !bStop) {
            float t = 0.0f;
            if (i == 0 || i == 1) {
                t = (Core::StaticRef<float>(0xD6B868) - dirAngle) * w3 * Core::StaticRef<float>(0xD6B3D0) + Core::StaticRef<float>(0xDCC5FC);
            } else if (i == 6 || i == 7) {
                t = (dirAngle - Core::StaticRef<float>(0xEBE6C4) + Core::StaticRef<float>(0xD6B868)) * w3 * Core::StaticRef<float>(0xD6B3D0) + Core::StaticRef<float>(0xDCC5FC);
            }
            if (t > one) t = one;
            else if (t <= 0.0f) t = 0.0f;
            curW = t;
            if (t != 0.0f) {
                w0 = 0.0f;
                w1 = one - t;
            }
        }

        if (dirAngle == lo) {
            if (w0 != 0.0f) PushMoveBlendSlot(tbl8[i], w0, rate);
            if (w1 != 0.0f && !arg3) AddMoveBlendSlotCall(tbl8Sprint[i], w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (curW != 0.0f && !arg3) AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            break;
        }
        if (dirAngle == hi) {
            int32 next = (i + 1) & 7;
            if (w0 != 0.0f) AddMoveBlendSlotCall(tbl8[next], w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (w1 != 0.0f && !arg3) AddMoveBlendSlotCall(tbl8Sprint[next], w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            if (curW != 0.0f && !arg3) AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            break;
        }
        if (dirAngle > lo && hi > dirAngle) {
            float frac = (hi == lo) ? one : ((dirAngle - lo) / (hi - lo));
            int32 next = (i + 1) & 7;
            if (w0 != 0.0f) {
                AddMoveBlendSlotCall(tbl8[i], (one - frac) * w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
                AddMoveBlendSlotCall(tbl8[next], frac * w0, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            if (w1 != 0.0f && !arg3) {
                AddMoveBlendSlotCall(tbl8Sprint[i], (one - frac) * w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
                AddMoveBlendSlotCall(tbl8Sprint[next], frac * w1, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            if (curW != 0.0f && !arg3) {
                AddMoveBlendSlotCall(0x32, curW, 0.0f, 0.0f, 0.0f, 0, -1, rate);
            }
            break;
        }
        lo = hi;
        hi += Core::StaticRef<float>(0xD582B0);
    }

    if (w2 != 0.0f) {
        PushMoveBlendSlot(0xF, w2, one);
    }
    return false;
}

// 0xA38020
bool __thiscall CPedMoveBlend::ProcessTurnAndMovement(uint32* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);

    if (arg0[0] == 0x3Bu) {
        bool bAny = false;
        uint8* node = Core::Hook::CallMethod<uint8*>(0x9EC060, animPlayer, 0, 2);
        while (node) {
            if (*reinterpret_cast<uint32*>(node + 0x10) != 0x3Bu) {
                Core::Hook::CallMethod<void>(0xA4DBC0, node, Core::StaticRef<float>(0xD886BC));
                bAny = true;
            }
            node = Core::Hook::CallMethod<uint8*>(0x9EC0F0, animPlayer, 0, 2);
        }
        if (bAny) {
            Core::Hook::CallMethod<void>(0x9EFEB0, animPlayer, 0x3B, 0xF, Core::StaticRef<float>(0xDCB2A0), -1);
            return true;
        }
    }

    if (!(ped[0x29C] & 4u)) {
        if (void* a = Core::Hook::CallMethod<void*>(0x9EBE90, animPlayer, 0, 8)) {
            Core::Hook::CallMethod<void>(0xA4DBC0, a, Core::StaticRef<float>(0xD886BC));
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if (*reinterpret_cast<uint32*>(ped + 0x2A0) & 0x400u) {
        *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
    }

    void* a52 = Core::Hook::CallMethod<void*>(0x9EBEE0, animPlayer, 0x52, 0);
    float mag0 = (*reinterpret_cast<float*>(self + 0x4) * *reinterpret_cast<float*>(self + 0x4))
        + (*reinterpret_cast<float*>(self + 0x8) * *reinterpret_cast<float*>(self + 0x8));
    float mag1 = (*reinterpret_cast<float*>(self + 0xC) * *reinterpret_cast<float*>(self + 0xC))
        + (*reinterpret_cast<float*>(self + 0x10) * *reinterpret_cast<float*>(self + 0x10));

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if (mag0 != 0.0f || mag1 != 0.0f || (*reinterpret_cast<uint32*>(ped + 0x2A0) & 0x400u)) {
        if (a52) {
            Core::Hook::CallMethod<void>(0xA4DBC0, a52, Core::StaticRef<float>(0xEBE008));
        }
        return false;
    }

    *reinterpret_cast<uint32*>(self + 0x50) &= ~0x40000u;
    bool bFlag89 = Core::Hook::CallMethod<bool>(0x89C780, ped);
    bool bBit10 = (*reinterpret_cast<uint32*>(self + 0x50) & 0x400u) != 0;

    if (bFlag89 && !bBit10) {
        void* sub2B0 = Core::Hook::CallMethod<void*>(0x9A9CF0, *reinterpret_cast<uint8**>(self + 0x24) + 0x2B0);
        int32 id = (sub2B0 && Core::Hook::CallMethod<bool>(0x89BD60, sub2B0)) ? 0x3D : 0x3C;
        Core::Hook::CallMethod<void>(0x9EFE40, animPlayer, id, 0x50, 0, 0, Core::StaticRef<float>(0xDCB2A0), -1);
        if (void* aF = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0xF)) {
            Core::Hook::CallMethod<void>(0xA4DBC0, aF, Core::StaticRef<float>(0xD886BC));
        }
        *reinterpret_cast<uint32*>(self + 0x40) = *reinterpret_cast<uint32*>(self + 0x3C);
        ResetBlendFlagsAndSaveTargetHeading();
        StartIdleAnim(*reinterpret_cast<void**>(self + 0x24), arg0);
        return true;
    }

    if (!bFlag89 && bBit10) {
        void* sub2B0 = Core::Hook::CallMethod<void*>(0x9A9CF0, *reinterpret_cast<uint8**>(self + 0x24) + 0x2B0);
        int32 id = (sub2B0 && Core::Hook::CallMethod<bool>(0x89BD60, sub2B0)) ? 0x3D : 0x3C;
        Core::Hook::CallMethod<void>(0x9EFE40, animPlayer, id, 0x51, 0, 0, Core::StaticRef<float>(0xDCB2A0), -1);
        if (void* aF = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0xF)) {
            Core::Hook::CallMethod<void>(0xA4DBC0, aF, Core::StaticRef<float>(0xD886BC));
        }
        *reinterpret_cast<uint32*>(self + 0x40) = *reinterpret_cast<uint32*>(self + 0x3C);
        ResetBlendFlagsAndSaveTargetHeading();
        StartIdleAnim(*reinterpret_cast<void**>(self + 0x24), arg0);
        return true;
    }

    uint8* a51 = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x51);
    uint8* a50 = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, 0x50);
    if ((a51 && !((a51[0x46] >> 2) & 1u)) || (a50 && !((a50[0x46] >> 2) & 1u))) {
        ResetBlendFlagsAndSaveTargetHeading();
        StartIdleAnim(*reinterpret_cast<void**>(self + 0x24), arg0);
        return true;
    }

    return false;
}

// 0xA38330
bool __thiscall CPedMoveBlend::ProcessJumpOrVaultBlend(uint32* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!((*reinterpret_cast<uint32*>(arg0 + 0x378 / 4) >> 9) & 1u))
        return false;

    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* anim = Core::Hook::CallStdcall<uint8*>(0xBC97E0, *reinterpret_cast<void**>(ped + 0x78));
    Core::StaticRef<uint8*>(0x165D184) = anim;
    if (!anim)
        return false;

    *reinterpret_cast<float*>(self + 0x78) = Core::StaticRef<float>(0xF359C8);
    float three = Core::StaticRef<float>(0xD885D4);
    *reinterpret_cast<float*>(self + 0x54) = three;
    Core::StaticRef<float>(0x165D190) = three;
    *reinterpret_cast<float*>(self + 0x1C) = three;

    int32 animId = *reinterpret_cast<int32*>(anim + 0xC);
    if (animId == 0x37 || animId == 0x38) {
        float x = *reinterpret_cast<float*>(self + 0xC);
        float y = *reinterpret_cast<float*>(self + 0x10);
        if (Core::StaticRef<float>(0xDAFA18) >= (x * x + y * y)) {
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x20000u;
            Core::Hook::CallMethod<void>(0xA4DBC0, anim, Core::StaticRef<float>(0xD886BC));
            return false;
        }
    }

    using GetDurFn = float(__thiscall*)(void*);
    float dur = reinterpret_cast<GetDurFn>((*reinterpret_cast<void***>(anim))[0x8 / 4])(anim);
    if (*reinterpret_cast<float*>(anim + 0x4C) >= (1.0f - dur)) {
        *reinterpret_cast<uint32*>(self + 0x50) |= 0x20000u;
        return false;
    }

    *reinterpret_cast<uint32*>(self + 0x50) |= 0x100000u;
    void* animPlayer = *reinterpret_cast<void**>((*reinterpret_cast<uint8**>(self + 0x24)) + 0x78);
    Core::Hook::CallMethod<void>(0x9EFEB0, animPlayer, arg0[0], animId, Core::StaticRef<float>(0xF35974), arg0[1]);

    if (animId == 0x37 || animId == 0x38) {
        float one = Core::StaticRef<float>(0xD50398);
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        *reinterpret_cast<float*>(self + 0x8) = one;
        uint8* sub6C = *reinterpret_cast<uint8**>((*reinterpret_cast<uint8**>(self + 0x24)) + 0x6C);
        if (!sub6C || !sub6C[0xE]) {
            *reinterpret_cast<float*>(self + 0xC) = 0.0f;
            *reinterpret_cast<float*>(self + 0x10) = one;
        }
    } else if (animId == 0x39 || animId == 0x3A) {
        float two = Core::StaticRef<float>(0xDCB2A4);
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        *reinterpret_cast<float*>(self + 0x8) = two;
        if (!Core::Hook::CallMethod<bool>(0x4443C0, *reinterpret_cast<void**>(self + 0x24))) {
            *reinterpret_cast<float*>(self + 0xC) = 0.0f;
            *reinterpret_cast<float*>(self + 0x10) = two;
        }
    } else {
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        *reinterpret_cast<float*>(self + 0x8) = three;
        if (!Core::Hook::CallMethod<bool>(0x4443C0, *reinterpret_cast<void**>(self + 0x24))) {
            *reinterpret_cast<float*>(self + 0xC) = 0.0f;
            *reinterpret_cast<float*>(self + 0x10) = three;
        }
    }

    float outVal = 0.0f;
    if (Core::Hook::CallMethod<bool>(0xA4C8F0, Core::StaticRef<void*>(0x165D184), 0x4000, &outVal, 0.0f, 1.0f)) {
        uint8* curAnim = Core::StaticRef<uint8*>(0x165D184);
        if (*reinterpret_cast<float*>(curAnim + 0x4C) >= outVal) {
            *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x28) * Core::StaticRef<float>(0x11DDE7C);
            Core::StaticRef<uint8>(0x165D16A) = 1;
            return true;
        }
    }

    *reinterpret_cast<float*>(self + 0x28) = 0.0f;
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);
    if (!sub6C || !sub6C[0xE]) {
        Core::Hook::CallMethod<void>(0x89D110, ped, *reinterpret_cast<float*>(ped + 0xAB0));
    }
    return true;
}

// =========================================================================
// Slice 3: 0xA38610 .. 0xA3B448
// =========================================================================

// 0xA38610
CVector* __thiscall CPedMoveBlend::ComputeDesiredVelocity(CVector* outVel, float dt)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    CVector vel(
        *reinterpret_cast<float*>(ped + 0xAA0),
        *reinterpret_cast<float*>(ped + 0xAA4),
        *reinterpret_cast<float*>(ped + 0xAA8));

    void* veh = *reinterpret_cast<void**>(ped + 0xAC0);
    if (veh && ((*reinterpret_cast<uint32*>(static_cast<uint8*>(veh) + 0x28) & 0x3C0u) != 0xC0u)) {
        using GetWeightFn = float(__thiscall*)(void*);
        if (reinterpret_cast<GetWeightFn>((*reinterpret_cast<void***>(veh))[0x58 / 4])(veh) > Core::StaticRef<float>(0xD50964)) {
            void* subTask = Core::Hook::CallMethod<void*>(0x9E7080, veh);
            if (subTask) {
                CVector4 tmp1, tmp2;
                using Fn58 = void(__thiscall*)(void*, uint8*, CVector4*, uint32);
                using Fn5C = CVector*(__thiscall*)(void*, CVector4*, uint8*, float, uint32);
                reinterpret_cast<Fn58>((*reinterpret_cast<void***>(subTask))[0x58 / 4])(subTask, ped + 0xAD0, &tmp1, 0);
                CVector* v5C = reinterpret_cast<Fn5C>((*reinterpret_cast<void***>(subTask))[0x5C / 4])(subTask, &tmp2, ped + 0xAD0, dt, 0);
                vel.x += tmp1.x + v5C->x;
                vel.y += tmp1.y + v5C->y;
                vel.z += tmp1.z + v5C->z;
            } else {
                CVector4 tmp;
                using FnEC = CVector*(__thiscall*)(void*, CVector4*);
                CVector* vEC = reinterpret_cast<FnEC>((*reinterpret_cast<void***>(veh))[0xEC / 4])(veh, &tmp);
                vel.x += vEC->x;
                vel.y += vEC->y;
                vel.z += vEC->z;
            }
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    *reinterpret_cast<uint32*>(ped + 0x2A0) &= ~0x8u;
    ped = *reinterpret_cast<uint8**>(self + 0x24);

    if (Core::StaticRef<float>(0xD8860C) > *reinterpret_cast<float*>(ped + 0xB18)
        && ped[0x219] && !(ped[0x14F] & 1u))
    {
        uint8* curVeh = *reinterpret_cast<uint8**>(ped + 0xAC0);
        if (curVeh && ((*reinterpret_cast<uint32*>(curVeh + 0x28) & 0x3C0u) == 0x80u)
            && *reinterpret_cast<int32*>(curVeh + 0x1354) == 2)
        {
            *reinterpret_cast<float*>(ped + 0xB00) = 0.0f;
            *reinterpret_cast<float*>(ped + 0xB04) = 0.0f;
            *reinterpret_cast<float*>(ped + 0xB08) = 0.0f;
        } else if (*reinterpret_cast<float*>(ped + 0xB00) == 0.0f
            && *reinterpret_cast<float*>(ped + 0xB04) == 0.0f
            && *reinterpret_cast<float*>(ped + 0xB08) == 0.0f)
        {
            *reinterpret_cast<float*>(ped + 0xB00) = *reinterpret_cast<float*>(ped + 0xAD0);
            *reinterpret_cast<float*>(ped + 0xB04) = *reinterpret_cast<float*>(ped + 0xAD4);
            *reinterpret_cast<float*>(ped + 0xB08) = *reinterpret_cast<float*>(ped + 0xAD8);
            *reinterpret_cast<float*>(ped + 0xB0C) = *reinterpret_cast<float*>(ped + 0xADC);
        } else {
            CVector n(*reinterpret_cast<float*>(ped + 0xB10), *reinterpret_cast<float*>(ped + 0xB14), 0.0f);
            Core::Hook::CallMethod<void>(0x42F5D0, &n);
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            float dx = *reinterpret_cast<float*>(ped + 0xAD0) - *reinterpret_cast<float*>(ped + 0xB00);
            float dy = *reinterpret_cast<float*>(ped + 0xAD4) - *reinterpret_cast<float*>(ped + 0xB04);
            float dz = *reinterpret_cast<float*>(ped + 0xAD8) - *reinterpret_cast<float*>(ped + 0xB08);
            float d = dx * n.x + dy * n.y + dz * n.z;
            if (Core::StaticRef<float>(0xEBE0E0) > d) {
                float proj = Core::StaticRef<float>(0xEBDED0) - (vel.x * n.x + vel.y * n.y + vel.z * n.z);
                if (proj > Core::StaticRef<float>(0xDECD60)) {
                    *reinterpret_cast<uint32*>(ped + 0x2A0) |= 0x8u;
                    vel.x += n.x * proj;
                    vel.y += n.y * proj;
                    vel.z += n.z * proj;
                }
            }
        }
    } else {
        *reinterpret_cast<float*>(ped + 0xB00) = 0.0f;
        *reinterpret_cast<float*>(ped + 0xB04) = 0.0f;
        *reinterpret_cast<float*>(ped + 0xB08) = 0.0f;
    }

    CVector4 curVelOut;
    using GetCurVelFn = CVector*(__thiscall*)(void*, CVector4*);
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    CVector* curVel = reinterpret_cast<GetCurVelFn>((*reinterpret_cast<void***>(ped))[0xEC / 4])(ped, &curVelOut);

    outVel->x = vel.x - curVel->x;
    outVel->y = vel.y - curVel->y;
    if ((*reinterpret_cast<uint32*>(self + 0x50) >> 6) & 1u) {
        outVel->z = vel.z - curVel->z;
    } else {
        outVel->z = 0.0f;
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    float maxAccel = (*reinterpret_cast<uint32*>(ped + 0x2A0) & 0x1000u)
        ? Core::StaticRef<float>(0xF359A8) : Core::StaticRef<float>(0xF359A4);

    if (*reinterpret_cast<uint32*>(ped + 0x29C) & 1u) {
        uint8* curVeh = *reinterpret_cast<uint8**>(ped + 0xAC0);
        if (curVeh && ((*reinterpret_cast<uint32*>(curVeh + 0x28) & 0x3C0u) == 0x80u)
            && *reinterpret_cast<int32*>(curVeh + 0x1354) != 2
            && *reinterpret_cast<int32*>(curVeh + 0x1354) != 3)
        {
            maxAccel = Core::StaticRef<float>(0xD50398);
        } else {
            maxAccel = Core::StaticRef<float>(0xD88610);
        }
    }

    if (ped[0x211]) {
        uint8* curVeh = *reinterpret_cast<uint8**>(ped + 0xAC0);
        if (curVeh && ((*reinterpret_cast<uint32*>(curVeh + 0x28) & 0x3C0u) == 0x80u)
            && *reinterpret_cast<int32*>(curVeh + 0x1354) != 2)
        {
            maxAccel = Core::StaticRef<float>(0xD885EC);
        }
    } else if (ped[0x299] > 0) {
        maxAccel = Core::StaticRef<float>(0xD8861C);
    }

    float lenSq = outVel->x * outVel->x + outVel->y * outVel->y + outVel->z * outVel->z;
    float limit = maxAccel * dt;
    if (lenSq > limit * limit) {
        float scale = limit / std::sqrtf(lenSq);
        outVel->x *= scale;
        outVel->y *= scale;
        outVel->z *= scale;
    }
    return outVel;
}

// 0xA38B20
CVector* __thiscall CPedMoveBlend::TransformMoveVector(CVector* outVec, const CMatrix* mat)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);

    float pitchX = *reinterpret_cast<float*>(ped + 0xB18) * mat->up.z
        + *reinterpret_cast<float*>(ped + 0xB14) * mat->up.y
        + *reinterpret_cast<float*>(ped + 0xB10) * mat->up.x;
    float pitchY = *reinterpret_cast<float*>(ped + 0xB14) * mat->right.y
        + *reinterpret_cast<float*>(ped + 0xB18) * mat->right.z
        + *reinterpret_cast<float*>(ped + 0xB10) * mat->right.x;

    float one = Core::StaticRef<float>(0xD50398);
    float s1 = one - pitchX * pitchX;
    if (s1 < 0.0f) s1 = 0.0f;
    float c1 = std::sqrtf(s1);

    float s2 = one - pitchY * pitchY;
    if (s2 < 0.0f) s2 = 0.0f;
    float c2 = std::sqrtf(s2);

    outVec->x = Core::StaticRef<float>(0x18A8C60);
    outVec->y = Core::StaticRef<float>(0x18A8C64);
    outVec->z = Core::StaticRef<float>(0x18A8C68);

    float moveX = *reinterpret_cast<float*>(ped + 0xE0);
    float moveY = *reinterpret_cast<float*>(ped + 0xE4);
    float moveZ = *reinterpret_cast<float*>(ped + 0xE8);

    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);
    uint8* blendAnim = Core::Hook::CallMethod<uint8*>(0x9EB810, animPlayer);
    float effY = moveY;
    if (blendAnim && *reinterpret_cast<float*>(blendAnim + 0x48) > Core::StaticRef<float>(0xDECD60)) {
        float w = *reinterpret_cast<float*>(blendAnim + 0x48);
        float invW = one - w;
        moveX = *reinterpret_cast<float*>(blendAnim + 0x50) * w + invW * moveX;
        effY = *reinterpret_cast<float*>(blendAnim + 0x54) * w + invW * moveY;
    }

    float vy = effY * c1;
    float vx = moveX * c2;
    outVec->x += mat->up.x * vy + mat->right.x * vx;
    outVec->y += mat->up.y * vy + mat->right.y * vx;
    outVec->z += mat->up.z * vy + mat->right.z * vx;

    if (*reinterpret_cast<uint32*>(self + 0x50) & 0x40u) {
        outVec->x += mat->at.x * moveZ;
        outVec->y += mat->at.y * moveZ;
        outVec->z += mat->at.z * moveZ;
    }
    return outVec;
}

// 0xA38DA0
void __thiscall CPedMoveBlend::StopMovement(bool bForceStop)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);
    if (sub6C && sub6C[0xE])
        return;

    *reinterpret_cast<uint32*>(self + 0x50) |= 1u;
    *reinterpret_cast<float*>(self + 0x4) = 0.0f;
    *reinterpret_cast<float*>(self + 0x8) = 0.0f;
    *reinterpret_cast<float*>(self + 0xC) = 0.0f;
    *reinterpret_cast<float*>(self + 0x10) = 0.0f;

    if (!bForceStop)
        return;

    *reinterpret_cast<float*>(self + 0x54) = 0.0f;
    if (ped[0x26C] & 4u)
        return;

    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);
    uint8* node = Core::Hook::CallMethod<uint8*>(0x9EC060, animPlayer, 0, 2);
    while (node) {
        if (*reinterpret_cast<uint32*>(node + 0xC) != 0xFu) {
            Core::Hook::CallMethod<void>(0xA4DBC0, node, Core::StaticRef<float>(0xEBE00C));
            Core::Hook::CallMethod<void>(0x9EF450, animPlayer, node);
        }
        node = Core::Hook::CallMethod<uint8*>(0x9EC0F0, animPlayer, 0, 2);
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    int32 idx = *reinterpret_cast<int32*>(subA90 + 0x3C);
    uint32* animData = Core::Hook::Call<uint32*>(0xC54AD0, idx);
    if (!animData) {
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
        animData = Core::Hook::Call<uint32*>(0xC54F10, *reinterpret_cast<int32*>(subA90 + 0x3C));
    }
    uint32 unk = Core::Hook::CallMethod<uint32>(0x89C8C0, *reinterpret_cast<void**>(self + 0x24));
    Core::Hook::Call<void*>(0xA34CE0, animPlayer, animData[0], 0xF, Core::StaticRef<float>(0xDCB2A0), unk);
}

// 0xA38EB0
void __thiscall CPedMoveBlend::UpdateAimingBlend(float pitch, float yaw)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    if ((*reinterpret_cast<uint32*>(ped + 0xBF0) >> 1) & 1u) {
        int32 state = *reinterpret_cast<int32*>(ped + 0xA84);
        uint32 flags29C = *reinterpret_cast<uint32*>(ped + 0x29C);
        if (state == 1 || state == 2 || (flags29C & 1u) || (*reinterpret_cast<int32*>(ped + 0xB90) >= 2 && !(flags29C & 0x40u))) {
            float minV = Core::StaticRef<float>(0xEBDED4);
            float maxV = Core::StaticRef<float>(0xD50398);
            if (state == 1 || state == 2 || (flags29C & 1u)) {
                float yClamped = yaw;
                if (yClamped < minV) yClamped = minV;
                else if (yClamped > maxV) yClamped = maxV;
                *reinterpret_cast<float*>(ped + 0xC10) = static_cast<float>(std::asin(static_cast<double>(yClamped)));
            } else {
                *reinterpret_cast<float*>(ped + 0xC10) = 0.0f;
            }
            float pClamped = pitch;
            if (pClamped < minV) pClamped = minV;
            else if (pClamped > maxV) pClamped = maxV;
            *reinterpret_cast<float*>(ped + 0xC08) =
                Core::StaticRef<float>(0xD50964) * static_cast<float>(std::asin(static_cast<double>(pClamped)))
                + Core::StaticRef<float>(0xD88620) * *reinterpret_cast<float*>(ped + 0xC08);
            return;
        }

        float p = *reinterpret_cast<float*>(ped + 0xC08);
        float y = *reinterpret_cast<float*>(ped + 0xC10);
        float decay = 0.0f;
        if (p != 0.0f || y != 0.0f) {
            double expVal = static_cast<double>(Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xDCEA38));
            decay = static_cast<float>(std::pow(Core::StaticRef<double>(0xD887A0), expVal));
        }
        *reinterpret_cast<float*>(ped + 0xC08) = (WrapPiDiff(p) > Core::StaticRef<float>(0xDAFA18)) ? (p * decay) : 0.0f;
        *reinterpret_cast<float*>(ped + 0xC10) = (WrapPiDiff(y) > Core::StaticRef<float>(0xD529C4)) ? (y * decay) : 0.0f;
        return;
    }

    if (*reinterpret_cast<int32*>(ped + 0xB90) >= 2 && !ped[0x219]) {
        float y = *reinterpret_cast<float*>(ped + 0xC10);
        if (WrapPiDiff(y) > Core::StaticRef<float>(0xD529C4)) {
            double expVal = static_cast<double>(Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xDCEA38));
            *reinterpret_cast<float*>(ped + 0xC10) = y * static_cast<float>(std::pow(Core::StaticRef<double>(0xD887A0), expVal));
        }
        *reinterpret_cast<float*>(ped + 0xC10) = 0.0f;
    }
}

// 0xA39120
void* __cdecl CPedMoveBlend::AllocFromPool()
{
    void* slot = Core::Hook::CallMethod<void*>(0x439CB0, Core::StaticRef<void*>(0x164B01C));
    if (slot) {
        return Core::Hook::CallMethod<void*>(0x9AD950, slot);
    }
    return nullptr;
}

// 0xA39140
bool __thiscall CPedMoveBlend::UpdateStrafeMovement(uint8* arg0, void* arg1)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    bool bCrouched = ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u) != 0;

    if (!(ped[0x118] & 1u)) {
        float minSpd = bCrouched ? Core::StaticRef<float>(0xF3598C) : Core::StaticRef<float>(0xF35994);
        if (*reinterpret_cast<float*>(self + 0x8) >= Core::StaticRef<float>(0xD50398)
            && *reinterpret_cast<float*>(self + 0x10) >= minSpd)
        {
            float heading = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));
            float absH = WrapPiDiff(heading);
            float turnThresh = bCrouched ? Core::StaticRef<float>(0xF35988) : Core::StaticRef<float>(0xF35990);
            if (absH > turnThresh && !Core::Hook::CallMethod<bool>(0x4443C0, ped)) {
                float maxT = Core::StaticRef<float>(0xEBDFF0);
                float ratio = (absH - turnThresh) / (maxT - turnThresh);
                float val = *reinterpret_cast<float*>(self + 0x10) - (ratio * (*reinterpret_cast<float*>(self + 0x10) - minSpd));
                *reinterpret_cast<float*>(self + 0x10) = Core::Hook::Call<float>(0x41EBC0, val, minSpd);
            }
        }
    }

    float one = Core::StaticRef<float>(0xD50398);
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if (Core::Hook::Call<bool>(0xA348A0, ped) && !(*reinterpret_cast<uint32*>(ped + 0x268) & 0x800u)) {
        if (*reinterpret_cast<float*>(self + 0x8) > one)
            *reinterpret_cast<float*>(self + 0x8) = one;
        uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);
        if (!sub6C || !sub6C[0xE]) {
            if (*reinterpret_cast<float*>(self + 0x10) > one)
                *reinterpret_cast<float*>(self + 0x10) = one;
        }
    }

    float curY = *reinterpret_cast<float*>(self + 0x8);
    if (curY > *reinterpret_cast<float*>(self + 0x70))
        curY = *reinterpret_cast<float*>(self + 0x70);
    *reinterpret_cast<float*>(self + 0x8) = curY;
    const float savedCurY = curY;

    UpdateHeadingAndSpeed(arg0);
    if (ProcessJumpOrVaultBlend(reinterpret_cast<uint32*>(arg0))) {
        *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
        return true;
    }

    if (Core::StaticRef<uint8>(0x165D179) && !Core::Hook::Call<bool>(0x462F90) && bCrouched) {
        if (*reinterpret_cast<float*>(self + 0x10) >= one) {
            float t = Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xF3599C) + Core::StaticRef<float>(0x165D17C);
            float twoPi = Core::StaticRef<float>(0xEBDEE4);
            while (t > twoPi) t -= twoPi;
            Core::StaticRef<float>(0x165D17C) = t;
            float s = static_cast<float>(std::sin(static_cast<double>(t))) * Core::StaticRef<float>(0xF35998) + *reinterpret_cast<float*>(self + 0x8);
            if (s < one) s = one;
            else if (s > Core::StaticRef<float>(0xD885D4)) s = Core::StaticRef<float>(0xD885D4);
            *reinterpret_cast<float*>(self + 0x8) = s;
        }
    }

    float moveVec[3] = { *reinterpret_cast<float*>(self + 0x4), *reinterpret_cast<float*>(self + 0x8), 0.0f };
    float desY = *reinterpret_cast<float*>(self + 0x10);
    if (!bCrouched && desY > 0.0f && one > desY && *reinterpret_cast<float*>(self + 0x8) > 0.0f && one > *reinterpret_cast<float*>(self + 0x8)) {
        Core::StaticRef<float>(0xF359D0) = *reinterpret_cast<float*>(self + 0x8);
    }

    Core::StaticRef<float>(0x165D190) = Core::StaticRef<float>(0xD885D4);
    uint8* outAnim1 = nullptr;
    float outF2 = savedCurY;
    float res = Core::Hook::CallMethod<float>(0x928ED0, arg0, *reinterpret_cast<void**>(self + 0x24), &outAnim1, &outF2, &moveVec[1], desY);
    Core::StaticRef<float>(0x165D194) = res;

    if (outAnim1) {
        *reinterpret_cast<float*>(outAnim1 + 0x54) = Core::StaticRef<float>(0x165D16C);
    }

    float f350 = *reinterpret_cast<float*>(arg0 + 0x350);
    int32 id4 = *reinterpret_cast<int32*>(arg0 + 0x4);
    if (id4 != -1) {
        uint8* e4 = Core::Hook::Call<uint8*>(0xC54AD0, id4);
        if (!e4) e4 = Core::Hook::Call<uint8*>(0xC54F10, id4);
        if (*reinterpret_cast<float*>(e4 + 0x350) > f350)
            f350 = *reinterpret_cast<float*>(e4 + 0x350);
    }

    if (*reinterpret_cast<int16*>(self + 0xAA) != 0 || ((*reinterpret_cast<uint8**>(self + 0x24))[0x14F] & 1u)) {
        if (f350 > Core::StaticRef<float>(0xDCB2A4))
            f350 = Core::StaticRef<float>(0xDCB2A4);
    }
    f350 += one;
    if (Core::StaticRef<float>(0x165D194) > f350)
        Core::StaticRef<float>(0x165D194) = f350;

    Core::Hook::CallMethod<void>(0xA34B90, this, arg0);

    if (Core::StaticRef<float>(0x165D194) <= one) {
        *reinterpret_cast<uint32*>(self + 0x74) = Core::StaticRef<uint32>(0x11DDE74);
        *reinterpret_cast<float*>(self + 0x78) = Core::StaticRef<float>(0xF359C8);
        *reinterpret_cast<int32*>(self + 0x88) = -1;
        *reinterpret_cast<int32*>(self + 0x84) = 0x1C;
        *reinterpret_cast<float*>(self + 0x8C) = 0.0f;
    }

    if (Core::StaticRef<float>(0x165D194) == one
        && ((*reinterpret_cast<uint32*>(arg0 + 0x378) & 0x3u) != 0))
    {
        uint8* intel = *reinterpret_cast<uint8**>((*reinterpret_cast<uint8**>(self + 0x24)) + 0x224);
        void* task = Core::Hook::CallMethod<void*>(0x9E5730, intel + 0x44);
        using GetTaskTypeFn = int32(__thiscall*)(void*);
        if (reinterpret_cast<GetTaskTypeFn>((*reinterpret_cast<void***>(task))[0xC / 4])(task) == 0x125) {
            *reinterpret_cast<float*>(self + 0x90) = 0.0f;
            return true;
        }
        uint8* turnAnim = static_cast<uint8*>(ProcessStartTurnBlend(reinterpret_cast<uint32*>(arg0)));
        if (!turnAnim)
            return false;
        if (Core::Hook::CallMethod<float>(0xA4DB70, turnAnim) >= 0.0f) {
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
            return true;
        }
    }

    *reinterpret_cast<float*>(self + 0x90) = 0.0f;
    *reinterpret_cast<int32*>(self + 0x48) = outAnim1 ? *reinterpret_cast<int32*>(outAnim1 + 0xC) : -1;

    float g194 = Core::StaticRef<float>(0x165D194);
    if (g194 == Core::StaticRef<float>(0xEBDED4)) {
        *reinterpret_cast<float*>(self + 0x1C) = Core::StaticRef<float>(0x165D190);
        *reinterpret_cast<float*>(self + 0x20) = g194;
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        if (!ped[0x218] && ped[0x219]) {
            uint8* pedIntel = *reinterpret_cast<uint8**>(ped + 0x224);
            void* activeTask = Core::Hook::CallMethod<void*>(0x438B00, pedIntel);
            if (activeTask) {
                using GetTaskTypeFn = int32(__thiscall*)(void*);
                if (reinterpret_cast<GetTaskTypeFn>((*reinterpret_cast<void***>(activeTask))[0xC / 4])(activeTask) == 4) {
                    ped = *reinterpret_cast<uint8**>(self + 0x24);
                    if (!Core::Hook::CallMethod<bool>(0x4443C0, ped)) {
                        Core::Hook::CallMethod<void>(0x89D110, ped, *reinterpret_cast<float*>(ped + 0xAB0));
                    }
                }
            }
        }
        *reinterpret_cast<float*>(self + 0x30) = *reinterpret_cast<float*>(self + 0x28) * Core::StaticRef<float>(0x11DDE7C);
        float f89 = Core::Hook::CallMethod<float>(0x89D180, *reinterpret_cast<void**>(self + 0x24));
        *reinterpret_cast<float*>(self + 0x30) *= (f89 / Core::StaticRef<float>(0xDB0870));
        if (outAnim1) {
            using GetRateFn = float(__thiscall*)(void*);
            float t = reinterpret_cast<GetRateFn>((*reinterpret_cast<void***>(outAnim1))[0x8 / 4])(outAnim1)
                + *reinterpret_cast<float*>(outAnim1 + 0x4C);
            if (t >= Core::StaticRef<float>(0xF27D88)) {
                *reinterpret_cast<uint16*>(outAnim1 + 0x46) |= 2u;
                Core::Hook::CallMethod<void>(0xA4DBC0, outAnim1, Core::StaticRef<float>(0xEBDED4));
            }
        }
        *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
        return true;
    }

    uint8* foundAnim = nullptr;
    if (g194 == Core::StaticRef<float>(0xEBE0FC)) {
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        float blendRate = (*reinterpret_cast<uint32*>(ped + 0x29C) & 0x4u)
            ? Core::StaticRef<float>(0xDCAFF0)
            : Core::StaticRef<float>(0xDCB2A0);
        *reinterpret_cast<uint32*>(self + 0x1C) = Core::StaticRef<uint32>(0x165D190);
        *reinterpret_cast<uint32*>(self + 0x20) = Core::StaticRef<uint32>(0x165D194);
        void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);
        static const int32 kCheckSlots[11] = { 0x1C, 0x12, 0x15, 0x16, 0x13, 0x14, 0x17, 0x1A, 0x1B, 0x18, 0x19 };
        for (int32 s : kCheckSlots) {
            foundAnim = Core::Hook::CallMethod<uint8*>(0x9EBF50, animPlayer, s);
            if (foundAnim)
                break;
        }
        bool bFallthrough = false;
        if (foundAnim && Core::Hook::CallMethod<float>(0xA4DB70, foundAnim) < 0.0f) {
            Core::StaticRef<float>(0x165D194) = 0.0f;
            bFallthrough = true;
        } else {
            if (foundAnim) {
                *reinterpret_cast<uint32*>(foundAnim + 0x4) &= ~0x8010u;
            }
            *reinterpret_cast<float*>(self + 0x30) = 0.0f;
            uint32 unk = Core::Hook::CallMethod<uint32>(0x89C8C0, *reinterpret_cast<void**>(self + 0x24));
            Core::Hook::Call<void*>(0xA34CE0, animPlayer, *reinterpret_cast<uint32*>(arg0), 0xF, blendRate, unk);
            if (*reinterpret_cast<uint32*>(self + 0x50) & 0x1000u) {
                *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
                return true;
            }
            bFallthrough = true;
        }
        (void)bFallthrough;
    }

    // 0xA398D6 path
    float wOut[6]{};
    reinterpret_cast<int32*>(wOut)[0] = -1;
    reinterpret_cast<int32*>(wOut)[1] = -1;
    BuildBlendWeights(wOut);
    uint32 d69 = Core::StaticRef<uint8>(0x165D169) ? 1u : 0u;
    uint32 fl = *reinterpret_cast<uint32*>(self + 0x50);
    *reinterpret_cast<uint32*>(self + 0x50) = fl ^ (((d69 << 4) ^ fl) & 0x10u);

    float spd28 = *reinterpret_cast<float*>(self + 0x28);
    float cur194 = Core::StaticRef<float>(0x165D194);
    if (spd28 == 0.0f && cur194 != 0.0f && cur194 != one && cur194 != Core::StaticRef<float>(0xEBE0FC)) {
        *reinterpret_cast<float*>(self + 0x78) -= Core::StaticRef<float>(0x11DDE7C);
        Core::StaticRef<float>(0x165D190) = Core::StaticRef<float>(0xD885D4);
    } else if (cur194 <= 0.0f) {
        *reinterpret_cast<float*>(self + 0x20) = cur194;
        if (*reinterpret_cast<uint32*>(self + 0x50) & 0x1000u) {
            *reinterpret_cast<float*>(self + 0x30) = 0.0f;
            ped = *reinterpret_cast<uint8**>(self + 0x24);
            uint32 unk = Core::Hook::CallMethod<uint32>(0x89C8C0, ped);
            Core::Hook::Call<void*>(0xA34CE0, *reinterpret_cast<void**>(ped + 0x78), *reinterpret_cast<uint32*>(self + 0x3C), 0xF, Core::StaticRef<float>(0xDCB2A0), unk);
            return true;
        }
        return ProcessStopBlend(reinterpret_cast<uint32*>(arg0), foundAnim, Core::StaticRef<float>(0xDCB2A0), false);
    } else if (cur194 == one) {
        Core::StaticRef<uint8>(0x165D16A) = 1;
        float ns = *reinterpret_cast<float*>(self + 0x28) * Core::StaticRef<float>(0xDCB2A0);
        *reinterpret_cast<float*>(self + 0x28) = ns;
        if (ns < 0.0f && *reinterpret_cast<float*>(self + 0x2C) > ns) {
            *reinterpret_cast<float*>(self + 0x28) = *reinterpret_cast<float*>(self + 0x2C);
            return false;
        }
        if (ns > 0.0f && ns > *reinterpret_cast<float*>(self + 0x2C)) {
            *reinterpret_cast<float*>(self + 0x28) = *reinterpret_cast<float*>(self + 0x2C);
            return false;
        }
        return false;
    } else {
        if (CanTriggerQuickTurn(arg0)) {
            *reinterpret_cast<uint32*>(self + 0x50) &= ~0x8u;
            float diff = *reinterpret_cast<float*>(self + 0x8) - Core::StaticRef<float>(0xD885EC);
            int32 idx = static_cast<int32>((diff > 0.0f) ? diff : 0.0f) * 2 + ((*reinterpret_cast<float*>(self + 0x2C) < 0.0f) ? 1 : 0);
            static const int32 kQuickTurnIds[6] = { 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C };
            uint8* pedPtr = *reinterpret_cast<uint8**>(self + 0x24);
            Core::Hook::CallMethod<void>(0x9EFEB0, *reinterpret_cast<void**>(pedPtr + 0x78), *reinterpret_cast<uint32*>(arg0), kQuickTurnIds[idx], Core::StaticRef<float>(0xF35974), id4);
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x100010u;
            *reinterpret_cast<float*>(self + 0x8) = 0.0f;
            *reinterpret_cast<uint32*>(self + 0x7C) = Core::StaticRef<uint32>(0x11DDE74);
            if ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u) {
                uint8* subF2 = Core::Hook::CallMethod<uint8*>(0x9AD2D0, Core::StaticRef<void*>(0xF21A70), 1, 0);
                if (subF2) {
                    using GetPedFn = void*(__thiscall*)(void*);
                    if (reinterpret_cast<GetPedFn>((*reinterpret_cast<void***>(subF2))[0xC / 4])(subF2) == *reinterpret_cast<void**>(self + 0x24)) {
                        uint8* sub27 = Core::Hook::CallMethod<uint8*>(0x9AD2D0, Core::StaticRef<void*>(0xF21A70), 0x27, 0);
                        if (!(sub27[0x13C] & 4u)) {
                            subF2[0x38D] |= 4u;
                            *reinterpret_cast<uint32*>(subF2 + 0x354) = Core::StaticRef<uint32>(0xDB2398);
                            uint8* g7c = Core::Hook::Call<uint8*>(0x7C3900);
                            if (g7c[0x3289] || g7c[0x328B]) {
                                subF2[0x38D] &= ~4u;
                            }
                        }
                    }
                }
            }
            *reinterpret_cast<uint32*>(self + 0x50) |= 0x40000u;
            return true;
        }

        // 0xA39B63 path
        bool bHasBlend = false;
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        if (*reinterpret_cast<uint32*>(ped + 0x26C) & 0x4000u) {
            *reinterpret_cast<float*>(self + 0x78) = Core::StaticRef<float>(0xF359C8);
            float three = Core::StaticRef<float>(0xD885D4);
            *reinterpret_cast<float*>(self + 0x54) = three;
            Core::StaticRef<float>(0x165D190) = three;
            bHasBlend = true;
        }

        float blendTime = *reinterpret_cast<float*>(self + 0x78);
        float blendParam = 0.0f;
        if (blendTime > 0.0f && Core::StaticRef<float>(0xF359C8) > 0.0f) {
            blendParam = one - (blendTime / Core::StaticRef<float>(0xF359C8));
            if (blendParam < Core::StaticRef<float>(0xDAFA18))
                blendParam = Core::StaticRef<float>(0xDAFA18);
            *reinterpret_cast<float*>(self + 0x78) = blendTime - Core::StaticRef<float>(0x11DDE7C);
            bHasBlend = true;
        } else {
            float gD4 = Core::StaticRef<float>(0xF359D4);
            if (one > gD4) {
                blendParam = gD4;
                bHasBlend = true;
            } else {
                blendParam = one;
            }
        }

        float speedScale = blendParam * (*reinterpret_cast<float*>(self + 0x28));
        float res928 = Core::Hook::CallMethod<float>(0x928AA0, arg0, speedScale, wOut, 1.0f, &Core::StaticRef<uint8>(0x165D16A));
        if (((*reinterpret_cast<uint32*>(self + 0x50) >> 7) & 1u) || bHasBlend) {
            Core::StaticRef<uint8>(0x165D16A) = 1;
        }

        float three = Core::StaticRef<float>(0xD885D4);
        float target190 = res928 - three;
        float cur190 = *reinterpret_cast<float*>(self + 0x1C) - three;
        float diff190 = target190 - cur190;
        float step190 = 1.0f;
        if (Core::Hook::Call<float>(0x424C00, target190) < Core::StaticRef<float>(0xF35A28)) {
            step190 = Core::Hook::Call<float>(0x424C00, diff190) * Core::StaticRef<float>(0xF35A24);
            if (step190 > 1.0f)
                step190 = one;
        }

        bool bCrouched = ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u) != 0;
        float fA = bCrouched ? Core::StaticRef<float>(0xF359AC) : Core::StaticRef<float>(0xF359B4);
        float fB = bCrouched ? Core::StaticRef<float>(0xF359B0) : Core::StaticRef<float>(0xF359B8);
        float dtStep = Core::StaticRef<float>(0x11DDE7C) * step190;
        float interp190 = 0.0f;
        if (diff190 > 0.0f) {
            if (cur190 < 0.0f) {
                interp190 = Core::Hook::Call<float>(0x41EBF0, target190, fB * dtStep + cur190);
            } else {
                if (target190 < Core::StaticRef<float>(0xF35A20))
                    dtStep *= Core::StaticRef<float>(0xF35A1C);
                interp190 = Core::Hook::Call<float>(0x41EBF0, target190, fA * dtStep + cur190);
            }
        } else {
            if (cur190 > 0.0f) {
                interp190 = Core::Hook::Call<float>(0x41EBC0, target190, cur190 - fB * dtStep);
            } else {
                if (target190 > Core::StaticRef<float>(0xEBDED0) - Core::StaticRef<float>(0xF35A20))
                    dtStep *= Core::StaticRef<float>(0xF35A1C);
                interp190 = Core::Hook::Call<float>(0x41EBC0, target190, cur190 - fA * dtStep);
            }
        }

        float new190 = interp190 + three;
        if (Core::StaticRef<uint8>(0xF35A1A))
            Core::StaticRef<uint8>(0x165D16A) = 1;
        Core::StaticRef<float>(0x165D190) = new190;
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if (Core::StaticRef<uint8>(0x165D176)
        || (Core::StaticRef<uint8>(0xF3597F)
            && ped[0xA70] != 2
            && (!(ped[0xF4] & 4u) || *reinterpret_cast<float*>(ped + 0xF8) > Core::StaticRef<float>(0xD50978))))
    {
        Core::StaticRef<float>(0x165D190) = Core::StaticRef<float>(0xD885D4);
        Core::StaticRef<uint8>(0x165D16A) = 1;
    }

    int32 tbl[6] = { -1, -1, 0, 0, 0, 0 };
    Core::Hook::CallMethod<void>(0xA34D40, this, tbl);
    Core::Hook::CallMethod<void>(0xA34DD0, this, arg0, arg1, wOut, tbl);
    *reinterpret_cast<uint32*>(self + 0x1C) = Core::StaticRef<uint32>(0x165D190);
    *reinterpret_cast<uint32*>(self + 0x20) = Core::StaticRef<uint32>(0x165D194);
    return false;
}

// 0xA39EB0
bool __thiscall CPedMoveBlend::UpdateNormalMovement(uint8* arg0)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<uint32*>(self + 0x50) &= ~0x20u;
    uint32 flags = *reinterpret_cast<uint32*>(self + 0x50);
    *reinterpret_cast<int32*>(self + 0x48) = -1;

    float step = Core::StaticRef<float>(0xF27D5C) * Core::StaticRef<float>(0x11DDE7C);
    if (flags & 0x10000u)
        step *= Core::StaticRef<float>(0xD88610);
    if (flags & 0x20u)
        step = 0.0f;

    float magSq = *reinterpret_cast<float*>(self + 0x4) * *reinterpret_cast<float*>(self + 0x4)
        + *reinterpret_cast<float*>(self + 0x8) * *reinterpret_cast<float*>(self + 0x8);
    if (Core::StaticRef<float>(0xDCB2AC) > magSq) {
        *reinterpret_cast<uint32*>(self + 0x74) = Core::StaticRef<uint32>(0x11DDE74);
    }

    *reinterpret_cast<float*>(self + 0x4) = StepMoveCoord(*reinterpret_cast<float*>(self + 0x4), *reinterpret_cast<float*>(self + 0xC), step);
    *reinterpret_cast<float*>(self + 0x8) = StepMoveCoord(*reinterpret_cast<float*>(self + 0x8), *reinterpret_cast<float*>(self + 0x10), step);

    UpdateHeadingOnly();

    float desired = *reinterpret_cast<float*>(self + 0x28) * Core::StaticRef<float>(0x11DDE7C);
    Core::StaticRef<float>(0x165D170) = desired;
    *reinterpret_cast<float*>(self + 0x30) = desired;

    bool bClamp = true;
    if ((*reinterpret_cast<uint32*>(arg0 + 0x378) >> 7) & 1u) {
        bClamp = false;
    } else {
        int32 id4 = *reinterpret_cast<int32*>(arg0 + 0x4);
        if (id4 != -1) {
            uint8* e4 = Core::Hook::Call<uint8*>(0xC54AD0, id4);
            if ((*reinterpret_cast<uint32*>(e4 + 0x378) >> 7) & 1u)
                bClamp = false;
        }
    }

    if (bClamp) {
        float minV = Core::StaticRef<float>(0xEBDED4);
        float maxV = Core::StaticRef<float>(0xD50398);
        float& x = *reinterpret_cast<float*>(self + 0x4);
        if (x < minV) x = minV;
        else if (x > maxV) x = maxV;
        float& y = *reinterpret_cast<float*>(self + 0x8);
        if (y < minV) y = minV;
        else if (y > maxV) y = maxV;
    }

    float two = Core::StaticRef<float>(0xDCB2A4);
    Core::StaticRef<float>(0x165D190) = *reinterpret_cast<float*>(self + 0x4) + two;
    Core::StaticRef<float>(0x165D194) = *reinterpret_cast<float*>(self + 0x8) + two;

    float moveVec[2] = { *reinterpret_cast<float*>(self + 0x4), *reinterpret_cast<float*>(self + 0x8) };
    bool bRet = ((*reinterpret_cast<uint32*>(arg0 + 0x378) >> 6) & 1u)
        ? Blend8WayStrafe(reinterpret_cast<uint32*>(arg0), moveVec, 0.0f, bClamp)
        : Blend4WayStrafe(reinterpret_cast<uint32*>(arg0), moveVec, 0.0f, bClamp);

    if ((*reinterpret_cast<uint32*>(self + 0x50) >> 7) & 1u) {
        Core::StaticRef<uint8>(0x165D16A) = 1;
    }
    return bRet;
}

// 0xA3A200
uint8 __thiscall CPedMoveBlend::ProcessMovement()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint8* ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* sub6C = *reinterpret_cast<uint8**>(ped + 0x6C);

    if (sub6C && sub6C[0xE]) {
        if (ped[0x219]) {
            uint8* sub808 = sub6C + 0x808;
            if (!Core::Hook::CallMethod<bool>(0x4A5120, sub808, 0xD3)
                && !Core::Hook::CallMethod<bool>(0x4A5120, sub808, 0xFE)
                && !Core::Hook::CallMethod<bool>(0x4A5120, sub808, 0x11F))
            {
                *reinterpret_cast<uint32*>(self + 0x50) |= 0x2000u;
            } else {
                *reinterpret_cast<uint32*>(self + 0x50) &= ~0x2000u;
            }
        } else {
            *reinterpret_cast<uint32*>(self + 0x50) &= ~0x2000u;
        }
    }

    *reinterpret_cast<uint32*>(self + 0x50) &= ~0x300000u;
    Core::StaticRef<uint8*>(0x165D184) = nullptr;
    ped = *reinterpret_cast<uint8**>(self + 0x24);

    if (ped[0x26C] & 4u) {
        Core::Hook::CallMethod<void>(0x89D1B0, ped, 1);
        *reinterpret_cast<int32*>(self + 0x48) = -1;
        *reinterpret_cast<float*>(self + 0x30) = 0.0f;
        *reinterpret_cast<float*>(self + 0x5C) = 0.0f;
        *reinterpret_cast<float*>(self + 0x4C) = 0.0f;
        *reinterpret_cast<float*>(self + 0x1C) = Core::StaticRef<float>(0xD885D4);
        *reinterpret_cast<float*>(self + 0x20) = 0.0f;
        ResetBlendFlagsAndSaveTargetHeading();
        return 1;
    }

    uint32 flags29C = *reinterpret_cast<uint32*>(ped + 0x29C);
    if (flags29C & 0x100u)
        return static_cast<uint8>(flags29C);

    if ((flags29C & 0x200u) || (*reinterpret_cast<uint32*>(ped + 0x268) & 0x1000u)) {
        *reinterpret_cast<float*>(self + 0xC) = 0.0f;
        *reinterpret_cast<float*>(self + 0x10) = 0.0f;
    }

    Core::StaticRef<float>(0x165D16C) = *reinterpret_cast<float*>(ped + 0x28C) * Core::StaticRef<float>(0xF35948);
    uint8* intel = *reinterpret_cast<uint8**>(ped + 0x224);
    uint8* task410 = Core::Hook::CallMethod<uint8*>(0x426850, intel + 0x44, 0x410);
    bool bTask410_3 = (task410 && task410[0x72] == 3);
    if (Core::Hook::CallMethod<bool>(0x89C780, ped) && bTask410_3) {
        *reinterpret_cast<float*>(self + 0xC) = 0.0f;
        *reinterpret_cast<float*>(self + 0x10) = 0.0f;
    }

    uint32 fl50 = *reinterpret_cast<uint32*>(self + 0x50);
    if (fl50 & 0x4000u) {
        *reinterpret_cast<uint32*>(self + 0xC) = *reinterpret_cast<uint32*>(self + 0x14);
        *reinterpret_cast<uint32*>(self + 0x10) = *reinterpret_cast<uint32*>(self + 0x18);
    }
    *reinterpret_cast<uint32*>(self + 0x50) = fl50 & ~0x4000u;

    using Fn4C = void(__thiscall*)(void*);
    reinterpret_cast<Fn4C>((*reinterpret_cast<void***>(this))[0x4C / 4])(this);

    Core::StaticRef<uint8>(0x165D169) = 1;
    Core::StaticRef<uint8>(0xF27D44) = 1;

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    uint8* subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    int32 animGroup = *reinterpret_cast<int32*>(subA90 + 0x3C);
    void* animPlayer = *reinterpret_cast<void**>(ped + 0x78);

    void* g80 = Core::Hook::Call<void*>(0x908A00, animGroup);
    if (!Core::Hook::Call<bool>(0x832DD0, g80, Core::StaticRef<void*>(0xF27324))) {
        animGroup = 0x31;
    }

    bool bFlag11 = false;
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if (*reinterpret_cast<uint32*>(ped + 0x2A0) & 0x400u) {
        uint8* pIntel = *reinterpret_cast<uint8**>(ped + 0x224);
        uint8* node = *reinterpret_cast<uint8**>(pIntel + 0x2E0);
        if (node) {
            uint32 prevLvl = (*reinterpret_cast<uint32*>(node + 0x8) >> 1) & 7u;
            while (node) {
                uint32 curLvl = (*reinterpret_cast<uint32*>(node + 0x8) >> 1) & 7u;
                if (prevLvl < curLvl)
                    break;
                prevLvl = curLvl;
                if (*reinterpret_cast<int32*>(node + 0x4) == 0x41E) {
                    int32 tType = 0;
                    uint8 d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0;
                    Core::Hook::CallMethod<void>(0x9A4520, pIntel + 0x2E0, &tType, &d6, &d5, &d4, &d3, &d2, &d1);
                    if (tType == 5 || tType == 8 || tType == 9) {
                        bFlag11 = true;
                        *reinterpret_cast<int32*>(self + 0x40) = -1;
                    }
                    break;
                }
                node = *reinterpret_cast<uint8**>(node + 0xC);
            }
        }
    }

    if (!bFlag11) {
        if (animGroup == 0x3B || *reinterpret_cast<int32*>(self + 0x40) == 0x3B || animGroup == 0x34
            || (*reinterpret_cast<uint32*>(*reinterpret_cast<uint8**>(self + 0x24) + 0x2A0) & 0x400u))
        {
            *reinterpret_cast<int32*>(self + 0x40) = -1;
        }
    }

    int32 grp40 = *reinterpret_cast<int32*>(self + 0x40);
    if (grp40 != -1 && (*reinterpret_cast<uint32*>(self + 0x50) & 0x40000u)) {
        if (((*reinterpret_cast<uint32*>(self + 0x50) >> 1) & 1u) != 0 || (grp40 != 0x34 && grp40 != 0x35)) {
            bool bIs89 = Core::Hook::CallMethod<bool>(0x89C780, *reinterpret_cast<void**>(self + 0x24));
            bool bIs53 = IsAnimGroup53_59(grp40);
            if ((bIs89 || bIs53) && (!bIs89 || !bIs53)) {
                animGroup = grp40;
            }
        }
    }

    uint8* animData = Core::Hook::Call<uint8*>(0xC54AD0, animGroup);
    if (!animData) animData = Core::Hook::Call<uint8*>(0xC54F10, animGroup);

    if (!((*reinterpret_cast<uint32*>(animData + 0x378) >> 13) & 1u)) {
        Core::Hook::CallMethod<void>(0xC56860, animData, *reinterpret_cast<void**>(self + 0x24));
    }

    int32 altGroup = *reinterpret_cast<int32*>(animData + 0x4);
    uint8* altAnimData = nullptr;
    if (altGroup != -1) {
        altAnimData = Core::Hook::Call<uint8*>(0xC54AD0, altGroup);
        if (!((*reinterpret_cast<uint32*>(altAnimData + 0x378) >> 13) & 1u)) {
            Core::Hook::CallMethod<void>(0xC56860, altAnimData, *reinterpret_cast<void**>(self + 0x24));
        }
    }

    *reinterpret_cast<float*>(self + 0x30) = 0.0f;
    Core::StaticRef<uint8>(0x165D16A) = 0;
    Core::StaticRef<float>(0xF359D4) = Core::StaticRef<float>(0xD50398);
    Core::StaticRef<uint8>(0x165D18C) = 0;

    if (((*reinterpret_cast<uint32*>(self + 0x50) >> 1) & 1u) != 0) {
        if (!(*reinterpret_cast<uint32*>(self + 0x50) & 0x4u)) {
            int32 g40 = *reinterpret_cast<int32*>(self + 0x40);
            if (animGroup != g40 && g40 != -1) {
                Core::Hook::CallStdcall<void>(0xA34C80, animPlayer, g40, Core::StaticRef<float>(0xD886BC));
            }
        }
        bool bHasBit5 = ((*reinterpret_cast<uint32*>(animData + 0x378) >> 5) & 1u) != 0;
        if (!bHasBit5 && altGroup != -1) {
            uint8* eAlt = Core::Hook::Call<uint8*>(0xC54AD0, altGroup);
            bHasBit5 = ((*reinterpret_cast<uint32*>(eAlt + 0x378) >> 5) & 1u) != 0;
        }
        if (!bHasBit5) {
            animGroup = 0x2F;
            animData = Core::Hook::Call<uint8*>(0xC54AD0, 0x2F);
            if (!animData) animData = Core::Hook::Call<uint8*>(0xC54F10, 0x2F);
            altGroup = -1;
        }
    }

    if (Core::Hook::CallMethod<bool>(0x89C780, *reinterpret_cast<void**>(self + 0x24))) {
        int32 t38 = *reinterpret_cast<int32*>(self + 0x38);
        if (t38 >= 0 && static_cast<int32>(Core::StaticRef<uint32>(0x11DDE74) - *reinterpret_cast<uint32*>(self + 0x34)) > t38) {
            Core::Hook::CallMethod<void>(0x8A70C0, *reinterpret_cast<void**>(self + 0x24), 0, -1);
            *reinterpret_cast<int32*>(self + 0x34) = 0;
            *reinterpret_cast<int32*>(self + 0x38) = -1;
        }
    }

    if (ProcessTurnAndMovement(reinterpret_cast<uint32*>(animData)))
        return 1;

    *reinterpret_cast<float*>(self + 0x4C) = 0.0f;
    Core::StaticRef<float>(0xF359D0) = Core::StaticRef<float>(0xD50398);

    if (*reinterpret_cast<uint32*>(self + 0x50) & 1u) {
        int32 id48 = *reinterpret_cast<int32*>(self + 0x48);
        if (id48 != -1) {
            if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, id48)) {
                Core::Hook::CallMethod<void>(0x9EF450, animPlayer, a);
            }
        }
        int32* slots = reinterpret_cast<int32*>(self + 0xAC);
        for (int32 i = 0; i < 9; ++i) {
            if (slots[i] != -1 && slots[i] != 0xF) {
                if (void* a = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, slots[i])) {
                    Core::Hook::CallMethod<void>(0x9EF450, animPlayer, a);
                }
            }
        }
        Core::Hook::CallMethod<void>(0x9EF440, animPlayer);
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        *reinterpret_cast<float*>(self + 0x10) = 0.0f;
        *reinterpret_cast<float*>(self + 0x54) = Core::StaticRef<float>(0xD885D4);
    }

    *reinterpret_cast<uint32*>(self + 0x50) &= ~1u;
    Core::Hook::CallMethod<void>(0xA344C0, &Core::StaticRef<uint8>(0x165D198));
    Core::StaticRef<float>(0x165D190) = 0.0f;
    Core::StaticRef<float>(0x165D194) = 0.0f;

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
    bool bStrafe = ((*reinterpret_cast<uint32*>(subA90 + 0x50) >> 1) & 1u) != 0;
    bool bDone = false;
    bool bFlag12 = false;
    float headingDiff = 0.0f;

    if (bStrafe) {
        bool bHasBit5 = ((*reinterpret_cast<uint32*>(animData + 0x378) >> 5) & 1u) != 0;
        if (!bHasBit5 && altGroup != -1) {
            uint8* eAlt = Core::Hook::Call<uint8*>(0xC54AD0, altGroup);
            bHasBit5 = ((*reinterpret_cast<uint32*>(eAlt + 0x378) >> 5) & 1u) != 0;
        }
        if (!bHasBit5) {
            animData = Core::Hook::Call<uint8*>(0xC54AD0, 0x31);
        }
        bDone = UpdateNormalMovement(animData);
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        headingDiff = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));
    } else {
        if (0.0f > *reinterpret_cast<float*>(self + 0x8))
            *reinterpret_cast<float*>(self + 0x8) = 0.0f;
        *reinterpret_cast<float*>(self + 0x4) = 0.0f;
        bDone = UpdateStrafeMovement(animData, altAnimData);
        if (!bDone) {
            UpdateSlopeAndObstacleCheck(animData);
        } else {
            *reinterpret_cast<int16*>(self + 0xAA) = 0;
        }
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        headingDiff = Core::Hook::Call<float>(0xA4E400, *reinterpret_cast<float*>(ped + 0xAB4) - *reinterpret_cast<float*>(ped + 0xAB0));
        bFlag12 = (Core::StaticRef<float>(0x165D194) == Core::StaticRef<float>(0xD50398));
    }

    if (bFlag11) {
        float neg8 = Core::StaticRef<float>(0xEBE008);
        Core::Hook::CallStdcall<void>(0xA34C80, animPlayer, 0x36, neg8);
        Core::Hook::CallStdcall<void>(0xA34C80, animPlayer, 0x37, neg8);
        Core::Hook::CallStdcall<void>(0xA34C80, animPlayer, 0x39, neg8);
        Core::Hook::CallStdcall<void>(0xA34C80, animPlayer, 0x35, neg8);
        if (Core::Hook::CallMethod<bool>(0x89C780, *reinterpret_cast<void**>(self + 0x24))) {
            Core::Hook::CallMethod<void>(0x9EFEB0, animPlayer, 0x36, 0xF, Core::StaticRef<float>(0xD885D4), -1);
        }
    }

    if (bDone) {
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        if ((ped[0x219] && Core::StaticRef<uint8>(0xF3597D)) || (!ped[0x219] && Core::StaticRef<uint8>(0xF3597E))) {
            ApplyPedVelocityToTask();
        }
        ResetBlendFlagsAndSaveTargetHeading();
        StartIdleAnim(*reinterpret_cast<void**>(self + 0x24), reinterpret_cast<uint32*>(animData));
        return 1;
    }

    Core::Hook::CallMethod<void>(0xA34710, &Core::StaticRef<uint8>(0x165D198));
    Core::Hook::CallMethod<void>(0xA34A10, &Core::StaticRef<uint8>(0x165D198));

    if ((*reinterpret_cast<uint32*>(animData + 0x378) >> 11) & 1u) {
        int16 lean = *reinterpret_cast<int16*>(self + 0xAA);
        if (lean > 0) {
            Core::Hook::CallMethod<void>(0xA347A0, &Core::StaticRef<uint8>(0x165D198), 0x1C, 0x4C);
            Core::Hook::CallMethod<void>(0xA347A0, &Core::StaticRef<uint8>(0x165D198), 0x28, 0x4E);
        } else if (lean < 0) {
            Core::Hook::CallMethod<void>(0xA347A0, &Core::StaticRef<uint8>(0x165D198), 0x1C, 0x4D);
            Core::Hook::CallMethod<void>(0xA347A0, &Core::StaticRef<uint8>(0x165D198), 0x28, 0x4F);
        }
    }

    if ((*reinterpret_cast<uint32*>(self + 0x50) & 0x40000u)
        && *reinterpret_cast<int32*>(self + 0x40) != -1
        && *reinterpret_cast<int32*>(self + 0x3C) != -1
        && *reinterpret_cast<int32*>(self + 0x40) != *reinterpret_cast<int32*>(self + 0x3C))
    {
        Core::Hook::CallMethod<void>(0xA34820, &Core::StaticRef<uint8>(0x165D198), *reinterpret_cast<int32*>(self + 0x3C), *reinterpret_cast<int32*>(self + 0x40));
    }

    bool bHas40000 = ((*reinterpret_cast<uint32*>(self + 0x50) >> 18) & 1u) != 0;
    ped = *reinterpret_cast<uint8**>(self + 0x24);
    int32 g40 = *reinterpret_cast<int32*>(self + 0x40);
    int32 g3C = *reinterpret_cast<int32*>(self + 0x3C);
    int32 pedBA4 = *reinterpret_cast<int32*>(ped + 0xBA4);
    float blendOutRate = ((g40 == 0x36 && g3C == pedBA4)
        || (g40 == pedBA4 && g3C == 0x36)
        || (g40 == 0x37 && g3C == 0x32)
        || (g40 == 0x32 && g3C == 0x37))
        ? Core::StaticRef<float>(0xD50398)
        : Core::StaticRef<float>(0xDCB2A4);

    float fadeRate = Core::StaticRef<float>(0xEBDED4);
    if (Core::StaticRef<void*>(0x165D184)) {
        fadeRate = Core::StaticRef<float>(0xEBE374);
    } else if (*reinterpret_cast<uint32*>(ped + 0x26C) & 0x2000u) {
        fadeRate = Core::StaticRef<float>(0xEBE008);
    }

    int32 slotCountSnapshot = Core::StaticRef<int32>(0x165D198);
    int32 iterCount = 0;
    uint8* slotBase = &Core::StaticRef<uint8>(0x165D19C);
    uint8* activeSlot2 = Core::Hook::CallMethod<uint8*>(0x9EC060, animPlayer, 0, 2);
    while (activeSlot2) {
        int32 sId = *reinterpret_cast<int32*>(activeSlot2 + 0xC);
        if (sId != 0x26 && sId != 0x27 && sId != 0x30 && sId != 0x31) {
            int32 idx = 0;
            for (; idx < slotCountSnapshot; ++idx) {
                uint8* entry = slotBase + idx * 0x20;
                if (sId == *reinterpret_cast<int32*>(entry)) {
                    int32 aGrp = *reinterpret_cast<int32*>(activeSlot2 + 0x10);
                    if (aGrp == animGroup || aGrp == altGroup) {
                        int32 cur40 = *reinterpret_cast<int32*>(self + 0x40);
                        if (cur40 == -1 || aGrp != cur40)
                            break;
                    }
                }
            }
            if (idx == slotCountSnapshot) {
                if (*reinterpret_cast<float*>(activeSlot2 + 0x58) <= 0.0f
                    && Core::Hook::CallMethod<float>(0xA4DB70, activeSlot2) <= 0.0f)
                {
                    Core::Hook::CallMethod<void>(0x9EF450, animPlayer, activeSlot2);
                } else {
                    int32 aGrp = *reinterpret_cast<int32*>(activeSlot2 + 0x10);
                    int32 cur40 = *reinterpret_cast<int32*>(self + 0x40);
                    if (aGrp == cur40 && aGrp != *reinterpret_cast<int32*>(self + 0x3C)) {
                        float rem = *reinterpret_cast<float*>(activeSlot2 + 0x58)
                            - Core::StaticRef<float>(0x11DDE7C) * blendOutRate;
                        if (rem <= Core::StaticRef<float>(0xDECD60)) {
                            Core::Hook::CallMethod<void>(0x9EF450, animPlayer, activeSlot2);
                        } else {
                            Core::Hook::CallMethod<void>(0xA34680, &Core::StaticRef<uint8>(0x165D198),
                                *reinterpret_cast<int32*>(activeSlot2 + 0xC), rem, 0.0f, 0.0f, 0.0f, 0, cur40, 1.0f);
                            bHas40000 = true;
                        }
                    } else {
                        if (Core::Hook::CallMethod<float>(0xA4DB70, activeSlot2) > fadeRate) {
                            Core::Hook::CallMethod<void>(0xA4DBC0, activeSlot2, fadeRate);
                        }
                    }
                }
            }
        }
        ++iterCount;
        activeSlot2 = Core::Hook::CallMethod<uint8*>(0x9EC0F0, animPlayer, 0, 2);
    }

    int32 cur40 = *reinterpret_cast<int32*>(self + 0x40);
    if (cur40 != -1) {
        int32 gIdx = Core::Hook::CallMethod<int32>(0x92F590, Core::StaticRef<void*>(0x1548F88), cur40);
        if (gIdx == -1 || !Core::Hook::Call<bool>(0x832DD0, reinterpret_cast<void*>(gIdx), Core::StaticRef<void*>(0xF27324))) {
            bHas40000 = false;
        }
    }
    if (*reinterpret_cast<int32*>(self + 0x40) != -1 && !bHas40000) {
        *reinterpret_cast<int32*>(self + 0x40) = -1;
    }

    if (slotCountSnapshot == 0 && iterCount == 0 && Core::StaticRef<int32>(0x165D198) < 9) {
        int32 nIdx = Core::StaticRef<int32>(0x165D198);
        uint8* entry = slotBase + nIdx * 0x20;
        float one = Core::StaticRef<float>(0xD50398);
        *reinterpret_cast<int32*>(entry + 0x00) = 0xF;
        *reinterpret_cast<int32*>(entry + 0x04) = -1;
        *reinterpret_cast<float*>(entry + 0x08) = one;
        *reinterpret_cast<float*>(entry + 0x0C) = 0.0f;
        *reinterpret_cast<float*>(entry + 0x10) = 0.0f;
        *reinterpret_cast<float*>(entry + 0x14) = 0.0f;
        *reinterpret_cast<int32*>(entry + 0x18) = 0;
        *reinterpret_cast<float*>(entry + 0x1C) = one;
        Core::StaticRef<int32>(0x165D198) = nIdx + 1;
    }

    if (bHas40000) {
        Core::Hook::CallMethod<void>(0xA34A10, &Core::StaticRef<uint8>(0x165D198));
    }

    if (bFlag12
        || Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x12)
        || Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0x17))
    {
        if (void* aF = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0xF)) {
            Core::Hook::CallMethod<void>(0xA4DBC0, aF, Core::StaticRef<float>(0xEBE2AC));
        }
    }

    Core::Hook::CallMethod<void>(0x9EF440, animPlayer);

    int32* dstSlots = reinterpret_cast<int32*>(self + 0xAC);
    for (int32 i = 0; i < 9; ++i) {
        dstSlots[i] = *reinterpret_cast<int32*>(slotBase + i * 0x20);
    }

    float one = Core::StaticRef<float>(0xD50398);
    int32 writeIdx = 0;
    for (int32 i = 0; i < 9; ++i) {
        uint8* entry = slotBase + i * 0x20;
        int32 sId = *reinterpret_cast<int32*>(entry + 0x00);
        float weight = *reinterpret_cast<float*>(entry + 0x08);
        if (sId == -1 || weight == 0.0f)
            continue;

        int32 grp = *reinterpret_cast<int32*>(entry + 0x04);
        if (grp == -1) {
            uint8* node = *reinterpret_cast<uint8**>(entry + 0x18);
            grp = (node && (node[0x4] & 2u)) ? altGroup : animGroup;
        }

        if (weight > one) {
            *reinterpret_cast<float*>(entry + 0x08) = one;
        } else if (weight < 0.0f) {
            *reinterpret_cast<float*>(entry + 0x08) = 0.0f;
        }

        float rate = *reinterpret_cast<float*>(entry + 0x0C);
        dstSlots[writeIdx++] = sId;
        uint8* anim = Core::Hook::CallMethod<uint8*>(0x9EBE90, animPlayer, grp, sId);
        if (!anim) {
            if (Core::Hook::Call<bool>(0x909430, grp, sId)) {
                anim = Core::Hook::CallMethod<uint8*>(0x9EFEB0, animPlayer, grp, sId, rate, -1);
            } else if (altGroup != -1 && Core::Hook::Call<bool>(0x909430, altGroup, sId)) {
                anim = Core::Hook::CallMethod<uint8*>(0x9EFEB0, animPlayer, altGroup, sId, rate, -1);
            }
        }

        if (anim) {
            Core::Hook::CallMethod<void>(0xA4C470, anim, *reinterpret_cast<float*>(entry + 0x08));
            Core::Hook::CallMethod<void>(0xA4DBC0, anim, rate);
            float f1C = *reinterpret_cast<float*>(entry + 0x1C);
            float spdScale = (f1C == one) ? (*reinterpret_cast<float*>(self + 0x6C) * Core::StaticRef<float>(0xF359D0))
                                          : (*reinterpret_cast<float*>(self + 0x6C) * f1C);
            *reinterpret_cast<float*>(anim + 0x54) = spdScale;
            *reinterpret_cast<uint32*>(anim + 0x4) |= 0x10u;
            if (sId == 0x37 || sId == 0x38 || sId == 0x39 || sId == 0x3B) {
                Core::StaticRef<void*>(0x165D184) = anim;
                *reinterpret_cast<uint32*>(self + 0x50) |= 0x10u;
                *reinterpret_cast<float*>(self + 0x30) = 0.0f;
            }
        }
    }

    *reinterpret_cast<uint32*>(self + 0x1C) = Core::StaticRef<uint32>(0x165D190);
    *reinterpret_cast<uint32*>(self + 0x20) = Core::StaticRef<uint32>(0x165D194);

    if (Core::StaticRef<uint8>(0x165D16A)) {
        ped = *reinterpret_cast<uint8**>(self + 0x24);
        subA90 = *reinterpret_cast<uint8**>(ped + 0xA90);
        bool bStrafePost = ((*reinterpret_cast<uint32*>(subA90 + 0x50) >> 1) & 1u) != 0;
        float sumVel = bStrafePost
            ? Core::StaticRef<float>(0x165D170)
            : (Core::StaticRef<float>(0x165D20C)
                + Core::StaticRef<float>(0x165D1EC)
                + Core::StaticRef<float>(0x165D1CC)
                + Core::StaticRef<float>(0x165D1AC));
        float maxStep = *reinterpret_cast<float*>(self + 0x28) * Core::StaticRef<float>(0x11DDE7C);
        float diffVel = maxStep - sumVel;
        if (headingDiff != 0.0f) {
            float negOne = Core::StaticRef<float>(0xEBDED4);
            float sDiff = (diffVel < 0.0f) ? negOne : ((diffVel == 0.0f) ? 0.0f : one);
            float sMax = (maxStep < 0.0f) ? negOne : ((maxStep == 0.0f) ? 0.0f : one);
            if (sDiff == sMax) {
                float sHead = (headingDiff < 0.0f) ? negOne : one;
                if (sHead == sMax) {
                    if (Core::StaticRef<uint8>(0x165D18C)) {
                        *reinterpret_cast<float*>(self + 0x30) = diffVel;
                    } else {
                        float scale = ((*reinterpret_cast<uint32*>(self + 0x50) >> 13) & 1u)
                            ? Core::StaticRef<float>(0xF27D78)
                            : Core::StaticRef<float>(0xF27D7C);
                        float v = scale * diffVel;
                        if (bStrafePost) {
                            v += Core::StaticRef<float>(0x165D170);
                        }
                        *reinterpret_cast<float*>(self + 0x30) = v;
                        if ((maxStep < 0.0f && maxStep > v) || (maxStep > 0.0f && v > maxStep)) {
                            *reinterpret_cast<float*>(self + 0x30) = maxStep;
                        }
                    }
                }
            }
        }
    }

    ped = *reinterpret_cast<uint8**>(self + 0x24);
    if ((ped[0x219] && Core::StaticRef<uint8>(0xF3597D)) || (!ped[0x219] && Core::StaticRef<uint8>(0xF3597E))) {
        ApplyPedVelocityToTask();
    }
    *reinterpret_cast<uint32*>(self + 0x50) &= ~0x40000u;
    ResetBlendFlagsAndSaveTargetHeading();
    StartIdleAnim(*reinterpret_cast<void**>(self + 0x24), reinterpret_cast<uint32*>(animData));

    if (*reinterpret_cast<uint32*>(self + 0x50) & 0x800u) {
        if (void* aF = Core::Hook::CallMethod<void*>(0x9EBF50, animPlayer, 0xF)) {
            int32 rnd = Core::Hook::Call<int32>(0xD0D21A);
            float phase = static_cast<float>(rnd) * Core::StaticRef<float>(0xEBE17C) * Core::StaticRef<float>(0xEBE034);
            Core::Hook::CallMethod<void>(0xA4D6E0, aF, phase);
        }
        *reinterpret_cast<uint32*>(self + 0x50) &= ~0x800u;
    }
    return 1;
}

// 0xA3B0A0
void* __thiscall CPedMoveBlend::ScalarDeletingDtor(uint8 flags)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD88654);
    if (flags & 1u) {
        Core::Hook::CallMethod<void>(0xC79660, Core::StaticRef<void*>(0x18A82B4), this);
    }
    return this;
}

// 0xA3B0D0
void __thiscall CPedMoveBlend::ClearSubArray()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (self[0x8]) {
        uint16 count = *reinterpret_cast<uint16*>(self + 0x4);
        void** arr = *reinterpret_cast<void***>(self);
        for (uint16 i = 0; i < count; ++i) {
            void* elem = arr[i];
            if (elem) {
                using DtorFn = void(__thiscall*)(void*, uint32);
                reinterpret_cast<DtorFn>((*reinterpret_cast<void***>(elem))[0])(elem, 1);
            }
        }
    }
    *reinterpret_cast<uint16*>(self + 0x4) = 0;
}

// 0xA3B120
void __thiscall CPedMoveBlend::UpdateSubArraySlot4()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!self[0x8])
        return;

    uint16 count = *reinterpret_cast<uint16*>(self + 0x4);
    void** arr = *reinterpret_cast<void***>(self);
    for (uint16 i = 0; i < count; ++i) {
        void* elem = arr[i];
        using Fn4 = void(__thiscall*)(void*);
        reinterpret_cast<Fn4>((*reinterpret_cast<void***>(elem))[1])(elem);
    }
}

// 0xA3B150
void __thiscall CPedMoveBlend::UpdateSubArraySlot8()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!self[0x8])
        return;

    uint16 count = *reinterpret_cast<uint16*>(self + 0x4);
    void** arr = *reinterpret_cast<void***>(self);
    for (uint16 i = 0; i < count; ++i) {
        void* elem = arr[i];
        using Fn8 = void(__thiscall*)(void*);
        reinterpret_cast<Fn8>((*reinterpret_cast<void***>(elem))[2])(elem);
    }
}

// 0xA3B180
void __thiscall CPedMoveBlend::SetSubArrayEntry(int32 index, void* val)
{
    void** arr = *reinterpret_cast<void***>(this);
    arr[index] = val;
}

// 0xA3B190
void __thiscall CPedMoveBlend::ProcessSubArraySlot10(void* arg)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    uint16 count = *reinterpret_cast<uint16*>(self + 0x4);
    void** arr = *reinterpret_cast<void***>(self);
    for (uint16 i = 0; i < count; ++i) {
        void* elem = arr[i];
        using Fn14 = bool(__thiscall*)(void*);
        if (!reinterpret_cast<Fn14>((*reinterpret_cast<void***>(elem))[5])(elem)) {
            using Fn10 = void(__thiscall*)(void*, void*);
            reinterpret_cast<Fn10>((*reinterpret_cast<void***>(elem))[4])(elem, arg);
        }
    }
}

// 0xA3B1D0
void __thiscall CPedMoveBlend::DestroySubObject(int32* arg)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::Call<void>(0x5B1C10, *reinterpret_cast<void**>(self));
    *reinterpret_cast<uint32*>(self) = 0;
    *reinterpret_cast<uint16*>(self + 0x4) = 0;
    *reinterpret_cast<uint16*>(self + 0x6) = 0;
    Core::Hook::CallMethod<void>(0xB367C0, self + 0xC);
    if (arg) {
        *reinterpret_cast<int32*>(self + 0xC) = *arg;
    }
    Core::Hook::CallMethod<void>(0xB367C0, self + 0x18);
    *reinterpret_cast<int32*>(self + 0x18) = -1;
}

// 0xA3B220
void __thiscall CPedMoveBlend::ClearSubArrayThunk()
{
    ClearSubArray();
}

// 0xA3B230
void* __thiscall CPedMoveBlend::Alloc16AndStore(void* val)
{
    void** mem = Core::Hook::CallMethod<void**>(0xABDDF0, this, 0x10);
    *mem = val;
    return val;
}

// 0xA3B250
void __cdecl CPedMoveBlend::InvokeCallback_11DE7F8()
{
    auto fn = Core::StaticRef<void(__cdecl*)()>(0x11DE7F8);
    if (fn) {
        fn();
    }
}

// 0xA3B260
void* __thiscall CPedMoveBlend::CtorSubA3B260(void* arg)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    Core::Hook::CallMethod<void>(0x91FE20, this, arg);
    *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD88A64);
    *reinterpret_cast<int32*>(self + 0x8F4) = 7;
    return this;
}

// 0xA3B290
void __cdecl CPedMoveBlend::SetupDebugRenderStates()
{
    Core::Hook::Call<void>(0x62D290);
    Core::Hook::Call<void>(0x62D2D0, 7, 1);
    Core::Hook::Call<void>(0x62D2D0, 2, 0);
    Core::Hook::Call<void>(0x62D2D0, 0xF, 0xF);
    Core::Hook::Call<void>(0x98F4E0, 0.0f);
    Core::Hook::Call<void>(0x98F4E0, 1.0f);
}

// 0xA3B2D0
void __thiscall CPedMoveBlend::RenderDebugOverlay(uint8* arg)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int32*>(self + 0x8F4) = 7;
    if (!Core::StaticRef<uint8>(0x10C7F6F)) {
        if (Core::Hook::Call<int32>(0x846470, 0) != -1) {
            *reinterpret_cast<int32*>(self + 0x8F4) = 8;
        }
    }

    Core::Hook::CallMethod<void>(0x91FCC0, this, arg);
    if (!arg[0x300])
        return;

    float* rect = Core::Hook::CallMethod<float*>(0x62F1A0, arg + 0x10);
    float r0 = rect[0], r1 = rect[1], r2 = rect[2], r3 = rect[3];

    alignas(16) uint8 spriteBuf[0x400]{};
    Core::Hook::CallMethod<void>(0x62DA20, spriteBuf);
    Core::Hook::CallMethod<void>(0x443250, self + 0xB0, spriteBuf);

    bool bWide1 = Core::Hook::Call<bool>(0x6220F0);
    int32 h1 = bWide1 ? Core::StaticRef<int32>(0xFDCEA8) : Core::StaticRef<int32>(0xFDCEB0);
    int32 y2 = static_cast<int32>(static_cast<float>(h1) * r3);

    bool bWide2 = Core::Hook::Call<bool>(0x6220F0);
    int32 w1 = bWide2 ? Core::StaticRef<int32>(0xFDCEA4) : Core::StaticRef<int32>(0xFDCEAC);
    int32 x2 = static_cast<int32>(static_cast<float>(w1) * r2);

    bool bWide3 = Core::Hook::Call<bool>(0x6220F0);
    int32 h0 = bWide3 ? Core::StaticRef<int32>(0xFDCEA8) : Core::StaticRef<int32>(0xFDCEB0);
    int32 y1 = static_cast<int32>(static_cast<float>(h0) * r1);

    bool bWide4 = Core::Hook::Call<bool>(0x6220F0);
    int32 w0 = bWide4 ? Core::StaticRef<int32>(0xFDCEA4) : Core::StaticRef<int32>(0xFDCEAC);
    int32 x1 = static_cast<int32>(static_cast<float>(w0) * r0);

    Core::Hook::CallMethod<void>(0x62E2D0, self + 0xB0, x1, y1, x2, y2, 0.0f, 1.0f);
    Core::Hook::CallMethod<void>(0x62E7C0, self + 0xB0, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    Core::Hook::CallMethod<void>(0x62DD70, spriteBuf);
}

// =========================================================================
// Hook Registration (46 functions)
// =========================================================================

void CPedMoveBlend::InjectHooks()
{
    // Slice 1
    Core::Hook::Install(0xA35050, &CPedMoveBlend::IsAnimGroup53_59);
    Core::Hook::Install(0xA35070, &CPedMoveBlend::ResetBlendFlagsAndSaveTargetHeading);
    Core::Hook::Install(0xA350D0, &CPedMoveBlend::ApplyPedVelocityToTask);
    Core::Hook::Install(0xA35180, &CPedMoveBlend::GetEntityPositionByIndex);
    Core::Hook::Install(0xA351D0, &CPedMoveBlend::ComputeBlendSegment);
    Core::Hook::Install(0xA35260, &CPedMoveBlend::HasActiveMoveAnimInSlot2);
    Core::Hook::Install(0xA352C0, &CPedMoveBlend::HasActiveLocomotionAnimInSlot2);
    Core::Hook::Install(0xA35310, &CPedMoveBlend::IsMovingOrTurning);
    Core::Hook::Install(0xA35370, &CPedMoveBlend::ClampMoveSpeeds);
    Core::Hook::Install(0xA353F0, &CPedMoveBlend::SetUnk6C);
    Core::Hook::Install(0xA35400, &CPedMoveBlend::GetDefaultFloat_F359A4);
    Core::Hook::Install(0xA35410, &CPedMoveBlend::Ctor);
    Core::Hook::Install(0xA35430, &CPedMoveBlend::StartIdleAnim);
    Core::Hook::Install(0xA35470, &CPedMoveBlend::StartIdleAnimFromPed);
    Core::Hook::Install(0xA354B0, &CPedMoveBlend::ProcessStopBlend);
    Core::Hook::Install(0xA35900, &CPedMoveBlend::ProcessStartTurnBlend);

    // Slice 2
    Core::Hook::Install(0xA35E40, &CPedMoveBlend::BuildBlendWeights);
    Core::Hook::Install(0xA35EE0, &CPedMoveBlend::CanTriggerQuickTurn);
    Core::Hook::Install(0xA35F70, &CPedMoveBlend::UpdateSlopeAndObstacleCheck);
    Core::Hook::Install(0xA362E0, &CPedMoveBlend::UpdateHeadingAndSpeed);
    Core::Hook::Install(0xA36820, &CPedMoveBlend::UpdateHeadingOnly);
    Core::Hook::Install(0xA36AA0, &CPedMoveBlend::Blend4WayStrafe);
    Core::Hook::Install(0xA37540, &CPedMoveBlend::Blend8WayStrafe);
    Core::Hook::Install(0xA38020, &CPedMoveBlend::ProcessTurnAndMovement);
    Core::Hook::Install(0xA38330, &CPedMoveBlend::ProcessJumpOrVaultBlend);

    // [BISECT] Slice 3A ON, Slice 3B (ProcessMovement..RenderDebugOverlay) OFF:
    Core::Hook::Install(0xA38610, &CPedMoveBlend::ComputeDesiredVelocity);
    Core::Hook::Install(0xA38B20, &CPedMoveBlend::TransformMoveVector);
    Core::Hook::Install(0xA38DA0, &CPedMoveBlend::StopMovement);
    Core::Hook::Install(0xA38EB0, &CPedMoveBlend::UpdateAimingBlend);
    Core::Hook::Install(0xA39120, &CPedMoveBlend::AllocFromPool);
    Core::Hook::Install(0xA39140, &CPedMoveBlend::UpdateStrafeMovement);
    Core::Hook::Install(0xA39EB0, &CPedMoveBlend::UpdateNormalMovement);
    // TODO: ProcessMovement (0xA3A200) causes walk->idle stutter (walk anim 28 not found via 0x9288F0); keep disabled until resolved.
    // Core::Hook::Install(0xA3A200, &CPedMoveBlend::ProcessMovement);
    Core::Hook::Install(0xA3B0A0, &CPedMoveBlend::ScalarDeletingDtor);
    Core::Hook::Install(0xA3B0D0, &CPedMoveBlend::ClearSubArray);
    Core::Hook::Install(0xA3B120, &CPedMoveBlend::UpdateSubArraySlot4);
    Core::Hook::Install(0xA3B150, &CPedMoveBlend::UpdateSubArraySlot8);
    Core::Hook::Install(0xA3B180, &CPedMoveBlend::SetSubArrayEntry);
    Core::Hook::Install(0xA3B190, &CPedMoveBlend::ProcessSubArraySlot10);
    Core::Hook::Install(0xA3B1D0, &CPedMoveBlend::DestroySubObject);
    Core::Hook::Install(0xA3B220, &CPedMoveBlend::ClearSubArrayThunk);
    Core::Hook::Install(0xA3B230, &CPedMoveBlend::Alloc16AndStore);
    Core::Hook::Install(0xA3B250, &CPedMoveBlend::InvokeCallback_11DE7F8);
    Core::Hook::Install(0xA3B260, &CPedMoveBlend::CtorSubA3B260);
    Core::Hook::Install(0xA3B290, &CPedMoveBlend::SetupDebugRenderStates);
    Core::Hook::Install(0xA3B2D0, &CPedMoveBlend::RenderDebugOverlay);
}
