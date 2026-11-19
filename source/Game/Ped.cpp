#include "StdInc.h"
#include "Game/Ped.h"
#include "Game/Pools.h"
#include "Game/PlayerInfo.h"
#include "Game/ModelInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>

void CPed::InjectHooks()
{
    Core::Hook::Install(0x8ABF10, &CPed::Ctor);
    Core::Hook::Install(0x8ACAC0, &CPed::DeletingDtor);
    Core::Hook::Install(0x8A9030, &CPed::IsValidOrPlayer);
    Core::Hook::Install(0x8A92C0, &CPed::SetPedState);
    Core::Hook::Install(0x89E5A0, &CPed::UpdateHealthFlags);
    Core::Hook::Install(0x8AC150, &CPed::InitHealth);
    Core::Hook::Install(0x8AC180, &CPed::SetHealth);
    Core::Hook::Install(0x8AC1B0, &CPed::AddHealth);

    InjectHooks_Slice1();
    InjectHooks_Slice2();
    InjectHooks_Slice3();
    InjectHooks_Slice4();
    InjectHooks_Slice5();
    InjectHooks_Slice6();
    InjectHooks_Slice7();
    InjectHooks_Slice8();

    Core::Log::Info("CPed hooks installed (159 functions)");
}

CPed* __thiscall CPed::Ctor(const uint8* initParams, int32 modelIndex)
{
    CPhysical::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD69844));

    m_bIsDead = 0;
    m_bIsInjured = 0;
    m_bIsCriticallyInjured = 0;
    m_fPedHealth = 0.0f;
    m_nPedUnk218 = initParams[0];
    m_nPedUnk219 = initParams[1];
    m_pPedData = nullptr;
    m_bHasPedData = 0;

    Core::Hook::CallMethod<void>(0x9E2320, m_pedSubObj238);
    Core::Hook::CallMethod<void>(0x9AB4A0, m_pedSubObj2B0);
    Core::Hook::CallMethod<void>(0xAAD110, m_pedSubObj3D0);

    Core::Hook::CallMethod<void>(0xCE8270, m_pedSpeechAudio);
    *reinterpret_cast<void**>(m_pedSpeechAudio) = reinterpret_cast<void*>(Core::Rebase(0xD697FC));
    Core::Hook::CallMethod<void>(0xCED1D0, m_pedSpeechAudio + 0xD0);
    Core::Hook::CallMethod<void>(0xCED1D0, m_pedSpeechAudio + 0x120);
    Core::Hook::CallMethod<void>(0xCECF80, m_pedSpeechAudio + 0x170);
    Core::Hook::CallMethod<void>(0xCECF80, m_pedSpeechAudio + 0x18C);

    Core::Hook::CallMethod<void>(0xCF0E80, m_audPlaceableTracker);
    *reinterpret_cast<void**>(m_audPlaceableTracker) = reinterpret_cast<void*>(Core::Rebase(0xD69690));
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 8) = 0;

    m_pMyVehicle = nullptr;
    for (uint32& val : m_voiceData)
        val = 0;

    Core::Hook::CallMethod<void>(0x4475C0, &m_nPedUnkB94);
    Core::Hook::CallMethod<void>(0x447290, &m_nPedUnkB98);
    Core::Hook::CallMethod<void>(0x94E500, m_pedSubObjBC0);
    Core::Hook::CallMethod<void>(0x9EB2C0, m_pedSubObjD98);

    m_pPedUnkE14 = nullptr;
    m_fPedUnkE18 = 0.0f;
    m_fPedUnkE1C = 0.0f;
    m_nPedUnkE20 = 0;
    m_vecPedUnkE30 = {};
    m_vecPedUnkE40 = {};
    m_fPedUnkE50 = 0.0f;
    m_nPedUnkE54 = -1;
    m_nPedUnkE58 = -1;
    m_nPedUnkE5C = -1;

    m_nFlags2 = (m_nFlags2 & ~0x300u) | 0xC0u;

    void* animPool = Core::StaticRef<void*>(0x18A82B8);
    uint8* obj = Core::Hook::CallMethod<uint8*>(0x96D520, animPool);
    if (obj)
        Core::Hook::CallMethod<void>(0xA66720, obj);

    m_pPedData = obj;
    m_bHasPedData = 1;

    auto* pedModel = static_cast<CPedModelInfo*>(CModelInfo::ms_modelInfoPtrs[modelIndex]);
    *reinterpret_cast<uint32*>(obj + 0x12C) = pedModel->m_nPedType;

    Core::Hook::CallMethod<void>(0x8AA570, this);
    return this;
}

CPed* __thiscall CPed::DeletingDtor(uint8 flags)
{
    Core::Hook::CallMethod<void>(0x8AC1E0, this);
    if ((flags & 1) != 0)
        Core::Hook::CallMethod<void>(0x448300, CPools::ms_pPedPool, this);
    return this;
}

bool __thiscall CPed::IsValidOrPlayer()
{
    CPool<CPed>* pool = CPools::ms_pPedPool;
    const int32 index = pool->GetIndex(this);
    if (index < 0 || index >= pool->m_nSize)
        return false;

    if (m_pAnimBlender)
        return true;

    CPed* player = CPlayerInfo::FindPlayerPed(nullptr);
    return this == player;
}

void __thiscall CPed::SetPedState(int32 state)
{
    if (state == 1 || state == 2)
    {
        Core::Hook::CallMethod<void>(0x89DC70, this);
        if ((m_nPedFlags2 & 0x10u) != 0)
        {
            const int32 handle = CPools::ms_pPedPool->GetHandle(CPools::ms_pPedPool->GetIndex(this));
            Core::Hook::Call<void>(0x80E6A0, handle, 2);
        }
    }

    m_nPedState = state;
}

int32 __thiscall CPed::UpdateHealthFlags(float health)
{
    m_bIsCriticallyInjured = (health < 50.0f) ? 1 : 0;
    m_bIsInjured = (health < 100.0f) ? 1 : 0;

    int32 dead = 0;
    if (health <= 0.0f)
    {
        dead = 1;
        m_bIsDead = 1;
    }
    else
    {
        m_bIsDead = 0;
    }

    m_fPedHealth = health;
    return dead;
}

int32 __thiscall CPed::InitHealth(float health)
{
    CPhysical::InitHealth(health);
    return UpdateHealthFlags(health);
}

int32 __thiscall CPed::SetHealth(float health, int32 arg)
{
    CPhysical::SetHealth(health, arg);
    return UpdateHealthFlags(health);
}

int32 __thiscall CPed::AddHealth(float delta)
{
    CPhysical::AddHealth(delta);
    using GetHealthFn = float(__thiscall*)(const CPed*);
    const float current = reinterpret_cast<GetHealthFn>(m_vftable[63])(this);
    return UpdateHealthFlags(current);
}

// --- Slice 1 ---
void CPed::InjectHooks_Slice1()
{
    Core::Hook::Install(0x89C390, &CPed::SetPedUnk7C8);
    Core::Hook::Install(0x89C560, &CPed::UpdateUnk7C0And7C4);
    Core::Hook::Install(0x89C5B0, &CPed::IsAllowedAnimId);
    Core::Hook::Install(0x89C8B0, &CPed::SetUnkB8C);
    Core::Hook::Install(0x89CE20, &CPed::ResetUnkBB0);
    Core::Hook::Install(0x89CF80, &CPed::GetPedIntelligenceUnk08);
    Core::Hook::Install(0x89CFA0, &CPed::SetUnkVtbl52_322);
    Core::Hook::Install(0x89CFC0, &CPed::GetUnkVtbl53_322);
    Core::Hook::Install(0x89CFE0, &CPed::SetUnkVtbl52_324);
    Core::Hook::Install(0x89D000, &CPed::GetUnkVtbl53_324);
    Core::Hook::Install(0x89D040, &CPed::StartUnkA74Timer);
    Core::Hook::Install(0x89D070, &CPed::GetUnkA74TimerProgress);
    Core::Hook::Install(0x89D0E0, &CPed::GetUnkA74Float);
    Core::Hook::Install(0x89D0F0, &CPed::SetUnkAB0);
    Core::Hook::Install(0x89D110, &CPed::SetUnkAB4);
    Core::Hook::Install(0x89D130, &CPed::SetHeadingToPoint2D);
    Core::Hook::Install(0x89D180, &CPed::GetUnkAB8);
    Core::Hook::Install(0x89D190, &CPed::CallIntelligenceVtbl9);
    Core::Hook::Install(0x89D1B0, &CPed::CallIntelligenceVtbl10);
    Core::Hook::Install(0x89D640, &CPed::DecayAudioTimers);
    Core::Hook::Install(0x89D840, &CPed::GetMaxFromPedUnk224);
    Core::Hook::Install(0x89D8B0, &CPed::GetVecPedUnkE30IfSet);
    Core::Hook::Install(0x89D8D0, &CPed::ResetUnkAB8);
}

int32 __thiscall CPed::SetPedUnk7C8(int32 value, int32 unused)
{
    (void)unused;
    m_nRagdollState = value;
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x7CC) = Core::StaticRef<uint32>(0x11DDE74);
    return value;
}

int32 __thiscall CPed::UpdateUnk7C0And7C4(int32 arg)
{
    int32 result = 0;
    if (m_pAnimBlendInst)
    {
        using VtblFn = int32(__thiscall*)(void*, int32);
        auto** vtbl = *reinterpret_cast<void***>(m_pAnimBlendInst);
        result = reinterpret_cast<VtblFn>(vtbl[2])(m_pAnimBlendInst, arg);
    }

    void* unk7C4 = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x7C4);
    if (unk7C4)
    {
        using VtblFn = int32(__thiscall*)(void*, int32);
        auto** vtbl = *reinterpret_cast<void***>(unk7C4);
        return reinterpret_cast<VtblFn>(vtbl[2])(unk7C4, arg);
    }

    return result;
}

uint8 __cdecl CPed::IsAllowedAnimId(const int32* animId)
{
    switch (*animId)
    {
    case 0x12D:
    case 0x12F:
    case 0x131:
    case 0x133:
    case 0x17A:
        return 0;
    default:
        return 1;
    }
}

int32 __thiscall CPed::SetUnkB8C(int32 value)
{
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xB8C) = value;
    return value;
}

void __thiscall CPed::ResetUnkBB0()
{
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xBB0) = 438;
}

float __thiscall CPed::GetPedIntelligenceUnk08()
{
    return *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(m_pMoveBlend) + 8);
}

int32 __thiscall CPed::SetUnkVtbl52_322(uint8 value)
{
    using GetSubFn = uint8*(__thiscall*)(CPed*);
    uint8* sub = reinterpret_cast<GetSubFn>(m_vftable[52])(this);
    sub[322] = value;
    return reinterpret_cast<int32>(sub);
}

uint8 __thiscall CPed::GetUnkVtbl53_322()
{
    using GetSubFn = const uint8*(__thiscall*)(CPed*);
    const uint8* sub = reinterpret_cast<GetSubFn>(m_vftable[53])(this);
    return sub[322];
}

int32 __thiscall CPed::SetUnkVtbl52_324(int32 value)
{
    using GetSubFn = uint8*(__thiscall*)(CPed*);
    uint8* sub = reinterpret_cast<GetSubFn>(m_vftable[52])(this);
    *reinterpret_cast<int32*>(sub + 324) = value;
    return reinterpret_cast<int32>(sub);
}

float __thiscall CPed::GetUnkVtbl53_324()
{
    using GetSubFn = const uint8*(__thiscall*)(CPed*);
    const uint8* sub = reinterpret_cast<GetSubFn>(m_vftable[53])(this);
    return *reinterpret_cast<const float*>(sub + 324);
}

uint32 __thiscall CPed::StartUnkA74Timer(int32 value)
{
    auto* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int32*>(self + 0xA74) = value;
    const uint32 now = Core::StaticRef<uint32>(0x11DDE74);
    m_nHeadLookStartMs = now;
    m_bHeadLookActive = 1;
    return now;
}

float __thiscall CPed::GetUnkA74TimerProgress()
{
    if (!m_bHeadLookActive)
        return 1.0f;

    const uint32 start = m_nHeadLookStartMs;
    const uint32 now = Core::StaticRef<uint32>(0x11DDE74);
    if (now >= start && (now - start) <= 500u)
        return static_cast<float>(static_cast<double>(now) - static_cast<double>(start)) * 0.0020000001f;

    m_bHeadLookActive = 0;
    return 1.0f;
}

float __thiscall CPed::GetUnkA74Float()
{
    return *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(this) + 0xA74);
}

void __thiscall CPed::SetUnkAB0(int32 value)
{
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xAB0) = value;
}

void __thiscall CPed::SetUnkAB4(int32 value)
{
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xAB4) = value;
}

void __thiscall CPed::SetHeadingToPoint2D(const float* targetPos)
{
    const double angle = Core::Hook::Call<double>(0xA4E320, targetPos[0], targetPos[1], m_pMatrix->pos.x, m_pMatrix->pos.y);
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(this) + 0xAB4) = static_cast<float>(angle);
}

float __thiscall CPed::GetUnkAB8()
{
    return *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(this) + 0xAB8);
}

int32 __thiscall CPed::CallIntelligenceVtbl9()
{
    using VtblFn = int32(__thiscall*)(void*, CPed*);
    auto** vtbl = *reinterpret_cast<void***>(m_pMoveBlend);
    return reinterpret_cast<VtblFn>(vtbl[9])(m_pMoveBlend, this);
}

int32 __thiscall CPed::CallIntelligenceVtbl10(int32 arg)
{
    using VtblFn = int32(__thiscall*)(void*, int32);
    auto** vtbl = *reinterpret_cast<void***>(m_pMoveBlend);
    return reinterpret_cast<VtblFn>(vtbl[10])(m_pMoveBlend, arg);
}

void __thiscall CPed::DecayAudioTimers()
{
    if (!m_bIsDead || (m_nPedState != 1 && m_nPedState != 2))
    {
        if (m_pPedIntelligence)
        {
            auto* self = reinterpret_cast<uint8*>(this);
            const float dt = Core::StaticRef<float>(0x11DDE7C);

            float& t1 = *reinterpret_cast<float*>(self + 0x7B0);
            t1 = (t1 <= 0.0f) ? 0.0f : (t1 - dt);

            float& t2 = *reinterpret_cast<float*>(self + 0x7B4);
            t2 = (t2 <= 0.0f) ? 0.0f : (t2 - dt);
        }
    }
}

uint8 __thiscall CPed::GetMaxFromPedUnk224(float* outMax)
{
    uint8 found = 0;
    const auto* base = reinterpret_cast<const uint8*>(m_pPedIntelligence);

    for (int32 offset = 68; offset < 88; offset += 4)
    {
        void* obj = *reinterpret_cast<void* const*>(base + offset);
        if (!obj)
            continue;

        using VtblFn = double(__thiscall*)(void*, CPed*);
        auto** vtbl = *reinterpret_cast<void***>(obj);
        const double val = reinterpret_cast<VtblFn>(vtbl[6])(obj, this);
        if (val < 0.0)
            return 0;

        const float fval = static_cast<float>(val);
        if (!found || fval > *outMax)
        {
            *outMax = fval;
            found = 1;
        }
    }

    return found;
}

CVector* __thiscall CPed::GetVecPedUnkE30IfSet()
{
    return m_nPedUnkE20 ? &m_vecPedUnkE30 : nullptr;
}

void __thiscall CPed::ResetUnkAB8()
{
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(this) + 0xAB8) = 11.0f;
}
// --- Slice 2 ---
// =========================================================================
// CPed Slice 2 (0x89D8F0 .. 0x89F530)
// 57 functions in slice (0x89E5A0 already hooked in Ped.cpp -> skipped):
//   - 21 hooked below (pure C++ or verified external calling conventions)
//   - 36 skipped: non-CPed helpers/sub-objects (0x89DD20, 0x89DEB0, 0x89DED0,
//     0x89DF30, 0x89DF90, 0x89E010, 0x89E640, 0x89E690, 0x89E6B0, 0x89EA80,
//     0x89EAB0, 0x89EBF0), pure thunks (0x89DD80, 0x89E6A0), or functions
//     calling external sub_XXXXX whose __thiscall/__cdecl requires disasm
//     verification (0x89D950, 0x89D9C0, 0x89DA20, 0x89DB00, 0x89DB40,
//     0x89DC70, 0x89DCE0, 0x89DD90, 0x89DFB0, 0x89E1F0, 0x89E250, 0x89E270,
//     0x89E300, 0x89E3E0, 0x89E660, 0x89EC20, 0x89ED00, 0x89EFE0, 0x89F050,
//     0x89F140, 0x89F390, 0x89F530).
// =========================================================================

void CPed::InjectHooks_Slice2()
{
    Core::Hook::Install(0x89D8F0, &CPed::RemoveFromVehicle);
    Core::Hook::Install(0x89DCB0, &CPed::CanUseUnkB38);
    Core::Hook::Install(0x89DE90, &CPed::IsSpecialPedType);
    Core::Hook::Install(0x89DFD0, &CPed::SetUnkE10);
    Core::Hook::Install(0x89E000, &CPed::GetUnkE10);
    Core::Hook::Install(0x89E070, &CPed::ClearUnkD68Refs);
    Core::Hook::Install(0x89E0A0, &CPed::AddUnkD68Ref);
    Core::Hook::Install(0x89E0E0, &CPed::RemoveUnkD68Ref);
    Core::Hook::Install(0x89E120, &CPed::IsPedType2NotModel);
    Core::Hook::Install(0x89E150, &CPed::HasUnkTask1310);
    Core::Hook::Install(0x89E1B0, &CPed::GetSpecialWeaponType);
    Core::Hook::Install(0x89E2D0, &CPed::GetModelUnkB0B1);
    Core::Hook::Install(0x89E450, &CPed::CheckRandomSeedThreshold);
    Core::Hook::Install(0x89E480, &CPed::SetUnkD78);
    Core::Hook::Install(0x89E4B0, &CPed::SetUnkFlag20AndNotifyD68);
    Core::Hook::Install(0x89E570, &CPed::GetUnkAnimWeight);
    Core::Hook::Install(0x89E610, &CPed::SetUnkB98);
    Core::Hook::Install(0x89E620, &CPed::AddUnkB98);
    Core::Hook::Install(0x89E930, &CPed::SetPositionRaw);
    Core::Hook::Install(0x89E980, &CPed::GetBoneMatrix);
    Core::Hook::Install(0x89F450, &CPed::CollectNearbyEntityCallback);
}

int32 __thiscall CPed::RemoveFromVehicle(uint8 arg)
{
    int32 result = 0;
    m_nFlags |= 0x400u;
    if ((m_nPedFlags2 & 4u) != 0)
    {
        uint8* veh = reinterpret_cast<uint8*>(m_pMyVehicle);
        if (veh)
        {
            if (*reinterpret_cast<CPed**>(veh + 0xFA0) == this)
                result = Core::Hook::CallMethod<int32>(0x9C5B00, veh, arg);
            else
                result = Core::Hook::CallMethod<int32>(0x9C5760, veh, this);

            m_nPedFlags2 &= ~4u;
            if (m_pMyVehicle)
                Core::Hook::CallMethod<void>(0x9E6580, m_pMyVehicle, &m_pMyVehicle);
            m_pMyVehicle = nullptr;
        }
    }
    return result;
}

bool __thiscall CPed::CanUseUnkB38(float val) const
{
    if ((m_nPedFlags & 8u) == 0)
        return true;
    const float limit = *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(this) + 0xB38);
    return val < limit;
}

bool __thiscall CPed::IsSpecialPedType() const
{
    const uint32 pedType = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pPedData) + 0x12C);
    return pedType >= 3 && pedType <= 14;
}

int32 __thiscall CPed::SetUnkE10(CEntity* entity)
{
    auto** slot = reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0xE10);
    if (*slot)
    {
        Core::Hook::CallMethod<void>(0x9E6580, *slot, slot);
        *slot = nullptr;
    }
    *slot = entity;
    if (entity)
        return Core::Hook::CallMethod<int32>(0x9E6550, entity, slot);
    return 0;
}

CEntity* __thiscall CPed::GetUnkE10() const
{
    return *reinterpret_cast<CEntity* const*>(reinterpret_cast<const uint8*>(this) + 0xE10);
}

int32 __thiscall CPed::ClearUnkD68Refs()
{
    auto** arr = reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0xD68);
    for (int32 i = 0; i < 4; ++i)
    {
        if (arr[i])
            Core::Hook::CallMethod<void>(0x9E6580, arr[i], &arr[i]);
    }
    return 0;
}

int32 __thiscall CPed::AddUnkD68Ref(CEntity* entity)
{
    auto** arr = reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0xD68);
    for (int32 i = 0; i < 4; ++i)
    {
        if (!arr[i])
        {
            arr[i] = entity;
            return Core::Hook::CallMethod<int32>(0x9E6550, entity, &arr[i]);
        }
    }
    return 4;
}

int32 __thiscall CPed::RemoveUnkD68Ref(CEntity* entity)
{
    auto** arr = reinterpret_cast<CEntity**>(reinterpret_cast<uint8*>(this) + 0xD68);
    for (int32 i = 0; i < 4; ++i)
    {
        if (arr[i] == entity)
        {
            if (arr[i])
                Core::Hook::CallMethod<void>(0x9E6580, arr[i], &arr[i]);
            arr[i] = nullptr;
            return i;
        }
    }
    return 4;
}

bool __thiscall CPed::IsPedType2NotModel() const
{
    const uint32 pedType = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pPedData) + 0x12C);
    return pedType == 2 && m_nModelIndex != Core::StaticRef<int32>(0x1230AC0);
}

uint8 __thiscall CPed::HasUnkTask1310() const
{
    const auto* sub = reinterpret_cast<const uint8*>(m_pPedIntelligence);
    const auto* node = *reinterpret_cast<const uint32* const*>(sub + 0x2E0);
    if (node)
    {
        uint32 prio = (node[2] >> 1) & 7u;
        while (prio >= ((node[2] >> 1) & 7u) || prio < 2)
        {
            prio = (node[2] >> 1) & 7u;
            if (node[1] == 1310)
                return 1;
            node = reinterpret_cast<const uint32*>(node[3]);
            if (!node)
                break;
        }
    }
    const uint32 flags270 = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(this) + 0x270);
    return static_cast<uint8>((flags270 >> 1) & 1u);
}

int32 __cdecl CPed::GetSpecialWeaponType(const CPed* ped)
{
    const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[ped->m_nModelIndex]);
    const int32 type = *reinterpret_cast<const int32*>(modelInfo + 0xC4);
    if (type == 82) return 133;
    if (type == 85) return 134;
    if (type == 84) return 135;
    if (type == 86) return 136;
    return -1;
}

int32 __thiscall CPed::GetModelUnkB0B1(int32* outA, int32* outB) const
{
    const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    *outA = *reinterpret_cast<const int8*>(modelInfo + 0xB0);
    const int32 b = *reinterpret_cast<const int8*>(modelInfo + 0xB1);
    *outB = b;
    return b;
}

bool __thiscall CPed::CheckRandomSeedThreshold() const
{
    return (static_cast<float>(m_nRandomSeed) * 0.000030518509f) <= 0.66000003f;
}

int32 __thiscall CPed::SetUnkD78(int32 val)
{
    auto* slot = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xD78);
    if (*slot)
        Core::Hook::CallMethod<void>(0x89DC70, this);
    *slot = val;
    return val;
}

int32 __thiscall CPed::SetUnkFlag20AndNotifyD68(uint8 flag)
{
    int32 result = static_cast<int32>((m_nFlags ^ (32u * flag)) & 0x20u);
    m_nFlags ^= static_cast<uint32>(result);

    void** arr = reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0xD68);
    using NotifyFn = int32(__thiscall*)(void*, int32);
    for (int32 i = 0; i < 4; ++i)
    {
        if (arr[i])
        {
            void** vtbl = *reinterpret_cast<void***>(arr[i]);
            result = reinterpret_cast<NotifyFn>(vtbl[12])(arr[i], flag);
        }
    }
    return result;
}

double __thiscall CPed::GetUnkAnimWeight() const
{
    const auto* base = reinterpret_cast<const uint8*>(this);
    const int32 mode = *reinterpret_cast<const int32*>(base + 0x144);
    if (mode == 0)
        return 0.0;
    if (mode == 2)
        return 1.0;
    if (m_nRagdollState > 2)
        return *reinterpret_cast<const float*>(base + 0x8C4);
    return *reinterpret_cast<const float*>(base + 0x7E4);
}

uint32* __thiscall CPed::SetUnkB98(int32 value)
{
    return Core::Hook::CallMethod<uint32*>(0x447350, &m_nPedUnkB98, value);
}

uint32* __thiscall CPed::AddUnkB98(int32 delta)
{
    const int32 current = Core::Hook::CallMethod<int32>(0x447370, &m_nPedUnkB98);
    return SetUnkB98(current + delta);
}

int32 __thiscall CPed::SetPositionRaw(const float* pos, int32 /*arg2*/, int32 /*arg3*/)
{
    if (m_pMatrix)
    {
        auto* mat = reinterpret_cast<float*>(m_pMatrix);
        mat[12] = pos[0];
        mat[13] = pos[1];
        mat[14] = pos[2];
        mat[15] = pos[3];
        return reinterpret_cast<int32>(m_pMatrix);
    }
    m_vecPos.x = pos[0];
    m_vecPos.y = pos[1];
    m_vecPos.z = pos[2];
    return reinterpret_cast<int32>(pos);
}

const float* __thiscall CPed::GetBoneMatrix(int32 boneIndex)
{
    using GetFragFn = void*(__thiscall*)(CPed*);
    using GetSkelFn = uint8*(__thiscall*)(void*);

    uint8* skel = nullptr;
    void* frag = reinterpret_cast<GetFragFn>(m_vftable[40])(this);
    if (frag)
    {
        void** fragVtbl = *reinterpret_cast<void***>(frag);
        skel = reinterpret_cast<GetSkelFn>(fragVtbl[56])(frag);
    }
    else
    {
        skel = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(this) + 0x100);
    }

    if (skel)
    {
        const auto* matrices = *reinterpret_cast<const float* const*>(skel + 0x14);
        return matrices + (boneIndex << 4);
    }

    auto* fallback = reinterpret_cast<int32*>(Core::Rebase(0x1639460));
    fallback[0] = 0x3F800000;
    fallback[1] = 0;
    fallback[2] = 0;
    fallback[4] = 0;
    fallback[5] = 0x3F800000;
    fallback[6] = 0;
    fallback[8] = 0;
    fallback[9] = 0;
    fallback[10] = 0x3F800000;

    const float* pos = m_pMatrix ? &m_pMatrix->pos.x : &m_vecPos.x;
    auto* fallbackF = reinterpret_cast<float*>(fallback);
    fallbackF[12] = pos[0];
    fallbackF[13] = pos[1];
    fallbackF[14] = pos[2];
    return fallbackF;
}

bool __cdecl CPed::CollectNearbyEntityCallback(float* entity, float* sphere)
{
    const auto* ent = reinterpret_cast<const CEntity*>(entity);
    const uint32 type = (ent->m_nFlags2 >> 6) & 0xFu;
    if (type != 4 && type != 2)
        return true;

    const float* pos = ent->m_pMatrix ? &ent->m_pMatrix->pos.x : &ent->m_vecPos.x;
    const float dx = pos[0] - sphere[0];
    const float dy = pos[1] - sphere[1];
    const float dz = pos[2] - sphere[2];
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    using GetBoundRadiusFn = double(__thiscall*)(const CEntity*);
    const double boundRadius = reinterpret_cast<GetBoundRadiusFn>(ent->m_vftable[22])(ent);

    int32& count = Core::StaticRef<int32>(0x122F8C0);
    if (boundRadius + sphere[4] > dist)
    {
        auto* out = reinterpret_cast<float*>(reinterpret_cast<uint8*>(Core::Rebase(0x122F8D0)) + 32 * count++);
        out[4] = static_cast<float>(boundRadius);
        out[0] = pos[0];
        out[1] = pos[1];
        out[2] = pos[2];
        out[3] = pos[3];
    }
    return count < 64;
}
// --- Slice 3 ---
void CPed::InjectHooks_Slice3()
{
    Core::Hook::Install(0x8A2CF0, &CPed::UpdateVehicularWindExposure);
    Core::Hook::Install(0x8A3390, &CPed::IsValidForSomething8A3390);
    Core::Hook::Install(0x8A3770, &CPed::SetCoverPositions8A3770);
    Core::Hook::Install(0x8A3EC0, &CPed::Unk8A3EC0);
    Core::Hook::Install(0x8A5030, &CPed::Unk8A5030);
    Core::Hook::Install(0x8A55C0, &CPed::GetLocalBoundBox8A55C0);

    Core::Log::Info("CPed Slice 3 hooks installed (6 functions)");
}

