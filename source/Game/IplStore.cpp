#include "StdInc.h"
#include "Game/IplStore.h"
#include "Game/ModelInfo.h"
#include "Game/Streaming.h"
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

    void* (__cdecl* const kGetDefArray)(int32) = &CIplStore::GetDefArray;

    void __declspec(naked) HookedGetDefArray()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetDefArray
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }
}

void CIplStore::InjectHooks()
{
    Core::Hook::Install(0xB267E0, &CIplStore::CreatePool);
    Core::Hook::Install(0xB25B50, &CIplStore::AllocateDefArray);
    Core::Hook::Install(0xB25BB0, &HookedGetDefArray);
    Core::Hook::Install(0xB25BC0, &CIplStore::RequestIplIfInStreaming);
    Core::Hook::Install(0xB25BF0, &CIplStore::FindIplSlotByHash);
    Core::Hook::Install(0xB26500, &CIplStore::FindIplSlot);
    Core::Hook::Install(0xB25C40, &CIplStore::AllocateIplSlotData);
    Core::Hook::Install(0xB26120, &CIplStore::SetIplDisabled);
    Core::Hook::Install(0xB26310, &CIplStore::RemoveRelatedIpls);
    Core::Hook::Install(0xB26360, &CIplStore::RequestIplAndMark);
    Core::Hook::Install(0xB263A0, &CIplStore::RemoveIplAndMark);
    Core::Hook::Install(0xB26520, &CIplStore::SetIfIplIsRequired);
    Core::Hook::Install(0xB26560, &CIplStore::SetIfIplIsRequiredReducedBB);
    Core::Hook::Install(0xB25C80, &CIplStore::CheckIfIplLoadedAtPos);
    Core::Hook::Install(0xB265C0, &CIplStore::RequestIplAtPosImmediate);

    Core::Log::Info("CIplStore hooks installed (15 functions)");
}

// 0xB267E0
CPool<IplDef>* __cdecl CIplStore::CreatePool(int32 size, const char* name)
{
    auto* pool = static_cast<CPoolBase*>(RageAlloc(sizeof(CPoolBase)));
    if (pool)
    {
        pool->Init(size, name, sizeof(IplDef));
        ms_pPool = reinterpret_cast<CPool<IplDef>*>(pool);
    }
    else
    {
        ms_pPool = nullptr;
    }
    return ms_pPool;
}

// 0xB25B50
int32 __cdecl CIplStore::AllocateDefArray(uint32 count)
{
    void* mem = nullptr;
    if (count != 0)
    {
        const size_t bytes = ((static_cast<uint64>(count) >> 30) != 0) ? static_cast<size_t>(-1) : (4u * count);
        mem = RageAlloc(bytes);
    }

    const int32 index = ms_nDefArrayCount;
    GetDefArrays()[index] = mem;
    ms_nDefArrayCount = index + 1;
    return index;
}

// 0xB25BB0
void* __cdecl CIplStore::GetDefArray(int32 index)
{
    return GetDefArrays()[index];
}

// 0xB25BC0
int32 __cdecl CIplStore::RequestIplIfInStreaming(int32 slot, int32 flags)
{
    if (CStreaming::IsObjectInStreaming(slot, ms_nStreamingModuleId))
        return CStreaming::RequestModel(slot, ms_nStreamingModuleId, flags);
    return 0;
}

// 0xB25BF0
int32 __cdecl CIplStore::FindIplSlotByHash(uint32 hash)
{
    CPool<IplDef>* pool = ms_pPool;
    for (int32 i = 0; i < pool->m_nSize; ++i)
    {
        IplDef* def = pool->GetAt(i);
        if (def && def->m_nHash == hash)
            return i;
    }
    return -1;
}

// 0xB26500
int32 __cdecl CIplStore::FindIplSlot(const char* name)
{
    return FindIplSlotByHash(CModelInfo::HashString(name, 0));
}

// 0xB25C40
void* __cdecl CIplStore::AllocateIplSlotData(int32 slot)
{
    IplDef* def = ms_pPool->GetAt(slot);
    void* mem = RageAlloc(4);
    def->m_pData = mem;
    return mem;
}

