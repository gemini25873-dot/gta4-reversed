#include "StdInc.h"
#include "Game/Physical.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <algorithm>
#include <cmath>
#include <intrin.h>

namespace
{
    inline uint8 GetPhysicsLevelState(void* phInst)
    {
        const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(phInst) + 8);
        const uint8* phLevel = *reinterpret_cast<const uint8* const*>(Core::Rebase(0x1666880));
        const uint8* levelObjects = *reinterpret_cast<const uint8* const*>(phLevel + 0x70);
        return *reinterpret_cast<const uint8*>(levelObjects + static_cast<uint32>(levelIdx) * 8 + 4);
    }

    inline bool IsPhysicsLevelIndexValidAndActive(void* phInst)
    {
        if (!phInst)
            return false;
        const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(phInst) + 8);
        if (levelIdx == 0xFFFF)
            return false;
        const uint8* phLevel = *reinterpret_cast<const uint8* const*>(Core::Rebase(0x1666880));
        const int32 maxObjects = *reinterpret_cast<const uint16*>(phLevel + 0x26);
        if (static_cast<int32>(levelIdx) >= maxObjects)
            return false;
        const uint8* levelObjects = *reinterpret_cast<const uint8* const*>(phLevel + 0x70);
        return (*reinterpret_cast<const uint8*>(levelObjects + static_cast<uint32>(levelIdx) * 8 + 4) & 3) == 0;
    }

    inline int32 ResolveAttachComponent(CPhysical* entity, int16 boneIndex)
    {
        using GetFragInstFn = void*(__thiscall*)(CPhysical*, int32);
        using GetFragInst0Fn = void*(__thiscall*)(CPhysical*);
        if (!reinterpret_cast<GetFragInst0Fn>(entity->m_vftable[0xA0 / 4])(entity) || boneIndex <= 0)
            return 0;

        void* fragInst = reinterpret_cast<GetFragInst0Fn>(entity->m_vftable[0xA0 / 4])(entity);
        void* fragType = Core::Hook::CallMethod<void*>(0x51DDD0, fragInst);
        const int32 comp = Core::Hook::Call<int32>(0xA4B480, fragType, static_cast<int32>(boneIndex));
        return comp >= 0 ? comp : 0;
    }

    inline uintptr_t* GetVTable(void* obj)
    {
        return *reinterpret_cast<uintptr_t**>(obj);
    }

    template<typename Ret, typename This, typename... Args>
    inline Ret CallVFunc(This* obj, size_t index, Args... args)
    {
        using Fn = Ret(__thiscall*)(This*, Args...);
        return reinterpret_cast<Fn>(GetVTable(obj)[index])(obj, args...);
    }

    inline void CopyMatrix3x4(CMatrix* dst, const CMatrix* src)
    {
        dst->right = src->right;
        dst->up    = src->up;
        dst->at    = src->at;
        dst->pos   = src->pos;
    }

    inline bool IsFiniteFloat(float v)
    {
        return !std::isnan(v);
    }

    struct CVector4
    {
        CVector vec{};
        float   w = 0.0f;
    };
}

void CPhysical::InjectHooks()
{
    Core::Hook::Install(0xA4B290, &CPhysical::Ctor);
    Core::Hook::Install(0xA47570, &CPhysical::Dtor);
    Core::Hook::Install(0xA48610, &CPhysical::DeletingDtor);
    Core::Hook::Install(0xA48E40, &CPhysical::SetPosition);
    Core::Hook::Install(0xA47A40, &CPhysical::SetHeading);
    Core::Hook::Install(0xA47930, &CPhysical::InitHealth);
    Core::Hook::Install(0xA47950, &CPhysical::SetHealth);
    Core::Hook::Install(0xA47970, &CPhysical::AddHealth);
    Core::Hook::Install(0x447B30, &CPhysical::GetHealth);


    // Slice 1
    Core::Hook::Install(0xA47600, &CPhysical::RemoveInstConstraints);
    Core::Hook::Install(0xA47660, &CPhysical::ActivatePhysics);
    Core::Hook::Install(0xA476B0, &CPhysical::DeactivatePhysics);
    Core::Hook::Install(0xA476F0, &CPhysical::TryDeactivatePhysics);
    Core::Hook::Install(0xA47750, &CPhysical::SetMoveSpeed);
    Core::Hook::Install(0xA477F0, &CPhysical::GetMoveSpeed);
    Core::Hook::Install(0xA47840, &CPhysical::SetTurnSpeed);
    Core::Hook::Install(0xA478E0, &CPhysical::GetTurnSpeed);
    Core::Hook::Install(0xA47990, &CPhysical::SetPhysUnk1E4);
    Core::Hook::Install(0xA47AE0, &CPhysical::ClearPhysUnk16C);
    Core::Hook::Install(0xA47B30, &CPhysical::HasPhysUnk154Value);
    Core::Hook::Install(0xA47B80, &CPhysical::ApplyForce);
    Core::Hook::Install(0xA47C30, &CPhysical::ApplyForceCenter);
    Core::Hook::Install(0xA47C80, &CPhysical::ApplyForceWithTorque);
    Core::Hook::Install(0xA47D40, &CPhysical::ApplyTorque);
    Core::Hook::Install(0xA47D90, &CPhysical::ApplyImpulse);
    Core::Hook::Install(0xA47E40, &CPhysical::ApplyImpulseCenter);
    Core::Hook::Install(0xA47E90, &CPhysical::ApplyImpulseToLevel);
    Core::Hook::Install(0xA47F20, &CPhysical::ApplyImpulseToLevelScaled);
    Core::Hook::Install(0xA47FB0, &CPhysical::GetVelocityAtPoint);
    Core::Hook::Install(0xA48070, &CPhysical::SetPhysFlag4);
    Core::Hook::Install(0xA480A0, &CPhysical::RemovePhysUnk154Value);
    Core::Hook::Install(0xA48120, &CPhysical::SetPhysUnk1F8);
    Core::Hook::Install(0xA48160, &CPhysical::UpdatePhysUnk1F8);
    Core::Hook::Install(0xA481B0, &CPhysical::AttachToEntitySimple);
    Core::Hook::Install(0xA483D0, &CPhysical::AttachToWorld);

    // Slice 2
    Core::Hook::Install(0xA48550, &CPhysical::DetachFromEntity);
    Core::Hook::Install(0xA485A0, &CPhysical::CheckPhysUnk11C);
    Core::Hook::Install(0xA485F0, &CPhysical::GetModelBoundRadiusNeg);
    Core::Hook::Install(0xA48630, &CPhysical::AddToPhysicsWorld);
    Core::Hook::Install(0xA486E0, &CPhysical::RemoveFromPhysicsWorld);
    Core::Hook::Install(0xA48770, &CPhysical::SyncPhysicsMatrix);
    Core::Hook::Install(0xA488D0, &CPhysical::SetPhysFlag);
    Core::Hook::Install(0xA489C0, &CPhysical::SetColliderScale);
    Core::Hook::Install(0xA48A40, &CPhysical::UpdateTransformFromPhysics);
    Core::Hook::Install(0xA48EB0, &CPhysical::SetMatrixAndWarp);
    Core::Hook::Install(0xA48FA0, &CPhysical::ApplyImpactImpulse);
    Core::Hook::Install(0xA49290, &CPhysical::ProcessImpactEffect);
    Core::Hook::Install(0xA494B0, &CPhysical::CalcEffectiveMass);
    Core::Hook::Install(0xA49650, &CPhysical::UpdatePhysicsActiveState);
    Core::Hook::Install(0xA496E0, &CPhysical::CopyAttachTransform);
    Core::Hook::Install(0xA49840, &CPhysical::MarkPhysUnk1F4Bit7);
    Core::Hook::Install(0xA49860, &CPhysical::CheckPhysCollisionFilter);
    Core::Hook::Install(0xA49900, &CPhysical::AttachToEntityQuat);
    Core::Hook::Install(0xA49A60, &CPhysical::AttachToWorldQuat);

    // Slice 3
    Core::Hook::Install(0xA49A90, &CPhysical::AttachToEntity);
    Core::Hook::Install(0xA4A020, &CPhysical::UpdateAttachRotation);
    Core::Hook::Install(0xA4A320, &CPhysical::ProcessAttach);
    Core::Hook::Install(0xA4A890, &CPhysical::UpdateAttachment);
    Core::Hook::Install(0xA4ACB0, &CPhysical::CanBeDamaged);
    Core::Hook::Install(0xA4AF20, &CPhysical::UpdateSubmersion);
    Core::Hook::Install(0x447B20, &CPhysical::IsPhysFlag118Bit3Clear);
    Core::Hook::Install(0x447B50, &CPhysical::SetPhysVec200);

    Core::Log::Info("CPhysical hooks installed (62 functions)");
}

CPhysical* __thiscall CPhysical::Ctor()
{
    CDynamicEntity::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD89F2C));
    Core::Hook::CallMethod<void>(0xC6D760, m_physAnimArray);

    m_nPhysUnk1AC = 0;
    m_nPhysUnk1B0 = 0;
    m_nPhysUnk1B4 = 0;
    m_nPhysUnk1B8 = 0;
    m_pPhysUnk1E4 = nullptr;
    m_nPhysUnk1E8 = 0;
    m_nPhysUnk1EC = -1;
    m_fMaxHealth = 100.0f;

    Core::Hook::CallMethod<void>(0x9EA400, this);

    m_pMatrix->right = { 1.0f, 0.0f, 0.0f };
    m_pMatrix->up    = { 0.0f, 1.0f, 0.0f };
    m_pMatrix->at    = { 0.0f, 0.0f, 1.0f };
    m_pMatrix->pos   = { 0.0f, 0.0f, 0.0f };

    m_nFlags |= 1u;
    m_pPhysicsInst = nullptr;
    m_nPhysUnk150 = 0;

    m_vecMoveSpeed = {};
    m_vecTurnSpeed = {};
    m_nPhysUnk170 = 0;
    m_nPhysUnk174 = 0;
    m_pPhysUnk16C = nullptr;
    m_vecPhysUnk180 = {};
    m_vecPhysUnk190 = {};
    m_nPhysFlags118 &= 0xFFE00000u;
    m_vecPhysUnk1A0 = {};
    m_fHealth = 100.0f;
    m_nPhysUnk11C = 0;
    m_pAttachedTo = nullptr;
    m_vecAttachOffset = {};
    m_vecAttachQuat[0] = 0.0f;
    m_vecAttachQuat[1] = 0.0f;
    m_vecAttachQuat[2] = 0.0f;
    m_vecAttachQuat[3] = 1.0f;
    m_nPhysUnk1E0 = 0;
    m_nPhysUnk1E2 = 0;
    m_pPhysUnk1F8 = nullptr;
    m_nPhysUnk1F4 = 0;
    m_vecPhysUnk200 = {};
    m_fPhysUnk120 = 1.1f;

    return this;
}

int32 __thiscall CPhysical::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD89F2C));

    if (m_pPhysUnk16C)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk16C, &m_pPhysUnk16C);
        m_pPhysUnk16C = nullptr;
    }
    if (m_pPhysUnk1E4)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk1E4, &m_pPhysUnk1E4);
        m_pPhysUnk1E4 = nullptr;
    }
    if (m_pAttachedTo)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);
        m_pAttachedTo = nullptr;
    }
    if (m_pPhysUnk1F8)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk1F8, &m_pPhysUnk1F8);
        m_pPhysUnk1F8 = nullptr;
    }

    Core::Hook::CallMethod<void>(0xC6D7A0, m_physAnimArray);
    return CDynamicEntity::Dtor();
}