// 0x8A2CF0
int32 __thiscall CPed::UpdateVehicularWindExposure()
{
    auto* self = reinterpret_cast<uint8*>(this);
    const int32 stateA = *reinterpret_cast<const int32*>(self + 0x450);
    const int32 stateB = *reinterpret_cast<const int32*>(self + 0x44C);
    if (stateA == 15 || stateB == 15)
    {
        m_fPedUnkE1C = 0.0f;
        return 15;
    }

    int32 result = 15;
    const float timeStep = Core::StaticRef<float>(0x11DDE7C);
    const float decayRate = Core::StaticRef<float>(0xF21FE8);
    m_fPedUnkE1C -= timeStep * decayRate;
    if (m_fPedUnkE1C < 0.0f)
    {
        m_fPedUnkE1C = 0.0f;
    }

    if ((m_nPedFlags2 & 4u) != 0 && m_pMyVehicle)
    {
        auto* vehBytes = reinterpret_cast<uint8*>(m_pMyVehicle);
        if (*reinterpret_cast<const int32*>(vehBytes + 0x1354) == 1)
        {
            void** vehVftable = *reinterpret_cast<void***>(m_pMyVehicle);
            using GetMoveSpeedFn = int32(__thiscall*)(CVehicle*, CVector*);
            CVector moveSpeed{};
            result = reinterpret_cast<GetMoveSpeedFn>(vehVftable[59])(m_pMyVehicle, &moveSpeed);
            const auto* outVec = reinterpret_cast<const CVector*>(result);
            float speed = std::sqrt(outVec->x * outVec->x + outVec->y * outVec->y + outVec->z * outVec->z);

            const float minSpeed = Core::StaticRef<float>(0xF21D74);
            const float maxSpeed = Core::StaticRef<float>(0xF21D78);
            if (speed > minSpeed)
            {
                if (maxSpeed <= speed)
                {
                    speed = maxSpeed;
                }
                m_fPedUnkE1C = (speed - minSpeed) / (maxSpeed - minSpeed);
            }
        }
    }

    const float exposure = m_fPedUnkE1C;
    if (exposure > 0.0f)
    {
        const CVector& camPos = Core::StaticRef<CVector>(0x11DC4E0);
        const CVector& pedPos = m_pMatrix->pos;
        result = static_cast<int32>(reinterpret_cast<uintptr_t>(&m_pMatrix->pos));

        const float dx = camPos.x - pedPos.x;
        const float dy = camPos.y - pedPos.y;
        const float dz = camPos.z - pedPos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        const float maxDist = Core::StaticRef<float>(0xF21FE4);
        if (dist > maxDist)
        {
            m_fPedUnkE1C = 0.0f;
            return result;
        }

        float distT = 0.0f;
        const float nearDist = Core::StaticRef<float>(0xF21D80);
        const float farDist = Core::StaticRef<float>(0xF21D84);
        if (dist > nearDist)
        {
            if (farDist <= dist)
            {
                dist = farDist;
            }
            distT = (dist - nearDist) / (farDist - nearDist);
        }

        const float nearScale = Core::StaticRef<float>(0xF21D88);
        const float farScale = Core::StaticRef<float>(0xF21D8C);
        m_fPedUnkE1C = (((farScale - nearScale) * distT) + nearScale) * exposure;
    }

    if (m_fPedUnkE1C < 0.0f)
    {
        m_fPedUnkE1C = 0.0f;
    }
    else if (m_fPedUnkE1C > 1.0f)
    {
        m_fPedUnkE1C = 1.0f;
    }

    return result;
}

// 0x8A3390
bool __thiscall CPed::IsValidForSomething8A3390()
{
    if ((m_nPedFlags2 & 0x2000u) != 0)
    {
        return false;
    }

    if (m_bIsDead && (m_nPedState == 1 || m_nPedState == 2))
    {
        return false;
    }

    if (!m_pPedIntelligence)
    {
        return false;
    }

    using IsPlayerFn = uint8(__thiscall*)(CPed*);
    if (reinterpret_cast<IsPlayerFn>(m_vftable[74])(this) != 0 && m_nUnkA80 == 1)
    {
        return false;
    }

    if (m_nUnk6C && *reinterpret_cast<const uint8*>(static_cast<uintptr_t>(m_nUnk6C) + 14) != 0)
    {
        return false;
    }

    return true;
}

// 0x8A3770
const float* __thiscall CPed::SetCoverPositions8A3770(const float* posA, const float* posB)
{
    auto* dstA = reinterpret_cast<float*>(&m_vecPedUnkE30);
    dstA[0] = posA[0];
    dstA[1] = posA[1];
    dstA[2] = posA[2];
    dstA[3] = posA[3];

    auto* dstB = reinterpret_cast<float*>(&m_vecPedUnkE40);
    dstB[0] = posB[0];
    dstB[1] = posB[1];
    dstB[2] = posB[2];
    dstB[3] = posB[3];

    m_nPedUnkE20 = 1;
    return posB;
}

// 0x8A3EC0
int32 __thiscall CPed::Unk8A3EC0()
{
    return Core::Hook::CallMethod<int32>(0x89D8F0, this, 1);
}

// 0x8A5030
bool __thiscall CPed::Unk8A5030(CVector* outVec, int32 /*arg*/)
{
    *outVec = CVector{ 0.0f, 0.0f, 0.0f };
    using IsPlayerFn = bool(__thiscall*)(CPed*);
    return reinterpret_cast<IsPlayerFn>(m_vftable[74])(this);
}

// 0x8A55C0
float* __thiscall CPed::GetLocalBoundBox8A55C0(float* outBox)
{
    const auto* unk7C4 = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(this) + 0x7C4);
    if (unk7C4)
    {
        const auto* sub = *reinterpret_cast<const uint8* const*>(unk7C4 + 4);
        if (sub)
        {
            const auto* boxData = *reinterpret_cast<const float* const*>(sub + 12);
            if (boxData)
            {
                outBox[0] = boxData[8];
                outBox[1] = boxData[9];
                outBox[2] = boxData[10];
                outBox[4] = boxData[4];
                outBox[5] = boxData[5];
                outBox[6] = boxData[6];
                return outBox;
            }
        }
    }

    float tmpBox[8]{};
    using GetBoundBoxFn = void(__thiscall*)(CPed*, float*);
    reinterpret_cast<GetBoundBoxFn>(m_vftable[27])(this, tmpBox);
    outBox[0] = tmpBox[0];
    outBox[1] = tmpBox[1];
    outBox[2] = tmpBox[2];
    outBox[4] = tmpBox[4];
    outBox[5] = tmpBox[5];
    outBox[6] = tmpBox[6];
    return outBox;
}
// --- Slice 4 ---
void CPed::InjectHooks_Slice4()
{
    Core::Hook::Install(0x8A6E00, &CPed::IsEntityWithinRange);
    Core::Hook::Install(0x8A7710, &CPed::SetVoiceGroup);
    Core::Hook::Install(0x8A7740, &CPed::InitVoiceByPedType);
    Core::Hook::Install(0x8AA480, &CPed::ClearWeaponObject);
    Core::Hook::Install(0x8AAC30, &CPed::ResetRagdollIfActiveSimple);
    Core::Hook::Install(0x8AAC50, &CPed::ResetRagdollIfActive);
    Core::Hook::Install(0x8AACD0, &CPed::UpdateRagdollTimeout);
    Core::Hook::Install(0x8AC500, &CPed::SetModelIndexDefault);
    Core::Hook::Install(0x8AC520, &CPed::ProcessRagdollOrPhysical);
    Core::Hook::Install(0x8AC9D0, &CPed::AttachWeaponOrProp);
}

// 0x8A6E00
bool __thiscall CPed::IsEntityWithinRange(const CEntity* entity, float range)
{
    return Core::Hook::CallMethod<bool>(0x89F390, this, &entity->GetPosition(), range);
}

// 0x8A7710
int32 __thiscall CPed::SetVoiceGroup(int32 voiceGroup)
{
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0xBB4) = voiceGroup;
    const int32 voiceId = Core::Hook::Call<int32>(0xB3C5A0, voiceGroup);
    return Core::Hook::CallMethod<int32>(0x7B7B00, m_voiceData, voiceId);
}

// 0x8A7740
int32 __thiscall CPed::InitVoiceByPedType()
{
    if (m_nPedUnk219)
        return SetVoiceGroup(0);

    const int32 pedType = *reinterpret_cast<const int32*>(static_cast<const uint8*>(m_pPedData) + 0x12C);
    if (pedType >= 0 && pedType <= 22)
        return SetVoiceGroup(pedType + 1);

    return pedType;
}

// 0x8AA480
int32 __thiscall CPed::ClearWeaponObject(bool bKeepUnk)
{
    if (Core::Hook::CallMethod<bool>(0x8A5700, this))
    {
        if (bKeepUnk)
            Core::Hook::Call<void>(0x9E3980, this, 0, 0, 1);
        else
            Core::Hook::Call<void>(0x9E30A0, this, 0, 0);

        if (m_nPedUnkE58 != -1 && m_nPedUnkE5C != -1)
            Core::Hook::Call<void>(0x9E3540, this, 0, m_nPedUnkE58, m_nPedUnkE5C, -1, 0, 0);
    }

    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x264) &= ~0x8000u;
    m_fPedUnkE50 = 0.0f;
    return Core::Hook::CallMethod<int32>(0x8A57E0, this, -1, -1);
}

// 0x8AAC30
int32 __thiscall CPed::ResetRagdollIfActiveSimple()
{
    void* ragdollInst = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x7C4);
    if (m_pPhysicsInst && m_pPhysicsInst == ragdollInst)
        return Core::Hook::CallMethod<int32>(0x8A6910, this, 0, 1, 0);

    return Core::Hook::CallMethod<int32>(0xA486E0, this);
}

// 0x8AAC50
int32 __thiscall CPed::ResetRagdollIfActive()
{
    void* ragdollInst = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(this) + 0x7C4);
    if (m_pPhysicsInst && m_pPhysicsInst == ragdollInst)
    {
        uint8* const* slots = reinterpret_cast<uint8* const*>(static_cast<const uint8*>(m_pPedIntelligence) + 0x44);
        uint8* task = nullptr;
        for (int32 i = 0; i < 5; ++i)
        {
            if (slots[i])
            {
                task = slots[i];
                break;
            }
        }

        if (task)
        {
            uint8* tail = nullptr;
            while (task)
            {
                tail = task;
                task = *reinterpret_cast<uint8**>(task + 8);
            }

            using GetTaskIdFn = int32(__thiscall*)(void*);
            void** vtbl = *reinterpret_cast<void***>(tail);
            if (reinterpret_cast<GetTaskIdFn>(vtbl[3])(tail) == 218)
            {
                Core::Hook::CallMethod<void>(0xABCFD0, this, 1);
                return Core::Hook::CallMethod<int32>(0xA476B0, this);
            }
        }

        Core::Hook::CallMethod<int32>(0x8A6910, this, 1, 1, 1);
    }

    return Core::Hook::CallMethod<int32>(0xA476B0, this);
}

// 0x8AACD0
uint32 __thiscall CPed::UpdateRagdollTimeout()
{
    if (m_nRagdollState == 6)
    {
        const uint32 elapsed = Core::StaticRef<uint32>(0x11DDE74) - *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x7CC);
        if (elapsed > 0x3E8u)
            return Core::Hook::CallMethod<uint32>(0x8A6910, this, 1, 1, 1);
        return elapsed;
    }

    return 0;
}

// 0x8AC500
int32 __thiscall CPed::SetModelIndexDefault(int32 modelIndex)
{
    return Core::Hook::CallMethod<int32>(0x8AAA80, this, modelIndex, -1, 1);
}

// 0x8AC520
int32 __thiscall CPed::ProcessRagdollOrPhysical()
{
    const uint8* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (unk6C && unk6C[14])
        return ResetRagdollIfActive();

    return Core::Hook::CallMethod<int32>(0xA476B0, this);
}

// 0x8AC9D0
bool __thiscall CPed::AttachWeaponOrProp(int32 arg)
{
    uint8 buf8[16]{};
    uint8 buf9[16]{};

    if (Core::Hook::CallMethod<bool>(0x89E1F0, this))
    {
        const int32 slot = Core::Hook::CallMethod<int32>(0x89E250, this);
        if (slot >= 0)
        {
            uint32* flags264 = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x264);
            if ((*flags264 & 0x8000u) == 0 && (m_nPedUnkE58 == -1 || m_nPedUnkE5C == -1))
            {
                const int32 v4 = Core::Hook::Call<int32>(0x9E3000, this, 0);
                if (v4 != -1)
                {
                    const int32 v5 = Core::Hook::Call<int32>(0x9E3020, this, 0);
                    Core::Hook::CallMethod<void>(0x8A57E0, this, v4, v5);
                }
            }

            if (arg != -1)
                Core::Hook::CallMethod<void>(0x8AB9C0, this, buf9, buf8);

            const int32 v6 = Core::Hook::CallMethod<int32>(0x8A5780, this, slot);
            if (Core::Hook::Call<uint8>(0x9E30F0, this) < 2u || Core::Hook::Call<int32>(0x9E3000, this, 0) != -1)
            {
                Core::Hook::Call<void>(0x9E3540, this, 0, slot, v6, arg, buf9, buf8);
                *flags264 |= 0x8000u;
                m_fPedUnkE50 = 0.0f;
            }
        }
    }

    return Core::Hook::CallMethod<bool>(0x8A5700, this);
}
// --- Slice 5 ---
// =========================================================================
// CPed Slice 5 (0x89C100 .. 0x89DC70)
// 41 functions in slice:
//   - 36 hooked below
//   - 5 skipped:
//     * 0x89C5A0: pure 5-byte thunk to 0x9E6A80
//     * 0x89C900, 0x89C980, 0x89C9D0: reference TlsIndex / NtCurrentTeb()->ThreadLocalStoragePointer
//     * 0x89D7C0: calls CRT __m128 atan2 (0xD123AC) with custom-register arguments dropped by decompiler
// =========================================================================

void CPed::InjectHooks_Slice5()
{
    Core::Hook::Install(0x89C100, &CPed::ResetPhysicsCollider89C100);
    Core::Hook::Install(0x89C200, &CPed::CollectEntities89C200);
    Core::Hook::Install(0x89C2C0, &CPed::UpdateUnk7C4Matrix89C2C0);
    Core::Hook::Install(0x89C350, &CPed::SetPedUnk7C0);
    Core::Hook::Install(0x89C370, &CPed::SetPedUnk7C4);
    Core::Hook::Install(0x89C3B0, &CPed::UpdateModelPedFlags89C3B0);
    Core::Hook::Install(0x89C460, &CPed::UpdateState89C460);
    Core::Hook::Install(0x89C520, &CPed::ResetSubObj89C520);
    Core::Hook::Install(0x89C630, &CPed::CanUseAnim89C630);
    Core::Hook::Install(0x89C780, &CPed::GetIntelligenceUnk3C);
    Core::Hook::Install(0x89C7A0, &CPed::IsWeaponAllowed89C7A0);
    Core::Hook::Install(0x89C8C0, &CPed::GetUnkBA4State);
    Core::Hook::Install(0x89CA40, &CPed::CheckCondition89CA40);
    Core::Hook::Install(0x89CC30, &CPed::CheckCondition89CC30);
    Core::Hook::Install(0x89CD90, &CPed::PlayAnim89CD90);
    Core::Hook::Install(0x89CE30, &CPed::PlayAngryAnim89CE30);
    Core::Hook::Install(0x89CF00, &CPed::PlayAnim445_89CF00);
    Core::Hook::Install(0x89D1C0, &CPed::SetTaskTimer89D1C0);
    Core::Hook::Install(0x89D200, &CPed::ClearZeroTasks89D200);
    Core::Hook::Install(0x89D270, &CPed::ApplyMovement89D270);
    Core::Hook::Install(0x89D360, &CPed::UpdateUnk137_89D360);
    Core::Hook::Install(0x89D3A0, &CPed::ProcessFall89D3A0);
    Core::Hook::Install(0x89D5C0, &CPed::SetFlags156_89D5C0);
    Core::Hook::Install(0x89D6C0, &CPed::ProcessSpeech89D6C0);
    Core::Hook::Install(0x89D950, &CPed::SetTask89D950);
    Core::Hook::Install(0x89D9C0, &CPed::SetTask89D9C0);
    Core::Hook::Install(0x89DA20, &CPed::SetTaskWithVehicleCheck89DA20);
    Core::Hook::Install(0x89DB00, &CPed::GetModelProp89DB00);
    Core::Hook::Install(0x89DB40, &CPed::UpdateAudio89DB40);
    Core::Hook::Install(0x89DC70, &CPed::ClearUnkD78_89DC70);
}

// 0x89C100
void __thiscall CPed::ResetPhysicsCollider89C100()
{
    auto* self = reinterpret_cast<uint32*>(this);
    uint32 fragInst = self[14];
    if (!fragInst)
        return;

    using GetTypeFn = int32(__thiscall*)(uint32);
    auto* fragVtbl = *reinterpret_cast<void***>(fragInst);
    if (reinterpret_cast<GetTypeFn>(fragVtbl[1])(fragInst) == 3)
        return;

    fragVtbl = *reinterpret_cast<void***>(self[14]);
    if (reinterpret_cast<GetTypeFn>(fragVtbl[1])(self[14]) >= 13)
        return;

    Core::Hook::CallMethod<uint32>(0x5171A0, reinterpret_cast<void*>(self[14]), self[8]);
    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (collider)
    {
        Core::Hook::CallMethod<void>(0x69C760, collider, self[14]);

        collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        using VoidVtblFn = void(__thiscall*)(void*);
        auto* colVtbl = *reinterpret_cast<void***>(collider);
        reinterpret_cast<VoidVtblFn>(colVtbl[3])(collider);

        collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        colVtbl = *reinterpret_cast<void***>(collider);
        reinterpret_cast<VoidVtblFn>(colVtbl[10])(collider);

        collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        Core::Hook::CallMethod<void>(0x5D35A0, collider, reinterpret_cast<uint8*>(collider) + 208);

        collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        if (*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(collider) + 24))
        {
            collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
            void* subCollider = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(collider) + 24);
            Core::Hook::CallMethod<void>(0x69AE50, subCollider, int8(0));
        }
    }

    uint32 flags = self[9];
    if ((flags & 2) != 0 && (flags & 0x1C) == 0 && (flags & 1) != 0)
    {
        using SelfVtblFn = int32(__thiscall*)(CPed*);
        auto* selfVtbl = *reinterpret_cast<void***>(this);
        if (reinterpret_cast<SelfVtblFn>(selfVtbl[41])(this))
            Core::Hook::CallMethod<uint32>(0x699F90, Core::StaticRef<void*>(0x1666884), self[14], int8(0), int8(0));
    }
    else
    {
        Core::Hook::CallMethod<uint32>(0x69A240, Core::StaticRef<void*>(0x1666884), self[14], int8(0));
    }
}

// 0x89C200
int32 __thiscall CPed::CollectEntities89C200(int32 outArray, int32* outCount, int32 maxCount, int32 arg5)
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    int32 result = Core::Hook::CallMethod<int32>(0x9E7140, this, outArray, outCount, maxCount, arg5);

    if ((m_nPedFlags2 & 4u) != 0)
    {
        uint32 veh = reinterpret_cast<uint32>(m_pMyVehicle);
        if (veh)
        {
            int32 count = *outCount;
            bool found = false;
            int32 idx = 0;
            if (count > 0)
            {
                auto* arr = reinterpret_cast<uint32*>(outArray);
                while (!found)
                {
                    found = (arr[idx++] == veh);
                    if (idx >= count)
                        break;
                }
            }
            if (!found && count < maxCount)
            {
                *reinterpret_cast<uint32*>(outArray + 4 * count) = veh;
                ++(*outCount);
                using VehCollectFn = int32(__thiscall*)(uint32, int32, int32*, int32, int32);
                auto* vehVtbl = *reinterpret_cast<void***>(veh);
                return reinterpret_cast<VehCollectFn>(vehVtbl[46])(veh, outArray, outCount, maxCount, 0);
            }
        }
    }

    auto* slots = reinterpret_cast<int32*>(selfBytes + 3432);
    for (int32 i = 0; i < 4; ++i)
    {
        if (*outCount >= maxCount)
            break;
        result = slots[i];
        if (result)
            *reinterpret_cast<int32*>(outArray + 4 * (*outCount)++) = result;
    }
    return result;
}

// 0x89C2C0
void __thiscall CPed::UpdateUnk7C4Matrix89C2C0(int8 arg)
{
    auto* self = reinterpret_cast<uint32*>(this);
    uint32 unk7C4 = reinterpret_cast<uint32>(m_pRagdollInst);
    if (!unk7C4)
        return;

    using Vtbl4Fn = void(__thiscall*)(uint32, uint32);
    if (arg)
    {
        Core::Hook::CallMethod<uint32>(0x5171A0, reinterpret_cast<void*>(unk7C4), self[8]);
        auto* vtbl = *reinterpret_cast<void***>(reinterpret_cast<uint32>(m_pRagdollInst));
        reinterpret_cast<Vtbl4Fn>(vtbl[4])(reinterpret_cast<uint32>(m_pRagdollInst), reinterpret_cast<uint32>(m_pRagdollInst) + 16);
        m_nRagdollAnimFlags &= ~0x4000000u;
    }
    else
    {
        if ((m_nRagdollAnimFlags & 0x4000000u) == 0)
        {
            auto* vtbl = *reinterpret_cast<void***>(unk7C4);
            reinterpret_cast<Vtbl4Fn>(vtbl[4])(unk7C4, unk7C4 + 16);
            m_nRagdollAnimFlags |= 0x4000000u;
        }
        Core::Hook::CallMethod<uint32>(0x5171A0, reinterpret_cast<void*>(reinterpret_cast<uint32>(m_pRagdollInst)), self[8]);
    }

    uint16 modelIdx = *reinterpret_cast<const uint16*>(reinterpret_cast<uint32>(m_pRagdollInst) + 8);
    if (modelIdx != 0xFFFF)
        Core::Hook::CallMethod<uint32>(0x6A3960, Core::StaticRef<void*>(0x1666880), static_cast<uint32>(modelIdx), uint32(0));
}

// 0x89C350
int32 __thiscall CPed::SetPedUnk7C0(void* ptr)
{
    m_pAnimBlendInst = ptr;
    *reinterpret_cast<CPed**>(reinterpret_cast<uint8*>(ptr) + 12) = this;
    return Core::Hook::CallMethod<int32>(0x686AD0, m_pAnimBlendInst, int32(64), int8(1));
}

// 0x89C370
int32 __thiscall CPed::SetPedUnk7C4(void* ptr)
{
    m_pRagdollInst = ptr;
    *reinterpret_cast<CPed**>(reinterpret_cast<uint8*>(ptr) + 12) = this;
    return Core::Hook::CallMethod<int32>(0x686AD0, ptr, int32(64), int8(1));
}

// 0x89C3B0
int32 __thiscall CPed::UpdateModelPedFlags89C3B0()
{
    const int32 modelIdx = *reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(this) + 46);
    const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[modelIdx]);
    const uint32 flags = *reinterpret_cast<const uint32*>(modelInfo + 288);

    if ((flags & 0x4000000u) != 0
        || (flags & 0x8000000u) != 0
        || (flags & 0x10000000u) != 0
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(29))
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(30))
        || modelIdx == Core::StaticRef<int32>(0x12303AC))
    {
        return Core::Hook::CallMethod<int32>(0x9C22C0, m_pPedIntelligence, int32(2));
    }

    if (Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(32)))
        return Core::Hook::CallMethod<int32>(0x9C22C0, m_pPedIntelligence, int32(4));

    return Core::Hook::CallMethod<int32>(0x9C22C0, m_pPedIntelligence, int32(3));
}

// 0x89C460
int32 __thiscall CPed::UpdateState89C460()
{
    const auto* selfBytes = reinterpret_cast<const uint8*>(this);
    if (m_nPedUnk219)
        return Core::Hook::CallMethod<int32>(0x9C22C0, m_pPedIntelligence, int32(-2));

    const int32 modelIdx = m_nModelIndex;
    const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[modelIdx]);

    if (selfBytes[2672] != 2
        || (*reinterpret_cast<const uint32*>(modelInfo + 288) & 0x4000000u) != 0
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(27))
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(28))
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(29))
        || Core::Hook::CallMethod<uint8>(0xBC60D0, this, int32(30)))
    {
        UpdateModelPedFlags89C3B0();
        const int32 state = *reinterpret_cast<const int32*>(
            reinterpret_cast<const uint8*>(m_pPedIntelligence) + 208);
        if (state == 2)
            return Core::Hook::CallMethod<int32>(0x9C41F0, m_pPedIntelligence, int32(14));
        if (state == 3)
            return Core::Hook::CallMethod<int32>(0x9C41F0, m_pPedIntelligence, int32(12));
        return state;
    }

    Core::Hook::CallMethod<int32>(0x9C41F0, m_pPedIntelligence, int32(10));
    return Core::Hook::CallMethod<int32>(0x9C22C0, m_pPedIntelligence, int32(-1));
}

// 0x89C520
int8 __thiscall CPed::ResetSubObj89C520()
{
    if (!static_cast<uint8>(Core::Hook::CallMethod<int32>(0x9E7CF0, this)))
        return 0;

    auto* self = reinterpret_cast<uint32*>(this);
    Core::Hook::Call<uint32>(0x9E3110, self, static_cast<int32>(self[135] + 128), self[135]);
    Core::Hook::Call<uint32>(0x8DB8B0, self, static_cast<int32>(self[135] + 128), self[135]);
    return 1;
}

// 0x89C630
int8 __cdecl CPed::CanUseAnim89C630(int32 a1, const int32* animId, const CPed* ped)
{
    if (a1 == 83)
    {
        const uint32 p5 = Core::Hook::CallMethod<uint32>(0x9C55E0, const_cast<CPed*>(ped), int32(5));
        const bool b5 = !p5
            || *reinterpret_cast<const float*>(p5 + 28) >= 0.0099999998f
            || !static_cast<uint8>(Core::Hook::CallMethod<uint32>(0xA5E400, reinterpret_cast<void*>(p5), uint32(0)));

        const uint32 p7 = Core::Hook::CallMethod<uint32>(0x9C55E0, const_cast<CPed*>(ped), int32(7));
        const bool b7 = !p7
            || *reinterpret_cast<const float*>(p7 + 28) >= 0.0099999998f
            || !static_cast<uint8>(Core::Hook::CallMethod<uint32>(0xA5E400, reinterpret_cast<void*>(p7), uint32(0)));

        if ((b5 && (*animId == 319 || *animId == 332))
            || (b7 && (*animId == 320 || *animId == 333)))
        {
            return 0;
        }
    }
    else if (a1 == 84)
    {
        const int32 modelIdx = *reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(ped) + 46);
        const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[modelIdx]);
        if ((*reinterpret_cast<const uint32*>(modelInfo + 148) & 0x20u) == 0)
        {
            switch (*animId)
            {
            case 313:
            case 314:
            case 317:
            case 318:
            case 321:
            case 322:
            case 327:
            case 328:
            case 334:
            case 335:
            case 339:
            case 340:
            case 375:
            case 376:
                return 0;
            default:
                break;
            }
        }
    }
    return 1;
}

// 0x89C780
int32 __thiscall CPed::GetIntelligenceUnk3C()
{
    const auto* intel = reinterpret_cast<const uint32*>(m_pMoveBlend);
    return Core::Hook::Call<int32>(0xA35050, intel[15]);
}

