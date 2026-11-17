#include "StdInc.h"
#include "Game/Building.h"
#include "Game/Pools.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <intrin.h>

namespace
{
    void* RageAlloc(size_t size)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using AllocFn = void*(__thiscall*)(void*, size_t, size_t, uint32);
        return reinterpret_cast<AllocFn>(vtable[2])(allocator, size, 16, 0);
    }
}

void CBuilding::InjectHooks()
{
    Core::Hook::Install(0xB1B3E0, &CBuilding::Ctor);
    Core::Hook::Install(0xB1B410, &CBuilding::Dtor);
    Core::Hook::Install(0xB1B4B0, &CBuilding::DeletingDtor);
    Core::Hook::Install(0xB1B430, &CBuilding::ReplaceWithNewModel);
    Core::Hook::Install(0xB1B470, &CBuilding::InitPool);

    Core::Log::Info("CBuilding hooks installed (5 functions)");
}

// 0xB1B3E0
CBuilding* __thiscall CBuilding::Ctor()
{
    CEntity::Ctor();
    m_nFlags |= 5u;
    m_nFlags2 = (m_nFlags2 & 0xFFFFFC7Fu) | 0x40u;
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDA3A3C));
    return this;
}

// 0xB1B410
void __thiscall CBuilding::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDA3A3C));
    Core::Hook::CallMethod<void>(0x9E69D0, this);
    Core::Hook::CallMethod<void>(0x9EA4E0, this);
}

// 0xB1B4B0
CBuilding* __thiscall CBuilding::DeletingDtor(uint8 flags)
{
    Dtor();
    if ((flags & 1) != 0)
        CPools::ms_pBuildingPool->Delete(this);
    return this;
}

// 0xB1B430
int16 __thiscall CBuilding::ReplaceWithNewModel(int16 modelIndex)
{
    using Fn17 = int16(__thiscall*)(CBuilding*);
    using Fn44 = void(__thiscall*)(CBuilding*);

    int16 result = reinterpret_cast<Fn17>(m_vftable[17])(this);
    if (m_pPhysicsInst)
    {
        reinterpret_cast<Fn44>(m_vftable[44])(this);
        Core::Hook::CallMethod<void>(0x9E6FF0, this, 0);
        result = modelIndex;
    }
    m_nModelIndex = modelIndex;
    return result;
}

// 0xB1B470
void* __cdecl CBuilding::InitPool()
{
    auto* pool = static_cast<CPoolBase*>(RageAlloc(sizeof(CPoolBase)));
    if (pool)
    {
        pool->Init(32000, "Building", sizeof(CBuilding));
        CPools::ms_pBuildingPool = reinterpret_cast<CPool<CBuilding>*>(pool);
    }
    else
    {
        CPools::ms_pBuildingPool = nullptr;
    }
    return CPools::ms_pBuildingPool;
}