CPhysical* __thiscall CPhysical::DeletingDtor(uint8 flags)
{
    Dtor();
    if ((flags & 1) != 0)
        Core::Hook::Call<void>(0x5B1C10, this);
    return this;
}

void* __thiscall CPhysical::SetPosition(const float* pos, bool skipPhysics, int32 arg)
{
    void* result = m_pMatrix;
    if (m_pMatrix)
    {
        m_pMatrix->pos.x = pos[0];
        m_pMatrix->pos.y = pos[1];
        m_pMatrix->pos.z = pos[2];
        m_pMatrix->pad3  = pos[3];
    }
    else
    {
        result = const_cast<float*>(pos);
        m_vecPos.x = pos[0];
        m_vecPos.y = pos[1];
        m_vecPos.z = pos[2];
    }

    if (m_pMatrix && !skipPhysics)
    {
        using OnWarpFn = void*(__thiscall*)(CPhysical*, int32);
        return reinterpret_cast<OnWarpFn>(m_vftable[45])(this, arg);
    }

    return result;
}

void __thiscall CPhysical::SetHeading(float heading)
{
    if (m_pMatrix)
    {
        const float c = std::cos(heading);
        const float s = std::sin(heading);
        m_pMatrix->right = { c, s, 0.0f };
        m_pMatrix->up    = { -s, c, 0.0f };
        m_pMatrix->at    = { 0.0f, 0.0f, 1.0f };
    }
    else
    {
        m_fPad1C = heading;
    }

    if (m_pMatrix)
    {
        using OnWarpFn = void(__thiscall*)(CPhysical*, int32);
        reinterpret_cast<OnWarpFn>(m_vftable[45])(this, 0);
    }
}

void __thiscall CPhysical::InitHealth(float health)
{
    m_fMaxHealth = health;
}

void __thiscall CPhysical::SetHealth(float health, int32 /*arg*/)
{
    m_fMaxHealth = health;
}

void __thiscall CPhysical::AddHealth(float delta)
{
    m_fMaxHealth += delta;
}

float __thiscall CPhysical::GetHealth() const
{
    return m_fMaxHealth;
}

// --- Slice 1 (0xA47600..0xA483D0) ---

// 0xA47600
void __stdcall CPhysical::RemoveInstConstraints(void* phInst)
{
    uint8* physMgr = *reinterpret_cast<uint8**>(Core::Rebase(0x1666884));
    void* phSim = *reinterpret_cast<void**>(physMgr + 8);
    void* items[8]{};
    const int32 count = Core::Hook::CallMethod<int32>(0x6B7700, phSim, phInst, items, 8);
    for (int32 i = 0; i < count; ++i)
    {
        if (items[i])
            Core::Hook::CallMethod<void>(0x6B66F0, phSim, items[i]);
    }
    Core::Hook::CallMethod<void>(0x686AD0, phInst, 0x2000u, false);
}

// 0xA47660
void __thiscall CPhysical::ActivatePhysics()
{
    if (!m_pPhysicsInst)
        return;

    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIdx == 0xFFFF)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
    {
        void* physMgr = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
        void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        Core::Hook::CallMethod<void>(0x69A5B0, physMgr, static_cast<uint32>(levelIdx), collider, 0);
    }
}

// 0xA476B0
void __thiscall CPhysical::DeactivatePhysics()
{
    if (!IsPhysicsLevelIndexValidAndActive(m_pPhysicsInst))
        return;

    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8);
    void* physMgr = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
    Core::Hook::CallMethod<void>(0x69A690, physMgr, static_cast<uint32>(levelIdx));
}

// 0xA476F0
void __thiscall CPhysical::TryDeactivatePhysics()
{
    if ((m_nFlags & 0x1Cu) == 0)
        return;

    if (!IsPhysicsLevelIndexValidAndActive(m_pPhysicsInst))
        return;

    if ((m_nFlags2 & 0x3C0u) == 0xC0u)
    {
        Core::Hook::CallMethod<void>(0x8AAC50, this);
    }
    else
    {
        using Fn = void(__thiscall*)(CPhysical*);
        reinterpret_cast<Fn>(m_vftable[0xDC / 4])(this);
    }
}

// 0xA47750
void __thiscall CPhysical::SetMoveSpeed(const CVector* speed)
{
    if (!m_pPhysicsInst)
        return;

    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIdx == 0xFFFF)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
    {
        if (speed->x != 0.0f || speed->y != 0.0f || speed->z != 0.0f)
            ActivatePhysics();
    }

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (collider && *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8) != 0xFFFF)
    {
        void* collider2 = Core::Hook::CallMethod<void*>(0x9E7080, this);
        void** vft = *reinterpret_cast<void***>(collider2);
        reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0xA8 / 4])(collider2, speed);
    }
}

// 0xA477F0
CVector* __thiscall CPhysical::GetMoveSpeed(CVector* out)
{
    uint8* collider = static_cast<uint8*>(Core::Hook::CallMethod<void*>(0x9E7080, this));
    if (collider)
    {
        out->x = *reinterpret_cast<const float*>(collider + 0x110);
        out->y = *reinterpret_cast<const float*>(collider + 0x114);
        out->z = *reinterpret_cast<const float*>(collider + 0x118);
    }
    else
    {
        out->x = 0.0f;
        out->y = 0.0f;
        out->z = 0.0f;
    }
    return out;
}

// 0xA47840
void __thiscall CPhysical::SetTurnSpeed(const CVector* speed)
{
    if (!m_pPhysicsInst)
        return;

    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8);
    if (levelIdx == 0xFFFF)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
    {
        if (speed->x != 0.0f || speed->y != 0.0f || speed->z != 0.0f)
            ActivatePhysics();
    }

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (collider && *reinterpret_cast<const uint16*>(static_cast<const uint8*>(m_pPhysicsInst) + 8) != 0xFFFF)
    {
        void* collider2 = Core::Hook::CallMethod<void*>(0x9E7080, this);
        void** vft = *reinterpret_cast<void***>(collider2);
        reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0xB0 / 4])(collider2, speed);
    }
}

// 0xA478E0
CVector* __thiscall CPhysical::GetTurnSpeed(CVector* out)
{
    uint8* collider = static_cast<uint8*>(Core::Hook::CallMethod<void*>(0x9E7080, this));
    if (collider)
    {
        out->x = *reinterpret_cast<const float*>(collider + 0x120);
        out->y = *reinterpret_cast<const float*>(collider + 0x124);
        out->z = *reinterpret_cast<const float*>(collider + 0x128);
    }
    else
    {
        out->x = 0.0f;
        out->y = 0.0f;
        out->z = 0.0f;
    }
    return out;
}

// 0xA47990
void __thiscall CPhysical::SetPhysUnk1E4(CPhysical* target)
{
    if (!m_pPhysicsInst)
        return;
    if (target && !target->m_pPhysicsInst)
        return;

    if (target != m_pPhysUnk1E4)
    {
        if (m_pPhysUnk1E4)
            Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk1E4, &m_pPhysUnk1E4);
        m_pPhysUnk1E4 = target;
        if (target)
            Core::Hook::CallMethod<void>(0x9E6550, target, &m_pPhysUnk1E4);

        uint8* ent = static_cast<uint8*>(m_pPhysUnk1E4);
        if (!ent)
            return;

        const uint32 type = (*reinterpret_cast<const uint32*>(ent + 0x28) >> 6) & 0xFu;
        if (type == 2)
            ent = *reinterpret_cast<uint8**>(ent + 0xFA0);
        else if (type != 3)
            return;

        if (!ent || ent[0x219] == 0)
            return;

        uint8* sub = *reinterpret_cast<uint8**>(ent + 0x228);
        if (!sub)
            return;

        const uint32 myType = (m_nFlags2 >> 6) & 0xFu;
        if (myType == 3)
            *reinterpret_cast<uint32*>(sub + 0x554) |= 4u;
        else if (myType == 2)
            *reinterpret_cast<uint32*>(sub + 0x554) |= 8u;
    }
}

// 0xA47AE0
void __thiscall CPhysical::ClearPhysUnk16C(bool keepCount)
{
    m_nPhysUnk170 = 0;
    m_nPhysUnk174 = 0;
    if (m_pPhysUnk16C)
    {
        Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk16C, &m_pPhysUnk16C);
        m_pPhysUnk16C = nullptr;
    }
    if (!keepCount)
        m_nPhysUnk150 = 0;
}

// 0xA47B30
bool __thiscall CPhysical::HasPhysUnk154Value(uint32 value)
{
    if (((m_nPhysFlags118 >> 4) & 1u) == 0)
        return false;

    const int32 count = static_cast<int32>(m_nPhysUnk150);
    const uint32* arr = reinterpret_cast<const uint32*>(&m_vecMoveSpeed);
    for (int32 i = 0; i < count; ++i)
    {
        if (arr[i] == value)
            return true;
    }
    return false;
}

// 0xA47B80
void __thiscall CPhysical::ApplyForce(const CVector* force, const CVector* offset, int32 component)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const CVector worldPos{ pos.x + offset->x, pos.y + offset->y, pos.z + offset->z };
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*, const CVector*, int32)>(vft[0x80 / 4])(collider, force, &worldPos, component);
}

// 0xA47C30
void __thiscall CPhysical::ApplyForceCenter(const CVector* force)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0x84 / 4])(collider, force);
}

// 0xA47C80
void __thiscall CPhysical::ApplyForceWithTorque(const CVector* force, const CVector* offset)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    const CVector torque{
        force->z * offset->y - force->y * offset->z,
        force->x * offset->z - force->z * offset->x,
        force->y * offset->x - force->x * offset->y
    };

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0x88 / 4])(collider, &torque);
}

// 0xA47D40
void __thiscall CPhysical::ApplyTorque(const CVector* torque)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0x88 / 4])(collider, torque);
}

// 0xA47D90
void __thiscall CPhysical::ApplyImpulse(const CVector* impulse, const CVector* offset, int32 component)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const CVector worldPos{ pos.x + offset->x, pos.y + offset->y, pos.z + offset->z };
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*, const CVector*, int32, float)>(vft[0x8C / 4])(collider, impulse, &worldPos, component, 1.0f);
}

// 0xA47E40
void __thiscall CPhysical::ApplyImpulseCenter(const CVector* impulse)
{
    if (!m_pPhysicsInst)
        return;

    if ((GetPhysicsLevelState(m_pPhysicsInst) & 3) == 1)
        ActivatePhysics();

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (!collider)
        return;

    collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    void** vft = *reinterpret_cast<void***>(collider);
    reinterpret_cast<void(__thiscall*)(void*, const CVector*)>(vft[0x90 / 4])(collider, impulse);
}

// 0xA47E90
void __thiscall CPhysical::ApplyImpulseToLevel(const CVector* impulse, const CVector* offset, int32 component, void* phInst)
{
    void* inst = phInst ? phInst : m_pPhysicsInst;
    if (!inst)
        return;

    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const CVector worldPos{ pos.x + offset->x, pos.y + offset->y, pos.z + offset->z };
    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(inst) + 8);
    void* physMgr = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
    Core::Hook::CallMethod<void>(0x699BF0, physMgr, static_cast<uint32>(levelIdx), impulse, &worldPos, component, false, 1.0f);
}

// 0xA47F20
void __thiscall CPhysical::ApplyImpulseToLevelScaled(const CVector* impulse, const CVector* offset, int32 component, void* phInst, float scale)
{
    void* inst = phInst ? phInst : m_pPhysicsInst;
    if (!inst)
        return;

    const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
    const CVector worldPos{ pos.x + offset->x, pos.y + offset->y, pos.z + offset->z };
    const uint16 levelIdx = *reinterpret_cast<const uint16*>(static_cast<const uint8*>(inst) + 8);
    void* physMgr = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
    Core::Hook::CallMethod<void>(0x699BF0, physMgr, static_cast<uint32>(levelIdx), impulse, &worldPos, component, true, scale);
}