// 0x89C7A0
bool __thiscall CPed::IsWeaponAllowed89C7A0()
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    if (selfBytes[537] && *reinterpret_cast<const int32*>(selfBytes + 2960) > 3)
        return false;

    const uint32 slot = Core::Hook::CallMethod<uint32>(0x9A9D30, selfBytes + 688);
    if (!slot)
        return true;

    const uint32 weaponId = *reinterpret_cast<const uint32*>(slot + 24);
    const auto* weaponInfo = reinterpret_cast<const uint8*>(Core::Hook::Call<uint32>(0x924E80, weaponId));
    const uint32 weaponType = *reinterpret_cast<const uint32*>(weaponInfo + 8);
    if (!weaponType || weaponType == 6)
        return true;

    return (*reinterpret_cast<const uint32*>(weaponInfo + 32) & 0x8000u) != 0;
}

// 0x89C810
// UNHOOK_SAFE_SKIP: declared static __cdecl in Ped.h but x86 is __thiscall (reads ecx+0x78)
int8 __cdecl CPed::HasActiveTask89C810()
{
    return 0;
}

// 0x89C870
// UNHOOK_SAFE_SKIP: declared static __cdecl in Ped.h but x86 is __thiscall (reads ecx+0x78)
bool __cdecl CPed::CheckTaskFlag80000()
{
    return false;
}

// 0x89C890
// UNHOOK_SAFE_SKIP: declared static __cdecl in Ped.h but x86 is __thiscall (reads ecx+0x78)
bool __cdecl CPed::CheckTaskFlag20000000()
{
    return false;
}

// 0x89C8C0
int32 __thiscall CPed::GetUnkBA4State()
{
    const uint32 handle = reinterpret_cast<const uint32*>(this)[745];
    uint32 ptr = Core::Hook::Call<uint32>(0xC54AD0, handle);
    if (!ptr)
        ptr = Core::Hook::Call<uint32>(0xC54F10, handle);

    const int32 val = *reinterpret_cast<const int32*>(ptr + 4);
    return (val == -1) ? 49 : val;
}

// 0x89CA40
bool __thiscall CPed::CheckCondition89CA40(int32 a2)
{
    if (!a2)
        return true;

    auto* self = reinterpret_cast<uint32*>(this);
    if ((m_nRagdollAnimFlags & 0x300000u) != 0)
        return true;

    const uint32 v3 = self[14];
    if (v3 && v3 == reinterpret_cast<uint32>(m_pRagdollInst))
        return true;

    if (static_cast<uint8>(GetIntelligenceUnk3C())
        || static_cast<int32>(m_nMoveState) > 2
        || (m_pMyVehicle && static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x9C5D60, m_pMyVehicle, this))))
    {
        return true;
    }

    if (m_pMyVehicle
        && *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pMyVehicle) + 4948) == 1
        && static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x9C5D30, m_pMyVehicle, this)))
    {
        return true;
    }

    auto* taskMgr = reinterpret_cast<uint8*>(m_pPedIntelligence) + 0x2E0;
    if (static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(1900), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(1040), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(740), int8(0)))
        || (static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(734), int8(0)))
            && Core::Hook::CallMethod<uint32>(0x9A48C0, taskMgr, int32(734), int32(5)) >= 0xFu)
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(738), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(2117), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(254), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(1600), int8(0))))
    {
        return true;
    }

    const uint32 mat = *reinterpret_cast<const uint32*>(a2 + 32);
    int32 v7[4]{};
    int32 v8[4]{};
    v8[0] = -1072247836;
    v8[1] = 0;
    v8[2] = -1077342245;
    v7[0] = 1075235812;
    v7[1] = 0;
    v7[2] = 1070141403;
    return static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x950430, m_pedSubObjBC0, static_cast<int32>(mat + 48), v8, v7)) == 0;
}

// 0x89CC30
int8 __thiscall CPed::CheckCondition89CC30()
{
    auto* self = reinterpret_cast<uint32*>(this);
    if ((self[168] & 0x8000u) != 0)
        return 1;

    const uint32 v3 = self[14];
    if (v3 && v3 == reinterpret_cast<uint32>(m_pRagdollInst))
        return 1;

    if (static_cast<uint8>(GetIntelligenceUnk3C())
        || static_cast<int32>(m_nMoveState) > 2
        || (m_pMyVehicle && static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x9C5D60, m_pMyVehicle, this))))
    {
        return 1;
    }

    if (m_pMyVehicle
        && *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pMyVehicle) + 4948) == 1
        && static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x9C5D30, m_pMyVehicle, this)))
    {
        return 1;
    }

    auto* taskMgr = reinterpret_cast<uint8*>(m_pPedIntelligence) + 0x2E0;
    if (static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(1900), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(1040), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(740), int8(0)))
        || (static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(734), int8(0)))
            && Core::Hook::CallMethod<uint32>(0x9A48C0, taskMgr, int32(734), int32(5)) >= 0xFu)
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(738), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(2117), int8(0)))
        || static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(254), int8(0))))
    {
        return 1;
    }

    const int32 idx = static_cast<int32>(Core::Hook::CallMethod<uint32>(0x9A6AB0, taskMgr, uint32(0)));
    if (idx != -1)
    {
        const uint32 info = Core::Hook::Call<uint32>(0x8F4570, idx);
        if (info && (*reinterpret_cast<const uint32*>(info + 80) & 0x100u) != 0)
            return 1;
    }
    return 0;
}

// 0x89CD90
void __thiscall CPed::PlayAnim89CD90(int32 a2, uint32 a3, int32 a4, int32 a5)
{
    (void)a4;
    (void)a5;
    const int32 dict = static_cast<int32>(
        Core::Hook::CallMethod<uint32>(0x92F590, Core::StaticRef<void*>(0x1548F88), reinterpret_cast<const uint32*>(this)[747]));
    if (dict >= 0 && a2)
    {
        if (Core::Hook::Call<uint32>(0x909240, dict, a2))
        {
            const float blend = Core::StaticRef<float>(0xF21FE0);
            const int32 anim = Core::Hook::CallMethod<int32>(0x9EFF90, reinterpret_cast<void* const*>(this)[30], dict, a2, int32(294912), int32(6), blend);
            if (anim && a3)
            {
                const double duration = Core::Hook::CallMethod<double>(0xA4C450, reinterpret_cast<void*>(anim));
                *reinterpret_cast<float*>(anim + 84) = static_cast<float>(duration * 1000.0 / static_cast<double>(a3));
            }
        }
    }
}

// 0x89CE30
void __thiscall CPed::PlayAngryAnim89CE30(uint32 a2)
{
    const auto* selfBytes = reinterpret_cast<const uint8*>(this);
    if (m_bIsDead)
    {
        if (m_nPedState == 1 || m_nPedState == 2)
            return;
    }

    if (m_pPedIntelligence)
    {
        if (a2 >= 2)
            Core::Hook::Call<int32>(0xD0D21A);

        const int32 dict = static_cast<int32>(
            Core::Hook::Call<uint32>(0x908A00, *reinterpret_cast<const uint32*>(selfBytes + 2988)));

        auto& initFlags = Core::StaticRef<uint32>(0x1230118);
        auto& angryA = Core::StaticRef<int32>(0x1230114);
        auto& angryB = Core::StaticRef<int32>(0x1230110);
        if ((initFlags & 1u) == 0)
        {
            initFlags |= 1u;
            angryA = Core::Hook::Call<int32>(0x7BDBF0, "angry_a");
        }
        if ((initFlags & 2u) == 0)
        {
            initFlags |= 2u;
            angryB = Core::Hook::Call<int32>(0x7BDBF0, "angry_b");
        }

        const int32 hash = angryA;
        if (Core::Hook::Call<uint32>(0x909240, dict, hash))
            Core::Hook::CallMethod<int32>(0x9EFF90, reinterpret_cast<void* const*>(this)[30], dict, hash, int32(294912), int32(6), 30.0f);
    }
}

// 0x89CF00
void __thiscall CPed::PlayAnim445_89CF00(uint32 a2)
{
    const auto* selfBytes = reinterpret_cast<const uint8*>(this);
    if (m_bIsDead)
    {
        if (m_nPedState == 1 || m_nPedState == 2)
            return;
    }

    if (m_pPedIntelligence)
    {
        if (a2 > 2)
            Core::Hook::Call<int32>(0xD0D21A);

        const uint32 voice = *reinterpret_cast<const uint32*>(selfBytes + 2988);
        if (Core::Hook::Call<uint32>(0x909430, voice, int32(445)))
            Core::Hook::CallMethod<int32>(0x9EFE40, reinterpret_cast<void* const*>(this)[30], static_cast<int32>(voice), int32(445), int32(294912), int32(6), 30.0f, int32(-1));
    }
}

// 0x89D020
// UNHOOK_SAFE_SKIP: declared static __stdcall in Ped.h but x86 is __thiscall thunk (adjusts ecx += 0xB44 and jmps to 0xB3C710)
int32 __stdcall CPed::Helper89D020(int32 a1)
{
    return static_cast<int32>(Core::Hook::CallStdcall<uint32>(0xB3C710, a1));
}

// 0x89D030
// UNHOOK_SAFE_SKIP: declared static __stdcall in Ped.h but x86 is __thiscall thunk (loads ecx = m_pMoveBlend and jmps to 0xBC9870)
int32 __stdcall CPed::Helper89D030(int8 a1)
{
    return static_cast<int32>(Core::Hook::CallStdcall<uint32>(0xBC9870, a1));
}

// 0x89D1C0
int32 __thiscall CPed::SetTaskTimer89D1C0(float a2)
{
    void* subObj = reinterpret_cast<void*>(reinterpret_cast<const uint32*>(this)[30]);
    int32 result = Core::Hook::CallMethod<int32>(0x9EBF60, subObj);
    while (result)
    {
        if (*reinterpret_cast<const int32*>(result + 8) > 0)
        {
            *reinterpret_cast<uint32*>(result + 4) |= 0x4000u;
            Core::Hook::CallMethod<uint32>(0xA4DBC0, reinterpret_cast<void*>(result), a2);
        }
        result = Core::Hook::CallMethod<int32>(0x9EBFB0, subObj);
    }
    return result;
}

// 0x89D200
int32 __thiscall CPed::ClearZeroTasks89D200()
{
    void* subObj = reinterpret_cast<void*>(reinterpret_cast<const uint32*>(this)[30]);
    int32 result = Core::Hook::CallMethod<int32>(0x9EBF60, subObj);
    for (int32 i = result; result; i = result)
    {
        if (!*reinterpret_cast<const int32*>(i + 8))
        {
            *reinterpret_cast<uint32*>(i + 4) |= 0x4000u;
            Core::Hook::CallMethod<uint32>(0xA4DBC0, reinterpret_cast<void*>(i), 1000.0f);
            Core::Hook::CallMethod<uint32>(0xA4C470, reinterpret_cast<void*>(i), 0.0f);
            Core::Hook::CallMethod<uint32>(0x9EF450, subObj, i);
        }
        result = Core::Hook::CallMethod<int32>(0x9EBFB0, subObj);
    }
    return result;
}

// 0x89D260
// UNHOOK_SAFE_SKIP: declared static __cdecl in Ped.h, but 0x89D260 is __thiscall taking CPed* in ecx ([ecx+0x78] passed as ecx to 0x9ED080)
int32 __cdecl CPed::Helper89D260()
{
    return static_cast<int32>(Core::Hook::CallStdcall<uint32>(0x9ED080, uint32(0), int32(16), int8(1)));
}

// 0x89D270
void __thiscall CPed::ApplyMovement89D270(float a2)
{
    const auto* selfBytes = reinterpret_cast<const uint8*>(this);
    void* intel = m_pMoveBlend;

    if ((m_nPedFlags2 & 1u) != 0 || (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(intel) + 80) & 0x40u) != 0)
    {
        if ((selfBytes[482] & 0xFu) < 2)
        {
            const uint32 v3 = *reinterpret_cast<const uint32*>(selfBytes + 56);
            if (!v3 || v3 != *reinterpret_cast<const uint32*>(selfBytes + 1988))
            {
                float v6[4]{};
                float v7[4]{};
                using IntelGetVecFn = void(__thiscall*)(void*, float*, float);
                auto* intelVtbl = *reinterpret_cast<void***>(intel);
                reinterpret_cast<IntelGetVecFn>(intelVtbl[4])(intel, v6, a2);

                const int32 obj = Core::Hook::CallMethod<int32>(0x9E86F0, this);
                using GetScaleFn = double(__thiscall*)(int32);
                auto* objVtbl = *reinterpret_cast<void***>(obj);
                const float scale = static_cast<float>(reinterpret_cast<GetScaleFn>(objVtbl[9])(obj));

                const float invA2 = 1.0f / a2;
                v7[0] = (v6[0] * scale) * invA2;
                v7[1] = invA2 * (v6[1] * scale);
                v7[2] = (v6[2] * scale) * invA2;
                Core::Hook::CallMethod<uint32>(0xA47C30, this, v7);
            }
        }
    }
}

// 0x89D360
int32 __thiscall CPed::UpdateUnk137_89D360()
{
    void* subObj = reinterpret_cast<uint8*>(m_pPedIntelligence) + 132;
    Core::Hook::CallMethod<int32>(0xAA7D70, subObj);
    Core::Hook::CallMethod<uint32>(0xAA7E00, subObj, int8(1));
    return Core::Hook::CallMethod<int32>(0xAA7DA0, subObj);
}

// 0x89D3A0
int8 __thiscall CPed::ProcessFall89D3A0(int8 a2)
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    int8 v3 = 1;

    if ((selfBytes[36] & 0x1Cu) != 0)
        return 1;
    if (m_nRagdollState > 2)
        return 0;

    const uint32 unk56 = *reinterpret_cast<const uint32*>(selfBytes + 56);
    const uint16 unk56Word8 = *reinterpret_cast<const uint16*>(unk56 + 8);
    if (unk56Word8 == 0xFFFF)
        return 1;

    void* intel = m_pMoveBlend;
    using IntelCheckFn = uint8(__thiscall*)(void*);
    auto* intelVtbl = *reinterpret_cast<void***>(intel);

    if (*reinterpret_cast<const uint32*>(selfBytes + 2752)
        || *reinterpret_cast<const uint32*>(selfBytes + 324) == 1
        || static_cast<uint8>(Core::Hook::CallMethod<int32>(0x4D9080, selfBytes + 224))
        || reinterpret_cast<IntelCheckFn>(intelVtbl[6])(intel)
        || (m_nPedFlags2 & 1u) == 0
        || a2
        || static_cast<uint8>(Core::Hook::CallMethod<int32>(0x454450, this)))
    {
        v3 = 0;
    }

    const uint32 unk1666880 = Core::StaticRef<uint32>(0x1666880);
    const uint32 tblBase = *reinterpret_cast<const uint32*>(unk1666880 + 112);
    const uint32 entryFlags = *reinterpret_cast<const uint32*>(tblBase + 8 * static_cast<uint32>(unk56Word8) + 4);

    bool doImpulse = false;
    if ((entryFlags & 3u) == 1)
    {
        if (v3)
            return v3;
        Core::Hook::CallMethod<int32>(0xA47660, this);
        doImpulse = true;
    }
    else
    {
        const int32 col = Core::Hook::CallMethod<int32>(0x9E7080, this);
        if (col && *reinterpret_cast<const uint32*>(col + 24)
            && static_cast<uint8>(Core::Hook::CallMethod<int32>(0x4E42C0, *reinterpret_cast<void**>(col + 24))))
        {
            if (v3)
            {
                v3 = 0;
                return v3;
            }
            Core::Hook::CallMethod<uint32>(0x69AE50, *reinterpret_cast<void**>(col + 24), int8(1));
            v3 = 0;
            doImpulse = true;
        }
        else
        {
            return 0;
        }
    }

    if (doImpulse)
    {
        void* taskMgr = reinterpret_cast<uint8*>(m_pPedIntelligence) + 736;
        if ((m_nPedFlags2 & 1u) == 0
            && !static_cast<uint8>(Core::Hook::CallMethod<int32>(0x426A90, this))
            && !static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(254), int8(0)))
            && !static_cast<uint8>(Core::Hook::CallMethod<uint32>(0x426910, taskMgr, int32(288), int8(0))))
        {
            const double angle = Core::Hook::Call<double>(0x438960, -3.1415927f, 3.1415927f);
            const float sinVal = static_cast<float>(std::sin(angle));
            const float cosVal = static_cast<float>(std::cos(angle));

            const int32 obj = Core::Hook::CallMethod<int32>(0x9E86F0, this);
            using GetScaleFn = double(__thiscall*)(int32);
            auto* objVtbl = *reinterpret_cast<void***>(obj);
            const double v10 = reinterpret_cast<GetScaleFn>(objVtbl[9])(obj) * 0.2;
            const float v11 = static_cast<float>(v10);

            float vec[4]{};
            vec[0] = (-0.0f - sinVal) * v11;
            vec[1] = cosVal * v11;
            vec[2] = static_cast<float>(v10 * 0.5);
            Core::Hook::CallMethod<uint32>(0xA47E40, this, vec);
        }
    }
    return v3;
}

// 0x89D5C0
int32 __thiscall CPed::SetFlags156_89D5C0(int8 a2, int32 a3)
{
    auto* self = reinterpret_cast<uint32*>(this);
    const uint32 flags = m_nSpeechAnimFlags;
    uint32 newFlags = 0;

    if (a2)
    {
        if ((flags & 0x300u) != 0)
        {
            const uint32 result = flags | 0x100u;
            m_nSpeechAnimFlags = result;
            return static_cast<int32>(result);
        }
        newFlags = flags | 0x100u;
    }
    else
    {
        if ((flags & 0xC00u) != 0)
        {
            const uint32 result = flags | 0x400u;
            m_nSpeechAnimFlags = result;
            return static_cast<int32>(result);
        }
        newFlags = flags | 0x400u;
    }

    m_nSpeechAnimFlags = newFlags;
    self[83] = static_cast<uint32>(a3);
    if (a2)
        return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(0), uint32(0)));
    else
        return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(1), uint32(0)));
}

// 0x89D6C0
int32 __thiscall CPed::ProcessSpeech89D6C0()
{
    auto* self = reinterpret_cast<uint32*>(this);
    void* subObj = reinterpret_cast<void*>(self[30]);
    int32 result = Core::Hook::CallMethod<int32>(0x9EBF60, subObj);
    if (!result)
        return result;

    DecayAudioTimers();

    int32 v4 = 0;
    int32 v5 = 0;
    uint8 v6[4]{};
    uint8 v7[4]{};
    Core::Hook::CallMethod<uint32>(
        0x9EC1C0,
        subObj,
        uint32(0),
        int32(4),
        reinterpret_cast<void*>(Core::Rebase(0x1000000)),
        int8(1),
        &v4,
        v7,
        &v5,
        v6);

    result = v4;
    if (v4)
    {
        if ((*reinterpret_cast<const uint32*>(v4 + 4) & 0x200000u) != 0
            || (*reinterpret_cast<const uint32*>(v4 + 4) & 0x1000000u) == 0)
        {
            result = v5;
            v4 = v5;
        }
        if (result && (*reinterpret_cast<const uint8*>(result + 7) & 1u) != 0)
        {
            const uint32 v3 = *reinterpret_cast<const uint32*>(result + 116);
            if ((v3 & 0x20u) != 0)
            {
                Core::Hook::CallMethod<uint32>(0x94E890, m_pedSubObjBC0, self + 680);
                result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(0), uint32(0)));
            }
            if ((v3 & 0x80u) != 0)
                result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(2), uint32(0)));
            if ((v3 & 0x40u) != 0)
            {
                Core::Hook::CallMethod<uint32>(0x94E890, m_pedSubObjBC0, self + 680);
                result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(1), uint32(0)));
            }
            if ((v3 & 0x100u) != 0)
                return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xAB06E0, m_pedSubObj3D0, uint32(3), uint32(0)));
        }
    }
    return result;
}

// 0x89D950
int32 __thiscall CPed::SetTask89D950(int32 a2, int16 a3, int32 a4, int32 a5, int32 a6, int32 a7)
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int32*>(selfBytes + 3416) = a6;
    *reinterpret_cast<int32*>(selfBytes + 3420) = a7;
    const int32 result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49900, this, a2, a3, a4, a5, uint32(0)));
    if ((*reinterpret_cast<const uint16*>(selfBytes + 482) & 0x2000u) != 0)
    {
        *reinterpret_cast<uint32*>(selfBytes + 36) |= 1u;
        return static_cast<int32>(Core::Hook::CallMethod<uint32>(0x686AD0, m_pAnimBlendInst, int32(8), int8(1)));
    }
    return result;
}

// 0x89D9C0
int32 __thiscall CPed::SetTask89D9C0(int32 a2, int32 a3, int32 a4, int32 a5)
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int32*>(selfBytes + 3416) = a4;
    *reinterpret_cast<int32*>(selfBytes + 3420) = a5;
    const int32 result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49A60, this, a2, a3, uint32(0)));
    if ((*reinterpret_cast<const uint16*>(selfBytes + 482) & 0x2000u) != 0)
    {
        *reinterpret_cast<uint32*>(selfBytes + 36) |= 1u;
        return static_cast<int32>(Core::Hook::CallMethod<uint32>(0x686AD0, m_pAnimBlendInst, int32(8), int8(1)));
    }
    return result;
}

// 0x89DA20
int32 __thiscall CPed::SetTaskWithVehicleCheck89DA20(int32 a2, int32 a3, int32 a4, int32 a5)
{
    auto* self = reinterpret_cast<uint32*>(this);
    self[856] = static_cast<uint32>(a4);
    if (a5 >= 0)
        self[857] = static_cast<uint32>(a5);

    if ((*reinterpret_cast<const uint32*>(a2 + 40) & 0x3C0u) == 0x80u)
    {
        if (*reinterpret_cast<const uint32*>(a2 + 4944) == 1)
        {
            if ((self[153] & 6u) != 2u)
                return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49900, this, a2, int16(-1), a3 | 0x1800, uint32(0), uint32(0)));
            return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49900, this, a2, int16(-1), a3 | 0x800, uint32(0), uint32(0)));
        }

        const int32 v7 = *reinterpret_cast<const int32*>(a2 + 4948);
        const int32 modelIdx = *reinterpret_cast<const int16*>(a2 + 46);
        const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[modelIdx]);
        if ((v7 == 4 && a4 != 1 && a4 != 3)
            || ((*reinterpret_cast<const uint32*>(modelInfo + 148) & 0x20u) != 0 && a4 != 1 && a4 != 3)
            || v7 == 2)
        {
            return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49900, this, a2, int16(-1), a3 | 0x800, uint32(0), uint32(0)));
        }
    }

    return static_cast<int32>(Core::Hook::CallMethod<uint32>(0xA49900, this, a2, int16(-1), a3, uint32(0), uint32(0)));
}

// 0x89DB00
int32 __thiscall CPed::GetModelProp89DB00(uint8 a2)
{
    const auto* modelInfo = reinterpret_cast<const uint8*>(CModelInfo::ms_modelInfoPtrs[m_nModelIndex]);
    const uint8 prop = static_cast<uint8>(Core::Hook::Call<uint32>(0x8DA710, this, static_cast<uint32>(a2)));
    return static_cast<int32>(Core::Hook::CallMethod<uint32>(
        0x8DBFC0,
        reinterpret_cast<void*>(Core::Rebase(0x1350404)),
        *reinterpret_cast<const uint32*>(modelInfo + 60),
        static_cast<uint32>(a2),
        static_cast<uint32>(prop)));
}

// 0x89DB40
void __thiscall CPed::UpdateAudio89DB40()
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    if ((selfBytes[244] & 0x20u) == 0)
    {
        using AudioVtblFn = void(__thiscall*)(uint8*);
        auto* audioObj = selfBytes + 3008;
        auto* audioVtbl = *reinterpret_cast<void***>(audioObj);
        reinterpret_cast<AudioVtblFn>(audioVtbl[1])(audioObj);

        Core::Hook::CallMethod<int32>(0x9E1190, this);
        m_nRagdollAnimFlags |= 0x1000000u;
    }

    if (Core::StaticRef<uint8>(0xF0C511) && !Core::StaticRef<uint8>(0x10D0189))
    {
        const auto* mat = *reinterpret_cast<const float* const*>(selfBytes + 32);
        const float px = mat[12];
        const float py = mat[13];
        const float pz = mat[14];

        const float rx = Core::StaticRef<float>(0x11DC4E0);
        const float ry = Core::StaticRef<float>(0x11DC4E4);
        const float rz = Core::StaticRef<float>(0x11DC4E8);

        float distSq = ((pz - rz) * (pz - rz)) + ((py - ry) * (py - ry)) + ((px - rx) * (px - rx));
        const int32 player = Core::Hook::Call<int32>(0x817F40);
        if (player)
        {
            const auto* pMat = *reinterpret_cast<const float* const*>(player + 32);
            const float dx = px - pMat[12];
            const float dy = py - pMat[13];
            const float dz = pz - pMat[14];
            distSq = (dz * dz) + (dy * dy) + (dx * dx);
        }

        if (Core::Hook::Call<double>(0x799240) > static_cast<double>(distSq))
            Core::Hook::CallMethod<int32>(0x94F5D0, selfBytes + 3008);
    }
}

// 0x89DC70
int32 __thiscall CPed::ClearUnkD78_89DC70()
{
    auto* selfBytes = reinterpret_cast<uint8*>(this);
    int32 result = 0;
    void* unkD78 = *reinterpret_cast<void**>(selfBytes + 3448);
    if (unkD78)
    {
        result = static_cast<int32>(Core::Hook::CallMethod<uint32>(0xC68930, unkD78, this));
        *reinterpret_cast<uint32*>(selfBytes + 3448) = 0;
        *reinterpret_cast<uint64*>(selfBytes + 3460) = 0ULL;
        *reinterpret_cast<uint32*>(selfBytes + 3456) = 0;
    }
    return result;
}
// --- Slice 6 ---
void CPed::InjectHooks_Slice6()
{
    Core::Hook::Install(0x89DCE0, &CPed::SyncVehicleFlags);
    Core::Hook::Install(0x89DD20, &CPed::GetDominantAxisDir);
    Core::Hook::Install(0x89DD80, &CPed::CallUnkA66ED0);
    Core::Hook::Install(0x89DD90, &CPed::CountNearbyPassengers);
    Core::Hook::Install(0x89DED0, &CPed::RegisterPedGridEntry);
    Core::Hook::Install(0x89DF30, &CPed::UnregisterPedGridEntry);
    Core::Hook::Install(0x89DF90, &CPed::AdvancePedGridTick);
    Core::Hook::Install(0x89DFB0, &CPed::CheckVehicleUnk9C5D60);
    Core::Hook::Install(0x89E1F0, &CPed::CanUseUnkE1F0);
    Core::Hook::Install(0x89E250, &CPed::GetUnk9E27D0Minus1);
    Core::Hook::Install(0x89E300, &CPed::GetVehicleSeatOrTypeState);
    Core::Hook::Install(0x89E3E0, &CPed::UpdateVehicleSeatState);
    Core::Hook::Install(0x89E660, &CPed::AddToUnkB94AndCall);
    Core::Hook::Install(0x89EBF0, &CPed::ShutdownPedSubsystems);
    Core::Hook::Install(0x89EC20, &CPed::SetUnkA70State);
    Core::Hook::Install(0x89EFE0, &CPed::OnRagdollModelCheck);
    Core::Hook::Install(0x89F050, &CPed::UpdateRagdollDistanceCheck);
    Core::Hook::Install(0x89F530, &CPed::FindNearbyCoverPoint);
    Core::Hook::Install(0x89F7C0, &CPed::HasLineOfSightToEntity);
    Core::Hook::Install(0x89F9C0, &CPed::ProcessPedSpeechAndState);
    Core::Hook::Install(0x89FE30, &CPed::UpdatePedFlagsAndAudio);
    Core::Hook::Install(0x89FFB0, &CPed::UpdatePedMoodAudio);
    Core::Hook::Install(0x8A02D0, &CPed::ProcessBoneAttachmentEvents);
    Core::Hook::Install(0x8A0540, &CPed::ProcessAnimEventTrack);
    Core::Hook::Install(0x8A0680, &CPed::TriggerShockedSpeech);
    Core::Hook::Install(0x8A0720, &CPed::UpdateAnimEvents);
    Core::Hook::Install(0x8A07E0, &CPed::SetRagdollUnkB90);
    Core::Hook::Install(0x8A0850, &CPed::UpdateRagdollUnkBC9B00);
    Core::Hook::Install(0x8A08A0, &CPed::UpdateMovementAndSpeech);
}

