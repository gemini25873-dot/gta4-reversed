#include "StdInc.h"
#include "Game/Physical.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>

void CPhysical::InjectHooks()
{
    Core::Hook::Install(0xA4B290, &CPhysical::Ctor);
    Core::Hook::Install(0xA47570, &CPhysical::Dtor);
    Core::Hook::Install(0xA48610, &CPhysical::DeletingDtor);
    Core::Hook::Install(0xA48E40, &CPhysical::SetPosition);
    Core::Hook::Install(0xA47A40, &CPhysical::SetHeading);

    Core::Log::Info("CPhysical hooks yuklendi (5 fonksiyon)");
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