// 0xA47FB0
CVector* __thiscall CPhysical::GetVelocityAtPoint(CVector* out, const CVector* offset, bool isWorldPos, int32 component)
{
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;

    CVector worldPos = *offset;
    if (!isWorldPos)
    {
        const CVector& pos = m_pMatrix ? m_pMatrix->pos : m_vecPos;
        worldPos.x += pos.x;
        worldPos.y += pos.y;
        worldPos.z += pos.z;
    }

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    if (collider)
    {
        collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
        void** vft = *reinterpret_cast<void***>(collider);
        reinterpret_cast<void(__thiscall*)(void*, const CVector*, CVector*, int32)>(vft[0x58 / 4])(collider, &worldPos, out, component);
    }

    return out;
}

// 0xA48070
void __thiscall CPhysical::SetPhysFlag4()
{
    m_nFlags |= 0x20000000u;
    m_nPhysFlags118 = (m_nPhysFlags118 & 0xFFF7FFFFu) | 4u;
    *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0x144) = 0;
}

// 0xA480A0
void __thiscall CPhysical::RemovePhysUnk154Value(uint32 value)
{
    uint32* arr = reinterpret_cast<uint32*>(&m_vecMoveSpeed);
    for (int32 i = static_cast<int32>(m_nPhysUnk150) - 1; i >= 0; --i)
    {
        if (arr[i] == value)
        {
            const int32 last = static_cast<int32>(m_nPhysUnk150) - 1;
            for (int32 j = i; j < last; ++j)
                arr[j] = arr[j + 1];

            if (m_nPhysUnk150 == 0)
                break;
            --m_nPhysUnk150;
        }
    }
}

// 0xA48120
void __thiscall CPhysical::SetPhysUnk1F8(void* entity, uint8 flags)
{
    if (m_pPhysUnk1F8)
        Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk1F8, &m_pPhysUnk1F8);

    m_pPhysUnk1F8 = entity;
    m_nPhysUnk1F4 = flags | 3u;
    if (entity)
        Core::Hook::CallMethod<void>(0x9E6550, entity, &m_pPhysUnk1F8);
}

// 0xA48160
void __thiscall CPhysical::UpdatePhysUnk1F8()
{
    const uint8 f = m_nPhysUnk1F4;
    const bool shouldClear = (f & 0x20u)
        ? ((f & 1u) == 0)
        : (((f & 0x40u) != 0) && ((f & 2u) == 0));

    if (shouldClear)
    {
        if (m_pPhysUnk1F8)
            Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk1F8, &m_pPhysUnk1F8);
        m_nPhysUnk1F4 = 0;
        m_pPhysUnk1F8 = nullptr;
    }

    m_nPhysUnk1F4 &= 0x7Cu;
}

// 0xA481B0
void __thiscall CPhysical::AttachToEntitySimple(CPhysical* target, int16 targetBone, int16 myBone, uint16 attachFlags, const CVector* offset, float breakForce)
{
    if (!target)
        return;

    if (m_pAttachedTo)
        Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);

    m_pAttachedTo = target;
    Core::Hook::CallMethod<void>(0x9E6550, target, &m_pAttachedTo);

    m_nPhysUnk1E0 = static_cast<uint16>(targetBone);
    m_nPhysUnk1E2 = attachFlags;
    m_vecAttachOffset = {};
    m_vecAttachQuat[0] = 0.0f;
    m_vecAttachQuat[1] = 0.0f;
    m_vecAttachQuat[2] = 0.0f;
    m_vecAttachQuat[3] = 1.0f;

    uint8* physMgr = *reinterpret_cast<uint8**>(Core::Rebase(0x1666884));
    void* phSim = *reinterpret_cast<void**>(physMgr + 8);

    const int32 targetComp = ResolveAttachComponent(target, targetBone);
    const int32 myComp = ResolveAttachComponent(this, myBone);

    ActivatePhysics();

    if ((m_nPhysUnk1E2 & 0x20u) != 0)
    {
        uint8* constraint = Core::Hook::CallMethod<uint8*>(
            0x6B6990, phSim, offset, target->m_pPhysicsInst, m_pPhysicsInst, targetComp, myComp, 0.0f, 0);
        if (constraint)
        {
            const float minBreak = *reinterpret_cast<const float*>(Core::Rebase(0xDECD60));
            if (breakForce > minBreak)
                *reinterpret_cast<float*>(constraint + 0x138) = breakForce;
        }
        Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000u, true);
        Core::Hook::CallMethod<void>(0x686AD0, target->m_pPhysicsInst, 0x2000u, true);
    }

    if ((m_nPhysUnk1E2 & 0x40u) != 0)
    {
        void* constraint = Core::Hook::CallMethod<void*>(
            0x6B6990, phSim, offset, target->m_pPhysicsInst, m_pPhysicsInst, targetComp, myComp, 0.0f, 0);
        if (constraint)
            Core::Hook::CallMethod<void>(0x6FA810, constraint);
        Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000u, true);
        Core::Hook::CallMethod<void>(0x686AD0, target->m_pPhysicsInst, 0x2000u, true);
    }

    if ((target->m_nFlags2 & 0x3C0u) == 0xC0u)
        Core::Hook::CallMethod<void>(0x89E0A0, target, this);

    if (Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC2A10, this);
}

// 0xA483D0
void __thiscall CPhysical::AttachToWorld(int16 myBone, uint16 attachFlags, const CVector* worldPos, const CVector* offset, float breakForce)
{
    m_nPhysUnk1E0 = static_cast<uint16>(myBone);
    m_nPhysUnk1E2 = attachFlags;

    if (m_pAttachedTo)
        Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);

    m_pAttachedTo = nullptr;
    m_vecAttachOffset = {};
    m_vecAttachQuat[0] = 0.0f;
    m_vecAttachQuat[1] = 0.0f;
    m_vecAttachQuat[2] = 0.0f;
    m_vecAttachQuat[3] = 1.0f;

    const int32 myComp = ResolveAttachComponent(this, myBone);
    ActivatePhysics();

    uint8* physMgr = *reinterpret_cast<uint8**>(Core::Rebase(0x1666884));
    void* phSim = *reinterpret_cast<void**>(physMgr + 8);

    if ((m_nPhysUnk1E2 & 0x20u) != 0)
    {
        uint8* constraint = Core::Hook::CallMethod<uint8*>(
            0x6B6830, phSim, worldPos, worldPos, m_pPhysicsInst, myComp, 0.0f, 0, 0);
        if (constraint)
        {
            if (offset)
            {
                *reinterpret_cast<float*>(constraint + 0x50) = offset->x;
                *reinterpret_cast<float*>(constraint + 0x54) = offset->y;
                *reinterpret_cast<float*>(constraint + 0x58) = offset->z;
            }
            const float minBreak = *reinterpret_cast<const float*>(Core::Rebase(0xDECD60));
            if (breakForce > minBreak)
                *reinterpret_cast<float*>(constraint + 0x138) = breakForce;

            Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000u, true);
        }
    }

    if ((m_nPhysUnk1E2 & 0x40u) != 0)
    {
        void* constraint = Core::Hook::CallMethod<void*>(
            0x6B6830, phSim, worldPos, worldPos, m_pPhysicsInst, myComp, 0.0f, 1, 0);
        if (constraint)
            Core::Hook::CallMethod<void>(0x6FA810, constraint);
        Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000u, true);
    }
}

// --- Slice 2 (0xA48550..0xA49A60) ---

// 0xA48550
void __thiscall CPhysical::DetachFromEntity()
{
    if (m_pAttachedTo)
    {
        const uint32 flags2 = *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pAttachedTo) + 0x28);
        if ((flags2 & 0x3C0u) == 0xC0u)
            Core::Hook::CallMethod<void>(0x89E0E0, m_pAttachedTo, this);

        if (m_pAttachedTo)
            Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);
    }

    m_nPhysUnk1E0 = 0xFFFF;
    m_nPhysUnk1E2 = 0;
    m_pAttachedTo = nullptr;
}

// 0xA485A0
bool __thiscall CPhysical::CheckPhysUnk11C(uint16 value) const
{
    if (Core::Hook::CallMethod<bool>(0x7B79E0, const_cast<CPhysical*>(this)) && m_nPhysUnk11C == 0)
    {
        if ((value >= 0x1Fu && value <= 0x3Eu) || static_cast<uint16>(value - 0x3Fu) <= 7u)
            return true;
    }
    return m_nPhysUnk11C == value;
}

// 0xA485F0
double __thiscall CPhysical::GetModelBoundRadiusNeg() const
{
    auto** modelInfoPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
    uint8* modelInfo = modelInfoPtrs[m_nModelIndex];
    return -(*reinterpret_cast<float*>(modelInfo + 0x28));
}

// 0xA48630
void __thiscall CPhysical::AddToPhysicsWorld()
{
    if (!m_pPhysicsInst)
        return;

    CallVFunc<void>(this, 45, 1);

    if (CallVFunc<void*>(this, 40) == nullptr &&
        *reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(m_pPhysicsInst) + 4) != 0)
    {
        Core::Hook::CallMethod<void>(0x5171A0, m_pPhysicsInst, m_pMatrix);

        void* physSim = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
        if (m_nFlags & 4u)
        {
            Core::Hook::CallMethod<void>(0x69A2C0, physSim, m_pPhysicsInst, 0);
        }
        else if ((m_nFlags & 2u) != 0 && (m_nFlags & 0x18u) == 0 && (m_nFlags & 1u) != 0)
        {
            Core::Hook::CallMethod<void>(0x699F90, physSim, m_pPhysicsInst, 0, 0);
        }
        else
        {
            Core::Hook::CallMethod<void>(0x69A240, physSim, m_pPhysicsInst, 0);
        }
    }

    if (!Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC49F0, this);
}

// 0xA486E0
void __thiscall CPhysical::RemoveFromPhysicsWorld()
{
    if ((m_nPhysUnk1E2 & 0xFu) >= 2u)
        CallVFunc<void>(this, 69, 0);

    if (!m_pPhysicsInst)
        return;

    auto* instBytes = reinterpret_cast<uint8*>(m_pPhysicsInst);
    if ((*reinterpret_cast<uint16*>(instBytes + 0x0A) & 0x2000u) != 0)
        RemoveInstConstraints(m_pPhysicsInst);

    const int32 instType = CallVFunc<int32>(m_pPhysicsInst, 1);
    if (instType != 3 && CallVFunc<int32>(m_pPhysicsInst, 1) < 13)
    {
        const uint16 levelIdx = *reinterpret_cast<uint16*>(instBytes + 8);
        if (levelIdx != 0xFFFFu)
        {
            void* physSim = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
            Core::Hook::CallMethod<void>(0x69A470, physSim, static_cast<uint32>(levelIdx), 0);
        }
    }

    if (Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC2A10, this);
}

