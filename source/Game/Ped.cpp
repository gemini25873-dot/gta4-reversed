#include "StdInc.h"
#include "Game/Ped.h"
#include "Game/Pools.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CPed::InjectHooks()
{
    Core::Hook::Install(0x8ABF10, &CPed::Ctor);
    Core::Hook::Install(0x8ACAC0, &CPed::DeletingDtor);
    Core::Hook::Install(0x8A9030, &CPed::IsValidOrPlayer);
    Core::Hook::Install(0x8A92C0, &CPed::SetPedState);

    Core::Log::Info("CPed hooks installed (4 functions)");
}

CPed* __thiscall CPed::Ctor(const uint8* initParams, int32 modelIndex)
{
    CPhysical::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD69844));

    m_nPedUnk210 = 0;
    m_nPedUnk211 = 0;
    m_nPedUnk212 = 0;
    m_fPedUnk214 = 0.0f;
    m_nPedUnk218 = initParams[0];
    m_nPedUnk219 = initParams[1];
    m_pPedUnk21C = nullptr;
    m_bPedUnk220 = 0;

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

    for (uint32& val : m_nPedUnkB40)
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

    m_pPedUnk21C = obj;
    m_bPedUnk220 = 1;

    auto** modelPtrs = reinterpret_cast<uint8**>(Core::Rebase(0x15F73B0));
    *reinterpret_cast<uint32*>(obj + 0x12C) = *reinterpret_cast<uint32*>(modelPtrs[modelIndex] + 0x90);

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
    const int32 index = pool->GetIndex(this) >> 8;
    if (index < 0 || index >= pool->m_nSize)
        return false;

    if (m_pAnimBlender)
        return true;

    CPed* player = Core::Hook::Call<CPed*>(0x8788D0, 0);
    return this == player;
}

void __thiscall CPed::SetPedState(int32 state)
{
    if (state == 1 || state == 2)
    {
        Core::Hook::CallMethod<void>(0x89DC70, this);
        if ((m_nPedFlags2 & 0x10u) != 0)
        {
            const int32 handle = CPools::ms_pPedPool->GetIndex(this);
            Core::Hook::Call<void>(0x80E6A0, handle, 2);
        }
    }

    m_nPedState = state;
}
