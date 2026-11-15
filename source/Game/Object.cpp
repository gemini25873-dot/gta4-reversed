#include "StdInc.h"
#include "Game/Object.h"
#include "Game/Pools.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CObject::InjectHooks()
{
    Core::Hook::Install(0x8E0550, &CObject::Ctor);
    Core::Hook::Install(0x8E5840, &CObject::CtorWithModel);
    Core::Hook::Install(0x8E5810, &CObject::DeletingDtor);
    Core::Hook::Install(0x8DF650, &CObject::Init);

    Core::Log::Info("CObject hooks yuklendi (4 fonksiyon)");
}

CObject* __thiscall CObject::Ctor()
{
    CPhysical::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD6D47C));

    Core::Hook::CallMethod<void>(0xCF0E80, m_audPlaceableTracker);
    *reinterpret_cast<void**>(m_audPlaceableTracker) = reinterpret_cast<void*>(Core::Rebase(0xD69690));
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 8) = 0;

    Core::Hook::CallMethod<void>(0xC877A0, m_audObjectAudioEntity);
    Init();
    m_nCreatedBy = 0;
    return this;
}

CObject* __thiscall CObject::CtorWithModel(int32 modelIndex, uint8 bInitBounds, uint8 bCreatePhysics)
{
    CPhysical::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD6D47C));

    Core::Hook::CallMethod<void>(0xCF0E80, m_audPlaceableTracker);
    *reinterpret_cast<void**>(m_audPlaceableTracker) = reinterpret_cast<void*>(Core::Rebase(0xD69690));
    *reinterpret_cast<uint32*>(m_audPlaceableTracker + 8) = 0;

    Core::Hook::CallMethod<void>(0xC877A0, m_audObjectAudioEntity);
    Init();

    Core::Hook::CallMethod<void>(0x8DF5C0, this, modelIndex);
    if (bCreatePhysics)
        Core::Hook::CallMethod<void>(0x8E49E0, this);
    if (bInitBounds)
        Core::Hook::CallMethod<void>(0x8E0640, this);

    Core::Hook::CallMethod<void>(0x9E6890, this, -1);
    return this;
}

CObject* __thiscall CObject::DeletingDtor(uint8 flags)
{
    Core::Hook::CallMethod<void>(0x8E4870, this);
    if ((flags & 1) != 0)
        Core::Hook::CallMethod<void>(0xC79660, CPools::ms_pObjectPool, this);
    return this;
}

void __thiscall CObject::Init()
{
    m_nFlags2 = (m_nFlags2 & ~0x2C0u) | 0x110u;

    m_pMatrix->right = { 1.0f, 0.0f, 0.0f };
    m_pMatrix->up    = { 0.0f, 1.0f, 0.0f };
    m_pMatrix->at    = { 0.0f, 0.0f, 1.0f };
    m_pMatrix->pos   = { 0.0f, 0.0f, 0.0f };

    void** trackerVtbl = *reinterpret_cast<void***>(m_audPlaceableTracker);
    using TrackerAttachFn = void(__thiscall*)(void*, CObject*);
    reinterpret_cast<TrackerAttachFn>(trackerVtbl[2])(m_audPlaceableTracker, this);

    m_nObjFlags214 = (m_nObjFlags214 & ~0x6Eu) | 0x11u;
    m_nCreatedBy = 1;
    m_nObjUnk210 = 0x2000u;
    m_nObjUnk218 = 0;
    m_nObjUnk21C = 0;

    using SetUnkF0Fn = void(__thiscall*)(CObject*, float);
    reinterpret_cast<SetUnkF0Fn>(m_vftable[60])(this, Core::StaticRef<float>(0xDAF864));

    m_nObjUnk240 = 0;
    m_nObjUnk244 = 0;
    m_nObjUnk248 = 0;
    m_nObjUnk22B = 0;
    m_nObjUnk224 = 0;
    *reinterpret_cast<uint32*>(m_audObjectAudioEntity + 0x0C) = 0;
    *reinterpret_cast<uint32*>(m_audObjectAudioEntity + 0x10) = 0;
    *reinterpret_cast<uint32*>(m_audObjectAudioEntity + 0x14) = 0;

    m_fObjScale250 = Core::StaticRef<float>(0xD50398);
    m_nObjUnk220 = -1;
    m_nObjUnk228 = -1;
    m_nObjUnk254 = 0;
    m_nObjUnk258 = 0;
    m_pObjUnk25C = nullptr;
    m_pObjUnk260 = nullptr;
    *reinterpret_cast<uint32*>(m_audObjectAudioEntity + 0x18) = 0;
    *reinterpret_cast<uint32*>(m_audObjectAudioEntity + 0x1C) = 0;
}