// 0xA48770
void __thiscall CPhysical::SyncPhysicsMatrix(bool forceColliderUpdate)
{
    if (!m_pPhysicsInst)
        return;

    CallVFunc<void>(this, 67, &m_pMatrix->pos);

    auto* instBytes = reinterpret_cast<uint8*>(m_pPhysicsInst);
    CopyMatrix3x4(reinterpret_cast<CMatrix*>(instBytes + 0x10), m_pMatrix);

    void* collider = Core::Hook::CallMethod<void*>(0x9E7080, this);
    bool fromParent = false;
    if (!collider)
    {
        void* parent = CallVFunc<void*>(this, 40);
        if (parent && (m_nFlags2 & 0x3C0u) != 0xC0u)
        {
            parent = CallVFunc<void*>(this, 40);
            uint8* unk64 = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(parent) + 0x64);
            if (unk64 && (unk64 + 0x100) != nullptr)
            {
                collider = *reinterpret_cast<void**>(unk64 + 0x17C);
                fromParent = true;
            }
        }
    }

    if (collider)
    {
        CallVFunc<void>(collider, 10);
        if (forceColliderUpdate || fromParent)
        {
            Core::Hook::CallMethod<void>(0x5D35A0, collider, instBytes + 0x10);
            *reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(collider) + 0x294) = 0;
            Core::Hook::Call<void>(0xA71FD0, m_pPhysicsInst);
        }
    }

    const uint16 levelIdx = *reinterpret_cast<uint16*>(instBytes + 8);
    if (levelIdx != 0xFFFFu)
    {
        void* physLevel = *reinterpret_cast<void**>(Core::Rebase(0x1666880));
        Core::Hook::CallMethod<void>(0x6A3960, physLevel, static_cast<uint32>(levelIdx), 0);
        if (CallVFunc<int32>(m_pPhysicsInst, 1) >= 13)
            CallVFunc<void>(m_pPhysicsInst, 28);
    }

    if (Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC4C00, this, 0);
}

// 0xA488D0
void __thiscall CPhysical::SetPhysFlag(bool enable, bool isFlag10)
{
    if (m_pPhysicsInst)
    {
        if (enable)
        {
            m_nFlags |= (isFlag10 ? 0x10u : 0x04u);
            Core::Hook::CallMethod<void>(0xA476F0, this);
            return;
        }

        if (isFlag10)
            m_nFlags &= ~0x10u;
        else
            m_nFlags &= ~0x04u;

        const uint16 levelIdx = *reinterpret_cast<uint16*>(reinterpret_cast<uint8*>(m_pPhysicsInst) + 8);
        if (levelIdx != 0xFFFFu && (m_nFlags & 0x04u) == 0)
        {
            uint8* physLevel = *reinterpret_cast<uint8**>(Core::Rebase(0x1666880));
            const uint8* flagsArr = *reinterpret_cast<uint8**>(physLevel + 0x70);
            if ((flagsArr[levelIdx * 8 + 4] & 3u) == 2u)
            {
                void* physSim = *reinterpret_cast<void**>(Core::Rebase(0x1666884));
                Core::Hook::CallMethod<void>(0x69A470, physSim, static_cast<uint32>(levelIdx), 0);
                Core::Hook::CallMethod<void>(0x69A240, physSim, m_pPhysicsInst, 0);
            }

            if ((m_nFlags2 & 0x3C0u) == 0xC0u && (m_nFlags & 0x1Cu) == 0)
                Core::Hook::CallMethod<void>(0xA47660, this);
        }
    }
    else
    {
        if (isFlag10)
            m_nFlags = (m_nFlags & ~0x10u) | (enable ? 0x10u : 0u);
        else
            m_nFlags = (m_nFlags & ~0x04u) | (enable ? 0x04u : 0u);
    }
}

// 0xA489C0
void __thiscall CPhysical::SetColliderScale(float scale)
{
    void* collider = Core::Hook::CallMethod<void*>(0x9E86F0, this);
    if (!collider)
        return;

    auto** modelInfoPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
    uint8* modelInfo = modelInfoPtrs[m_nModelIndex];
    if (modelInfo && *reinterpret_cast<void**>(modelInfo + 4) == collider)
    {
        collider = CallVFunc<void*>(collider, 6);
        Core::Hook::CallMethod<void>(0x686A80, m_pPhysicsInst, collider);
    }

    CallVFunc<void>(collider, 15, scale);

    struct Vec4 { CVector vec; float w; } temp{};
    void* sub = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(collider) + 0x0C);
    CallVFunc<void>(sub, 20, &temp, scale);
    CallVFunc<void>(collider, 16, &temp);
}

// 0xA48A40
void __thiscall CPhysical::UpdateTransformFromPhysics(void* phInstPtr, bool skipUnk)
{
    auto* instBytes = reinterpret_cast<uint8*>(phInstPtr);
    const auto* instMat = reinterpret_cast<const CMatrix*>(instBytes + 0x10);

    bool valid = IsFiniteFloat(instMat->right.x) && IsFiniteFloat(instMat->right.y) && IsFiniteFloat(instMat->right.z) &&
                 IsFiniteFloat(instMat->up.x) && IsFiniteFloat(instMat->up.y) && IsFiniteFloat(instMat->up.z) &&
                 IsFiniteFloat(instMat->at.x) && IsFiniteFloat(instMat->at.y) && IsFiniteFloat(instMat->at.z) &&
                 IsFiniteFloat(instMat->pos.x) && IsFiniteFloat(instMat->pos.y) && IsFiniteFloat(instMat->pos.z);

    if (valid)
    {
        uint8* archetype = *reinterpret_cast<uint8**>(instBytes + 4);
        uint8* bound = *reinterpret_cast<uint8**>(archetype + 0x0C);
        auto** modelInfoPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
        const float maxZ = *reinterpret_cast<float*>(modelInfoPtrs[m_nModelIndex] + 0x1C) * 4.0f;
        if (*reinterpret_cast<float*>(bound + 8) > maxZ)
            valid = false;
    }

    if (valid && !Core::Hook::CallMethod<bool>(0x89BA00, const_cast<CMatrix*>(instMat), 0.5f))
        valid = false;

    if (valid)
    {
        const float px = *reinterpret_cast<const float*>(instBytes + 0x40);
        const float py = *reinterpret_cast<const float*>(instBytes + 0x44);
        const float pz = *reinterpret_cast<const float*>(instBytes + 0x48);
        const float magSq = px * px + py * py + pz * pz;
        if (magSq > 1e12f)
            valid = false;
    }

    if (!valid)
    {
        CMatrix safeMat;
        CopyMatrix3x4(&safeMat, m_pMatrix);
        safeMat.right = { 1.0f, 0.0f, 0.0f };
        safeMat.up    = { 0.0f, 1.0f, 0.0f };
        safeMat.at    = { 0.0f, 0.0f, 1.0f };

        const int32 levelIdx = *reinterpret_cast<uint16*>(instBytes + 8);
        uint8* physSim = *reinterpret_cast<uint8**>(Core::Rebase(0x1666884));
        uint8* physLevel = *reinterpret_cast<uint8**>(physSim + 4);
        if (levelIdx >= 0 && levelIdx < *reinterpret_cast<uint16*>(physLevel + 0x26))
        {
            const uint8* flagsArr = *reinterpret_cast<uint8**>(physLevel + 0x70);
            if ((flagsArr[levelIdx * 8 + 4] & 3u) == 0)
            {
                void** instArr = *reinterpret_cast<void***>(physLevel + 0x80);
                if (void* levelInst = instArr[levelIdx])
                {
                    CallVFunc<void>(levelInst, 3);

                    void* dynInst = Core::Hook::Call<void*>(0xD0F75A, phInstPtr, 0,
                        Core::Rebase(0x10C0D18), Core::Rebase(0x10C0D58), 0);
                    uint8* arch = *reinterpret_cast<uint8**>(instBytes + 4);
                    void* dynBound = Core::Hook::Call<void*>(0xD0F75A, *reinterpret_cast<void**>(arch + 0x0C), 0,
                        Core::Rebase(0x10C5460), Core::Rebase(0x10C54E0), 0);

                    if (dynInst && dynBound)
                    {
                        const float oldZ = *reinterpret_cast<float*>(reinterpret_cast<uint8*>(dynBound) + 8);
                        Core::Hook::CallMethod<void>(0x688F70, dynBound, 0);

                        if (CallVFunc<int32>(dynInst, 1) == 15 ||
                            ((*reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(dynInst) + 0x60) >> 3) & 1u) == 0)
                        {
                            CallVFunc<void>(dynInst, 28);
                        }
                        else
                        {
                            CallVFunc<void>(dynInst, 49);
                        }

                        if (oldZ != *reinterpret_cast<float*>(reinterpret_cast<uint8*>(dynBound) + 8))
                        {
                            void* level = *reinterpret_cast<void**>(Core::Rebase(0x1666880));
                            Core::Hook::CallMethod<void>(0x6A4470, level, static_cast<uint32>(levelIdx), oldZ, 0);
                        }
                    }
                }
            }
        }

        if ((m_nFlags2 & 0x3C0u) == 0xC0u)
            Core::Hook::CallMethod<void>(0x8AAC50, this);
        else
            CallVFunc<void>(this, 55);

        CallVFunc<void>(this, 1, &safeMat, 0, 1);
    }
    else
    {
        if (!skipUnk)
            CallVFunc<void>(this, 67, &m_pMatrix->pos);

        CopyMatrix3x4(m_pMatrix, instMat);

        if (Core::Hook::CallMethod<bool>(0x9E6F20, this))
            Core::Hook::Call<void>(0xBC4C00, this, 0);
        else if (Core::Hook::CallMethod<bool>(0x9E6F60, this))
            Core::Hook::Call<void>(0xBC49F0, this);
    }

    if ((m_nFlags & 0x08000000u) == 0)
        Core::Hook::CallMethod<void>(0x9E1F70, this);
}

// 0xA48EB0
void __thiscall CPhysical::SetMatrixAndWarp(const CMatrix* mat, bool skipPhysics, int32 warpArg)
{
    CMatrix localMat;
    CopyMatrix3x4(&localMat, mat);

    if (!Core::Hook::CallMethod<bool>(0x89BA00, &localMat, 0.02f))
        Core::Hook::CallMethod<void>(0x89E6B0, &localMat);

    Core::Hook::CallMethod<void>(0x9EAF60, this, &localMat, 0, 0);

    if (m_pMatrix && !skipPhysics)
        CallVFunc<void>(this, 45, warpArg);
}

// 0xA48FA0
void __thiscall CPhysical::ApplyImpactImpulse(
    CPhysical* attacker, const CVector* impulse, const float* unk180,
    const float* unk190, uint32 unk1A0, uint32 unk1A4, uint32 unk1A8)
{
    if (!m_pPhysicsInst || (attacker && !attacker->m_pPhysicsInst))
        return;

    float& peakImpulse = *reinterpret_cast<float*>(&m_nPhysUnk170);
    float& totalImpulse = *reinterpret_cast<float*>(&m_nPhysUnk174);
    const float magSq = impulse->MagnitudeSquared();

    bool updatePeak = true;
    if (magSq <= peakImpulse * peakImpulse && peakImpulse != 0.0f)
    {
        updatePeak = false;
        void* selfArch = Core::Hook::CallMethod<void*>(0x9E86F0, this);
        const float mass = CallVFunc<float>(selfArch, 9);
        const float coeff = Core::StaticRef<float>(0xF35B78);
        if (magSq > (coeff * coeff * mass * mass))
            totalImpulse += std::sqrt(magSq);
    }

    if (updatePeak)
    {
        if (m_pPhysUnk16C)
            Core::Hook::CallMethod<void>(0x9E6580, m_pPhysUnk16C, &m_pPhysUnk16C);
        m_pPhysUnk16C = attacker;
        if (attacker)
            Core::Hook::CallMethod<void>(0x9E6550, attacker, &m_pPhysUnk16C);

        const float mag = (std::sqrt(magSq) < 1e-6f) ? 1e-6f : std::sqrt(magSq);
        peakImpulse = mag;

        m_vecPhysUnk180 = { unk180[0], unk180[1], unk180[2] };
        *reinterpret_cast<float*>(&m_nPhysPad18C) = unk180[3];

        m_vecPhysUnk190 = { unk190[0], unk190[1], unk190[2] };
        *reinterpret_cast<float*>(&m_nPhysPad19C) = unk190[3];

        *reinterpret_cast<uint32*>(&m_vecPhysUnk1A0.x) = unk1A0;
        *reinterpret_cast<uint32*>(&m_vecPhysUnk1A0.y) = unk1A4;
        *reinterpret_cast<uint32*>(&m_vecPhysUnk1A0.z) = unk1A8;

        totalImpulse += mag;
    }

    if (attacker && ((m_nPhysFlags118 >> 4) & 1u) != 0)
    {
        m_nPhysPad114 = *reinterpret_cast<uint32*>(Core::Rebase(0x11DDE74));
        auto** hitEntities = reinterpret_cast<CPhysical**>(&m_vecMoveSpeed);
        bool found = false;
        for (int32 i = 0; i < m_nPhysUnk150; ++i)
        {
            if (hitEntities[i] == attacker)
            {
                found = true;
                break;
            }
        }
        if (!found && m_nPhysUnk150 < 6)
        {
            hitEntities[m_nPhysUnk150] = attacker;
            ++m_nPhysUnk150;
        }
    }

    if (m_nUnk6C && attacker && attacker->m_nUnk6C)
    {
        const uint32 attackerType = (attacker->m_nFlags2 >> 6) & 0xFu;
        if (attackerType > 1u && attackerType < 5u)
        {
            Core::Hook::CallMethod<void>(
                0x4E0520,
                reinterpret_cast<void*>(m_nUnk6C),
                reinterpret_cast<void*>(attacker->m_nUnk6C),
                std::sqrt(magSq));
        }
    }
}