int32 __thiscall CPed::SyncVehicleFlags()
{
    if ((m_nPedFlags2 & 4) == 0 || !m_pMyVehicle)
        return Core::Hook::CallMethod<int32>(0xA4AF20, this);

    auto* veh = reinterpret_cast<uint32*>(m_pMyVehicle);
    m_nFlags ^= (m_nFlags ^ veh[9]) & 0x20000000;
    const int32 result = static_cast<int32>((m_nPhysFlags118 ^ veh[70]) & 0x80000);
    m_nPhysFlags118 ^= result;
    return result;
}

int32 __stdcall CPed::GetDominantAxisDir(const float* mat)
{
    if (std::fabs(mat[2]) <= std::fabs(mat[6]))
        return mat[6] >= 0.0f ? 2 : 0;
    return mat[2] >= 0.0f ? 3 : 1;
}

int32 __thiscall CPed::CallUnkA66ED0()
{
    return Core::Hook::Call<int32>(0xA66ED0, static_cast<void*>(this));
}

int32 __thiscall CPed::CountNearbyPassengers(float radius)
{
    uint8** slots = reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(m_pPedIntelligence) + 360);
    const float radiusSq = radius * radius;
    const auto* selfPos = reinterpret_cast<const float*>(m_pMatrix);
    int32 count = 0;

    for (int32 i = 0; i < 16; ++i)
    {
        uint8* ped = slots[i];
        if (!ped)
            continue;

        const auto* otherPos = *reinterpret_cast<const float* const*>(ped + 0x20);
        const float dz = otherPos[14] - selfPos[14];
        const float dy = otherPos[13] - selfPos[13];
        const float dx = otherPos[12] - selfPos[12];
        const float distSq = dz * dz + dy * dy + dx * dx;

        if (radiusSq > distSq)
        {
            uint8* pedTaskMgr = *reinterpret_cast<uint8**>(ped + 548) + 736;
            if (Core::Hook::CallMethod<CPed*>(0x9A69E0, pedTaskMgr) == this
                && Core::Hook::CallMethod<uint8>(0x426910, pedTaskMgr, 734, int8(0)) != 0
                && Core::Hook::CallMethod<void*>(0x9A4850, pedTaskMgr, 734, 5) == m_pMyVehicle)
            {
                ++count;
            }
        }
    }

    return count;
}

int32 __cdecl CPed::RegisterPedGridEntry(uint8* entry)
{
    const int32 bucket = *reinterpret_cast<int32*>(entry + 4);
    auto** table = reinterpret_cast<uint8**>(Core::Rebase(0x122E208)) + 120 * bucket;

    int32 slot = 0;
    while (table[slot])
    {
        if (++slot >= 120)
            return slot;
    }

    table[slot] = entry;
    entry[8] = 0;

    const auto* bucketInfo = reinterpret_cast<const int32* const*>(Core::Rebase(0x12300E0))[bucket];
    const int32 divisor = bucketInfo[1];
    *reinterpret_cast<int32*>(entry) = slot % divisor;
    const int32 result = slot / divisor;
    ++reinterpret_cast<int32*>(Core::Rebase(0x122F88C))[bucket];
    return result;
}

int32* __cdecl CPed::UnregisterPedGridEntry(uint8* entry)
{
    const int32 bucket = *reinterpret_cast<int32*>(entry + 4);
    *reinterpret_cast<int32*>(entry) = -1;
    auto** table = reinterpret_cast<uint8**>(Core::Rebase(0x122E208)) + 120 * bucket;

    for (int32 i = 0; i < 120; ++i)
    {
        if (table[i] == entry)
        {
            table[i] = nullptr;
            --reinterpret_cast<int32*>(Core::Rebase(0x122F88C))[bucket];
            return reinterpret_cast<int32*>(bucket);
        }
    }

    return reinterpret_cast<int32*>(&table[120]);
}

int32 __cdecl CPed::AdvancePedGridTick()
{
    int32& tick = Core::StaticRef<int32>(0x122F8C8);
    const int32 result = ++tick;
    if (tick >= 120)
        tick = 0;
    return result;
}

int8 __thiscall CPed::CheckVehicleUnk9C5D60()
{
    if ((m_nPedFlags2 & 4) != 0 && m_pMyVehicle)
        return static_cast<int8>(Core::Hook::CallMethod<int32>(0x9C5D60, m_pMyVehicle, this));
    return 0;
}

// UNHOOK_SAFE_SKIP: declared static __stdcall in Ped.h, but 0x89E010 is __thiscall taking CPed* in ecx (passes &m_pedSpeechAudio in ecx to 0xA5AA70)
bool __stdcall CPed::TryTriggerPedAction(int32 arg1, float chance, int8 arg3, int8 arg4)
{
    return chance > (static_cast<float>(Core::Hook::Call<int32>(0xD0D21A)) * 0.000030518509f)
        && Core::Hook::CallStdcall<uint8>(0xA5AA70, arg1, arg3, arg4, 0, -1, 0, 0, 1.0f, 0, 0) != 0;
}

bool __thiscall CPed::CanUseUnkE1F0()
{
    if (Core::StaticRef<uint8>(0x122F8BF))
        return false;

    const auto* self = reinterpret_cast<const uint8*>(this);
    if ((*reinterpret_cast<const int16*>(self + 0x266) & 1) == 0)
        return false;

    using VtblFn = uint8(__thiscall*)(CPed*);
    if (reinterpret_cast<VtblFn>(m_vftable[74])(this))
        return true;

    if (m_nDummyConvertState == 2)
        return true;

    auto** modelInfos = reinterpret_cast<void**>(Core::Rebase(0x15F73B0));
    return modelInfos[m_nModelIndex] != nullptr
        && Core::Hook::CallMethod<uint8>(0x42FF90, reinterpret_cast<uint8*>(modelInfos[m_nModelIndex]) + 0x120, 9) != 0;
}

int32 __thiscall CPed::GetUnk9E27D0Minus1()
{
    return static_cast<int32>(static_cast<uint8>(Core::Hook::Call<int32>(0x9E27D0, static_cast<void*>(this), 0))) - 1;
}

int32 __thiscall CPed::GetVehicleSeatOrTypeState()
{
    const auto* veh = reinterpret_cast<const uint8*>(m_pMyVehicle);
    if (veh
        && (m_nPedFlags2 & 4) != 0
        && *reinterpret_cast<const int16*>(veh + 46) == Core::StaticRef<int32>(0x12308EC)
        && (*reinterpret_cast<CPed* const*>(veh + 4000) == this
            || (!Core::Hook::Call<uint8>(0x462F90)
                && Core::Hook::CallMethod<uint8>(0x426910, reinterpret_cast<uint8*>(m_pPedIntelligence) + 736, 734, int8(0)))))
    {
        return 15;
    }

    uint8 buf[36]{};
    if (Core::Hook::Call<uint8>(0xA982E0, this, buf, 3))
        return 12;
    if (Core::Hook::Call<uint8>(0xA982E0, this, buf, 5))
        return 13;
    if (Core::Hook::Call<uint8>(0xA982E0, this, buf, 8))
        return 14;
    return Core::Hook::Call<uint8>(0xA982E0, this, buf, 4) ? 11 : 15;
}

int8 __thiscall CPed::UpdateVehicleSeatState()
{
    const int32 state = GetVehicleSeatOrTypeState();
    int8 result = static_cast<int8>(Core::Hook::CallMethod<int32>(0x9AA330, m_pedSubObj2B0, state, 0, int8(0), this));

    if (Core::StaticRef<int32>(0x10F8044) == 2 && Core::Hook::Call<uint8>(0x462F90))
    {
        const auto* veh = reinterpret_cast<const uint8*>(m_pMyVehicle);
        if (veh
            && (m_nPedFlags2 & 4) != 0
            && *reinterpret_cast<CPed* const*>(veh + 4000) == this
            && *reinterpret_cast<const int32*>(veh + 4948) == 4)
        {
            result = static_cast<int8>(Core::Hook::CallMethod<int32>(0x9AA330, m_pedSubObj2B0, 0, 0, int8(0), this));
        }
    }

    return result;
}

// UNHOOK_SAFE_SKIP: 0x89E640 is __thiscall (adds 0xB94 to ECX before calling 0x447610), declared static __stdcall in Ped.h so ECX is lost
int32 __stdcall CPed::CallUnk447610(float value)
{
    return Core::Hook::CallStdcall<int32>(0x447610, value);
}

int32 __thiscall CPed::AddToUnkB94AndCall(float delta)
{
    const float sum = static_cast<float>(Core::Hook::CallMethod<double>(0x447640, reinterpret_cast<uint8*>(this) + 0xB94)) + delta;
    return Core::Hook::CallMethod<int32>(0x447610, reinterpret_cast<uint8*>(this) + 0xB94, sum);
}

int32 __cdecl CPed::ShutdownPedSubsystems()
{
    Core::Hook::Call<int32>(0xA679D0);
    Core::Hook::Call<int32>(0xA1E8A0);
    Core::Hook::Call<int32>(0xB3C690);
    Core::Hook::Call<int32>(0xBCB870);
    Core::Hook::Call<int32>(0x9C2D00);
    Core::Hook::Call<int32>(0x9725B0);
    return Core::Hook::Call<int32>(0x89C040);
}

int8 __thiscall CPed::SetUnkA70State(int8 state)
{
    auto* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<int8*>(self + 0xA70) = state;

    const auto* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (!unk6C || !unk6C[14])
        Core::Hook::CallMethod<int32>(0x89C460, this);

    int8 result = *reinterpret_cast<int8*>(self + 0xA70);
    if (result == 2)
    {
        Core::Hook::CallMethod<int32>(0x9C2310, m_pPedIntelligence, 30.0f);
        Core::Hook::CallMethod<int32>(0x9C22F0, m_pPedIntelligence, 30.0f);
        if (!m_nPedUnk219)
        {
            auto* sub224 = reinterpret_cast<uint8*>(m_pPedIntelligence);
            *reinterpret_cast<float*>(sub224 + 236) = 15.0f;
            *reinterpret_cast<int32*>(sub224 + 232) = 999;
        }
        Core::Hook::CallMethod<int32>(0xB2A8F0, self + 0x80, int8(1));
        result = static_cast<int8>(Core::Hook::Call<int32>(0xB29E50, self + 0x80));
        if (!m_nPedUnk219)
            *reinterpret_cast<uint32*>(self + 0x264) &= ~0x100u;
    }
    else if (result == 3)
    {
        Core::Hook::CallMethod<int32>(0xB2A8F0, self + 0x80, int8(1));
        return static_cast<int8>(Core::Hook::Call<int32>(0xB29E50, self + 0x80));
    }

    return result;
}

void __thiscall CPed::OnRagdollModelCheck(int32 inst, int32 step)
{
    Core::Hook::CallMethod<int32>(0xA48A40, this, inst, step);
    if (step == Core::StaticRef<int32>(0xF36DFC) - 1 && inst == reinterpret_cast<int32>(m_pAnimBlendInst))
    {
        const auto* unk7C4 = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(this) + 0x7C4);
        if (unk7C4)
        {
            const uint16 modelIdx = *reinterpret_cast<const uint16*>(unk7C4 + 8);
            const auto* mgr = Core::StaticRef<const uint8*>(0x1666880);
            if (modelIdx != 0xFFFF
                && (static_cast<int32>(modelIdx) >= static_cast<int32>(*reinterpret_cast<const uint16*>(mgr + 38))
                    || (*reinterpret_cast<const uint8*>(*reinterpret_cast<const uint8* const*>(mgr + 112) + 8 * modelIdx + 4) & 3) != 0))
            {
                Core::Hook::CallMethod<void>(0x89C2C0, this, int8(0));
            }
        }
    }
}

void __thiscall CPed::UpdateRagdollDistanceCheck(int8 forceReset)
{
    Core::Hook::CallMethod<int32>(0xA48770, this, forceReset);
    auto* self = reinterpret_cast<uint8*>(this);
    void* unk7C4 = m_pRagdollInst;

    if (!m_pPhysicsInst || m_pPhysicsInst != unk7C4)
    {
        const auto* v4 = static_cast<const float*>(unk7C4);
        if (v4)
        {
            const auto* mat = reinterpret_cast<const float*>(m_pMatrix);
            if (!forceReset)
            {
                const float dx = mat[12] - v4[16];
                const float dy = mat[13] - v4[17];
                const float dz = mat[14] - v4[18];
                if (dz * dz + dx * dx + dy * dy <= 25.0f)
                    return;
            }
            Core::Hook::CallMethod<void>(0x89C2C0, this, int8(1));
        }
    }

    if (forceReset)
    {
        *reinterpret_cast<uint64*>(self + 2788) = 0;
        *reinterpret_cast<uint32*>(self + 2784) = 0;
        *reinterpret_cast<uint64*>(self + 2804) = 0;
        *reinterpret_cast<uint32*>(self + 2800) = 0;
        *reinterpret_cast<uint64*>(self + 2820) = 0;
        *reinterpret_cast<uint32*>(self + 2816) = 0;
        if (m_nPedUnk219)
            *reinterpret_cast<uint32*>(self + 3056) |= 0xCu;
    }
}

int8 __thiscall CPed::FindNearbyCoverPoint()
{
    struct CoverCandidate
    {
        float x;
        float y;
        float z;
        float w;
        float radius;
        uint32 pad[3];
    };
    static_assert(sizeof(CoverCandidate) == 32, "CoverCandidate size");

    int32& count = Core::StaticRef<int32>(0x122F8C0);
    count = 0;

    const auto* mat = reinterpret_cast<const float*>(m_pMatrix);
    float query[5]{ mat[12], mat[13], mat[14], 0.0f, 25.0f };
    Core::Hook::Call<int32>(0x81B4A0, query, Core::Rebase(0x89F450), query, 20, 14);

    auto* candidates = reinterpret_cast<CoverCandidate*>(Core::Rebase(0x122F8D0));
    const float qx = query[0];
    const float qy = query[1];
    const float qz = query[2];
    const float qR = query[4];

    const auto* pedPool = Core::StaticRef<const uint32*>(0x18A82AC);
    const int32 pedPoolSize = static_cast<int32>(pedPool[2]);
    const auto* pedFlags = reinterpret_cast<const int8*>(pedPool[1]);
    const uint32 pedBase = pedPool[0];
    const uint32 pedStride = pedPool[3];

    for (int32 i = 0; i < pedPoolSize; ++i)
    {
        if (pedFlags[i] < 0)
            continue;
        const auto* ped = reinterpret_cast<const uint8*>(pedBase + i * pedStride);
        if (!ped || ped == reinterpret_cast<const uint8*>(this))
            continue;

        const auto* pMat = *reinterpret_cast<const float* const*>(ped + 0x20);
        const float dx = qx - pMat[12];
        const float dy = qy - pMat[13];
        const float dz = qz - pMat[14];
        if ((qR + 1.0f) > std::sqrt(dz * dz + dy * dy + dx * dx) && count < 64)
        {
            CoverCandidate& c = candidates[count++];
            c.x = pMat[12];
            c.y = pMat[13];
            c.z = pMat[14] - 1.0f;
            c.w = 0.0f;
            c.radius = 1.0f;
        }
    }

    const auto* vehPool = Core::StaticRef<const uint32*>(0x18A82BC);
    const int32 vehPoolSize = static_cast<int32>(vehPool[2]);
    const auto* vehFlags = reinterpret_cast<const int8*>(vehPool[1]);
    const uint32 vehBase = vehPool[0];
    const uint32 vehStride = vehPool[3];

    for (int32 i = 0; i < vehPoolSize; ++i)
    {
        if (vehFlags[i] < 0)
            continue;
        const auto* veh = reinterpret_cast<const uint8*>(vehBase + i * vehStride);
        if (!veh)
            continue;

        const auto* vMat = *reinterpret_cast<const float* const*>(veh + 0x20);
        const float dx = qx - vMat[12];
        const float dy = qy - vMat[13];
        const float dz = qz - vMat[14];
        if ((qR + 1.0f) > std::sqrt(dz * dz + dy * dy + dx * dx) && count < 64)
        {
            CoverCandidate& c = candidates[count++];
            c.x = vMat[12];
            c.y = vMat[13];
            c.z = vMat[14] - 1.0f;
            c.w = 0.0f;
            c.radius = 1.0f;
        }
    }

    int32 outPos[4]{};
    if (!Core::Hook::Call<int32>(0xAA66C0, reinterpret_cast<int32>(mat + 12), reinterpret_cast<int32>(outPos), 25.0f, 10, 0, count, reinterpret_cast<int32>(candidates)))
        return 0;

    using SetPosFn = void(__thiscall*)(CPed*, int32*, uint32, int32);
    reinterpret_cast<SetPosFn>(m_vftable[2])(this, outPos, 0, 1);
    return 1;
}

bool __thiscall CPed::HasLineOfSightToEntity(const CEntity* target, int8 skipAngleAndRangeCheck)
{
    const auto* tgt = reinterpret_cast<const uint8*>(target);
    const auto* tgtMat = *reinterpret_cast<const float* const*>(tgt + 0x20);
    const float* tgtPos = tgtMat ? (tgtMat + 12) : reinterpret_cast<const float*>(tgt + 16);
    const auto* selfMat = reinterpret_cast<const float*>(m_pMatrix);

    if (!skipAngleAndRangeCheck)
    {
        const float dx = tgtPos[0] - selfMat[12];
        const float dy = tgtPos[1] - selfMat[13];
        if (selfMat[5] * dy + selfMat[4] * dx < 0.0f)
            return false;

        const auto* sub224 = reinterpret_cast<const float*>(m_pPedIntelligence);
        const float dist = std::sqrt(dy * dy + dx * dx);
        float maxRange = sub224[56];
        if (maxRange <= sub224[57])
            maxRange = sub224[57];
        if (maxRange <= dist)
            return false;
    }

    uint32 losResult[21]{};
    losResult[4] = Core::StaticRef<uint32>(0x18A8C50);
    losResult[5] = Core::StaticRef<uint32>(0x18A8C54);
    losResult[8] = losResult[4];
    losResult[9] = losResult[5];
    losResult[12] = losResult[4];
    losResult[13] = losResult[5];
    losResult[6] = Core::StaticRef<uint32>(0x18A8C58);
    losResult[10] = losResult[6];
    losResult[14] = losResult[6];
    losResult[19] = 0x0000FFFFu;

    float startPos[4]{ selfMat[12], selfMat[13], selfMat[14] + 0.75f, selfMat[15] };
    float endPos[4]{ tgtPos[0], tgtPos[1], tgtPos[2], tgtPos[3] };
    const uint32 entityType = (*reinterpret_cast<const uint32*>(tgt + 40) >> 6) & 0xF;
    if (entityType == 3 || entityType == 6)
        endPos[2] += 0.75f;

    return Core::Hook::Call<int32>(0x996D80, startPos, endPos, 0, losResult, 134, 1, 0, 2) == 0;
}

int32 __thiscall CPed::ProcessPedSpeechAndState()
{
    auto* self = reinterpret_cast<uint8*>(this);
    const int32 unk6C = static_cast<int32>(m_nUnk6C);
    if (unk6C && *reinterpret_cast<const uint8*>(unk6C + 14))
        return unk6C;

    const int32 overrideSpeech = *reinterpret_cast<const int32*>(self + 2956);
    if (overrideSpeech != -1)
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, overrideSpeech);

    uint8* sub224 = reinterpret_cast<uint8*>(m_pPedIntelligence);
    void* audTracker = *reinterpret_cast<void* const*>(self + 0x78);
    const int32 v4 = ((m_nRenderAnimFlags & 0x100) != 0)
        ? Core::Hook::CallMethod<int32>(0x426850, sub224 + 0x44, 431)
        : 0;

    if ((*reinterpret_cast<const uint8*>(self + 668) & 4) != 0
        || ((*reinterpret_cast<const uint8*>(self + 280) & 1) != 0
            && (m_nFlags & 0x20000000u) != 0
            && Core::Hook::CallMethod<uint8>(0x426910, sub224 + 0x2E0, 287, int8(0)) != 0))
    {
        if (Core::Hook::Call<uint8>(0x929700, 59))
        {
            Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 59);
            const int32 res = Core::Hook::CallMethod<int32>(0x9EBE90, audTracker, 0, 8);
            if (res)
            {
                Core::Hook::CallMethod<int32>(0xA4DBC0, reinterpret_cast<void*>(res), -1.0f);
                void* objA90 = m_pMoveBlend;
                using VtblFn = int32(__thiscall*)(void*, int32);
                return reinterpret_cast<VtblFn>((*reinterpret_cast<void***>(objA90))[10])(objA90, 1);
            }
            return res;
        }
        return Core::Hook::CallMethod<int32>(0x9EFEB0, audTracker, 0, 8, 4.0f, -1);
    }

    if (v4)
    {
        const int32 v5 = Core::Hook::CallMethod<int32>(0x8959F0, reinterpret_cast<void*>(v4), this);
        const int32 v6 = Core::Hook::CallMethod<int32>(0x92F590, Core::StaticRef<void*>(0x1548F88), v5);
        if (v6 == -1 || !Core::Hook::Call<int32>(0x832DD0, v6, Core::StaticRef<int32>(0xF27324)))
            return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 58);
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, v5);
    }

    if (Core::Hook::CallMethod<uint8>(0x89C780, this))
    {
        const int32 v8 = Core::Hook::CallMethod<int32>(0x9A9D30, m_pedSubObj2B0);
        if (v8)
        {
            const uint32 weapFlags = *reinterpret_cast<const uint32*>(Core::Hook::Call<int32>(0x924E80, *reinterpret_cast<const int32*>(v8 + 24)) + 32);
            if ((weapFlags & 0x80) != 0)
            {
                if ((weapFlags & 0x400000) == 0 && Core::Hook::Call<uint8>(0x929700, 57))
                    return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 57);
                return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 55);
            }
            if (Core::Hook::CallMethod<uint8>(0x89BD60, reinterpret_cast<void*>(v8)))
            {
                if ((m_nRenderAnimFlags & 0x40) == 0)
                    return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 55);
                return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 53);
            }
        }
        if (*reinterpret_cast<const int8*>(self + 672) >= 0)
            return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 54);
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 53);
    }

    const int32 pedType = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(m_pPedData) + 300);
    if (pedType == 2
        && m_nModelIndex != Core::StaticRef<int32>(0x1230AC0)
        && (m_nRenderAnimFlags & 0x200) != 0)
    {
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 140);
    }

    if ((*reinterpret_cast<const uint8*>(self + 672) & 0x20) != 0)
    {
        using GetValFn = double(__thiscall*)(CPed*);
        if (m_nDummyConvertState == 1 && reinterpret_cast<GetValFn>(m_vftable[63])(this) < 175.0)
            return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 65);
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, m_nSpeechVoiceBank);
    }

    const int32 v10 = Core::Hook::CallMethod<int32>(0x9A9D30, m_pedSubObj2B0);
    if (v10)
    {
        const uint32 weapFlags = *reinterpret_cast<const uint32*>(Core::Hook::Call<int32>(0x924E80, *reinterpret_cast<const int32*>(v10 + 24)) + 32);
        if ((weapFlags & 0x80) != 0)
        {
            if ((weapFlags & 0x400000) == 0 && Core::Hook::Call<uint8>(0x929700, 56))
                return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 56);
            if (Core::Hook::Call<uint8>(0x89BB90, pedType))
                return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 51);
            return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 50);
        }
    }

    const uint32 flags672 = m_nRenderAnimFlags;
    const bool v12 = (*reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pMoveBlend) + 80) & 2) != 0
        && ((flags672 & 0x80u) != 0 || (flags672 & 0x400) != 0);

    if ((m_nModelIndex != Core::StaticRef<int32>(0x1230AC0) || (flags672 & 0x400) != 0) && v12)
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 52);

    if (v10 && Core::Hook::CallMethod<uint8>(0x4269D0, reinterpret_cast<void*>(v10)))
    {
        const int32 activeTask = Core::Hook::CallMethod<int32>(0x89EA80, sub224);
        if (activeTask)
        {
            using GetTaskTypeFn = int32(__thiscall*)(int32);
            const int32 taskId = reinterpret_cast<GetTaskTypeFn>((*reinterpret_cast<void***>(activeTask))[3])(activeTask);
            if (taskId == 1039 && *reinterpret_cast<const int32*>(activeTask + 52) != 4)
                return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 58);
        }
    }

    const bool isPlayerModel = (m_nModelIndex == Core::StaticRef<int32>(0x1230AC0));
    if (v10 && Core::Hook::CallMethod<uint8>(0x89BD60, reinterpret_cast<void*>(v10)) && !isPlayerModel)
    {
        if (Core::Hook::Call<uint8>(0x89BB90, pedType))
            return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 51);
        return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 50);
    }

    return Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, m_nSpeechVoiceBank);
}

int32 __thiscall CPed::UpdatePedFlagsAndAudio()
{
    auto* self = reinterpret_cast<uint8*>(this);
    auto* dwords = reinterpret_cast<uint32*>(this);
    void* audTracker = *reinterpret_cast<void* const*>(self + 0x78);

    if (Core::Hook::CallMethod<int32>(0x9EBFF0, audTracker, 0x80000000u, int8(1))
        && !Core::Hook::CallMethod<uint8>(0x426910, reinterpret_cast<uint8*>(m_pPedIntelligence) + 0x2E0, 1040, int8(0)))
    {
        if (!m_nPedUnk218 && m_nPedUnk219 && m_pPlayerInfo && dwords[746] == 71)
        {
            const auto* src = reinterpret_cast<const uint32*>(Core::Rebase(0x108A340));
            dwords[760] = src[0];
            dwords[761] = src[1];
            dwords[762] = src[2];
            dwords[763] = src[3];
        }
        else
        {
            using VtblFn = void(__thiscall*)(CPed*);
            reinterpret_cast<VtblFn>(m_vftable[52])(this);
            dwords[760] = Core::StaticRef<uint32>(0xF36DB0);
            dwords[761] = Core::StaticRef<uint32>(0xF36DB4);
            dwords[762] = Core::StaticRef<uint32>(0xF36DB8);
            dwords[763] = Core::StaticRef<uint32>(0xF36DBC);
        }
    }

    if ((m_nPedFlags2 & 4) != 0)
    {
        const auto* const* modelInfos = reinterpret_cast<const uint8* const*>(Core::Rebase(0x15F73B0));
        if ((*reinterpret_cast<const uint32*>(modelInfos[m_nModelIndex] + 288) & 2) != 0)
            dwords[746] = 69;
        else
            dwords[746] = 70;
    }

    int32 result = 0;
    if ((m_pPhysicsInst && m_pPhysicsInst == m_pRagdollInst)
        || (result = Core::Hook::CallMethod<uint8>(0x89C780, this)) != 0
        || static_cast<int32>(m_nMoveState) > 2
        || (dwords[168] & 0x8000) != 0)
    {
        result = Core::Hook::CallMethod<int32>(0x9EBFF0, audTracker, 0x80000u, int8(1));
        if (result)
            return Core::Hook::CallMethod<int32>(0xA4DBC0, reinterpret_cast<void*>(result), -8.0f);
    }

    return result;
}