// 0xB26120
int32 __cdecl CIplStore::SetIplDisabled(int32 slot, uint8 enabled)
{
    IplDef* def = ms_pPool->GetAt(slot);
    def->m_bDisableWhenFar = (enabled == 0) ? 1 : 0;
    return reinterpret_cast<int32>(def);
}

// 0xB26310
void __cdecl CIplStore::RemoveRelatedIpls(int32 parentIndex)
{
    CPool<IplDef>* pool = ms_pPool;
    for (int32 i = 0; i < pool->m_nSize; ++i)
    {
        IplDef* def = pool->GetAt(i);
        if (def && def->m_nParentIpl == parentIndex)
            CStreaming::RemoveModel(i, ms_nStreamingModuleId);
    }
}

// 0xB26360
int32 __cdecl CIplStore::RequestIplAndMark(int32 slot)
{
    IplDef* def = ms_pPool->GetAt(slot);
    const int32 res = CStreaming::RequestModel(slot, ms_nStreamingModuleId, 8);
    def->m_bDisableWhenFar = 1;
    def->m_bUnk55 = 0;
    return res;
}

// 0xB263A0
int32 __cdecl CIplStore::RemoveIplAndMark(int32 slot)
{
    IplDef* def = ms_pPool->GetAt(slot);
    const int32 res = CStreaming::RemoveModel(slot, ms_nStreamingModuleId);
    def->m_bDisableWhenFar = 1;
    def->m_bUnk55 = 0;
    return res;
}

namespace
{
    // 0x886F60
    bool IsPointInsideBounds(const float* bounds, const void* posPtr)
    {
        const auto* pos = static_cast<const float*>(posPtr);
        return pos[0] >= bounds[0] && pos[0] <= bounds[4]
            && pos[1] >= bounds[1] && pos[1] <= bounds[5]
            && pos[2] >= bounds[2] && pos[2] <= bounds[6];
    }

    // 0x4E0310
    bool IsPointNearBounds(const float* bounds, const void* posPtr, float radius)
    {
        const auto* pos = static_cast<const float*>(posPtr);
        return (pos[0] + radius) >= bounds[0] && (pos[0] - radius) <= bounds[4]
            && (pos[1] + radius) >= bounds[1] && (pos[1] - radius) <= bounds[5]
            && (pos[2] + radius) >= bounds[2] && (pos[2] - radius) <= bounds[6];
    }
}

// 0xB26520
int32 __cdecl CIplStore::SetIfIplIsRequired(const void* pos2D, IplDef* def)
{
    if (IsPointInsideBounds(def->m_fBounds, pos2D))
    {
        const int32 slot = ms_pPool->GetIndex(def);
        ms_pRequiredFlags[slot] |= 1u;
        return slot;
    }
    return 0;
}

// 0xB26560
int32 __cdecl CIplStore::SetIfIplIsRequiredReducedBB(const void* pos2D, IplDef* def)
{
    const float radius = def->m_bLargeBounds ? -230.0f : ms_fLoadRadius;
    if (IsPointNearBounds(def->m_fBounds, pos2D, radius))
    {
        const int32 slot = ms_pPool->GetIndex(def);
        ms_pRequiredFlags[slot] |= 1u;
        return slot;
    }
    return 0;
}

// 0xB25C80
void __cdecl CIplStore::CheckIfIplLoadedAtPos(const void* pos2D, IplDef* def)
{
    const float radius = def->m_bLargeBounds ? -290.0f : ms_fLoadRadius;
    if (!def->m_pData && IsPointNearBounds(def->m_fBounds, pos2D, radius) && !def->m_bDisableWhenFar)
        ms_bAllIplsLoaded = 0;
}

// 0xB265C0
void __cdecl CIplStore::RequestIplAtPosImmediate(const void* pos2D, IplDef* def)
{
    if (!def->m_pData && IsPointNearBounds(def->m_fBounds, pos2D, ms_fLoadRadius) && !def->m_bDisableWhenFar)
    {
        const int32 slot = ms_pPool->GetIndex(def);
        RequestIplIfInStreaming(slot, 24);
        CStreaming::LoadAllRequestedModels(1);
    }
}