// 0xA49290
void __thiscall CPhysical::ProcessImpactEffect()
{
    if (!m_pPhysUnk16C || !m_pPhysicsInst)
        return;

    uint8* arch = *reinterpret_cast<uint8**>(reinterpret_cast<uint8*>(m_pPhysicsInst) + 4);
    if (!arch)
        return;

    void* bound = *reinterpret_cast<void**>(arch + 0x0C);
    if (!bound)
        return;

    const uint32 matId = CallVFunc<uint32>(bound, 23, 0) & 0xFFu;
    void* matMgr = *reinterpret_cast<void**>(Core::Rebase(0x18A87B8));
    uint8* matInfo = CallVFunc<uint8*>(matMgr, 5, matId);
    if (*reinterpret_cast<uint16*>(matInfo + 0x20) == 0x14u)
        return;

    if (Core::Hook::CallMethod<int32>(0x89BDB0, matMgr, matId) == 0x13)
        return;

    const float peakImpulse = *reinterpret_cast<const float*>(&m_nPhysUnk170);
    if (peakImpulse <= 1000.0f)
    {
        void* col = Core::Hook::CallMethod<void*>(0x9E86F0, this);
        const float mass = CallVFunc<float>(col, 9);
        if ((peakImpulse / mass) <= 5.0f)
            return;
    }

    void* effectMgr = reinterpret_cast<void*>(Core::Rebase(0x12C0400));
    if (!Core::Hook::CallMethod<bool>(0x8C9990, effectMgr, m_pPhysUnk16C))
        return;

    struct ImpactEffectData
    {
        void*  attacker;        // +0x00
        uint32 pad04[3];        // +0x04..+0x0F
        float  unk190[4];       // +0x10..+0x1F
        float  negUnk180[3];    // +0x20..+0x28
        uint32 pad2C;           // +0x2C
        float  unk180_2[3];     // +0x30..+0x38
        uint32 pad3C;           // +0x3C
        uint32 unk1A8;          // +0x40
        uint32 unk1A4;          // +0x44
        uint32 zero48;          // +0x48
        float  intensity;       // +0x4C
        uint8  flag50;          // +0x50
    } data{};

    data.attacker = m_pPhysUnk16C;
    data.unk190[0] = m_vecPhysUnk190.x;
    data.unk190[1] = m_vecPhysUnk190.y;
    data.unk190[2] = m_vecPhysUnk190.z;
    data.unk190[3] = *reinterpret_cast<const float*>(&m_nPhysPad19C);

    data.negUnk180[0] = -m_vecPhysUnk180.x;
    data.negUnk180[1] = -m_vecPhysUnk180.y;
    data.negUnk180[2] = -m_vecPhysUnk180.z;

    data.unk180_2[0] = m_vecPhysUnk180.x;
    data.unk180_2[1] = m_vecPhysUnk180.y;
    data.unk180_2[2] = m_vecPhysUnk180.z;

    data.unk1A8 = *reinterpret_cast<const uint32*>(&m_vecPhysUnk1A0.z);
    data.unk1A4 = *reinterpret_cast<const uint32*>(&m_vecPhysUnk1A0.y);
    data.zero48 = 0;

    float intensity = (peakImpulse - 1000.0f) / (30000.0f - 1000.0f);
    if (intensity <= 0.1f)
        intensity = 0.1f;
    else if (intensity > 1.0f)
        intensity = 1.0f;
    data.intensity = intensity;
    data.flag50 = 0;

    Core::Hook::CallMethod<void>(0x8CDF00, effectMgr, &data, 0);
}

// 0xA494B0
double __thiscall CPhysical::CalcEffectiveMass(const CVector* dir, const CVector* pos)
{
    uint8* collider = Core::Hook::CallMethod<uint8*>(0x9E7080, this);
    if (collider)
    {
        const CVector* com = reinterpret_cast<const CVector*>(collider + 0x100);
        const float rx = pos->x - com->x;
        const float ry = pos->y - com->y;
        const float rz = pos->z - com->z;

        // IDA: v11 = a2[2]*v5 - a2[1]*v6; v12 = a2[0]*v6 - a2[2]*v13; v14 = a2[1]*v13 - a2[0]*v5;
        const float cx = dir->z * ry - dir->y * rz;
        const float cy = dir->x * rz - dir->z * rx;
        const float cz = dir->y * rx - dir->x * ry;

        // IDA: *(this[14] + 4) = m_pPhysicsInst->m_pArchetype (NOT collider!)
        void* arch = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(m_pPhysicsInst) + 4);
        const float* invInertia = CallVFunc<const float*>(arch, 12);
        const float ix = invInertia[0] * cx;
        const float iy = invInertia[1] * cy;
        const float iz = invInertia[2] * cz;

        const float tx = rz * iy - iz * ry;
        const float ty = iz * rx - rz * ix;
        const float tz = ry * ix - iy * rx;

        const float angTerm = dir->x * tx + dir->y * ty + dir->z * tz;
        const float invMass = CallVFunc<float>(arch, 10);
        return 1.0 / (static_cast<double>(invMass) + static_cast<double>(angTerm));
    }

    if (m_pPhysicsInst)
    {
        void* arch = *reinterpret_cast<void**>(reinterpret_cast<uint8*>(m_pPhysicsInst) + 4);
        return CallVFunc<float>(arch, 9);
    }

    return 1.0;
}

// 0xA49650
void __thiscall CPhysical::UpdatePhysicsActiveState()
{
    bool hasActiveCollider = false;
    uint8* collider = Core::Hook::CallMethod<uint8*>(0x9E7080, this);
    if (collider)
    {
        uint8* unk18 = *reinterpret_cast<uint8**>(collider + 0x18);
        if (unk18)
        {
            const uint32 state = *reinterpret_cast<uint32*>(unk18 + 0x18);
            if (state == 0 || state == 2)
                hasActiveCollider = true;
        }
    }

    m_nPhysFlags118 = (m_nPhysFlags118 & ~2u) | ((m_nPhysFlags118 & 1u) << 1);
    uint8* unk6C = reinterpret_cast<uint8*>(m_nUnk6C);
    if (!unk6C || *(unk6C + 0x0E) == 0)
        m_nPhysFlags118 &= ~1u;

    Core::Hook::CallMethod<void>(0xA47AE0, this, static_cast<uint32>(hasActiveCollider));
    CallVFunc<void>(this, 71);
}

// 0xA496E0
void __cdecl CPhysical::CopyAttachTransform(CEntity* src, CEntity* dst, const CVector* offset)
{
    struct Vec4 { CVector vec; float w; };
    Vec4 transformed{};
    Vec4* pTransformed = Core::Hook::Call<Vec4*>(0xC2FCC0, &transformed, src->m_pMatrix, offset);

    Vec4 velQuery{};
    const float timeStepScaled = *reinterpret_cast<float*>(Core::Rebase(0x11DDE7C)) * 45.0f;
    const CVector* vel = CallVFunc<const CVector*>(src, 59, &velQuery);

    Vec4 newPos{
        {
            vel->x * timeStepScaled + pTransformed->vec.x,
            vel->y * timeStepScaled + pTransformed->vec.y,
            vel->z * timeStepScaled + pTransformed->vec.z
        },
        0.0f
    };

    Core::Hook::Call<void>(0x8173C0, dst, 0);
    CopyMatrix3x4(dst->m_pMatrix, src->m_pMatrix);
    CallVFunc<void>(dst, 2, &newPos, 0, 0);

    Vec4 velQuery2{};
    const CVector* vel2 = CallVFunc<const CVector*>(src, 59, &velQuery2);
    Core::Hook::CallMethod<void>(0xA47750, dst, vel2);

    dst->m_nUnk41 = 2;
    Core::Hook::Call<void>(0x817350, dst, 0);
}

// 0xA49840
void __thiscall CPhysical::MarkPhysUnk1F4Bit7()
{
    if ((static_cast<int8>(m_nPhysUnk1F4) < 0) && (m_nPhysUnk1F4 & 0x08u) == 0)
        Core::Hook::CallMethod<void>(0xA48160, this);
    m_nPhysUnk1F4 |= 0x80u;
}

// 0xA49860
bool __thiscall CPhysical::CheckPhysCollisionFilter(void* arg0, bool arg1)
{
    if ((m_nPhysUnk1F4 & 0x08u) != 0)
    {
        CEntity* ent = Core::Hook::Call<CEntity*>(0xA72220, arg0);
        if (!ent || (ent->m_nFlags & 1u) == 0 || ent == this)
            return false;

        uint8* unk6C = reinterpret_cast<uint8*>(ent->m_nUnk6C);
        if (!unk6C || *(unk6C + 0x0E) == 0)
        {
            if (!Core::Hook::CallMethod<bool>(0x4443C0, this) || !unk6C)
                return false;
        }
    }
    else
    {
        if (!arg0 || *reinterpret_cast<void**>(reinterpret_cast<uint8*>(arg0) + 0x0C) != m_pPhysUnk1F8)
            return false;
    }

    if ((m_nPhysUnk1F4 & 0x40u) != 0)
    {
        if (arg1)
        {
            m_nPhysUnk1F4 |= 0x02u;
            return true;
        }
        m_nPhysUnk1F4 |= 0x01u;
        return false;
    }

    m_nPhysUnk1F4 |= 0x01u;
    return true;
}