int32 __thiscall CPed::UpdatePedMoodAudio()
{
    using Vtbl52Fn = uint8*(__thiscall*)(CPed*);
    uint8* sub = reinterpret_cast<Vtbl52Fn>(m_vftable[52])(this);
    if (!sub)
        return 0;
    if ((sub[114] & 8) == 0)
        return reinterpret_cast<int32>(sub);

    auto* self = reinterpret_cast<uint8*>(this);
    void* audTracker = *reinterpret_cast<void* const*>(self + 0x78);
    int32 result = reinterpret_cast<int32>(sub);
    const int32 speechId = m_nSpeechId;
    const int32 subSpeechId = m_nSubSpeechId;

    if (!m_nPedUnk218)
    {
        if (m_nPedUnk219 && m_pPlayerInfo)
        {
            if (subSpeechId == 442 || subSpeechId == 443 || subSpeechId == 444)
            {
                result = Core::Hook::CallMethod<int32>(0x9EBE90, audTracker, speechId, subSpeechId);
                if (!result)
                {
                    result = Core::Hook::Call<int32>(0x909430, speechId, subSpeechId);
                    if (result)
                        result = Core::Hook::CallMethod<int32>(0x9EFEB0, audTracker, speechId, subSpeechId, 30.0f, -1);
                }
            }
            else if (Core::Hook::CallMethod<uint8*>(0x4389D0, this)
                && Core::Hook::CallMethod<int32>(0x920FF0, Core::Hook::CallMethod<uint8*>(0x4389D0, this)) > 0)
            {
                const int32 bank = Core::Hook::Call<int32>(0x908A00, speechId);
                uint32& initFlags = Core::StaticRef<uint32>(0x1230128);
                int32& hash = Core::StaticRef<int32>(0x1230124);
                if ((initFlags & 1) == 0)
                {
                    initFlags |= 1u;
                    hash = static_cast<int32>(Core::Hook::Call<uint32>(0x5B1C30, "police_chase", 0u));
                }
                const int32 snd = Core::Hook::Call<int32>(0x909240, bank, hash);
                if (snd)
                {
                    result = Core::Hook::CallMethod<int32>(0x9EC180, audTracker, snd);
                    if (!result)
                        result = Core::Hook::CallMethod<int32>(0x9EFF90, audTracker, bank, hash, 16416, 5, 30.0f);
                }
                else
                {
                    result = Core::Hook::CallMethod<int32>(0x9EBFF0, audTracker, 0x40000, int8(1));
                    if (result)
                        result = Core::Hook::CallMethod<int32>(0xA4DBC0, reinterpret_cast<void*>(result), -8.0f);
                }
            }
            else
            {
                const auto* p552 = reinterpret_cast<const uint32*>(m_pPlayerInfo);
                const uint32 curTime = static_cast<uint32>(Core::StaticRef<uint64>(0x11DDE74));
                const int32 bank = Core::Hook::Call<int32>(0x908A00, speechId);
                uint32& initFlags = Core::StaticRef<uint32>(0x1230128);

                if (p552[343] <= curTime)
                {
                    int32& hash = Core::StaticRef<int32>(0x123011C);
                    if ((initFlags & 4) == 0)
                    {
                        initFlags |= 4u;
                        hash = static_cast<int32>(Core::Hook::Call<uint32>(0x5B1C30, "plyr_mood_normal", 0u));
                    }
                    const int32 snd = Core::Hook::Call<int32>(0x909240, bank, hash);
                    if (snd)
                    {
                        result = Core::Hook::CallMethod<int32>(0x9EC180, audTracker, snd);
                        if (!result)
                            result = Core::Hook::CallMethod<int32>(0x9EFF90, audTracker, bank, hash, 16416, 5, 30.0f);
                    }
                    else
                    {
                        result = Core::Hook::CallMethod<int32>(0x9EBFF0, audTracker, 0x40000, int8(1));
                        if (result)
                            result = Core::Hook::CallMethod<int32>(0xA4DBC0, reinterpret_cast<void*>(result), -8.0f);
                    }
                }
                else
                {
                    int32& hash = Core::StaticRef<int32>(0x1230120);
                    if ((initFlags & 2) == 0)
                    {
                        initFlags |= 2u;
                        hash = static_cast<int32>(Core::Hook::Call<uint32>(0x5B1C30, "plyr_mood_angry", 0u));
                    }
                    const int32 snd = Core::Hook::Call<int32>(0x909240, bank, hash);
                    if (snd)
                    {
                        result = Core::Hook::CallMethod<int32>(0x9EC180, audTracker, snd);
                        if (!result)
                            result = Core::Hook::CallMethod<int32>(0x9EFF90, audTracker, bank, hash, 16416, 5, 30.0f);
                    }
                    else
                    {
                        result = Core::Hook::CallMethod<int32>(0x9EBFF0, audTracker, 0x40000, int8(1));
                        if (result)
                            result = Core::Hook::CallMethod<int32>(0xA4DBC0, reinterpret_cast<void*>(result), -8.0f);
                    }
                }
            }
        }
    }

    if (m_nPedUnk218 || !m_nPedUnk219)
    {
        result = Core::Hook::CallMethod<int32>(0x9EBE90, audTracker, speechId, subSpeechId);
        if (!result)
        {
            result = Core::Hook::Call<int32>(0x909430, speechId, subSpeechId);
            if (result)
                return Core::Hook::CallMethod<int32>(0x9EFEB0, audTracker, speechId, subSpeechId, 30.0f, -1);
        }
    }

    return result;
}

void __thiscall CPed::ProcessBoneAttachmentEvents()
{
    const auto* self = reinterpret_cast<const uint8*>(this);
    if (m_pPhysicsInst && m_pPhysicsInst == m_pRagdollInst)
        return;

    void* audTracker = *reinterpret_cast<void* const*>(self + 0x78);
    uint8* selected = nullptr;
    for (int32 slot = 4; slot >= 0; --slot)
    {
        uint8* a = nullptr;
        uint8* b = nullptr;
        float wa = 0.0f;
        float wb = 0.0f;
        Core::Hook::CallMethod<int32>(0x9EC270, audTracker, slot, slot, &a, &wa, &b, &wb);

        if (a)
        {
            const uint8 flags = a[70];
            const float val = *reinterpret_cast<const float*>(a + 92);
            if ((flags & 0x20) != 0 && val == 0.0f)
                wa -= 0.40000001f;
            else if ((flags & 0x20) != 0 && val == 1.0f)
                wa += 0.40000001f;
        }

        if (b)
        {
            const uint8 flags = b[70];
            const float val = *reinterpret_cast<const float*>(b + 92);
            if ((flags & 0x20) != 0 && val == 0.0f)
                wb -= 0.40000001f;
            else if ((flags & 0x20) != 0 && val == 1.0f)
                wb += 0.40000001f;
        }

        if (wa <= wb)
            selected = b;
        else
        {
            selected = a;
            wb = wa;
        }

        if (selected && wb >= 0.60000002f)
            break;
        selected = nullptr;
    }

    if (!selected)
        return;

    const auto getSub = [](const uint8* node) -> const uint8* {
        return (*reinterpret_cast<const uint16*>(node + 68) == 1)
            ? *reinterpret_cast<const uint8* const*>(node + 64)
            : nullptr;
    };

    if ((getSub(selected)[40] & 8) != 0)
    {
        const uint8* anim = Core::Hook::CallMethod<const uint8*>(0x58ABB0, getSub(selected), int8(130), int16(0));
        bool keep = false;
        if (anim)
        {
            const int32 step = *reinterpret_cast<const uint16*>(anim + 4);
            int32 idx = 0;
            if (idx >= static_cast<int32>(*reinterpret_cast<const uint16*>(anim + 12)) - 1)
                idx = static_cast<int32>(*reinterpret_cast<const uint16*>(anim + 12)) - 1;
            const auto* tracks = *reinterpret_cast<const uint8* const*>(anim + 8);
            void* obj = *reinterpret_cast<void* const*>(*reinterpret_cast<const uint8* const*>(tracks + 4 * idx) + 4);
            int32 mask = 0;
            using Vtbl5Fn = void(__thiscall*)(void*, float, int32*);
            reinterpret_cast<Vtbl5Fn>((*reinterpret_cast<void***>(obj))[5])(obj, -static_cast<float>(idx * step), &mask);
            keep = (mask & 0x1000000) != 0;
        }
        if (!keep && *reinterpret_cast<uint32*>(selected + 128) != 0)
        {
            Core::Hook::CallMethod<int32>(0xAACAB0, m_pedSubObj3D0, *reinterpret_cast<uint32*>(selected + 128));
            *reinterpret_cast<uint32*>(selected + 128) = 0;
        }
    }

    if ((getSub(selected)[40] & 0x40) != 0)
        Core::Hook::CallMethod<void>(0x89CD90, this, *reinterpret_cast<int32*>(selected + 120), 0u, 0, 1);

    if (*reinterpret_cast<const int8*>(getSub(selected) + 40) < 0)
        Core::Hook::CallMethod<void>(0x89CD90, this, *reinterpret_cast<int32*>(selected + 124), 0u, 0, 1);
}

int32 __thiscall CPed::ProcessAnimEventTrack(const uint8* track, float timeSec, int32 unused)
{
    (void)unused;
    const float frame = timeSec * 30.0f;
    const int32 step = *reinterpret_cast<const uint16*>(track + 4);
    const int32 maxIdx = static_cast<int32>(*reinterpret_cast<const uint16*>(track + 12)) - 1;
    int32 idx = static_cast<int32>(frame / static_cast<float>(step));
    if (idx >= maxIdx)
        idx = maxIdx;

    const auto* entries = *reinterpret_cast<const uint8* const*>(track + 8);
    void* obj = *reinterpret_cast<void* const*>(*reinterpret_cast<const uint8* const*>(entries + 4 * idx) + 4);
    float outWeight = 0.0f;
    using Vtbl4Fn = int32(__thiscall*)(void*, float, float*);
    int32 result = reinterpret_cast<Vtbl4Fn>((*reinterpret_cast<void***>(obj))[4])(obj, frame - static_cast<float>(idx * step), &outWeight);

    if (outWeight > 0.0f)
    {
        void* audTracker = *reinterpret_cast<void* const*>(reinterpret_cast<const uint8*>(this) + 0x78);
        const auto* const* modelInfos = reinterpret_cast<const uint8* const*>(Core::Rebase(0x15F73B0));
        const int32 voiceId = *reinterpret_cast<const int32*>(modelInfos[m_nModelIndex] + 136);
        const int32 eventSoundId = static_cast<int32>(*reinterpret_cast<const uint16*>(track + 2)) + 431;

        int32 snd = Core::Hook::CallMethod<int32>(0x9EBE90, audTracker, voiceId, 430);
        if (!snd)
            snd = Core::Hook::CallMethod<int32>(0x9EFDC0, audTracker, voiceId, 430, -1);
        if (snd)
            Core::Hook::CallMethod<int32>(0xA4C470, reinterpret_cast<void*>(snd), 1.0f);

        result = Core::Hook::CallMethod<int32>(0x9EBE90, audTracker, voiceId, eventSoundId);
        if (!result)
            result = Core::Hook::CallMethod<int32>(0x9EFDC0, audTracker, voiceId, eventSoundId, -1);
        if (result)
        {
            float vol = Core::StaticRef<float>(0xF21EC8) * outWeight;
            if (vol > 1.0f)
                vol = 1.0f;
            return Core::Hook::CallMethod<int32>(0xA4C470, reinterpret_cast<void*>(result), vol);
        }
    }

    return result;
}

void __thiscall CPed::TriggerShockedSpeech()
{
    const auto* self = reinterpret_cast<const uint8*>(this);
    if (!m_bIsDead || (m_nPedState != 1 && m_nPedState != 2))
    {
        if (m_pPedIntelligence)
        {
            const int32 bank = Core::Hook::Call<int32>(0x908A00, m_nSpeechId);
            uint32& initFlags = Core::StaticRef<uint32>(0x1230130);
            int32& hash = Core::StaticRef<int32>(0x123012C);
            if ((initFlags & 1) == 0)
            {
                initFlags |= 1u;
                hash = static_cast<int32>(Core::Hook::Call<uint32>(0x5B1C30, "shocked", 0u));
            }
            if (Core::Hook::Call<int32>(0x909240, bank, hash))
            {
                void* audTracker = *reinterpret_cast<void* const*>(self + 0x78);
                Core::Hook::CallMethod<int32>(0x9EFF90, audTracker, bank, hash, 294912, 6, 30.0f);
            }
        }
    }
}

void __thiscall CPed::UpdateAnimEvents()
{
    auto* self = reinterpret_cast<uint8*>(this);
    if ((self[674] & 1) != 0)
        return;

    const int32 elapsedMs = Core::Hook::CallMethod<int32>(0xA58E00, m_pedSpeechAudio);
    if (elapsedMs)
    {
        const float timeSec = static_cast<float>(static_cast<double>(static_cast<uint32>(elapsedMs + 50)) * 0.001);
        const uint8* animData = nullptr;
        Core::Hook::CallMethod<int32>(0xA58E60, m_pedSpeechAudio, &animData);
        if (animData)
        {
            const int32 numTracks = *reinterpret_cast<const uint16*>(animData + 36);
            const auto* tracks = *reinterpret_cast<const uint8* const*>(animData + 32);
            for (int32 i = 0; i < numTracks; ++i)
            {
                const uint8* track = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(tracks) + 4 * i);
                if (track && *track == 22)
                    ProcessAnimEventTrack(track, timeSec, i);
            }
        }
        *reinterpret_cast<float*>(self + 3384) = timeSec;
    }
    else
    {
        *reinterpret_cast<uint32*>(self + 3384) = 0;
    }
}

int32 __thiscall CPed::SetRagdollUnkB90(int32 value)
{
    auto* dwords = reinterpret_cast<int32*>(this);
    m_nMoveState = value;

    const auto* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (!unk6C || !unk6C[14])
    {
        const float fval = static_cast<float>(Core::Hook::Call<double>(0xA28620, value));
        Core::Hook::CallMethod<int32>(0xBC9B00, m_pMoveBlend, fval, 0.0f, int8(0));
    }

    const int32 result = m_nMoveState;
    if (result == 3 || result == 4)
        return Core::Hook::Call<int32>(0x987730, 2, reinterpret_cast<int32>(m_pMatrix) + 48, reinterpret_cast<int32>(this), 0, 0, 0.0f);
    return result;
}

int32 __thiscall CPed::UpdateRagdollUnkBC9B00(float value)
{
    const auto* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
    if (!unk6C || !unk6C[14])
        return Core::Hook::CallMethod<int32>(0xBC9B00, m_pMoveBlend, value, *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(m_pMoveBlend) + 12), int8(0));
    return static_cast<int32>(m_nUnk6C);
}

int8 __thiscall CPed::UpdateMovementAndSpeech()
{
    if (m_nPedState == 2 || (m_nPedFlags2 & 4) != 0)
        Core::Hook::CallMethod<int32>(0x89D1B0, this, 1);

    ProcessPedSpeechAndState();

    auto* objA90 = reinterpret_cast<uint8*>(m_pMoveBlend);
    using Vtbl1Fn = void(__thiscall*)(void*);
    reinterpret_cast<Vtbl1Fn>((*reinterpret_cast<void***>(objA90))[1])(objA90);

    const float vx = *reinterpret_cast<const float*>(objA90 + 4);
    const float vy = *reinterpret_cast<const float*>(objA90 + 8);
    float speed = std::sqrt(vy * vy + vx * vx);
    if (speed < 0.0f)
        speed = 0.0f;
    else if (speed > 3.0f)
        speed = 3.0f;

    const int32 moveState = Core::Hook::Call<int32>(0xA28660, speed);
    *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 2960) = moveState;

    int32 result = moveState;
    if (moveState == 3 || moveState == 4)
    {
        const auto* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
        if (!unk6C || !unk6C[14])
        {
            const auto* slots = reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(m_pPedIntelligence) + 68);
            bool hasActiveTask = false;
            for (int32 i = 0; i < 5; ++i)
            {
                if (slots[i])
                {
                    using Vtbl3Fn = int32(__thiscall*)(const uint8*);
                    if (reinterpret_cast<Vtbl3Fn>((*reinterpret_cast<void* const* const*>(slots[i]))[3])(slots[i]) == 912
                        && Core::StaticRef<float>(0x16394E4) >= 0.2f)
                    {
                        hasActiveTask = true;
                    }
                    break;
                }
            }
            const bool flag952 = Core::Hook::CallMethod<int32>(0x426850, reinterpret_cast<uint8*>(m_pPedIntelligence) + 0x44, 952) != 0;
            result = flag952 ? 1 : 0;
            if (!hasActiveTask && !flag952)
                result = Core::Hook::Call<int32>(0x987730, 2, reinterpret_cast<int32>(m_pMatrix) + 48, reinterpret_cast<int32>(this), 0, 0, 0.0f);
        }
        else
        {
            result = static_cast<int32>(m_nUnk6C);
        }
    }

    return static_cast<int8>(result);
}
// --- Slice 7 ---
void CPed::InjectHooks_Slice7()
{
    Core::Hook::Install(0x8A13B0, &CPed::ProbeGroundCircle);
    Core::Hook::Install(0x8A15E0, &CPed::UpdateGroundCollision);
    Core::Hook::Install(0x8A2050, &CPed::ProcessContactPoints);
    Core::Hook::Install(0x8A23C0, &CPed::ApplyGroundSlopeVelocity);
    Core::Hook::Install(0x8A28B0, &CPed::ProcessPhysicsStep);
    Core::Hook::Install(0x8A2F00, &CPed::UpdateSpeechAndAudio);
    Core::Hook::Install(0x8A3100, &CPed::TriggerFootstepAudio);
    Core::Hook::Install(0x8A3250, &CPed::UpdatePositionOrHeadLook);
    Core::Hook::Install(0x8A33F0, &CPed::CanConvertToDummy);
    Core::Hook::Install(0x8A3850, &CPed::PreRenderPedInternal);
    Core::Hook::Install(0x8A3990, &CPed::PostRenderPedInternal);
    Core::Hook::Install(0x8A39E0, &CPed::SyncRagdollToMatrix);
    Core::Hook::Install(0x8A3B30, &CPed::RenderPed);
    Core::Hook::Install(0x8A3ED0, &CPed::TestLineOfSightToEntity);
    Core::Hook::Install(0x8A4020, &CPed::FindVehicleExitPosition);
    Core::Hook::Install(0x8A4BE0, &CPed::GetBonePosPtr);
    Core::Hook::Install(0x8A4C40, &CPed::GetBonePosition);
    Core::Hook::Install(0x8A4D00, &CPed::GetBoneMatrixOut);
    Core::Hook::Install(0x8A4E30, &CPed::GetBonePositionUpdated);
    Core::Hook::Install(0x8A4E70, &CPed::GetBoneMatrixUpdated);
    Core::Hook::Install(0x8A5060, &CPed::GetHeadOrCenterPosition);
    Core::Hook::Install(0x8A54B0, &CPed::GetClimbOrStepHeight);
    Core::Hook::Install(0x8A56D0, &CPed::UpdateTaskTimerD3C);
    Core::Hook::Install(0x8A5700, &CPed::HasHeldWeaponObject);
    Core::Hook::Install(0x8A5740, &CPed::SetWeaponVariation);
    Core::Hook::Install(0x8A5780, &CPed::GetWeaponVariation);
    Core::Hook::Install(0x8A57E0, &CPed::SetWeaponDrawable);
    Core::Hook::Install(0x8A5830, &CPed::ProcessAnimMoveAndTurn);
}

// 0x8A13B0
bool __cdecl CPed::ProbeGroundCircle(float* origin, float* outMinZ, float* outMaxZ, float radius, float steps, float depth, int32* outNormal, uint32* outMaterial)
{
    *outMinZ = 999999.0f;
    *outMaxZ = -999999.0f;
    bool hitAny = false;
    float angle = 0.0f;

    const uint32 unkX = Core::StaticRef<uint32>(0x18A8C50);
    const uint32 unkY = Core::StaticRef<uint32>(0x18A8C54);
    const uint32 unkZ = Core::StaticRef<uint32>(0x18A8C58);

    do
    {
        uint32 probe[24]{};
        probe[0] = 0;
        probe[4] = unkX;
        probe[5] = unkY;
        probe[6] = unkZ;
        probe[8] = unkX;
        probe[9] = unkY;
        probe[10] = unkZ;
        probe[12] = unkX;
        probe[13] = unkY;
        probe[14] = unkZ;
        probe[16] = 0;
        probe[17] = 0;
        probe[18] = 0;
        *reinterpret_cast<int16*>(&probe[19]) = -1;
        *reinterpret_cast<int16*>(reinterpret_cast<uint8*>(&probe[19]) + 2) = 0;
        probe[20] = 0;

        const float cosA = std::cos(angle);
        const float sinA = std::sin(angle);

        float seg[8]{};
        seg[0] = origin[0] + cosA * radius;
        seg[1] = origin[1] - sinA * radius;
        seg[2] = origin[2];
        seg[4] = seg[0];
        seg[5] = seg[1];
        seg[6] = origin[2] - depth;

        if (Core::Hook::CallMethod<int32>(0x6A6FC0, Core::StaticRef<void*>(0x1666880), seg, probe, 0, 142, -1, 7, 1, 0))
        {
            const float hitZ = *reinterpret_cast<const float*>(&probe[6]);
            if (*outMinZ > hitZ)
                *outMinZ = hitZ;
            if (hitZ > *outMaxZ)
                *outMaxZ = hitZ;

            outNormal[0] = static_cast<int32>(probe[8]);
            outNormal[1] = static_cast<int32>(probe[9]);
            outNormal[2] = static_cast<int32>(probe[10]);
            outNormal[3] = static_cast<int32>(probe[11]);
            hitAny = true;
            *outMaterial = probe[17];
        }

        angle += 6.2831855f / steps;
    } while (angle < 6.2831855f);

    return hitAny;
}

// 0x8A15E0
void __thiscall CPed::UpdateGroundCollision(int32 entity)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if ((m_nRagdollAnimFlags & 0x20000000u) != 0)
        return;

    if (Core::StaticRef<uint8>(0xF36DF8))
    {
        const uint32 phInst = *reinterpret_cast<uint32*>(self + 0x38);
        if (!phInst || phInst != *reinterpret_cast<uint32*>(self + 0x7C4))
        {
            if ((*reinterpret_cast<uint8*>(self + 0x26C) & 1u) != 0)
                m_nPedFlags2 = (m_nPedFlags2 & ~3u) | 2u;

            const float diffZ = m_fGroundZ - *reinterpret_cast<float*>(entity + 0x38);
            if (diffZ > -1.0f || ((m_nPedFlags2 & 2u) != 0 && diffZ > -1.3f))
            {
                m_nPedFlags2 |= 1u;
                m_fGroundDeltaZ = diffZ;
                if (self[0xD00] == 0)
                {
                    const uint8 bVar = self[0x298];
                    if (bVar)
                    {
                        const uint8* mat = *reinterpret_cast<const uint8**>(self + 0x20);
                        const float matZ = *reinterpret_cast<const float*>(mat + 0x38);
                        float pos[4]{
                            *reinterpret_cast<const float*>(mat + 0x30),
                            *reinterpret_cast<const float*>(mat + 0x34),
                            0.0f,
                            0.0f
                        };
                        float blend = static_cast<float>(bVar) * 0.050000001f;
                        if (blend > 1.0f)
                            blend = 1.0f;
                        pos[2] = (*reinterpret_cast<float*>(self + 0xAD8) + 1.0f) * blend + (1.0f - blend) * matZ;
                        Core::Hook::CallMethod<int32>(0x89E930, this, pos, 0, 0);
                    }
                }
            }
            else
            {
                *reinterpret_cast<uint32*>(self + 0xB24) = 0xC479C000u; // -999.0f
            }
            return;
        }
    }

    uint32* intelPtr = reinterpret_cast<uint32*>(m_pMoveBlend);
    using IntelCheck24Fn = uint8(__thiscall*)(uint32*);
    if ((reinterpret_cast<IntelCheck24Fn>((*reinterpret_cast<void***>(intelPtr))[6])(intelPtr) && Core::StaticRef<int32>(0xF21EDC))
        || Core::StaticRef<int32>(0x122F8C4))
    {
        Core::Hook::CallMethod<int8>(0x8A09E0, this);
        return;
    }

    uint32* unkAC0 = reinterpret_cast<uint32*>(self + 0xAC0);
    if (*unkAC0)
    {
        Core::Hook::CallMethod<int32>(0x9E6580, reinterpret_cast<void*>(*unkAC0), unkAC0);
        *unkAC0 = 0;
    }

    if ((*reinterpret_cast<uint8*>(self + 0x26C) & 1u) != 0)
        m_nPedFlags2 = (m_nPedFlags2 & ~3u) | 2u;

    const uint32 unk6C = *reinterpret_cast<uint32*>(self + 0x6C);
    const bool hasUnk6C = unk6C && *reinterpret_cast<uint8*>(unk6C + 0xE);
    const uint32 phInst = *reinterpret_cast<uint32*>(self + 0x38);
    if (*reinterpret_cast<int32*>(self + 0x7C8) > 3
        || *reinterpret_cast<uint16*>(phInst + 8) == 0xFFFF
        || (!hasUnk6C && ((self[0x24] & 1u) == 0 || (self[0x26C] & 4u) != 0))
        || (m_nRagdollAnimFlags & 0x20000000u) != 0)
    {
        return;
    }

    if ((*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(intelPtr) + 0x50) & 0x40u) != 0)
    {
        m_nPedFlags2 |= 1u;
        return;
    }

    float timeStepScale = Core::StaticRef<float>(0x11DDE7C) * 50.0f;
    if (timeStepScale < 2.0f)
        timeStepScale = 2.0f;

    float probeDepth = 1.0f;
    if ((m_nPedFlags2 & 2u) != 0)
        probeDepth = (timeStepScale * 0.15000001f) * 2.0f + 1.0f;

    float hitZ = 0.0f;
    float maxZ = 0.0f;
    float hitNormal[4]{};
    uint32 hitMaterial = 0;
    int32 hitEntity = 0;
    int32 hitComponent = 0;
    const uint8* entPos = reinterpret_cast<const uint8*>(entity + 0x30);

    if (Core::StaticRef<uint8>(0x122F8BD))
    {
        if (!ProbeGroundCircle(reinterpret_cast<float*>(entity + 0x30), &hitZ, &maxZ, Core::StaticRef<float>(0xF21EE0), 16.0f, probeDepth, reinterpret_cast<int32*>(hitNormal), &hitMaterial))
            return;
        hitZ = maxZ;
    }
    else
    {
        int32 results[384]{};
        Core::Hook::CallStdcall<void>(0x401020, results, 96, 16, Core::Rebase(0x425360));

        float segStart[4]{
            *reinterpret_cast<const float*>(entPos),
            *reinterpret_cast<const float*>(entPos + 4),
            *reinterpret_cast<const float*>(entPos + 8),
            0.0f
        };
        float segEnd[4]{
            segStart[0],
            segStart[1],
            segStart[2] - probeDepth,
            0.0f
        };

        uint8 probeCtx[32]{};
        Core::Hook::CallMethod<int32>(0x534930, probeCtx, segStart, segEnd);
        const int32 hitCount = Core::Hook::CallMethod<int32>(0x6A6FC0, Core::StaticRef<void*>(0x1666880), probeCtx, results, phInst, 174, -1, 7, 16, 0);

        int32 bestIdx = -1;
        float bestZ = 0.0f;
        for (int32 i = 0; i < hitCount; ++i)
        {
            const int32 phInstHit = results[i * 24];
            if (!phInstHit)
                break;
            const int32 ent = Core::Hook::Call<int32>(0xA72220, phInstHit);
            if (ent && (*reinterpret_cast<uint8*>(ent + 0x24) & 1u) != 0)
            {
                const float zVal = *reinterpret_cast<const float*>(&results[i * 24 + 16]);
                if (bestIdx < 0 || bestZ > zVal)
                {
                    bestIdx = i;
                    bestZ = zVal;
                }
            }
        }

        if (bestIdx > -1)
        {
            const int32 base = bestIdx * 24;
            const int32 hitPhInst = results[base];
            if (hitPhInst)
            {
                const int32 ent = Core::Hook::Call<int32>(0xA72220, hitPhInst);
                if (ent && Core::Hook::CallMethod<uint8>(0x4E0460, reinterpret_cast<void*>(ent)))
                {
                    if (Core::Hook::CallMethod<int32>(0x426800, reinterpret_cast<void*>(ent)) == reinterpret_cast<int32>(this))
                        return;
                }
            }
            hitMaterial = static_cast<uint32>(results[base + 18]);
            hitComponent = HIWORD(results[base + 20]);
            hitZ = *reinterpret_cast<const float*>(&results[base + 6]);
            hitNormal[0] = *reinterpret_cast<const float*>(&results[base + 8]);
            hitNormal[1] = *reinterpret_cast<const float*>(&results[base + 9]);
            hitNormal[2] = *reinterpret_cast<const float*>(&results[base + 10]);
            hitNormal[3] = *reinterpret_cast<const float*>(&results[base + 11]);
            hitEntity = Core::Hook::Call<int32>(0xA72220, hitPhInst);
        }
        else
        {
            if ((m_nPedFlags2 & 2u) == 0)
                return;

            int32 sphereResults[384]{};
            Core::Hook::CallStdcall<void>(0x401020, sphereResults, 96, 16, Core::Rebase(0x425360));
            const float radius = Core::StaticRef<float>(0xF21D2C);
            float capStart[4]{
                *reinterpret_cast<const float*>(entPos),
                *reinterpret_cast<const float*>(entPos + 4),
                *reinterpret_cast<const float*>(entPos + 8) - radius,
                0.0f
            };
            float capEnd[4]{
                capStart[0],
                capStart[1],
                *reinterpret_cast<const float*>(entPos + 8) - (probeDepth - radius),
                0.0f
            };
            uint8 capCtx[32]{};
            Core::Hook::CallMethod<int32>(0x534930, capCtx, capStart, capEnd);

            const int32 sphereHits = Core::Hook::CallMethod<int32>(0x6A71C0, Core::StaticRef<void*>(0x1666880), capStart, capEnd, radius, sphereResults, phInst, 174, -1, 7, 1, 16, 0);
            int32 bestSphere = -1;
            float bestSphereZ = 0.0f;
            for (int32 i = 0; i < sphereHits; ++i)
            {
                const int32 phInstHit = sphereResults[i * 24];
                if (!phInstHit)
                    break;
                const int32 ent = Core::Hook::Call<int32>(0xA72220, phInstHit);
                if (ent && (*reinterpret_cast<uint8*>(ent + 0x24) & 1u) != 0)
                {
                    const float zVal = *reinterpret_cast<const float*>(&sphereResults[i * 24 + 6]);
                    if (bestSphere < 0 || zVal > bestSphereZ)
                    {
                        bestSphere = i;
                        bestSphereZ = zVal;
                    }
                }
            }

            if (bestSphere <= -1)
                return;

            const int32 base = bestSphere * 24;
            const int32 hitPhInst = sphereResults[base];
            if (hitPhInst)
            {
                const int32 ent = Core::Hook::Call<int32>(0xA72220, hitPhInst);
                if (ent && Core::Hook::CallMethod<uint8>(0x4E0460, reinterpret_cast<void*>(ent)))
                {
                    if (Core::Hook::CallMethod<int32>(0x426800, reinterpret_cast<void*>(ent)) == reinterpret_cast<int32>(this))
                        return;
                }
            }
            hitMaterial = static_cast<uint32>(sphereResults[base + 18]);
            hitComponent = HIWORD(sphereResults[base + 20]);
            hitZ = *reinterpret_cast<const float*>(&sphereResults[base + 6]);
            hitNormal[0] = *reinterpret_cast<const float*>(&sphereResults[base + 8]);
            hitNormal[1] = *reinterpret_cast<const float*>(&sphereResults[base + 9]);
            hitNormal[2] = *reinterpret_cast<const float*>(&sphereResults[base + 10]);
            hitNormal[3] = *reinterpret_cast<const float*>(&sphereResults[base + 11]);
            hitEntity = Core::Hook::Call<int32>(0xA72220, hitPhInst);
        }
    }

    using IntelCheck32Fn = uint8(__thiscall*)(uint32*);
    if (!reinterpret_cast<IntelCheck32Fn>((*reinterpret_cast<void***>(intelPtr))[8])(intelPtr))
        return;

    m_nPedFlags2 |= 1u;
    const float invTimeStep = Core::StaticRef<float>(0xF21ED4);
    const float corrVel = ((hitZ - (*reinterpret_cast<float*>(entity + 0x38) - 1.0f)) + (0.15000001f / invTimeStep)) * (invTimeStep * 6.6666665f);

    const float* grav = Core::Hook::Call<const float*>(0x69A800);
    int32* collider = *reinterpret_cast<int32**>(phInst + 4);
    using GetMassFn = double(__thiscall*)(int32*);
    const float mass = static_cast<float>(reinterpret_cast<GetMassFn>((*reinterpret_cast<void***>(collider))[14])(collider));

    const float gravX = grav[0] * mass;
    const float gravY = grav[1] * mass;
    const float gravZ = grav[2] * mass;

    const int32 mover = Core::Hook::CallMethod<int32>(0x9E86F0, this);
    using GetUnk36Fn = double(__thiscall*)(int32);
    const float moverScale = static_cast<float>(reinterpret_cast<GetUnk36Fn>((*reinterpret_cast<void***>(mover))[9])(mover));

    float impulse[4]{
        (-corrVel * gravX) * moverScale,
        (-corrVel * gravY) * moverScale,
        (-corrVel * gravZ) * moverScale,
        0.0f
    };
    Core::Hook::CallMethod<int32>(0xA47C30, this, impulse);

    using GetVelFn = float*(__thiscall*)(CPed*, float*);
    const float* vel = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(this))[59])(this, impulse);
    const float relVel = vel[2] * hitNormal[2] + vel[1] * hitNormal[1] + vel[0] * hitNormal[0];
    float vertCorr = (relVel >= 0.0f) ? (relVel * Core::StaticRef<float>(0xF21ED0)) : (relVel * Core::StaticRef<float>(0xF21ECC));

    collider = *reinterpret_cast<int32**>(phInst + 4);
    vertCorr *= static_cast<float>(reinterpret_cast<GetMassFn>((*reinterpret_cast<void***>(collider))[14])(collider));
    if (vertCorr > 50.0f)
        vertCorr = 50.0f;
    else if (vertCorr < -50.0f)
        vertCorr = -50.0f;

    const int32 mover2 = Core::Hook::CallMethod<int32>(0x9E86F0, this);
    const float moverScale2 = static_cast<float>(reinterpret_cast<GetUnk36Fn>((*reinterpret_cast<void***>(mover2))[9])(mover2));
    impulse[0] = 0.0f;
    impulse[1] = 0.0f;
    impulse[2] = moverScale2 * vertCorr;
    Core::Hook::CallMethod<int32>(0xA47C30, this, impulse);

    *reinterpret_cast<float*>(self + 0xAD0) = *reinterpret_cast<const float*>(entPos);
    *reinterpret_cast<float*>(self + 0xAD4) = *reinterpret_cast<const float*>(entPos + 4);
    *reinterpret_cast<float*>(self + 0xAD8) = hitZ;
    *reinterpret_cast<float*>(self + 0xADC) = *reinterpret_cast<const float*>(entPos + 12);

    *reinterpret_cast<float*>(self + 0xB10) = hitNormal[0];
    *reinterpret_cast<float*>(self + 0xB14) = hitNormal[1];
    *reinterpret_cast<float*>(self + 0xB18) = hitNormal[2];
    *reinterpret_cast<float*>(self + 0xB1C) = hitNormal[3];

    if (self[0xD00] == 0)
    {
        float newPos[4]{
            *reinterpret_cast<const float*>(entPos),
            *reinterpret_cast<const float*>(entPos + 4),
            hitZ + 1.0f,
            0.0f
        };
        Core::Hook::CallMethod<int32>(0x89E930, this, newPos, 0, 0);
    }

    *reinterpret_cast<uint32*>(&m_nPhysUnk14C) = hitMaterial;
    Core::Hook::CallMethod<int32>(0xAAE360, m_pedSubObj3D0, hitMaterial, hitEntity, hitComponent);

    if (hitEntity && Core::Hook::CallMethod<uint8>(0x4E0460, reinterpret_cast<void*>(hitEntity)))
    {
        if (*unkAC0)
            Core::Hook::CallMethod<int32>(0x9E6580, reinterpret_cast<void*>(*unkAC0), unkAC0);
        *unkAC0 = static_cast<uint32>(hitEntity);
        Core::Hook::CallMethod<int32>(0x9E6550, reinterpret_cast<void*>(hitEntity), unkAC0);
    }
}

// 0x8A2050
int32 __thiscall CPed::ProcessContactPoints(int32 contacts, int32 phInst, int32 unused, int32 count)
{
    (void)unused;
    uint8* self = reinterpret_cast<uint8*>(this);
    int32 i = 0;

    for (; i < count; ++i)
    {
        uint8* contact = *reinterpret_cast<uint8**>(contacts + 4 * i);
        uint32* manifold = *reinterpret_cast<uint32**>(contact + 0xA0);
        const uint32 instA = manifold[13];
        const uint32 mode = (static_cast<uint32>(phInst) == instA) ? manifold[26] : manifold[28];

        if (mode == 1)
        {
            const uint32 otherInst = (static_cast<uint32>(phInst) == instA) ? manifold[14] : instA;
            const uint8* posPtr = (static_cast<uint32>(phInst) == instA) ? (contact + 0x40) : (contact + 0x30);

            bool reject = !Core::StaticRef<uint8>(0xF36DF8)
                || *reinterpret_cast<const float*>(posPtr + 8) <= m_fGroundZ
                || !otherInst;

            if (!reject)
            {
                using GetTypeFn = int32(__thiscall*)(uint32);
                if (reinterpret_cast<GetTypeFn>((*reinterpret_cast<void***>(otherInst))[1])(otherInst) == 15)
                    reject = true;
            }

            if (!reject)
            {
                const float nx = (static_cast<uint32>(phInst) == instA) ? *reinterpret_cast<const float*>(contact + 0x50) : -*reinterpret_cast<const float*>(contact + 0x50);
                const float ny = (static_cast<uint32>(phInst) == instA) ? *reinterpret_cast<const float*>(contact + 0x54) : -*reinterpret_cast<const float*>(contact + 0x54);
                const float nz = (static_cast<uint32>(phInst) == instA) ? *reinterpret_cast<const float*>(contact + 0x58) : -*reinterpret_cast<const float*>(contact + 0x58);

                const uint32 bodyPtr = (static_cast<uint32>(phInst) == instA) ? manifold[15] : manifold[16];
                const uint32 level = Core::StaticRef<uint32>(0x1666880);
                const uint32 instFlags = *reinterpret_cast<const uint32*>(*reinterpret_cast<const uint32*>(level + 0x70) + 8 * *reinterpret_cast<const uint16*>(otherInst + 8) + 4);

                if ((instFlags & 3u) == 2u)
                {
                    if (nz < 0.5f || (bodyPtr && *reinterpret_cast<const float*>(posPtr + 8) > *reinterpret_cast<const float*>(bodyPtr + 0x108)))
                        reject = true;
                }
                else if (nz < 0.5f || (bodyPtr && *reinterpret_cast<const float*>(posPtr + 8) > (*reinterpret_cast<const float*>(bodyPtr + 0x108) - 0.5f)))
                {
                    reject = true;
                }

                if (!reject)
                {
                    m_fGroundZ = *reinterpret_cast<const float*>(posPtr + 8);
                    *reinterpret_cast<float*>(self + 0xAD0) = *reinterpret_cast<const float*>(posPtr);
                    *reinterpret_cast<float*>(self + 0xAD4) = *reinterpret_cast<const float*>(posPtr + 4);
                    *reinterpret_cast<float*>(self + 0xAD8) = *reinterpret_cast<const float*>(posPtr + 8);
                    *reinterpret_cast<float*>(self + 0xADC) = *reinterpret_cast<const float*>(posPtr + 12);
                    *reinterpret_cast<float*>(self + 0xB10) = nx;
                    *reinterpret_cast<float*>(self + 0xB14) = ny;
                    *reinterpret_cast<float*>(self + 0xB18) = nz;

                    const int32 otherEnt = Core::Hook::Call<int32>(0xA72220, otherInst);
                    if (otherEnt)
                    {
                        const uint32 entType = (*reinterpret_cast<const uint32*>(otherEnt + 0x28) >> 6) & 0xFu;
                        if (entType > 1 && entType < 5)
                        {
                            uint32* unkAC0 = reinterpret_cast<uint32*>(self + 0xAC0);
                            if (*unkAC0)
                                Core::Hook::CallMethod<int32>(0x9E6580, reinterpret_cast<void*>(*unkAC0), unkAC0);
                            *unkAC0 = static_cast<uint32>(otherEnt);
                            Core::Hook::CallMethod<int32>(0x9E6550, reinterpret_cast<void*>(otherEnt), unkAC0);
                        }
                    }

                    const uint32 bound = *reinterpret_cast<const uint32*>(*reinterpret_cast<const uint32*>(otherInst + 4) + 0xC);
                    const uint8 boundType = *reinterpret_cast<const uint8*>(bound + 4);
                    uint32 material = 0;
                    if (boundType == 10 || boundType == 4 || boundType == 5)
                    {
                        const uint8* c = *reinterpret_cast<const uint8**>(contacts + 4 * i);
                        const uint16 polyIdx = (static_cast<uint32>(phInst) == *reinterpret_cast<const uint32*>(*reinterpret_cast<const uint32*>(c + 0xA0) + 0x34))
                            ? *reinterpret_cast<const uint16*>(c + 0xAA)
                            : *reinterpret_cast<const uint16*>(c + 0xA8);
                        uint32 matIdx = 0;
                        if (polyIdx < *reinterpret_cast<const uint32*>(bound + 0xCC))
                            matIdx = *reinterpret_cast<const uint8*>(*reinterpret_cast<const uint32*>(bound + 0x8C) + 32 * polyIdx + 12);
                        using GetMatFn = int32(__thiscall*)(uint32, uint32);
                        material = static_cast<uint32>(reinterpret_cast<GetMatFn>((*reinterpret_cast<void***>(bound))[23])(bound, matIdx));
                    }
                    else
                    {
                        material = Core::Hook::CallMethod<uint8>(0x89BB60, &contacts);
                    }

                    *reinterpret_cast<uint32*>(&m_nPhysUnk14C) = material;
                    uint32* m2 = *reinterpret_cast<uint32**>(*reinterpret_cast<const uint32*>(contacts + 4 * i) + 0xA0);
                    const uint32 comp = (static_cast<uint32>(phInst) == m2[13]) ? m2[28] : m2[26];
                    Core::Hook::CallMethod<int32>(0xAAE360, m_pedSubObj3D0, material, otherEnt, comp);
                }
            }

            if (contacts)
                Core::Hook::CallMethod<int32>(0x695880, *reinterpret_cast<void**>(*reinterpret_cast<const uint32*>(contacts + 4 * i) + 0xA0));
            *reinterpret_cast<uint8*>(*reinterpret_cast<const uint32*>(contacts + 4 * i) + 0xA5) &= ~1u;
        }

        uint8* c = *reinterpret_cast<uint8**>(contacts + 4 * i);
        uint32* m = *reinterpret_cast<uint32**>(c + 0xA0);
        const uint32 inst0 = m[13];
        const uint32 mode2 = (static_cast<uint32>(phInst) == inst0) ? m[26] : m[28];
        if (mode2 == 3)
        {
            const uint32 other = (static_cast<uint32>(phInst) == inst0) ? m[14] : inst0;
            const uint32 level = Core::StaticRef<uint32>(0x1666880);
            if (!other || (*reinterpret_cast<const uint32*>(*reinterpret_cast<const uint32*>(level + 0x70) + 8 * *reinterpret_cast<const uint16*>(other + 8) + 4) & 3u) == 2u)
            {
                c[0xA5] &= ~1u;
            }
            else
            {
                using GetMassFn = double(__thiscall*)(uint32);
                const uint32 archetype = *reinterpret_cast<const uint32*>(other + 4);
                if (reinterpret_cast<GetMassFn>((*reinterpret_cast<void***>(archetype))[9])(archetype) > 200.0)
                    c[0xA5] &= ~1u;
            }
        }
    }

    return i;
}

// 0x8A23C0
int32 __thiscall CPed::ApplyGroundSlopeVelocity()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    const uint32 phInst = *reinterpret_cast<uint32*>(self + 0x38);
    if ((phInst && phInst == *reinterpret_cast<uint32*>(self + 0x7C4)) || (m_nPedFlags2 & 1u) == 0)
        return static_cast<int32>(phInst);

    const float diffZ = m_fGroundDeltaZ;
    if (diffZ <= -2.0f || !Core::StaticRef<uint8>(0xF36DF8))
        return static_cast<int32>(phInst);

    float scale = (diffZ + 1.0f) * 6.0f + 1.0f;
    if (scale > 10.0f)
        scale = 10.0f;

    const float* grav = Core::Hook::Call<const float*>(0x69A800);
    int32* collider = *reinterpret_cast<int32**>(phInst + 4);
    using GetMassFn = double(__thiscall*)(int32*);
    const float mass = static_cast<float>(reinterpret_cast<GetMassFn>((*reinterpret_cast<void***>(collider))[14])(collider));

    const int32 mover = Core::Hook::CallMethod<int32>(0x9E86F0, this);
    using GetUnk36Fn = double(__thiscall*)(int32);
    const float moverScale = static_cast<float>(reinterpret_cast<GetUnk36Fn>((*reinterpret_cast<void***>(mover))[9])(mover)) * scale * -1.0f;

    float imp1[4]{
        grav[0] * mass * moverScale,
        grav[1] * mass * moverScale,
        grav[2] * mass * moverScale,
        0.0f
    };
    Core::Hook::CallMethod<int32>(0xA47C30, this, imp1);

    float normal[4]{
        *reinterpret_cast<float*>(self + 0xB10),
        *reinterpret_cast<float*>(self + 0xB14),
        *reinterpret_cast<float*>(self + 0xB18),
        0.0f
    };
    float vel[4]{};
    using GetVelFn = void(__thiscall*)(CPed*, float*);
    reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(this))[59])(this, vel);

    const float* prevPos = reinterpret_cast<const float*>(self + 0xAE0);
    bool handled = false;
    if ((prevPos[0] != 0.0f || prevPos[1] != 0.0f || prevPos[2] != 0.0f) && (self[0x14F] & 1u) != 0)
    {
        float dx = *reinterpret_cast<float*>(self + 0xAD0) - prevPos[0];
        float dy = *reinterpret_cast<float*>(self + 0xAD4) - prevPos[1];
        float dz = *reinterpret_cast<float*>(self + 0xAD8) - prevPos[2];
        if ((dz * dz + dy * dy + dx * dx) < 0.0625f)
        {
            if (Core::Hook::CallMethod<uint8>(0x4D9080, self + 0xAF0))
            {
                dx = *reinterpret_cast<float*>(self + 0xAD0) - *reinterpret_cast<float*>(self + 0xAF0);
                dy = *reinterpret_cast<float*>(self + 0xAD4) - *reinterpret_cast<float*>(self + 0xAF4);
                dz = *reinterpret_cast<float*>(self + 0xAD8) - *reinterpret_cast<float*>(self + 0xAF8);
            }
        }
        if ((dy * dy + dx * dx) > 0.050000001f)
        {
            const float upX = Core::StaticRef<float>(0x108A330);
            const float upY = Core::StaticRef<float>(0x108A334);
            const float upZ = Core::StaticRef<float>(0x108A338);
            const float cx = dy * upZ - dz * upY;
            const float cy = dz * upX - dx * upZ;
            const float cz = dx * upY - dy * upX;
            normal[0] = cy * dz - cz * dy;
            normal[1] = cz * dx - cx * dz;
            normal[2] = cx * dy - cy * dx;
            Core::Hook::CallMethod<int32>(0x42F5D0, normal);
        }
        handled = true;
    }

    if (!handled)
    {
        const uint32 groundEnt = *reinterpret_cast<uint32*>(self + 0xAC0);
        if (groundEnt)
        {
            if ((*reinterpret_cast<uint32*>(groundEnt + 0x28) & 0x3C0u) == 0x80u && *reinterpret_cast<uint32*>(groundEnt + 0x1354) == 2)
            {
                float tmp[4]{};
                const float* groundVel = Core::Hook::CallMethod<const float*>(0xA47FB0, reinterpret_cast<void*>(groundEnt), tmp, *reinterpret_cast<uint32*>(self + 0x20) + 0x30, 1, 0);
                vel[0] -= groundVel[0];
                vel[1] -= groundVel[1];
                vel[2] -= groundVel[2];
            }
            else
            {
                normal[0] = Core::StaticRef<float>(0x108A330);
                normal[1] = Core::StaticRef<float>(0x108A334);
                normal[2] = Core::StaticRef<float>(0x108A338);
            }
        }
    }

    float dot = vel[2] * normal[2] + vel[1] * normal[1] + vel[0] * normal[0];
    if (*reinterpret_cast<float*>(self + 0xB18) > 0.1f)
        dot /= normal[2];

    float coeff = 0.0f;
    if (dot >= 0.0f)
    {
        using IntelCheck32Fn = uint8(__thiscall*)(void*);
        void* intel = m_pMoveBlend;
        coeff = reinterpret_cast<IntelCheck32Fn>((*reinterpret_cast<void***>(intel))[8])(intel) ? Core::StaticRef<float>(0x122F8CC) : 0.0f;
    }
    else
    {
        coeff = Core::StaticRef<float>(0x12300D0);
    }

    float vert = static_cast<float>(reinterpret_cast<GetMassFn>((*reinterpret_cast<void***>(collider))[14])(collider)) * (coeff * dot);
    if (vert > 50.0f)
        vert = 50.0f;
    else if (vert < -50.0f)
        vert = -50.0f;

    const int32 mover2 = Core::Hook::CallMethod<int32>(0x9E86F0, this);
    const float moverScale2 = static_cast<float>(reinterpret_cast<GetUnk36Fn>((*reinterpret_cast<void***>(mover2))[9])(mover2));
    float imp2[4]{ 0.0f, 0.0f, moverScale2 * vert, 0.0f };
    return Core::Hook::CallMethod<int32>(0xA47C30, this, imp2);
}

// 0x8A28B0
void __thiscall CPed::ProcessPhysicsStep()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (m_nRagdollState > 3)
        return;

    using ProcessCtrlFn = void(__thiscall*)(CPed*, float);
    using IntelCheck24Fn = uint8(__thiscall*)(void*);

    const float timeStep = m_fCurrentHeading;
    if ((m_nPedFlags2 & 1u) != 0 || *reinterpret_cast<float*>(*reinterpret_cast<uint32*>(self + 0x20) + 0x28) <= 0.99000001f)
    {
        if ((self[0x1E2] & 0xFu) < 2u && (m_nPedFlags2 & 4u) == 0)
            reinterpret_cast<ProcessCtrlFn>((*reinterpret_cast<void***>(this))[3])(this, timeStep);
    }
    else
    {
        const uint32 unk6C = *reinterpret_cast<uint32*>(self + 0x6C);
        void* intel = m_pMoveBlend;
        uint8* taskMgr = static_cast<uint8*>(m_pPedIntelligence) + 0x2E0;
        if (Core::Hook::CallMethod<uint8>(0x9C3090, m_pPedIntelligence)
            || ((!unk6C || !*reinterpret_cast<uint8*>(unk6C + 0xE)) && reinterpret_cast<IntelCheck24Fn>((*reinterpret_cast<void***>(intel))[6])(intel))
            || Core::Hook::CallMethod<uint8>(0x426910, taskMgr, 108, 0)
            || Core::Hook::CallMethod<uint8>(0x426910, taskMgr, 254, 0))
        {
            reinterpret_cast<ProcessCtrlFn>((*reinterpret_cast<void***>(this))[3])(this, timeStep);
        }
    }
}

// 0x8A2F00
int32 __thiscall CPed::UpdateSpeechAndAudio()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    using VFunc296Fn = uint8(__thiscall*)(CPed*);
    int32 result = reinterpret_cast<VFunc296Fn>((*reinterpret_cast<void***>(this))[74])(this);
    if (result)
    {
        result = *reinterpret_cast<int32*>(self + 0xB90);
        float timerLimit = m_fGroundSlopeTimer;
        const float dt = Core::StaticRef<float>(0x11DDE7C);
        if (result == 4)
        {
            if (timerLimit > 1.0f)
                m_fGroundSlopeTimer = timerLimit - dt * 0.60000002f;
        }
        else if (result == 3 && timerLimit > 2.0f)
        {
            m_fGroundSlopeTimer = timerLimit - dt * 0.30000001f;
        }
        else
        {
            const float target = (result == 3) ? 2.0f : 4.0f;
            if (target > timerLimit)
                m_fGroundSlopeTimer = timerLimit + dt * 0.15000001f;
        }

        const float elapsed = *reinterpret_cast<float*>(self + 0xB30) + dt;
        const float limit = m_fGroundSlopeTimer;
        *reinterpret_cast<float*>(self + 0xB30) = elapsed;
        if (elapsed > limit)
        {
            Core::Hook::CallMethod<int32>(0xAB0540, m_pedSubObj3D0, limit);
            const uint32 playerInfo = reinterpret_cast<uint32>(m_pPlayerInfo);
            const bool flag = playerInfo && ((*reinterpret_cast<uint32*>(playerInfo + 0x4BC) >> 5) & 1u) != 0;

            const int32 idx = Core::StaticRef<int32>(0x10F9AC0);
            const float* arr = &Core::StaticRef<float>(0x11F5D34);
            if (arr[132 * idx] < 10.0f && (m_nPedFlags2 & 4u) == 0)
            {
                if (!Core::Hook::CallMethod<uint8>(0x8335F0, reinterpret_cast<void*>(Core::Rebase(0x11DC4B0)), this)
                    && Core::Hook::CallMethod<uint8>(0x9E85A0, this)
                    && !flag)
                {
                    const int32 skel = Core::Hook::CallMethod<int32>(0x447AE0, this);
                    const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, skel, 1205);
                    if (boneIdx != -1)
                    {
                        const int32 boneMat = Core::Hook::CallMethod<int32>(0x447A60, this, boneIdx);
                        if (boneMat)
                            Core::Hook::CallMethod<int32>(0xC1E860, reinterpret_cast<void*>(Core::Rebase(0x16D7078)), this, boneMat);
                    }
                }
            }
            m_nGroundContactTimeMs = 0;
        }
    }

    const int32 audio = *reinterpret_cast<int32*>(self + 0x78);
    if ((*reinterpret_cast<uint32*>(audio + 0x1A44) & 0x400000u) != 0)
    {
        bool found = false;
        result = Core::Hook::CallMethod<int32>(0x9EBF60, reinterpret_cast<void*>(audio));
        if (result)
        {
            do
            {
                if ((*reinterpret_cast<uint32*>(result + 0x74) & 0x400000u) != 0 && (*reinterpret_cast<uint32*>(result + 4) & 0x10u) != 0)
                    found = true;
                result = Core::Hook::CallMethod<int32>(0x9EBFB0, reinterpret_cast<void*>(*reinterpret_cast<int32*>(self + 0x78)));
            } while (result);

            if (found)
            {
                const int32 skel = Core::Hook::CallMethod<int32>(0x447AE0, this);
                result = Core::Hook::Call<int32>(0xA4B8D0, skel, 1205);
                if (result != -1)
                {
                    result = Core::Hook::CallMethod<int32>(0x447A60, this, result);
                    if (result)
                        return Core::Hook::CallMethod<int32>(0xC1EA70, reinterpret_cast<void*>(Core::Rebase(0x16D7078)), this, result);
                }
            }
        }
    }
    return result;
}

// 0x8A3100
void __thiscall CPed::TriggerFootstepAudio(int32 isLeftFoot)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (Core::Hook::Call<double>(0xB1E4C0, *reinterpret_cast<int32*>(self + 0x20) + 0x30) > 225.0)
        return;

    const uint32 matId = *reinterpret_cast<const uint32*>(&m_nPhysUnk14C);
    void* matMgr = Core::StaticRef<void*>(0x18A87B8);
    using GetMatDataFn = uint8*(__thiscall*)(void*, uint32);
    auto getMatData = reinterpret_cast<GetMatDataFn>((*reinterpret_cast<void***>(matMgr))[5]);

    const uint16 surfType = *reinterpret_cast<const uint16*>(getMatData(matMgr, matId) + 0x20);
    if (surfType == 30 || surfType == 31)
        return;

    const int32 boneTag = static_cast<uint8>(isLeftFoot) ? 421 : 1200;
    const int32 skel = Core::Hook::CallMethod<int32>(0x447AE0, this);
    const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, skel, boneTag);
    const int32 skelData = Core::Hook::CallMethod<int32>(0x447A20, this);
    const float* boneMat = reinterpret_cast<const float*>(*reinterpret_cast<const uint32*>(skelData + 0x14) + (boneIdx << 6));
    const float zVal = (boneMat[5] + boneMat[4]) * 0.0f + boneMat[6];

    const uint16 matGroup = *reinterpret_cast<const uint16*>(getMatData(matMgr, matId) + 0x20);
    const int32 audioMat = Core::Hook::CallMethod<int32>(0xC1E550, reinterpret_cast<void*>(Core::Rebase(0x16D7078)), matGroup);
    if (audioMat)
    {
        const int16 modelIdx = m_nModelIndex;
        const uint32* modelInfos = &Core::StaticRef<uint32>(0x15F73B0);
        const bool isFemale = (*reinterpret_cast<const uint32*>(modelInfos[modelIdx] + 0x120) & 2u) != 0;
        const uint8 footIdx = static_cast<uint8>(isLeftFoot);
        Core::Hook::CallMethod<int32>(
            0xC1E560,
            reinterpret_cast<void*>(Core::Rebase(0x16D7078)),
            this,
            audioMat,
            matId,
            boneMat,
            isLeftFoot,
            isFemale,
            *reinterpret_cast<float*>(self + 4 * footIdx + 0x7B0),
            self[footIdx + 0x7AE],
            zVal > Core::StaticRef<float>(0xF21FEC));
    }
}