// 0xA49900
void __thiscall CPhysical::AttachToEntityQuat(
    CEntity* target, int16 boneTag, uint16 flags, const float* offset, const float* quat)
{
    if (!target)
        return;

    if (m_pAttachedTo)
        Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);
    m_pAttachedTo = target;
    Core::Hook::CallMethod<void>(0x9E6550, target, &m_pAttachedTo);

    m_nPhysUnk1E0 = static_cast<uint16>(boneTag);
    m_nPhysUnk1E2 = flags;

    if ((target->m_nFlags2 & 0x3C0u) == 0xC0u)
    {
        m_nPhysUnk1E2 |= 0x200u;
        Core::Hook::CallMethod<void>(0x89E0A0, target, this);
    }

    if (offset)
    {
        m_vecAttachOffset = { offset[0], offset[1], offset[2] };
        *reinterpret_cast<float*>(&m_nPhysPad1CC) = offset[3];
    }
    else
    {
        m_vecAttachOffset = {};
    }

    if (quat)
    {
        m_vecAttachQuat[0] = quat[0];
        m_vecAttachQuat[1] = quat[1];
        m_vecAttachQuat[2] = quat[2];
        m_vecAttachQuat[3] = quat[3];
    }
    else
    {
        m_vecAttachQuat[0] = 0.0f;
        m_vecAttachQuat[1] = 0.0f;
        m_vecAttachQuat[2] = 0.0f;
        m_vecAttachQuat[3] = 1.0f;
    }

    if ((m_nFlags2 & 0x3C0u) == 0xC0u)
        Core::Hook::CallMethod<void>(0x8AAC50, this);
    else
        CallVFunc<void>(this, 55);

    m_nFlags &= ~1u;

    const uint32 targetType = (target->m_nFlags2 >> 6) & 0xFu;
    if (targetType > 1u && targetType < 5u && Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC2A10, this);
}

// 0xA49A60
void __thiscall CPhysical::AttachToWorldQuat(uint16 flags, const float* offset, const float* quat)
{
    CEntity* worldEntity = *reinterpret_cast<CEntity**>(Core::Rebase(0x1666894));
    AttachToEntityQuat(worldEntity, 0, flags, offset, quat);
    m_nPhysUnk1E2 |= 0x10u;
}

// --- Slice 3 (0xA49A90..0xA4AF20, 0x447B20, 0x447B50) ---

void __thiscall CPhysical::AttachToEntity(CEntity* target, int16 targetBoneId, int16 selfBoneId, uint16 attachFlags, const float* offset, const float* quat, const CVector* velOffset, float breakStrength)
{
    if (!target)
        return;

    if (m_pAttachedTo)
        Core::Hook::CallMethod<void>(0x9E6580, m_pAttachedTo, &m_pAttachedTo);

    m_pAttachedTo = target;
    Core::Hook::CallMethod<void>(0x9E6550, target, &m_pAttachedTo);

    m_nPhysUnk1E2 = attachFlags;
    m_nPhysUnk1E0 = static_cast<uint16>(targetBoneId);
    m_vecAttachOffset = {};
    m_vecAttachQuat[0] = 0.0f;
    m_vecAttachQuat[1] = 0.0f;
    m_vecAttachQuat[2] = 0.0f;
    m_vecAttachQuat[3] = 1.0f;

    if ((m_nPhysUnk1E2 & 0x0100) != 0)
    {
        CMatrix localMat{};
        localMat.right = { 1.0f, 0.0f, 0.0f };
        localMat.up    = { 0.0f, 1.0f, 0.0f };
        localMat.at    = { 0.0f, 0.0f, 1.0f };
        localMat.pos   = { 0.0f, 0.0f, 0.0f };

        if (quat)
            Core::Hook::CallMethod<void>(0x5BE050, &localMat, quat);

        if (offset)
        {
            localMat.pos.x = offset[0];
            localMat.pos.y = offset[1];
            localMat.pos.z = offset[2];
            localMat.pad3  = offset[3];
        }

        if (!target->m_pMatrix)
        {
            Core::Hook::CallMethod<void>(0x9EA460, target);
            Core::Hook::CallMethod<void>(0x9E9440, &target->m_vecPos, target->m_pMatrix);
        }

        CMatrix worldMat{};
        Core::Hook::CallMethod<void>(0x42F680, &worldMat, &localMat, target->m_pMatrix);

        using SetMatrixFn = void(__thiscall*)(CPhysical*, const CMatrix*, bool, bool);
        reinterpret_cast<SetMatrixFn>(m_vftable[1])(this, &worldMat, false, false);
    }

    void* physWorld = *reinterpret_cast<void**>(Core::StaticRef<uintptr_t>(0x1666884) + 0x8);

    using GetFragInstFn = void*(__thiscall*)(CEntity*);
    int32 targetBoneIdx = 0;
    void* targetFrag = reinterpret_cast<GetFragInstFn>(target->m_vftable[40])(target);
    if (targetFrag && targetBoneId > 0)
    {
        void* info = Core::Hook::CallMethod<void*>(0x51DDD0, targetFrag);
        targetBoneIdx = Core::Hook::Call<int32>(0xA4B480, info, static_cast<int32>(targetBoneId));
        if (targetBoneIdx < 0)
            targetBoneIdx = 0;
    }

    int32 selfBoneIdx = 0;
    void* selfFrag = reinterpret_cast<GetFragInstFn>(m_vftable[40])(this);
    if (selfFrag && selfBoneId > 0)
    {
        void* info = Core::Hook::CallMethod<void*>(0x51DDD0, selfFrag);
        selfBoneIdx = Core::Hook::Call<int32>(0xA4B480, info, static_cast<int32>(selfBoneId));
        if (selfBoneIdx < 0)
            selfBoneIdx = 0;
    }

    CVector4 pos1{};
    pos1.vec = m_pMatrix->pos;
    CVector4 pos2 = pos1;

    if (velOffset)
    {
        pos2.vec.x = m_pMatrix->right.x * velOffset->x + m_pMatrix->up.x * velOffset->y + m_pMatrix->at.x * velOffset->z + m_pMatrix->pos.x;
        pos2.vec.y = m_pMatrix->right.y * velOffset->x + m_pMatrix->up.y * velOffset->y + m_pMatrix->at.y * velOffset->z + m_pMatrix->pos.y;
        pos2.vec.z = m_pMatrix->right.z * velOffset->x + m_pMatrix->up.z * velOffset->y + m_pMatrix->at.z * velOffset->z + m_pMatrix->pos.z;
    }

    if (offset)
    {
        if (!target->m_pMatrix)
        {
            Core::Hook::CallMethod<void>(0x9EA460, target);
            Core::Hook::CallMethod<void>(0x9E9440, &target->m_vecPos, target->m_pMatrix);
        }

        CMatrix targetMat{};
        Core::Hook::CallMethod<void>(0x441ED0, &targetMat, target->m_pMatrix);

        if (targetBoneIdx > 0)
        {
            uint8* frag = reinterpret_cast<uint8*>(reinterpret_cast<GetFragInstFn>(m_vftable[40])(this));
            if (frag)
            {
                uint8* fragEntry = *reinterpret_cast<uint8**>(frag + 0x4);
                if (fragEntry)
                {
                    uint8* skel = *reinterpret_cast<uint8**>(fragEntry + 0xC);
                    if (skel && skel[0x4] == 0xC)
                    {
                        uintptr_t boneMat = *reinterpret_cast<uintptr_t*>(skel + 0x84) + static_cast<uintptr_t>(targetBoneIdx) * 0x40;
                        Core::Hook::CallMethod<void>(0x441ED0, &targetMat, reinterpret_cast<void*>(boneMat));
                        Core::Hook::CallMethod<void>(0x513430, &targetMat, frag + 0x10);
                    }
                }
            }
        }

        pos1.vec.x = targetMat.right.x * offset[0] + targetMat.up.x * offset[1] + targetMat.at.x * offset[2] + targetMat.pos.x;
        pos1.vec.y = targetMat.right.y * offset[0] + targetMat.up.y * offset[1] + targetMat.at.y * offset[2] + targetMat.pos.y;
        pos1.vec.z = targetMat.right.z * offset[0] + targetMat.up.z * offset[1] + targetMat.at.z * offset[2] + targetMat.pos.z;
    }

    Core::Hook::CallMethod<void>(0xA47660, this);

    if (((m_nPhysUnk1E2 >> 5) & 1) != 0)
    {
        void* constraint = Core::Hook::CallMethod<void*>(0x6B69D0, physWorld, &pos1, &pos2, target->m_pPhysicsInst, m_pPhysicsInst, targetBoneIdx, selfBoneIdx, 0.0f, 1, 0);
        if (constraint && breakStrength > 0.0f)
            *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(constraint) + 0x138) = breakStrength;

        Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000, 1);
        Core::Hook::CallMethod<void>(0x686AD0, target->m_pPhysicsInst, 0x2000, 1);
    }

    if (((m_nPhysUnk1E2 >> 6) & 1) != 0)
    {
        void* constraint = Core::Hook::CallMethod<void*>(0x6B69D0, physWorld, &pos1, &pos2, target->m_pPhysicsInst, m_pPhysicsInst, targetBoneIdx, selfBoneIdx, 0.0f, 1, 0);
        if (constraint)
            Core::Hook::CallMethod<void>(0x6FA810, constraint);

        Core::Hook::CallMethod<void>(0x686AD0, m_pPhysicsInst, 0x2000, 1);
        Core::Hook::CallMethod<void>(0x686AD0, target->m_pPhysicsInst, 0x2000, 1);
    }

    if ((target->m_nFlags2 & 0x3C0) == 0xC0)
        Core::Hook::CallMethod<void>(0x89E0A0, target, this);

    if (Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC2A10, this);
}

void __thiscall CPhysical::UpdateAttachRotation(const CVector* pos, const CVector* rot)
{
    if ((m_nPhysUnk1E2 & 0xF) != 4)
        return;

    void* physWorld = *reinterpret_cast<void**>(Core::StaticRef<uintptr_t>(0x1666884) + 0x8);
    void* constraints[8]{};
    const int32 count = Core::Hook::CallMethod<int32>(0x6B7700, physWorld, m_pPhysicsInst, constraints, 8);

    for (int32 i = 0; i < count; ++i)
    {
        uint8* constraint = reinterpret_cast<uint8*>(constraints[i]);
        if (*reinterpret_cast<int32*>(constraint + 0x1C) != 0)
            continue;

        Core::Hook::CallMethod<void>(0x6F9FC0, constraint, pos);

        if ((constraint[0xB8] & 1) == 0 || !rot)
            continue;

        const float sa = std::sin(rot->z);
        const float ca = std::cos(rot->z);
        const float sb = std::sin(rot->x);
        const float cb = std::cos(rot->x);
        const float sc = std::sin(rot->y);
        const float cc = std::cos(rot->y);

        CMatrix rotMat{};
        rotMat.right = { cc * ca - sa * sb * sc, cc * sa + ca * sb * sc, -cb * sc };
        rotMat.up    = { -sa * cb,               ca * cb,                sb };
        rotMat.at    = { sc * ca + sa * sb * cc, sc * sa - ca * sb * cc, cb * cc };

        float quat[4]{};
        Core::Hook::CallMethod<void>(0x506DB0, quat, &rotMat);

        for (int32 j = 0; j < 4; ++j)
        {
            *reinterpret_cast<float*>(constraint + 0x30 + j * 4) = quat[j];
            *reinterpret_cast<float*>(constraint + 0x40 + j * 4) = quat[j];
        }
    }
}