// 0x8A3250
int32 __thiscall CPed::UpdatePositionOrHeadLook()
{
    uint32* self = reinterpret_cast<uint32*>(this);
    using VFunc160Fn = int32(__thiscall*)(CPed*);
    using VFunc224Fn = int32(__thiscall*)(int32);
    using Sub80VFunc4Fn = int32(__thiscall*)(uint32*, const void*, int32);

    if (!Core::Hook::CallMethod<bool>(0x89C890, this))
        return reinterpret_cast<Sub80VFunc4Fn>((*reinterpret_cast<void***>(self + 32))[1])(self + 32, reinterpret_cast<const void*>(self[8] + 0x30), 0);

    int32 animObj = 0;
    const int32 v160 = reinterpret_cast<VFunc160Fn>((*reinterpret_cast<void***>(this))[40])(this);
    if (v160)
        animObj = reinterpret_cast<VFunc224Fn>((*reinterpret_cast<void***>(v160))[56])(v160);
    else
        animObj = static_cast<int32>(self[64]);

    const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, *reinterpret_cast<uint32*>(animObj + 4), 0);
    const int32 boneMat = Core::Hook::CallMethod<int32>(0x4D9100, this, boneIdx);
    const float by = *reinterpret_cast<const float*>(boneMat + 0x34);
    const float bz = *reinterpret_cast<const float*>(boneMat + 0x38);
    const float* m = reinterpret_cast<const float*>(self[8]);

    float outPos[4]{
        (m[8] * bz + m[4] * by + m[0] * 0.0f) + m[12],
        (m[9] * bz + m[5] * by + m[1] * 0.0f) + m[13],
        (m[10] * bz + m[6] * by + m[2] * 0.0f) + m[14],
        0.0f
    };
    return reinterpret_cast<Sub80VFunc4Fn>((*reinterpret_cast<void***>(self + 32))[1])(self + 32, outPos, 0);
}

// 0x8A33F0
bool __thiscall CPed::CanConvertToDummy(bool allowFlag4, bool checkStreamed)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (checkStreamed)
    {
        uint8* unk6C = *reinterpret_cast<uint8**>(self + 0x6C);
        if (unk6C)
        {
            using VFunc96Fn = uint8(__thiscall*)(uint8*);
            if (unk6C[0xE] || !reinterpret_cast<VFunc96Fn>((*reinterpret_cast<void***>(unk6C))[24])(unk6C))
                return false;
        }
    }

    if ((m_nPedFlags2 & 4u) == 0 || allowFlag4)
    {
        const int32 pInfo = *reinterpret_cast<const int32*>(Core::Hook::Call<int32>(0x8788D0, 0) + 0x228);
        const int32 target = pInfo ? (pInfo + 0x60) : 0;
        const int32 idx = *reinterpret_cast<const int32*>(target + 0x3D4);
        if ((idx < 0 || !Core::Hook::CallMethod<uint8>(0xA66C40, reinterpret_cast<void*>(Core::Rebase(0x1662178u) + static_cast<uint32>(idx) * 0x5Cu), this))
            && (m_nDummyConvertState == 1 || static_cast<uint32>(m_nDummyConvertState) - 2u >= 2u))
        {
            return true;
        }
    }
    return false;
}

// 0x8A3850
int32 __thiscall CPed::PreRenderPedInternal()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (Core::Hook::Call<uint8>(0x444370) && !Core::StaticRef<uint8>(0x10C7CD0))
        return 0;

    using VFunc208Fn = void*(__thiscall*)(CPed*);
    using VFunc204Fn = void(__thiscall*)(CPed*);
    void* info208_0 = reinterpret_cast<VFunc208Fn>((*reinterpret_cast<void***>(this))[52])(this);
    Core::Hook::CallMethod<int32>(0xA66860, info208_0, this);

    const int16 modelIdx = m_nModelIndex;
    const uint32 modelInfo = (&Core::StaticRef<uint32>(0x15F73B0))[modelIdx];
    if (modelInfo && *reinterpret_cast<const uint8*>(modelInfo + 0x8C) && self[0x219])
    {
        void* drawHandler = *reinterpret_cast<void**>(*reinterpret_cast<uint32*>(self + 0xEA8) + 0x58);
        using DrawVFunc12Fn = void(__thiscall*)(void*, CPed*);
        reinterpret_cast<DrawVFunc12Fn>((*reinterpret_cast<void***>(drawHandler))[3])(drawHandler, this);
    }

    if (Core::StaticRef<int32>(0x10D0170) == 1
        || Core::StaticRef<int32>(0x10E1984) != -1
        || Core::StaticRef<int32>(0xF0E4A0) == 18)
    {
        return 0;
    }

    if (Core::Hook::CallMethod<uint8>(0x9C3090, m_pPedIntelligence))
        *reinterpret_cast<uint32*>(self + 0xBF0) &= ~2u;

    if (Core::Hook::CallMethod<uint8>(0x9C2680, m_pPedIntelligence))
    {
        *reinterpret_cast<uint32*>(self + 0xBF0) &= ~2u;
    }
    else if (((*reinterpret_cast<uint32*>(self + 0xBF0) & 2u) != 0 || (!self[0x219] && *reinterpret_cast<float*>(self + 0xC08) != 0.0f))
        && !Core::Hook::Call<uint8>(0x444370)
        && (*reinterpret_cast<uint32*>(self + 0x28) & 0x800000u) == 0)
    {
        Core::Hook::CallMethod<int32>(0x952250, self + 0xBC0);
    }

    if ((*reinterpret_cast<int16*>(self + 0xF4) & 2) != 0)
    {
        using SubBC0VFunc4Fn = void(__thiscall*)(void*);
        reinterpret_cast<SubBC0VFunc4Fn>((*reinterpret_cast<void***>(self + 0xBC0))[1])(self + 0xBC0);
    }

    m_nRagdollAnimFlags |= 0x1000000u;
    const int32 res = Core::Hook::CallMethod<int32>(0x9DFF40, this);
    if (!res)
        reinterpret_cast<VFunc204Fn>((*reinterpret_cast<void***>(this))[51])(this);
    return res;
}

// 0x8A3990
void* __thiscall CPed::PostRenderPedInternal()
{
    uint32* self = reinterpret_cast<uint32*>(this);
    if (Core::Hook::Call<uint8>(0x444370))
        return nullptr;

    if (Core::StaticRef<int32>(0x10D0170) == 1
        || Core::StaticRef<int32>(0x10E1984) != -1
        || Core::StaticRef<int32>(0xF0E4A0) == 18)
    {
        return nullptr;
    }

    Core::Hook::CallMethod<int32>(0x9C31D0, reinterpret_cast<void*>(self[137]));
    uint32* pInfo = reinterpret_cast<uint32*>(self[138]);
    if (pInfo)
    {
        uint32* sub = pInfo + 24;
        sub[3] = 0;
        return sub;
    }
    return nullptr;
}

// 0x8A39E0
void __thiscall CPed::SyncRagdollToMatrix()
{
    uint32* self = reinterpret_cast<uint32*>(this);
    if (m_nRagdollState != 1)
        return;

    const float* src = reinterpret_cast<const float*>(self[8]);
    float* dst = reinterpret_cast<float*>(reinterpret_cast<uint32>(m_pRagdollInst)) + 4;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[8] = src[8];
    dst[9] = src[9];
    dst[10] = src[10];
    dst[12] = src[12];
    dst[13] = src[13];
    dst[14] = src[14];

    void* ragdollInst = reinterpret_cast<void*>(reinterpret_cast<uint32>(m_pRagdollInst));
    using RagVFunc16Fn = void(__thiscall*)(void*, uint32);
    using RagVFunc188Fn = void(__thiscall*)(void*);
    reinterpret_cast<RagVFunc16Fn>((*reinterpret_cast<void***>(ragdollInst))[4])(ragdollInst, self[8]);

    using VFunc160Fn = int32(__thiscall*)(CPed*);
    using VFunc224Fn = int32(__thiscall*)(int32);
    int32 animObj = 0;
    const int32 v160 = reinterpret_cast<VFunc160Fn>((*reinterpret_cast<void***>(this))[40])(this);
    if (v160)
        animObj = reinterpret_cast<VFunc224Fn>((*reinterpret_cast<void***>(v160))[56])(v160);
    else
        animObj = static_cast<int32>(self[64]);

    Core::Hook::CallMethod<int32>(0x63DD30, reinterpret_cast<void*>(animObj));
    reinterpret_cast<RagVFunc188Fn>((*reinterpret_cast<void***>(ragdollInst))[47])(ragdollInst);
    reinterpret_cast<RagVFunc188Fn>((*reinterpret_cast<void***>(ragdollInst))[47])(ragdollInst);
    Core::Hook::CallMethod<int32>(0x89C390, this, 2, 1);

    const uint32 bound = *reinterpret_cast<const uint32*>(*reinterpret_cast<const uint32*>(reinterpret_cast<uint32>(m_pRagdollInst) + 4) + 0xC);
    const float prevVal = *reinterpret_cast<const float*>(bound + 8);
    Core::Hook::CallMethod<int32>(0x688F70, reinterpret_cast<void*>(bound), 1);
    const uint16 levelIdx = *reinterpret_cast<const uint16*>(reinterpret_cast<uint32>(m_pRagdollInst) + 8);
    void* physLevel = Core::StaticRef<void*>(0x1666880);
    if (prevVal == *reinterpret_cast<const float*>(bound + 8))
        Core::Hook::CallMethod<int32>(0x6A3960, physLevel, levelIdx, 0);
    else
        Core::Hook::CallMethod<int32>(0x6A4470, physLevel, levelIdx, prevVal, 0);
}

// 0x8A3B30
int32 __thiscall CPed::RenderPed(int32 pass, int32 arg3, uint8 alpha, int32 arg5)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (pass == 1 && Core::StaticRef<uint8>(0x1135BCC))
        return 0;

    const int16 modelIdx = m_nModelIndex;
    const uint32 modelInfo = (&Core::StaticRef<uint32>(0x15F73B0))[modelIdx];
    using VFunc208Fn = uint32(__thiscall*)(CPed*);
    const uint32 info208 = reinterpret_cast<VFunc208Fn>((*reinterpret_cast<void***>(this))[52])(this);
    const bool flag8 = ((*reinterpret_cast<const uint8*>(info208 + 0x72) & 8u) != 0)
        || (*reinterpret_cast<const uint8*>(*reinterpret_cast<const uint32*>(modelInfo + 0x128) + 2) == 0);

    if ((*reinterpret_cast<const int16*>(self + 0x24) & 0x20) == 0 || (*reinterpret_cast<const int16*>(self + 0x29C) & 2) != 0)
        return 0;

    const uint8 isSpecial = *reinterpret_cast<const uint8*>(modelInfo + 0x8C);
    if (isSpecial && (*reinterpret_cast<const uint32*>(Core::StaticRef<uint32>(0x15349DC) + 0x8E8) & 0x1000000u) == 0)
        return 0;

    if ((m_nPedFlags2 & 4u) != 0)
    {
        const uint32 veh = reinterpret_cast<uint32>(m_pMyVehicle);
        if (veh && *reinterpret_cast<const uint32*>(veh + 0x6C) && self[0x219])
        {
            const uint8 vehAlpha = *reinterpret_cast<const uint8*>(Core::Hook::CallMethod<int32>(0x426AB0, this) + 0xFE8);
            if (alpha >= vehAlpha)
                alpha = vehAlpha;
        }
    }

    int32 drawCmd = 0;
    bool enqueueCmd = false;
    if (isSpecial && self[0x219])
    {
        const int32 veh = Core::Hook::CallMethod<int32>(0x426AB0, this);
        *reinterpret_cast<uint8*>(*reinterpret_cast<uint32*>(self + 0xEA8) + 0x5C) = (veh && *reinterpret_cast<const int32*>(veh + 0x1354) == 2) ? 1 : 0;
        float fVal = 0.0f;
        if (!Core::Hook::Call<uint8>(0x444370))
            fVal = *reinterpret_cast<const float*>(self + 0xE1C);
        if (pass != 3 || alpha > 0x61u)
        {
            if (void* cmdMem = Core::Hook::Call<void*>(0x7BDD80, 944, 0))
            {
                const int8 b91 = static_cast<int8>(self[0x5B]);
                const int8 c1 = static_cast<int8>(Core::Hook::CallMethod<int32>(0x4479A0, this));
                const int32 skelData = Core::Hook::CallMethod<int32>(0x447A20, this);
                drawCmd = Core::Hook::CallMethod<int32>(0x7C2B00, cmdMem, static_cast<int32>(modelIdx), *reinterpret_cast<int32*>(self + 0xEA8), skelData, pass, static_cast<int32>(alpha), b91, c1, arg3, fVal, 0);
            }
            enqueueCmd = true;
        }
    }
    else
    {
        const int32 drawable = *reinterpret_cast<const int32*>(*reinterpret_cast<const uint32*>(self + 0x34) + 4);
        const uint32 info208b = reinterpret_cast<VFunc208Fn>((*reinterpret_cast<void***>(this))[52])(this);
        if ((*reinterpret_cast<const uint8*>(info208b + 0x72) & 0x10u) == 0)
        {
            float fVal = 0.0f;
            if (!Core::Hook::Call<uint8>(0x444370))
                fVal = *reinterpret_cast<const float*>(self + 0xE1C);
            const int32 extra = Core::Hook::CallMethod<uint8>(0x9E2390, self + 0x238) ? *reinterpret_cast<const int32*>(self + 0x254) : 0;
            if (void* cmdMem = Core::Hook::Call<void*>(0x7BDD80, 160, 0))
            {
                const uint32 flags28 = (*reinterpret_cast<const uint32*>(self + 0x28) >> 28) & 0xFFFFFF01u;
                const int8 b91 = static_cast<int8>(self[0x5B]);
                const int8 c1 = static_cast<int8>(Core::Hook::CallMethod<int32>(0x4479A0, this));
                const int32 skelData = Core::Hook::CallMethod<int32>(0x447A20, this);
                drawCmd = Core::Hook::CallMethod<int32>(0x7C2BF0, cmdMem, static_cast<int32>(modelIdx), *reinterpret_cast<int32*>(self + 0x21C), skelData, drawable, pass, static_cast<int32>(alpha), b91, c1, static_cast<int8>(flag8), arg3, 0, static_cast<int8>(flags28), fVal, extra);
            }
            enqueueCmd = true;
        }
        else if (!pass)
        {
            if (void* cmdMem = Core::Hook::Call<void*>(0x7BDD80, 28, 0))
            {
                const int8 b91 = static_cast<int8>(self[0x5B]);
                const int32 skelData = Core::Hook::CallMethod<int32>(0x447A20, this);
                drawCmd = Core::Hook::CallMethod<int32>(0x7C30E0, cmdMem, skelData, 0, static_cast<int8>(alpha), b91, arg3);
            }
            enqueueCmd = true;
        }
    }

    if (enqueueCmd)
        Core::Hook::CallMethod<int32>(0x404E90, reinterpret_cast<void*>(drawCmd));

    const uint32 info208c = reinterpret_cast<VFunc208Fn>((*reinterpret_cast<void***>(this))[52])(this);
    if ((*reinterpret_cast<const uint8*>(info208c + 0x72) & 0x20u) != 0 || self[0x219])
    {
        int32 cmd2 = 0;
        if (void* cmdMem = Core::Hook::Call<void*>(0x7BDD80, 192, 0))
        {
            const uint32 flags28 = (*reinterpret_cast<const uint32*>(self + 0x28) >> 28) & 0xFFFFFF01u;
            const int8 b91 = static_cast<int8>(self[0x5B]);
            const int8 c1 = static_cast<int8>(Core::Hook::CallMethod<int32>(0x4479A0, this));
            const int32 skelData = Core::Hook::CallMethod<int32>(0x447A20, this);
            cmd2 = Core::Hook::CallMethod<int32>(0x7C2D50, cmdMem, static_cast<int32>(modelIdx), skelData, *reinterpret_cast<int32*>(self + 0x21C) + 0x80, pass, static_cast<int32>(alpha), b91, c1, static_cast<int8>(flag8), arg3, static_cast<int8>(flags28));
        }
        Core::Hook::CallMethod<int32>(0x404E90, reinterpret_cast<void*>(cmd2));
    }
    return 0;
}

// 0x8A3ED0
bool __thiscall CPed::TestLineOfSightToEntity(int32 entityA, int32 entityB)
{
    uint32 excludeList[6]{};
    int32 excludeCount = 0;
    using VFunc184Fn = void(__thiscall*)(CPed*, uint32*, int32*, int32, int32);
    reinterpret_cast<VFunc184Fn>((*reinterpret_cast<void***>(this))[46])(this, excludeList, &excludeCount, 4, entityB);

    excludeList[excludeCount] = reinterpret_cast<uint32>(this);
    excludeList[++excludeCount] = static_cast<uint32>(entityB);
    ++excludeCount;

    const uint32 unkX = Core::StaticRef<uint32>(0x18A8C50);
    const uint32 unkY = Core::StaticRef<uint32>(0x18A8C54);
    const uint32 unkZ = Core::StaticRef<uint32>(0x18A8C58);

    uint32 probe[24]{};
    probe[4] = unkX;
    probe[5] = unkY;
    probe[6] = unkZ;
    probe[8] = unkX;
    probe[9] = unkY;
    probe[10] = unkZ;
    probe[12] = unkX;
    probe[13] = unkY;
    probe[14] = unkZ;
    *reinterpret_cast<int16*>(&probe[19]) = -1;

    if (!Core::Hook::Call<uint8>(0x99BB10, entityA, excludeList, excludeCount, 142, -1))
    {
        const uint32 matB = *reinterpret_cast<const uint32*>(entityB + 0x20);
        const int32 posB = matB ? (matB + 0x30) : (entityB + 0x10);
        if (!Core::Hook::Call<int32>(0x996D80, posB, entityA + 0x30, excludeList, probe, 142, 1, excludeCount, 0))
            return true;
    }
    return false;
}

// 0x8A4020
bool __thiscall CPed::FindVehicleExitPosition(int32 exitType, int32 refMatrix, float* outMatrix)
{
    const uint32* self = reinterpret_cast<const uint32*>(this);
    const uint32 veh = self[111];
    const float* vehMat = *reinterpret_cast<const float**>(veh + 0x20);
    const float* refMat = reinterpret_cast<const float*>(refMatrix);

    float sideSign = 1.0f;
    const float dot = vehMat[2] * (refMat[14] - vehMat[14]) + vehMat[1] * (refMat[13] - vehMat[13]) + vehMat[0] * (refMat[12] - vehMat[12]);
    if (dot < 0.0f)
        sideSign = -1.0f;

    const uint32* modelInfos = &Core::StaticRef<uint32>(0x15F73B0);
    const uint32 vehModelInfo = modelInfos[*reinterpret_cast<const int16*>(veh + 0x2E)];
    const float sideDist = (*reinterpret_cast<const float*>(vehModelInfo + 0x30) + 0.44999999f) * sideSign;
    const float sx = vehMat[0] * sideDist;
    const float sy = vehMat[1] * sideDist;
    const float sz = vehMat[2] * sideDist;

    outMatrix[0] = refMat[0];
    outMatrix[1] = refMat[1];
    outMatrix[2] = refMat[2];
    outMatrix[4] = refMat[4];
    outMatrix[5] = refMat[5];
    outMatrix[6] = refMat[6];
    outMatrix[8] = refMat[8];
    outMatrix[9] = refMat[9];
    outMatrix[10] = refMat[10];
    outMatrix[12] = refMat[12];
    outMatrix[13] = refMat[13];
    outMatrix[14] = refMat[14];

    auto copyVehPos = [&]() {
        const float* vm = *reinterpret_cast<const float**>(veh + 0x20);
        outMatrix[12] = vm[12];
        outMatrix[13] = vm[13];
        outMatrix[14] = refMat[14];
        outMatrix[15] = vm[15];
        return vm;
    };

    switch (exitType)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    {
        const float* vm = copyVehPos();
        float fwdOffset = 0.0f;
        if (exitType == 1)
            fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x24) * 0.5f;
        else if (exitType == 2)
            fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x34) * 0.5f;
        else if (exitType == 3)
            fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x24);
        else
            fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x34);

        outMatrix[12] += vm[4] * fwdOffset + sx;
        outMatrix[13] += vm[5] * fwdOffset + sy;
        outMatrix[14] += vm[6] * fwdOffset + sz;
        return true;
    }
    case 5:
        outMatrix[12] -= sx * 2.0f;
        outMatrix[13] -= sy * 2.0f;
        outMatrix[14] -= sz * 2.0f;
        return true;
    case 6:
    {
        const float* vm = copyVehPos();
        const float fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x34) + 0.44999999f;
        outMatrix[12] += vm[4] * fwdOffset;
        outMatrix[13] += vm[5] * fwdOffset;
        outMatrix[14] += vm[6] * fwdOffset;
        return true;
    }
    case 7:
    {
        const float* vm = copyVehPos();
        const float fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x24) - 0.44999999f;
        outMatrix[12] += vm[4] * fwdOffset;
        outMatrix[13] += vm[5] * fwdOffset;
        outMatrix[14] += vm[6] * fwdOffset;
        return true;
    }
    case 8:
    {
        const float* vm = copyVehPos();
        const float fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x34) + 0.44999999f;
        outMatrix[12] += vm[4] * fwdOffset - sx;
        outMatrix[13] += vm[5] * fwdOffset - sy;
        outMatrix[14] += vm[6] * fwdOffset - sz;
        return true;
    }
    case 9:
    {
        const float* vm = copyVehPos();
        const float fwdOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x24) - 0.44999999f;
        outMatrix[12] += vm[4] * fwdOffset - sx;
        outMatrix[13] += vm[5] * fwdOffset - sy;
        outMatrix[14] += vm[6] * fwdOffset - sz;
        return true;
    }
    case 10:
    {
        const float* vm = *reinterpret_cast<const float**>(veh + 0x20);
        const float upOffset = *reinterpret_cast<const float*>(vehModelInfo + 0x38) + 1.0f;
        outMatrix[12] = vm[8] * upOffset;
        outMatrix[13] = vm[9] * upOffset;
        outMatrix[14] = vm[10] * upOffset;
        Core::Hook::CallMethod<int32>(0x48A330, const_cast<float*>(vm), outMatrix + 12);
        return true;
    }
    case 11:
    case 12:
    {
        const float searchRadius = (exitType == 11) ? 40.0f : 100.0f;
        float searchPos[4]{ refMat[12], refMat[13], refMat[14] + 2.0f, 0.0f };
        float foundPos[4]{};
        if (!Core::Hook::Call<int32>(0xAA66C0, searchPos, foundPos, searchRadius, 10, 0, 0, 0))
            return false;
        outMatrix[12] = foundPos[0];
        outMatrix[13] = foundPos[1];
        outMatrix[14] = foundPos[2];
        outMatrix[15] = foundPos[3];
        return true;
    }
    case 13:
    {
        struct NodeSearchResult
        {
            uint16 area;
            uint16 node;
            uint32 pad[7];
            uint32 wVal;
        } nodeRes{};
        Core::Hook::CallMethod<int32>(0xA06B20, reinterpret_cast<void*>(Core::Rebase(0x1640360)), &nodeRes, outMatrix + 12, 99999.898f, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 3.0f, -1, 0);
        if (nodeRes.area == 0xFFFF)
            return false;
        const uint32* regionPtrs = &Core::StaticRef<uint32>(0x1640B64);
        const int16* node = reinterpret_cast<const int16*>(regionPtrs[nodeRes.area] + 32 * nodeRes.node);
        outMatrix[12] = static_cast<float>(node[10]) * 0.125f;
        outMatrix[13] = static_cast<float>(node[11]) * 0.125f;
        outMatrix[14] = static_cast<float>(node[12]) * 0.015625f;
        *reinterpret_cast<uint32*>(&outMatrix[15]) = nodeRes.wVal;
        return true;
    }
    default:
        return true;
    }
}

// 0x8A4BE0
int32* __thiscall CPed::GetBonePosPtr(int32 boneId)
{
    uint32* self = reinterpret_cast<uint32*>(this);
    using VFunc160Fn = int32(__thiscall*)(CPed*);
    using VFunc224Fn = int32(__thiscall*)(int32);

    int32 animObj = 0;
    const int32 v160 = reinterpret_cast<VFunc160Fn>((*reinterpret_cast<void***>(this))[40])(this);
    if (v160)
        animObj = reinterpret_cast<VFunc224Fn>((*reinterpret_cast<void***>(v160))[56])(v160);
    else
        animObj = static_cast<int32>(self[64]);

    if (!animObj)
        return reinterpret_cast<int32*>(self[8] + 0x30);

    const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, *reinterpret_cast<uint32*>(animObj + 4), boneId);
    return Core::Hook::CallMethod<int32*>(0x89E980, this, boneIdx) + 12;
}

// 0x8A4C40
bool __thiscall CPed::GetBonePosition(void* outPos, int32 boneId)
{
    uint32* self = reinterpret_cast<uint32*>(this);
    using VFunc160Fn = int32(__thiscall*)(CPed*);
    using VFunc224Fn = int32(__thiscall*)(int32);

    int32 animObj = 0;
    const int32 v160 = reinterpret_cast<VFunc160Fn>((*reinterpret_cast<void***>(this))[40])(this);
    if (v160)
        animObj = reinterpret_cast<VFunc224Fn>((*reinterpret_cast<void***>(v160))[56])(v160);
    else
        animObj = static_cast<int32>(self[64]);

    float* dst = static_cast<float*>(outPos);
    if (animObj)
    {
        const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, *reinterpret_cast<uint32*>(animObj + 4), boneId);
        const float* mat = Core::Hook::CallMethod<const float*>(0x89E980, this, boneIdx);
        dst[0] = mat[12];
        dst[1] = mat[13];
        dst[2] = mat[14];
        dst[3] = mat[15];
        return (reinterpret_cast<const uint8*>(self)[0x29F] & 1u) != 0;
    }

    const float* m = reinterpret_cast<const float*>(self[8]);
    dst[0] = m[12];
    dst[1] = m[13];
    dst[2] = m[14];
    dst[3] = m[15];
    return false;
}

// 0x8A4D00
bool __thiscall CPed::GetBoneMatrixOut(float* outMatrix, int32 boneId)
{
    uint32* self = reinterpret_cast<uint32*>(this);
    using VFunc160Fn = int32(__thiscall*)(CPed*);
    using VFunc224Fn = int32(__thiscall*)(int32);

    int32 animObj = 0;
    const int32 v160 = reinterpret_cast<VFunc160Fn>((*reinterpret_cast<void***>(this))[40])(this);
    if (v160)
        animObj = reinterpret_cast<VFunc224Fn>((*reinterpret_cast<void***>(v160))[56])(v160);
    else
        animObj = static_cast<int32>(self[64]);

    if (animObj)
    {
        const int32 boneIdx = Core::Hook::Call<int32>(0xA4B8D0, *reinterpret_cast<uint32*>(animObj + 4), boneId);
        const float* src = Core::Hook::CallMethod<const float*>(0x89E980, this, boneIdx);
        outMatrix[0] = src[0];
        outMatrix[1] = src[1];
        outMatrix[2] = src[2];
        outMatrix[4] = src[4];
        outMatrix[5] = src[5];
        outMatrix[6] = src[6];
        outMatrix[8] = src[8];
        outMatrix[9] = src[9];
        outMatrix[10] = src[10];
        outMatrix[12] = src[12];
        outMatrix[13] = src[13];
        outMatrix[14] = src[14];
        return (reinterpret_cast<const uint8*>(self)[0x29F] & 1u) != 0;
    }

    const float* src = reinterpret_cast<const float*>(self[8]);
    outMatrix[0] = src[0];
    outMatrix[1] = src[1];
    outMatrix[2] = src[2];
    outMatrix[4] = src[4];
    outMatrix[5] = src[5];
    outMatrix[6] = src[6];
    outMatrix[8] = src[8];
    outMatrix[9] = src[9];
    outMatrix[10] = src[10];
    outMatrix[12] = src[12];
    outMatrix[13] = src[13];
    outMatrix[14] = src[14];
    return false;
}