void __thiscall CPhysical::ProcessAttach(uint32 flags)
{
    m_nFlags |= 1u;
    m_fBoundRadiusOffset = 0.0f;

    if (m_pAttachedTo)
    {
        auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
        if ((target->m_nFlags2 & 0x3C0) == 0xC0)
            Core::Hook::CallMethod<void>(0x89E0E0, target, this);
    }

    const uint8 flag0 = static_cast<uint8>(flags & 1u);
    if (flag0 && m_pPhysicsInst && *reinterpret_cast<int16*>(reinterpret_cast<uintptr_t>(m_pPhysicsInst) + 0x8) != -1)
        Core::Hook::CallMethod<void>(0xA47660, this);

    if (((m_nPhysUnk1E2 >> 5) & 1) == 0 && ((m_nPhysUnk1E2 >> 6) & 1) == 0)
    {
        if (m_pAttachedTo && (flags & 2u) != 0 && flag0)
        {
            auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
            bool handled = false;

            if ((target->m_nFlags2 & 0x3C0) == 0xC0 && static_cast<int16>(m_nPhysUnk1E0) > 0)
            {
                using GetFragInstFn = void*(__thiscall*)(CEntity*);
                auto getTargetFrag = reinterpret_cast<GetFragInstFn>(target->m_vftable[40]);
                void* fragInst = getTargetFrag(target);
                if (fragInst)
                {
                    using GetFragPosFn = const CVector*(__thiscall*)(void*);
                    const CVector* fragPos = reinterpret_cast<GetFragPosFn>((*reinterpret_cast<void***>(fragInst))[3])(fragInst);
                    const CVector* fragBase = reinterpret_cast<const CVector*>(reinterpret_cast<uintptr_t>(fragInst) + 0x40);
                    const float dx = fragBase->x - fragPos->x;
                    const float dy = fragBase->y - fragPos->y;
                    const float dz = fragBase->z - fragPos->z;

                    if (dx * dx + dy * dy + dz * dz < 1.0f)
                    {
                        CMatrix mat1{};
                        CMatrix mat2{};
                        const int32 boneId = static_cast<int16>(m_nPhysUnk1E0);
                        if (Core::Hook::Call<bool>(0xA4B9D0, &mat1, getTargetFrag(target), boneId, 1) &&
                            Core::Hook::Call<bool>(0xA4B9D0, &mat2, getTargetFrag(target), boneId, 0))
                        {
                            CMatrix attachMat{};
                            attachMat.right = { 1.0f, 0.0f, 0.0f };
                            attachMat.up    = { 0.0f, 1.0f, 0.0f };
                            attachMat.at    = { 0.0f, 0.0f, 1.0f };
                            attachMat.pos   = m_vecAttachOffset;
                            attachMat.pad3  = *reinterpret_cast<const float*>(&m_nPhysPad1CC);

                            Core::Hook::CallMethod<void>(0x5BE050, &attachMat, m_vecAttachQuat);

                            void* curFrag = getTargetFrag(target);
                            const CMatrix* targetFragMat = reinterpret_cast<const CMatrix*(__thiscall*)(void*)>((*reinterpret_cast<void***>(curFrag))[3])(curFrag);
                            Core::Hook::CallMethod<void>(0x513430, &mat1, &attachMat);
                            Core::Hook::CallMethod<void>(0x513430, &mat1, targetFragMat);

                            Core::Hook::CallMethod<void>(0x513430, &mat2, &attachMat);
                            Core::Hook::CallMethod<void>(0x513430, &mat2, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(curFrag) + 0x10));

                            const float invDt = Core::StaticRef<float>(0x11DDE80);
                            CVector vel{};
                            vel.x = (mat2.pos.x - mat1.pos.x) * invDt;
                            vel.y = (mat2.pos.y - mat1.pos.y) * invDt;
                            vel.z = (mat2.pos.z - mat1.pos.z) * invDt;

                            if (vel.x * vel.x + vel.y * vel.y + vel.z * vel.z > 900.0f)
                            {
                                Core::Hook::CallMethod<void>(0x454180, &vel);
                                const float maxSpd = Core::Hook::CallMethod<bool>(0x4443C0, m_pAttachedTo) ? 5.0f : 30.0f;
                                vel.x *= maxSpd;
                                vel.y *= maxSpd;
                                vel.z *= maxSpd;
                            }
                            Core::Hook::CallMethod<void>(0xA47750, this, &vel);

                            CMatrix deltaMat{};
                            Core::Hook::CallMethod<void>(0x516970, &deltaMat, &mat2, &mat1);

                            CVector4 axisAngle{};
                            Core::Hook::CallMethod<void>(0x5BE130, &deltaMat, &axisAngle);

                            CVector axis{};
                            float angle = 0.0f;
                            Core::Hook::CallMethod<void>(0x516F50, &axisAngle, &axis, &angle);

                            CVector turn{};
                            turn.x = axis.x * angle * invDt;
                            turn.y = axis.y * angle * invDt;
                            turn.z = axis.z * angle * invDt;

                            if (turn.x * turn.x + turn.y * turn.y + turn.z * turn.z > 100.0f)
                            {
                                Core::Hook::CallMethod<void>(0x454180, &turn);
                                const float maxTurn = Core::Hook::CallMethod<bool>(0x4443C0, m_pAttachedTo) ? 5.0f : 10.0f;
                                turn.x *= maxTurn;
                                turn.y *= maxTurn;
                                turn.z *= maxTurn;
                            }
                            Core::Hook::CallMethod<void>(0xA47840, this, &turn);

                            handled = true;
                        }
                    }
                }
            }

            if (Core::Hook::CallMethod<bool>(0x4E0460, m_pAttachedTo) &&
                reinterpret_cast<CEntity*>(m_pAttachedTo)->m_pPhysicsInst &&
                !handled)
            {
                const CVector* myPos = m_pMatrix ? &m_pMatrix->pos : &m_vecPos;
                CVector4 moveSpd{};
                CVector4 turnSpd{};
                Core::Hook::CallMethod<CVector*>(0xA47FB0, m_pAttachedTo, &moveSpd, myPos, 1, 0);
                Core::Hook::CallMethod<void>(0xA47750, this, &moveSpd);
                Core::Hook::CallMethod<CVector*>(0xA478E0, m_pAttachedTo, &turnSpd);
                Core::Hook::CallMethod<void>(0xA47840, this, &turnSpd);
            }
        }
    }
    else
    {
        void* physWorld = *reinterpret_cast<void**>(Core::StaticRef<uintptr_t>(0x1666884) + 0x8);
        void* constraints[8]{};
        const int32 count = Core::Hook::CallMethod<int32>(0x6B7700, physWorld, m_pPhysicsInst, constraints, 8);
        for (int32 i = 0; i < count; ++i)
        {
            if (constraints[i])
                Core::Hook::CallMethod<void>(0x6B66F0, physWorld, constraints[i]);
        }
    }

    if (m_pAttachedTo && (flags & 4u) != 0 &&
        Core::Hook::Call<bool>(0x99A260, this, m_pMatrix, m_pAttachedTo))
    {
        m_nPhysUnk1E2 = 1;
    }
    else
    {
        reinterpret_cast<void(__thiscall*)(CPhysical*)>(m_vftable[0x110 / 4])(this);
    }

    if (!Core::Hook::CallMethod<bool>(0x9E6F20, this))
        Core::Hook::Call<void>(0xBC49F0, this);
}

void __thiscall CPhysical::UpdateAttachment()
{
    if (Core::StaticRef<int32>(0x10F8044) >= 2 && m_pAttachedTo && (m_nFlags2 & 0x3C0) == 0x100)
    {
        void* objData = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(this) + 0x25C);
        if (objData && *reinterpret_cast<int32*>(reinterpret_cast<uintptr_t>(objData) + 0x18) == 0x24)
        {
            auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
            void* fragInst = reinterpret_cast<void*(__thiscall*)(CEntity*)>(target->m_vftable[40])(target);
            if (fragInst)
            {
                void* info = Core::Hook::CallMethod<void*>(0x51DDD0, fragInst);
                const int32 bone = Core::Hook::Call<int32>(0xA4B4C0, info, static_cast<int32>(static_cast<int16>(m_nPhysUnk1E0)));
                if (Core::Hook::CallMethod<bool>(0x517A60, fragInst, bone))
                {
                    void* unk = Core::Hook::Call<void*>(0x99C2B0, this);
                    if (unk)
                    {
                        Core::Hook::Call<void>(0x99C2F0, unk);
                        reinterpret_cast<void(__thiscall*)(CPhysical*, int32)>(m_vftable[0x114 / 4])(this, 3);
                        return;
                    }
                }
            }
        }
    }

    const uint16 mode = m_nPhysUnk1E2 & 0xF;
    if (mode == 2)
    {
        auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
        if (!target)
        {
            if ((m_nFlags2 & 0x3C0) == 0x100 && *reinterpret_cast<uint8*>(reinterpret_cast<uintptr_t>(this) + 0x22A) != 2)
                m_nFlags |= 0x400u;

            reinterpret_cast<void(__thiscall*)(CPhysical*, int32)>(m_vftable[0x114 / 4])(this, 1);
            return;
        }

        if ((target->m_nFlags2 & 0x3C0) == 0xC0 && target->m_pPhysicsInst &&
            target->m_pPhysicsInst == *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(target) + 0x7C4) &&
            ((m_nPhysUnk1E2 >> 12) & 1) != 0)
        {
            reinterpret_cast<void(__thiscall*)(CPhysical*, int32)>(m_vftable[0x114 / 4])(this, 1);
            return;
        }

        CMatrix localMat{};
        localMat.right = { 1.0f, 0.0f, 0.0f };
        localMat.up    = { 0.0f, 1.0f, 0.0f };
        localMat.at    = { 0.0f, 0.0f, 1.0f };
        localMat.pos   = { 0.0f, 0.0f, 0.0f };

        Core::Hook::CallMethod<void>(0x5BE050, &localMat, m_vecAttachQuat);
        localMat.pos  = m_vecAttachOffset;
        localMat.pad3 = *reinterpret_cast<const float*>(&m_nPhysPad1CC);

        if (!target->m_pMatrix)
        {
            Core::Hook::CallMethod<void>(0x9EA460, target);
            Core::Hook::CallMethod<void>(0x9E9440, &target->m_vecPos, target->m_pMatrix);
        }

        CMatrix targetMat{};
        Core::Hook::CallMethod<void>(0x441ED0, &targetMat, target->m_pMatrix);

        if (static_cast<int16>(m_nPhysUnk1E0) > 0)
        {
            const uint32 targetType = (target->m_nFlags2 >> 6) & 0xF;
            if (targetType > 1 && targetType < 5 && Core::Hook::CallMethod<int32>(0x447A20, target) != 0)
            {
                bool canGetBone = true;
                if ((target->m_nFlags2 & 0x3C0) == 0xC0 &&
                    (*reinterpret_cast<uint8*>(reinterpret_cast<uintptr_t>(target) + 0x29F) & 1) == 0)
                {
                    if (!Core::Hook::Call<bool>(0x405160))
                        canGetBone = false;
                    else
                        Core::Hook::CallMethod<void>(0x89DB40, target);
                }

                if (canGetBone)
                {
                    void* boneMat = Core::Hook::CallMethod<void*>(0x447A60, target, static_cast<int32>(static_cast<int16>(m_nPhysUnk1E0)));
                    Core::Hook::CallMethod<void>(0x441ED0, &targetMat, boneMat);
                }
            }
        }

        CMatrix worldMat{};
        Core::Hook::CallMethod<void>(0x42F680, &worldMat, &localMat, &targetMat);

        using SetMatrixFn = void(__thiscall*)(CPhysical*, const CMatrix*, bool, bool);
        reinterpret_cast<SetMatrixFn>(m_vftable[1])(this, &worldMat, false, false);

        if (((m_nPhysUnk1E2 >> 9) & 1) != 0)
            Core::Hook::CallMethod<void>(0x9E1190, this);

        Core::Hook::CallMethod<void>(0x9E1F70, this);

        if ((target->m_nFlags2 & 0x3C0) == 0xC0)
        {
            const uint32 arg1 = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(target) + 0xB0);
            const uint32 arg2 = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(target) + 0xB8);
            Core::Hook::CallMethod<void>(0xB2A0E0, m_portalTracker, arg1, arg2);
        }
        return;
    }

    if (mode == 3 || mode == 4 || mode == 7)
    {
        auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
        if (target && target->m_pPhysicsInst)
        {
            void* physWorld = *reinterpret_cast<void**>(Core::StaticRef<uintptr_t>(0x1666884) + 0x8);
            void* constraints[8]{};
            const int32 count = Core::Hook::CallMethod<int32>(0x6B7700, physWorld, m_pPhysicsInst, constraints, 8);
            for (int32 i = 0; i < count; ++i)
            {
                auto* c = reinterpret_cast<uint8*>(constraints[i]);
                if (c && (*reinterpret_cast<void**>(c + 0x18) == target->m_pPhysicsInst ||
                          *reinterpret_cast<void**>(c + 0x1C) == target->m_pPhysicsInst))
                {
                    return;
                }
            }
        }
        reinterpret_cast<void(__thiscall*)(CPhysical*)>(m_vftable[0x110 / 4])(this);
        return;
    }

    if (mode == 1)
    {
        auto* target = reinterpret_cast<CEntity*>(m_pAttachedTo);
        if (!target)
        {
            reinterpret_cast<void(__thiscall*)(CPhysical*)>(m_vftable[0x110 / 4])(this);
            return;
        }

        const float r1 = reinterpret_cast<float(__thiscall*)(CEntity*)>(target->m_vftable[0x58 / 4])(target);
        const float r2 = reinterpret_cast<float(__thiscall*)(CPhysical*)>(m_vftable[0x58 / 4])(this);
        const float rSum = r1 + r2;

        CVector4 b1{};
        CVector4 b2{};
        const CVector* c1 = reinterpret_cast<const CVector*(__thiscall*)(CEntity*, CVector4*)>(target->m_vftable[0x54 / 4])(target, &b1);
        const CVector* c2 = reinterpret_cast<const CVector*(__thiscall*)(CPhysical*, CVector4*)>(m_vftable[0x54 / 4])(this, &b2);

        const float dx = c2->x - c1->x;
        const float dy = c2->y - c1->y;
        const float dz = c2->z - c1->z;
        if (dx * dx + dy * dy + dz * dz > rSum * rSum)
            reinterpret_cast<void(__thiscall*)(CPhysical*)>(m_vftable[0x110 / 4])(this);
    }
}

bool __thiscall CPhysical::CanBeDamaged(CEntity* inflictor, int32 weaponType, bool checkUnk) const
{
    if (m_nUnk6C != 0)
    {
        const auto* unk6C = reinterpret_cast<const uint8*>(m_nUnk6C);
        if (unk6C[0xE] != 0 && checkUnk)
            return false;
    }

    if ((m_nPhysFlags118 & 0x400) != 0)
        return false;

    void* weaponInfo = Core::Hook::Call<void*>(0x924E80, weaponType);
    if (!weaponInfo)
        return false;

    const int32 damageType = *reinterpret_cast<int32*>(reinterpret_cast<uintptr_t>(weaponInfo) + 0xC);

    if (inflictor)
    {
        const uint32 type = (inflictor->m_nFlags2 >> 6) & 0xF;
        if (type > 1 && type < 5)
        {
            if (Core::Hook::CallMethod<const CPhysical*>(0x426800, inflictor) == this &&
                damageType != 3 && damageType != 4)
            {
                return false;
            }
        }
    }

    if ((m_nPhysFlags118 & 0x800) != 0 &&
        damageType != 3 && weaponType != 0x35 && weaponType != 0x33 && weaponType != 0x3A)
    {
        if (!inflictor)
            return false;

        const uint32 type = (inflictor->m_nFlags2 >> 6) & 0xF;
        if (type == 3)
        {
            if (*reinterpret_cast<const uint8*>(reinterpret_cast<uintptr_t>(inflictor) + 0x219) == 0)
                return false;
        }
        else if (type == 2)
        {
            if ((inflictor->m_nFlags2 & 0x7C00) != 0)
                return false;
        }
        else
        {
            return false;
        }
    }

    if ((m_nPhysFlags118 & 0x2000) != 0 &&
        damageType != 3 && weaponType != 0x35 && weaponType != 0x33 && weaponType != 0x3A)
    {
        if (!inflictor)
            return false;

        const uint32 type = (inflictor->m_nFlags2 >> 6) & 0xF;
        if (type == 3)
        {
            const uint16 id = *reinterpret_cast<const uint16*>(reinterpret_cast<uintptr_t>(inflictor) + 0xBB4);
            if (!Core::Hook::CallMethod<bool>(0xA485A0, const_cast<CPhysical*>(this), static_cast<uint32>(id)))
                return false;
        }
        else if (type == 2)
        {
            const uint8* driver = *reinterpret_cast<const uint8* const*>(reinterpret_cast<uintptr_t>(inflictor) + 0xFA0);
            if (!driver)
                return false;

            const uint16 id = *reinterpret_cast<const uint16*>(driver + 0xBB4);
            if (!Core::Hook::CallMethod<bool>(0xA485A0, const_cast<CPhysical*>(this), static_cast<uint32>(id)))
                return false;
        }
        else
        {
            return false;
        }
    }

    if ((m_nPhysFlags118 & 0x4000) != 0 &&
        damageType != 3 && weaponType != 0x35 && weaponType != 0x33 && weaponType != 0x3A &&
        inflictor)
    {
        const uint32 type = (inflictor->m_nFlags2 >> 6) & 0xF;
        if (type == 3)
        {
            if (*reinterpret_cast<const uint32*>(reinterpret_cast<uintptr_t>(inflictor) + 0xBB4) == static_cast<uint32>(m_nPhysUnk11C))
                return false;
        }
        else if (type == 2)
        {
            const uint8* driver = *reinterpret_cast<const uint8* const*>(reinterpret_cast<uintptr_t>(inflictor) + 0xFA0);
            if (driver && *reinterpret_cast<const uint32*>(driver + 0xBB4) == static_cast<uint32>(m_nPhysUnk11C))
                return false;
        }
    }

    if (damageType == 1)
    {
        if ((m_nPhysFlags118 & 0x200) != 0)
            return false;
    }
    else if (damageType == 2)
    {
        if ((m_nPhysFlags118 & 0x40) != 0)
            return false;
    }

    if (damageType == 3)
    {
        if (Core::Hook::CallMethod<bool>(0x95C0F0, const_cast<CPhysical*>(this)))
            return false;
    }
    else if (damageType == 4)
    {
        if ((m_nPhysFlags118 & 0x80) != 0)
            return false;
    }

    if ((weaponType == 0x2E || weaponType == 0x31 || weaponType == 0x32 || weaponType == 0x36) &&
        (m_nPhysFlags118 & 0x100) != 0)
    {
        return false;
    }

    return true;
}

void __thiscall CPhysical::UpdateSubmersion()
{
    void* modelInfo = reinterpret_cast<void**>(Core::Rebase(0x15F73B0))[m_nModelIndex];

    const CVector* bmin = reinterpret_cast<const CVector*(__thiscall*)(CPhysical*)>(m_vftable[0x60 / 4])(this);
    const CVector* bmax = reinterpret_cast<const CVector*(__thiscall*)(CPhysical*)>(m_vftable[0x64 / 4])(this);

    const CVector localCenter = {
        (bmax->x + bmin->x) * 0.5f,
        (bmax->y + bmin->y) * 0.5f,
        (bmax->z + bmin->z) * 0.5f
    };
    const CVector half = {
        (bmax->x - bmin->x) * 0.5f,
        (bmax->y - bmin->y) * 0.5f,
        (bmax->z - bmin->z) * 0.5f
    };

    const CVector worldCenter = {
        m_pMatrix->right.x * localCenter.x + m_pMatrix->up.x * localCenter.y + m_pMatrix->at.x * localCenter.z + m_pMatrix->pos.x,
        m_pMatrix->right.y * localCenter.x + m_pMatrix->up.y * localCenter.y + m_pMatrix->at.y * localCenter.z + m_pMatrix->pos.y,
        m_pMatrix->right.z * localCenter.x + m_pMatrix->up.z * localCenter.y + m_pMatrix->at.z * localCenter.z + m_pMatrix->pos.z
    };

    const float halfZExtent =
        std::fabs(m_pMatrix->right.z) * half.x +
        std::fabs(m_pMatrix->up.z)    * half.y +
        std::fabs(m_pMatrix->at.z)    * half.z;

    const float bottomZ = worldCenter.z - halfZExtent;

    const uint32 tlsIdx = Core::StaticRef<uint32>(0x17237B0);
    const uintptr_t* tlsArray = reinterpret_cast<const uintptr_t*>(__readfsdword(0x2C));
    const uint32 tlsFlags = *reinterpret_cast<const uint32*>(tlsArray[tlsIdx] + 0x70);
    const int32 waterIdx = ((tlsFlags & 2u) != 0 || (tlsFlags & 8u) != 0)
        ? Core::StaticRef<int32>(0x10F9AC0)
        : Core::StaticRef<int32>(0x10F9AC4);
    const float minWaterZ = reinterpret_cast<const float*>(Core::Rebase(0x15B75BC))[waterIdx];

    float waterZ = 0.0f;
    if (bottomZ < minWaterZ &&
        Core::Hook::Call<bool>(0x941DF0, worldCenter.x, worldCenter.y, worldCenter.z, &waterZ, 6.0f, 20.0f, 0))
    {
        if ((m_nFlags & 8u) != 0)
        {
            m_nPhysFlags118 &= ~0x80000u;
            return;
        }

        if ((m_nPhysFlags118 & 0x80000u) != 0)
        {
            if ((m_nFlags & 0x20000000u) == 0 && waterZ >= bottomZ &&
                m_vecPhysUnk200.z + *reinterpret_cast<const float*>(reinterpret_cast<uintptr_t>(modelInfo) + 0x38) >= waterZ)
            {
                m_nFlags |= 0x20000000u;
                m_nPhysFlags118 |= 0x80000u;
            }
            return;
        }

        if ((m_nFlags & 0x8000000u) != 0)
        {
            m_nFlags &= ~0x20000000u;
            m_nPhysFlags118 |= 0x80000u;
            *reinterpret_cast<uint32*>(&m_physAnimArray[0x20]) = 0;
            return;
        }

        const bool aboveWater = (waterZ >= bottomZ);
        CVector p1 = { worldCenter.x, worldCenter.y, bottomZ };
        CVector p2 = { worldCenter.x, worldCenter.y, waterZ };
        const bool blocked = Core::Hook::Call<bool>(0x9933E0, &p1, &p2, 0, 6, 0);
        const bool submerged = (aboveWater && !blocked);
        const bool hasLowFlag = (m_nPhysFlags118 & 3u) != 0;

        if (submerged && !hasLowFlag)
        {
            m_nFlags &= ~0x20000000u;
            m_nPhysFlags118 |= 0x80000u;
            *reinterpret_cast<uint32*>(&m_physAnimArray[0x20]) = 0;
            return;
        }

        m_nFlags |= 0x20000000u;
        m_nPhysFlags118 |= 0x80000u;
        return;
    }

    m_nFlags &= ~0x20000000u;
    m_nPhysFlags118 |= 0x80000u;
    *reinterpret_cast<uint32*>(&m_physAnimArray[0x20]) = 0;
}

bool __thiscall CPhysical::IsPhysFlag118Bit3Clear() const
{
    return ((~m_nPhysFlags118 >> 3) & 1u) != 0;
}

void __thiscall CPhysical::SetPhysVec200(const float* vec4)
{
    m_vecPhysUnk200.x = vec4[0];
    m_vecPhysUnk200.y = vec4[1];
    m_vecPhysUnk200.z = vec4[2];
    *reinterpret_cast<float*>(&m_nPhysPad20C) = vec4[3];
}