// 0x8A4E30
bool __thiscall CPed::GetBonePositionUpdated(void* outPos, int32 boneId, bool forceUpdate)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (forceUpdate && (self[0x29F] & 1u) == 0)
    {
        Core::Hook::CallMethod<int32>(0x9E1190, this);
        m_nRagdollAnimFlags |= 0x1000000u;
    }
    return GetBonePosition(outPos, boneId);
}

// 0x8A4E70
bool __thiscall CPed::GetBoneMatrixUpdated(float* outMatrix, int32 boneId, bool forceUpdate)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (forceUpdate && (self[0x29F] & 1u) == 0)
    {
        Core::Hook::CallMethod<int32>(0x9E1190, this);
        m_nRagdollAnimFlags |= 0x1000000u;
    }
    return GetBoneMatrixOut(outMatrix, boneId);
}

// 0x8A5060
void* __thiscall CPed::GetHeadOrCenterPosition(void* outPos)
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    const uint32 phInst = *reinterpret_cast<const uint32*>(self + 0x38);
    float* dst = static_cast<float*>(outPos);

    if ((self[0xF4] & 0x40u) != 0 || (phInst && phInst == *reinterpret_cast<const uint32*>(self + 0x7C4)))
    {
        const int16 modelIdx = m_nModelIndex;
        const uint32 modelInfo = (&Core::StaticRef<uint32>(0x15F73B0))[modelIdx];
        using GetBoneIdxFn = int32(__thiscall*)(uint32, int32);
        auto getBoneIdx = reinterpret_cast<GetBoneIdxFn>((*reinterpret_cast<void***>(modelInfo))[14]);

        const float* m6 = Core::Hook::CallMethod<const float*>(0x447A60, const_cast<CPed*>(this), getBoneIdx(modelInfo, 6));
        const float* m3 = Core::Hook::CallMethod<const float*>(0x447A60, const_cast<CPed*>(this), getBoneIdx(modelInfo, 3));
        dst[0] = m6[12] * 0.60000002f + m3[12] * 0.40000001f;
        dst[1] = m6[13] * 0.60000002f + m3[13] * 0.40000001f;
        dst[2] = m6[14] * 0.60000002f + m3[14] * 0.40000001f;
        dst[3] = 0.0f;
    }
    else
    {
        const float* mat = *reinterpret_cast<const float* const*>(self + 0x20);
        dst[0] = mat[12];
        dst[1] = mat[13];
        dst[2] = mat[14];
        dst[3] = mat[15];
    }
    return outPos;
}

// 0x8A54B0
int32 __thiscall CPed::GetClimbOrStepHeight()
{
    const uint32* self = reinterpret_cast<const uint32*>(this);
    float headMat[16]{};
    GetBoneMatrixUpdated(headMat, 1202, true);

    const float upZ = headMat[6];
    const float absUpZ = (upZ >= 0.0f) ? upZ : -upZ;
    if (absUpZ <= 0.60000002f)
    {
        const uint32 phInst = self[14];
        if (phInst && phInst == reinterpret_cast<uint32>(m_pRagdollInst))
        {
            float footMat[16]{};
            GetBoneMatrixUpdated(footMat, 420, true);
            const float footZ1 = footMat[14];
            GetBoneMatrixUpdated(footMat, 425, true);
            const float footZ2 = footMat[14];
            if ((headMat[14] - 0.5f) > footZ1 || (headMat[14] - 0.5f) > footZ2)
                return 4;
        }
        else
        {
            const float matZ = *reinterpret_cast<const float*>(self[8] + 0x38);
            const float dz = headMat[14] - matZ;
            const float absDz = (dz < 0.0f) ? -dz : dz;
            if (absDz < 0.34999999f)
                return 4;
        }
    }
    return GetDominantAxisDir(headMat);
}

// 0x8A56D0
int32 __thiscall CPed::UpdateTaskTimerD3C(int8 flag)
{
    uint32* self = reinterpret_cast<uint32*>(this);
    const int32 res = Core::Hook::Call<int32>(0x41EF00, self + 848, flag);
    self[847] = Core::StaticRef<uint32>(0x11DDE74);
    return res;
}

// 0x8A5700
bool __thiscall CPed::HasHeldWeaponObject()
{
    uint32* self = reinterpret_cast<uint32*>(this);
    if ((self[153] & 0x8000u) == 0)
        return false;
    const int32 weaponIdx = Core::Hook::CallMethod<int32>(0x89E250, this);
    return weaponIdx >= 0 && Core::Hook::Call<int32>(0x9E3000, this, 0) == weaponIdx;
}

// 0x8A5740
void __thiscall CPed::SetWeaponVariation(int32 variation)
{
    m_nPedUnkE54 = -1;
    if (variation >= 0)
    {
        const int32 weaponIdx = Core::Hook::CallMethod<int32>(0x89E250, this);
        if (weaponIdx >= 0 && variation < Core::Hook::Call<uint8>(0x9E2810, this, 0, weaponIdx))
            m_nPedUnkE54 = variation;
    }
}

// 0x8A5780
int32 __thiscall CPed::GetWeaponVariation(int32 slot)
{
    if (m_nPedUnkE54 != -1)
        return m_nPedUnkE54;

    if (HasHeldWeaponObject())
        return Core::Hook::Call<int32>(0x9E3020, this, 0);

    const int32 count = Core::Hook::Call<uint8>(0x9E2810, this, 0, slot);
    return static_cast<int32>((static_cast<float>(Core::Hook::Call<uint16>(0xD0D21A)) * 0.000030517578f) * static_cast<float>(count));
}

// 0x8A57E0
int32 __thiscall CPed::SetWeaponDrawable(int32 drawable, int32 texture)
{
    int32 clamped = static_cast<int32>(Core::Hook::Call<uint8>(0x9E27D0, this, 0));
    if (drawable < -1)
        clamped = -1;
    else if (drawable < clamped)
        clamped = drawable;

    m_nPedUnkE58 = clamped;
    if (clamped == -1)
    {
        m_nPedUnkE5C = -1;
        return -1;
    }
    m_nPedUnkE5C = texture;
    return texture;
}

// 0x8A5830
bool __thiscall CPed::ProcessAnimMoveAndTurn(int32 animData, float* outWeight1, float* outWeight2, float* outWeight3, float* outWeight4, int32 outDeltaPos, float* outHeading, uint8* outFlag1, uint8* outFlag2, uint8* outFlag3, uint8* outFlag4)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *outWeight1 = 1.0f;
    *outWeight2 = 1.0f;
    *outWeight3 = 1.0f;
    *outWeight4 = 1.0f;
    *reinterpret_cast<uint32*>(outDeltaPos) = 0;
    *reinterpret_cast<uint32*>(outDeltaPos + 4) = 0;
    *reinterpret_cast<uint32*>(outDeltaPos + 8) = 0;
    if (outHeading)
        *outHeading = 0.0f;
    *outFlag1 = 0;
    *outFlag2 = 0;
    *outFlag4 = 0;

    float phase = *reinterpret_cast<const float*>(animData + 0x4C);
    if (phase > 0.99000001f)
        phase = 1.0f;

    void* animClip = (*reinterpret_cast<const int16*>(animData + 0x44) == 1)
        ? *reinterpret_cast<void* const*>(animData + 0x40)
        : nullptr;

    const int32 animId = *reinterpret_cast<const int32*>(animData + 0xC);
    switch (animId)
    {
    case 311:
    case 312:
    case 313:
    case 314:
        *outWeight1 = phase;
        *outWeight2 = 0.0f;
        *outWeight3 = 0.0f;
        *outWeight4 = 0.0f;
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;

    case 315:
    case 316:
    case 317:
    case 318:
        *outWeight1 = 1.0f;
        *outWeight2 = 0.0f;
        *outWeight3 = 0.0f;
        *outWeight4 = 0.0f;
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;

    case 319:
    case 320:
    case 321:
    case 322:
    case 323:
    case 324:
    case 360:
    case 361:
    case 364:
    {
        float tMin = 0.5f;
        float tMax = 0.80000001f;
        if (animId == 360 || animId == 361)
        {
            tMin = 2.0f;
            tMax = 2.0f;
        }
        if (*reinterpret_cast<const int32*>(animData + 0x10) == 83 && (animId == 319 || animId == 320))
        {
            tMin = 0.2f;
            tMax = 0.40000001f;
        }
        if (Core::Hook::CallMethod<int32>(0x58ABB0, animClip, 128, 0))
        {
            Core::Hook::CallMethod<int32>(0xA4C8F0, reinterpret_cast<void*>(animData), 1, &tMin, 0.0f, 1.0f);
            Core::Hook::CallMethod<int32>(0xA4C8F0, reinterpret_cast<void*>(animData), 2, &tMax, 0.0f, 1.0f);
        }
        *outWeight1 = 1.0f;
        *outWeight2 = phase;
        *outWeight4 = phase;
        *outWeight3 = 0.0f;
        if (*reinterpret_cast<const float*>(animData + 0x4C) > tMin)
        {
            float w = (phase - tMin) / (tMax - tMin);
            if (w >= 1.0f)
                w = 1.0f;
            *outWeight3 = w;
        }
        if ((self[0x1E2] & 0xFu) >= 2u && *reinterpret_cast<uint32*>(self + 0x1BC))
        {
            const uint32 veh = Core::Hook::CallMethod<uint32>(0x426800, this);
            if ((*reinterpret_cast<const uint32*>(veh + 0x28) & 0x3C0u) == 0x80u
                && *reinterpret_cast<const uint32*>(veh + 0x1354) == 1)
            {
                *reinterpret_cast<float*>(self + 0x288) = 1.0f - *outWeight3;
            }
        }
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;
    }

    case 325:
    case 326:
    case 327:
    case 328:
    case 406:
    case 407:
    case 408:
    case 409:
    case 410:
    case 411:
    {
        if (animId >= 406)
        {
            const float fE08 = *reinterpret_cast<float*>(self + 0xE08);
            const float fE0C = *reinterpret_cast<float*>(self + 0xE0C);
            float scale = 1.0f;
            if (animId == 406 || animId == 409)
            {
                const float s = phase * 2.0f;
                scale = (s > 1.0f) ? 1.0f : ((s >= 0.0f) ? s : 0.0f);
            }
            else if (animId == 408 || animId == 411)
            {
                const float s = 1.0f - phase * 2.0f;
                scale = (s > 1.0f) ? 1.0f : ((s >= 0.0f) ? s : 0.0f);
            }
            *reinterpret_cast<float*>(outDeltaPos) = (animId <= 408) ? -(scale * fE0C) : (scale * fE0C);
            *reinterpret_cast<float*>(outDeltaPos + 8) = scale * fE08;
        }
        *outWeight1 = 1.0f;
        *outWeight2 = 1.0f;
        *outWeight3 = 1.0f;
        *outWeight4 = 1.0f;
        *outFlag3 = 1;
        return true;
    }

    case 329:
    case 330:
    case 331:
    {
        *outWeight1 = 1.0f;
        uint8 tmp1[16]{};
        uint8 tmp2[16]{};
        const float vCur = *Core::Hook::Call<const float*>(0x9ED1D0, tmp1, phase, *reinterpret_cast<const uint32*>(animData + 0x10), animId);
        const float vEnd = *Core::Hook::Call<const float*>(0x9ED1D0, tmp2, 1.0f, *reinterpret_cast<const uint32*>(animData + 0x10), animId);
        float ratio = Core::StaticRef<uint8>(0x123013C) ? phase : ((vCur / vEnd >= 0.0f) ? (vCur / vEnd) : -(vCur / vEnd));
        if (m_pMyVehicle)
        {
            const uint32 seat = Core::Hook::CallMethod<uint32>(0x9C6E40, m_pMyVehicle, this);
            if ((seat <= 1 && (animId == 330 || animId == 331)) || ((seat == 2 || seat == 3) && animId == 329))
                ratio -= 1.0f;
        }
        if (ratio <= 0.0099999998f)
            ratio = 0.0099999998f;
        *outWeight2 = ratio + 1.0f;
        *outWeight3 = 1.0f;
        *outWeight4 = 1.0f;
        *outFlag3 = 0;
        return true;
    }

    case 332:
    case 333:
    case 334:
    case 335:
    case 336:
    case 365:
    case 367:
    case 369:
    case 371:
    {
        *outWeight1 = 1.0f;
        *outWeight2 = 1.0f - phase;
        *outWeight4 = 1.0f - phase;
        float tMin = 0.5f;
        float tMax = 0.80000001f;
        if (Core::Hook::CallMethod<int32>(0x58ABB0, animClip, 128, 0))
        {
            Core::Hook::CallMethod<int32>(0xA4C8F0, reinterpret_cast<void*>(animData), 1, &tMin, 0.0f, 1.0f);
            Core::Hook::CallMethod<int32>(0xA4C8F0, reinterpret_cast<void*>(animData), 2, &tMax, 0.0f, 1.0f);
        }
        *outWeight3 = 1.0f;
        if (phase > tMin)
        {
            float w = (phase - tMin) / (tMax - tMin);
            if (w >= 1.0f)
                w = 1.0f;
            *outWeight3 = 1.0f - w;
        }
        if ((self[0x1E2] & 0xFu) >= 2u)
        {
            const uint32 veh = *reinterpret_cast<const uint32*>(self + 0x1BC);
            if (veh && (*reinterpret_cast<const uint32*>(veh + 0x28) & 0x3C0u) == 0x80u)
            {
                const uint32 v2 = Core::Hook::CallMethod<uint32>(0x426800, this);
                if (*reinterpret_cast<const uint32*>(v2 + 0x1354) == 1)
                    *reinterpret_cast<float*>(self + 0x288) = 1.0f - (*reinterpret_cast<const float*>(animData + 0x58) * *outWeight3);
            }
        }
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        if (outHeading && *reinterpret_cast<const float*>(animData + 0x4C) < 0.5f)
            *outHeading *= *reinterpret_cast<const float*>(animData + 0x58);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;
    }

    case 337:
    case 338:
    case 339:
    case 340:
    case 341:
        *outWeight1 = 1.0f;
        *outWeight2 = 0.0f;
        *outWeight3 = 0.0f;
        *outWeight4 = 0.0f;
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;

    case 342:
    case 343:
    case 344:
    case 345:
    case 346:
    case 347:
    {
        *outWeight1 = 1.0f;
        float tParam = 0.0f;
        float w = 1.0f - phase;
        if (Core::Hook::CallMethod<int32>(0x58ABB0, animClip, 128, 0)
            && !Core::Hook::CallMethod<uint8>(0xA4C8F0, reinterpret_cast<void*>(animData), 0x80000, &tParam, 0.0f, 1.0f))
        {
            w = (tParam <= phase) ? 1.0f : (1.0f - ((tParam - phase) / tParam));
        }

        if (animId == 346 || animId == 347)
        {
            *outWeight2 = 1.0f;
            *outWeight3 = 1.0f;
            *outWeight4 = 1.0f;
        }
        else
        {
            *outWeight2 = w;
            *outWeight3 = w;
            *outWeight4 = w;
        }
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = (animId == 346 || animId == 347) ? 1 : 0;
        return true;
    }

    case 348:
    case 349:
        *outWeight1 = 1.0f;
        *outWeight2 = 0.0f;
        *outWeight3 = 0.0f;
        *outWeight4 = 0.0f;
        *outFlag3 = 0;
        return true;

    case 352:
    case 353:
    case 354:
    case 355:
    {
        *outWeight1 = 1.0f;
        *outWeight2 = 1.0f - phase;
        float tMin = 0.0f;
        float tMax = 1.0f;
        if (Core::Hook::CallMethod<int32>(0x58ABB0, animClip, 128, 0))
        {
            if (!Core::Hook::CallMethod<uint8>(0xA4C8F0, reinterpret_cast<void*>(animData), 1, &tMin, 0.0f, 1.0f))
                tMin = 0.0f;
            if (!Core::Hook::CallMethod<uint8>(0xA4C8F0, reinterpret_cast<void*>(animData), 2, &tMax, 0.0f, 1.0f))
                tMax = 1.0f;
        }
        if (tMin <= phase)
            *outWeight3 = (tMax >= phase) ? ((tMax - phase) / (tMax - tMin)) : 0.0f;
        else
            *outWeight3 = 1.0f;

        if (animId == 352 || animId == 353)
        {
            *outFlag1 = 0;
            *outFlag2 = 1;
        }
        else
        {
            *outFlag1 = 1;
        }
        *outWeight4 = *outWeight3;
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;
        *outFlag3 = 0;
        return true;
    }

    case 356:
    case 357:
    case 358:
    case 359:
    {
        *outWeight1 = 1.0f;
        *outWeight2 = 0.0f;
        *outWeight3 = 0.0f;
        *outWeight4 = 0.0f;
        Core::Hook::CallMethod<void>(0x8A4AB0, this, animData, outDeltaPos, outHeading);
        *outFlag4 = 1;

        const uint32 veh = Core::Hook::CallMethod<uint32>(0x426800, this);
        float t1 = 0.0f;
        float t2 = 0.0f;
        if (veh
            && (*reinterpret_cast<const uint32*>(veh + 0x28) & 0x3C0u) == 0x80u
            && Core::Hook::CallMethod<uint8>(0xA4C8F0, reinterpret_cast<void*>(animData), 0x10000, &t1, 0.0f, 1.0f)
            && Core::Hook::CallMethod<uint8>(0xA4C8F0, reinterpret_cast<void*>(animData), 0x20000, &t2, 0.0f, 1.0f))
        {
            const float* vm = *reinterpret_cast<const float* const*>(veh + 0x20);
            const float absUpY = (vm[6] >= 0.0f) ? vm[6] : -vm[6];
            if (absUpY < 0.70700002f)
            {
                const uint32* modelInfos = &Core::StaticRef<uint32>(0x15F73B0);
                const uint32 vehMi = modelInfos[*reinterpret_cast<const int16*>(veh + 0x2E)];
                const int32 boneIdx = *reinterpret_cast<const int32*>(*reinterpret_cast<const uint32*>(vehMi + 0xCC) + 4);
                if (boneIdx != -1)
                {
                    const int32 boneMat = Core::Hook::CallMethod<int32>(0x4D9100, reinterpret_cast<void*>(veh), boneIdx);
                    const float dz = *reinterpret_cast<const float*>(boneMat + 0x38) - *reinterpret_cast<const float*>(vehMi + 0x28);
                    const float dist = std::sqrt((vm[8] * dz) * (vm[8] * dz) + (vm[9] * dz) * (vm[9] * dz));
                    const float p = *reinterpret_cast<const float*>(animData + 0x4C);
                    float w = 0.0f;
                    if (t1 <= p)
                    {
                        if (t2 >= p && (t2 - t1) > 0.0f)
                            w = 1.0f - ((p - t1) / (t2 - t1));
                    }
                    else
                    {
                        w = 1.0f;
                        if (t1 > 0.0f && (p / t1) <= 1.0f)
                            w = p / t1;
                    }
                    if (w > 0.0f)
                    {
                        float delta = (animId == 356 || animId == 357) ? (w * dist * 2.0f) : (w * dist);
                        float* outX = reinterpret_cast<float*>(outDeltaPos);
                        if (animId == 356 || animId == 359)
                            *outX += delta;
                        else
                            *outX -= delta;
                    }
                }
            }
        }
        *outFlag3 = 0;
        return true;
    }

    default:
        return false;
    }
}
// --- Slice 8 ---
void CPed::InjectHooks_Slice8()
{
    Core::Hook::Install(0x8A6750, &CPed::ShutdownSubsystems);
    Core::Hook::Install(0x8A6E40, &CPed::SelectAnimOrVoice);
    Core::Hook::Install(0x8A70C0, &CPed::UpdateSpeechState);
    Core::Hook::Install(0x8AA500, &CPed::IsPerformingUnkAction);
}

// 0x8A6750
int32 __cdecl CPed::ShutdownSubsystems()
{
    Core::Hook::Call<int32>(0x972690);
    Core::Hook::Call<int32>(0xA67A10);
    Core::Hook::Call<int32>(0xA1E8E0);
    Core::Hook::Call<void>(0xC28820); // nullsub (ret)
    Core::Hook::Call<void>(0xC28820); // nullsub (ret)
    int32 result = Core::Hook::Call<int32>(0x9C4B10);

    int32* table = &Core::StaticRef<int32>(0x12300E0);
    for (uint32 i = 0; i < 12; ++i)
    {
        if (table[i])
        {
            result = Core::Hook::Call<int32>(0x5B1C10, table[i]);
            table[i] = 0;
        }
    }
    return result;
}

// 0x8A6E40
int32 __cdecl CPed::SelectAnimOrVoice(int32 a1, int32* a2, int32 a3, int16* a4, int32 a5, int8 a6, int8 a7, int8 a8)
{
    using Fn440 = int32(__thiscall*)(int16*);
    void** vtbl = *reinterpret_cast<void***>(a4);
    auto fn440 = reinterpret_cast<Fn440>(vtbl[440 / 4]);

    if (fn440(a4) != -1 && a3
        && !*reinterpret_cast<const uint8*>(a3 + 536)
        && *reinterpret_cast<const uint8*>(a3 + 537)
        && !static_cast<uint8>(a5 & 0xFF))
    {
        uint8* taskMgr = *reinterpret_cast<uint8* const*>(a3 + 0x224) + 0x44;
        const int32 slot4 = Core::Hook::CallMethod<int32>(0x426850, taskMgr, 4);
        if (slot4)
        {
            const int32 v10 = fn440(a4);
            const int32 v11 = Core::Hook::CallMethod<int32>(0x9ADCB0, reinterpret_cast<void*>(slot4), v10);
            if (v11 != -1 && Core::Hook::Call<int32>(0x909430, v11, *a2))
                return v11;
        }
    }

    if (a7)
    {
        const int32 v12 = Core::Hook::Call<int32>(0x89E1B0, reinterpret_cast<int32>(a4));
        if (v12 != -1 && Core::Hook::Call<int32>(0x909430, v12, *a2))
            return v12;
    }

    const uintptr_t* modelInfos = &Core::StaticRef<uintptr_t>(0x15F73B0);
    const int32 v13 = *reinterpret_cast<const int32*>(modelInfos[a4[23]] + 200);
    const bool v19 = (v13 != 82 && v13 != -1);
    int32 v11 = -1;

    if (a3 && a8 && (*reinterpret_cast<const uint32*>(a3 + 624) & 0x8000u) == 0 && v19 && v13 != a1)
    {
        const int32 v14 = Core::Hook::CallMethod<int32>(0x92F590, Core::StaticRef<void*>(0x1548F88), v13);
        if (Core::Hook::Call<int32>(0x908FE0, v14))
        {
            v11 = SelectAnimOrVoice(v13, a2, a3, a4, 1, 1, 0, 0);
            if (v11 != -1)
                return v11;
        }
    }

    if (Core::Hook::Call<int32>(0x909430, a1, *a2) && Core::Hook::Call<uint8>(0x89C630, a1, a2, reinterpret_cast<int32>(a4)))
        return a1;

    if (!((v19 && v13 == a1) || !Core::Hook::Call<uint8>(0x89C5B0, a2)))
    {
        bool v16 = false;
        const int32* v17 = &Core::StaticRef<int32>(0xF21DB0);
        while (*v17 != -1 && v11 == -1)
        {
            if (*v17 == a1)
            {
                v11 = SelectAnimOrVoice(v17[1], a2, a3, a4, 1, 1, 0, 1);
                v16 = true;
            }
            v17 += 2;
        }
        if (!v16 && a1 != 82 && a1 != 81)
            v11 = SelectAnimOrVoice(82, a2, a3, a4, 1, 1, 0, 1);
    }

    if (v11 == -1 && !a6)
    {
        const int32* v18 = &Core::StaticRef<int32>(0xF21DF0);
        while (*v18 != -1 && v11 == -1)
        {
            if (*v18 == *a2)
            {
                *a2 = v18[1];
                v11 = SelectAnimOrVoice(a1, a2, a3, a4, a5, 0, 0, 1);
            }
            v18 += 2;
        }
    }

    return v11;
}

// 0x8A70C0
void __thiscall CPed::UpdateSpeechState(int8 a2, int32 a3)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if ((self[628] & 0x10) != 0)
        return;

    const bool v4 = Core::Hook::CallMethod<int32>(0x89C780, this) != 0;
    if (a2)
    {
        if (!v4)
        {
            uint8* taskMgr = static_cast<uint8*>(m_pPedIntelligence) + 0x44;
            const int32 v5 = Core::Hook::CallMethod<int32>(0x9A9D30, m_pedSubObj2B0);
            if (v5 && Core::Hook::CallMethod<uint8>(0x89BD60, reinterpret_cast<void*>(v5)))
            {
                if (!Core::Hook::CallMethod<int32>(0x426850, taskMgr, 1040) && !Core::Hook::CallMethod<int32>(0x426850, taskMgr, 1900))
                    Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 55);
                else
                    Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 53);
            }
            else if (!Core::Hook::CallMethod<int32>(0x426850, taskMgr, 1040))
            {
                Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 54);
            }
            else
            {
                Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, 53);
            }

            void* intel = m_pMoveBlend;
            void** vtbl = *reinterpret_cast<void***>(intel);
            reinterpret_cast<void(__thiscall*)(void*)>(vtbl[48 / 4])(intel);
        }

        uint8* intel = static_cast<uint8*>(m_pMoveBlend);
        *reinterpret_cast<uint32*>(intel + 52) = Core::StaticRef<uint32>(0x11DDE74);
        *reinterpret_cast<int32*>(intel + 56) = a3;
        return;
    }

    if (v4)
    {
        Core::Hook::CallMethod<int32>(0xBC9840, m_pMoveBlend, m_nSpeechVoiceBank);
        Core::Hook::CallMethod<int32>(0x89F9C0, this);
        uint8* intel = static_cast<uint8*>(m_pMoveBlend);
        *reinterpret_cast<uint32*>(intel + 52) = 0;
        *reinterpret_cast<int32*>(intel + 56) = -1;
        void** vtbl = *reinterpret_cast<void***>(intel);
        reinterpret_cast<void(__thiscall*)(void*)>(vtbl[48 / 4])(intel);
    }
}

// 0x8AA500
bool __thiscall CPed::IsPerformingUnkAction()
{
    if (!Core::Hook::CallMethod<int32>(0x89DE90, this))
        return false;
    if (!Core::Hook::CallMethod<uint8>(0x9AB6D0, m_pedSubObj2B0, static_cast<int8>(1)))
        return false;
    const uint8* obj = reinterpret_cast<const uint8*>(Core::Hook::CallMethod<int32>(0x8A6530, m_pPedIntelligence));
    return (*reinterpret_cast<const uint32*>(obj + 2292) & 0x60000u) != 0;
}

// 0x8AAD00
// UNHOOK_SAFE_SKIP: x86 disasm shows __thiscall (ecx = CPed*, passes this+0xB44 to 0xB3C6B0), but Ped.h declares static __stdcall missing ecx.
int32 __stdcall CPed::SetTaskBit96_A(int32 a1, int32 bit)
{
    uint32 mask[3]{};
    mask[bit >> 5] |= 1u << (bit & 0x1F);
    return Core::Hook::CallStdcall<int32>(0xB3C6B0, a1, mask[0], mask[1], mask[2]);
}

// 0x8AAD60
// UNHOOK_SAFE_SKIP: x86 disasm shows __thiscall (ecx = CPed*, passes this+0xB44 to 0xB3C640), but Ped.h declares static __stdcall missing ecx.
int32 __stdcall CPed::SetTaskBit96_B(int32 a1, int32 bit)
{
    uint32 mask[3]{};
    mask[bit >> 5] |= 1u << (bit & 0x1F);
    return Core::Hook::CallStdcall<int32>(0xB3C640, a1, mask[0], mask[1], mask[2]);
}
