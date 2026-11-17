#include "StdInc.h"
#include "Game/TxdStore.h"
#include "Game/ModelInfo.h"
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

    pgDictionaryBase* (__cdecl* const kPushCurrentTxd)() = &CTxdStore::PushCurrentTxd;

    // Preserve EDX for any LTCG caller around sub_8204F0
    void __declspec(naked) HookedPushCurrentTxd()
    {
        __asm {
            push edx
            mov eax, kPushCurrentTxd
            call eax
            pop edx
            ret
        }
    }
}

void CTxdStore::InjectHooks()
{
    Core::Hook::Install(0x8209F0, &CTxdStore::CreateTxdPool);
    Core::Hook::Install(0x41EC20, &CTxdStore::AddTxdSlot);
    Core::Hook::Install(0x41EC70, &CTxdStore::RemoveTxdSlot);
    Core::Hook::Install(0x41ECE0, &CTxdStore::FindTxdSlot);
    Core::Hook::Install(0x41F130, &CTxdStore::FindTxdSlotByHash);
    Core::Hook::Install(0x820A30, &CTxdStore::FindOrAddTxdSlot);
    Core::Hook::Install(0x41ED00, &CTxdStore::AddRef);
    Core::Hook::Install(0x41ED30, &CTxdStore::RemoveRef);
    Core::Hook::Install(0x7D9540, &CTxdStore::GetNumRefs);
    Core::Hook::Install(0x820510, &CTxdStore::GetTxd);
    Core::Hook::Install(0x820440, &CTxdStore::RemoveTxd);
    Core::Hook::Install(0x820540, &CTxdStore::UnloadTxd);
    Core::Hook::Install(0x820A50, &CTxdStore::RemoveUnusedTxds);
    Core::Hook::Install(0x820AD0, &CTxdStore::Shutdown);
    Core::Hook::Install(0x820AF0, &CTxdStore::GarbageCollect);
    Core::Hook::Install(0x8204F0, &HookedPushCurrentTxd);
    Core::Hook::Install(0x820900, &CTxdStore::SetCurrentTxd);
    Core::Hook::Install(0x820960, &CTxdStore::PopCurrentTxd);
    Core::Hook::Install(0x8209C0, &CTxdStore::SetDictionaryParent);
    Core::Hook::Install(0x820B00, &CTxdStore::SetupTxdParent);
    Core::Hook::Install(0x820BB0, &CTxdStore::SetTxdDictionary);
    Core::Hook::Install(0x8204A0, &CTxdStore::GetDependencies);
    Core::Hook::Install(0x820D40, &CTxdStore::LoadTxd);

    Core::Log::Info("CTxdStore hooks installed (23 functions)");
}

// 0x8209F0
CPool<TxdDef>* __cdecl CTxdStore::CreateTxdPool(int32 size, const char* name)
{
    auto* pool = static_cast<CPoolBase*>(RageAlloc(sizeof(CPoolBase)));
    if (pool)
    {
        pool->Init(size, name, sizeof(TxdDef));
        ms_pTxdPool = reinterpret_cast<CPool<TxdDef>*>(pool);
    }
    else
    {
        ms_pTxdPool = nullptr;
    }
    return ms_pTxdPool;
}

// 0x41EC20
int32 __cdecl CTxdStore::AddTxdSlot(const char* name)
{
    TxdDef* def = static_cast<TxdDef*>(ms_pTxdPool->New());
    def->m_pDictionary = nullptr;
    def->m_nRefCount = 0;
    def->m_nHash = CModelInfo::HashString(name, 0);
    def->m_nParent = -1;
    return ms_pTxdPool->GetIndex(def);
}

// 0x41EC70
int32 __cdecl CTxdStore::RemoveTxdSlot(int32 slot)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    if (def->m_pDictionary)
        RemoveTxd(slot);

    return ms_pTxdPool->Delete(ms_pTxdPool->GetAt(slot));
}

// 0x41ECE0
int32 __cdecl CTxdStore::FindTxdSlot(const char* name)
{
    return FindTxdSlotByHash(CModelInfo::HashString(name, 0));
}

// 0x41F130
int32 __cdecl CTxdStore::FindTxdSlotByHash(uint32 hash)
{
    CPool<TxdDef>* pool = ms_pTxdPool;
    for (int32 i = 0; i < pool->m_nSize; ++i)
    {
        TxdDef* def = pool->GetAt(i);
        if (def && def->m_nHash == hash)
            return i;
    }
    return -1;
}

// 0x820A30
int32 __cdecl CTxdStore::FindOrAddTxdSlot(const char* name)
{
    const int32 slot = FindTxdSlot(name);
    return (slot == -1) ? AddTxdSlot(name) : slot;
}

// 0x41ED00
int32 __cdecl CTxdStore::AddRef(int32 slot)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    return ++def->m_nRefCount;
}

// 0x41ED30
int32 __cdecl CTxdStore::RemoveRef(int32 slot)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    if (--def->m_nRefCount <= 0)
    {
        const uint8 isStreamed = Core::Hook::Call<uint8>(0x832E60, slot, ms_nStreamingModuleId);
        if (!isStreamed)
            return RemoveTxd(slot);
        return isStreamed;
    }
    return 0;
}

// 0x7D9540
int32 __cdecl CTxdStore::GetNumRefs(int32 slot)
{
    return ms_pTxdPool->GetAt(slot)->m_nRefCount;
}

// 0x820510
pgDictionaryBase* __cdecl CTxdStore::GetTxd(int32 slot)
{
    return ms_pTxdPool->GetAt(slot)->m_pDictionary;
}

// 0x820440
int32 __cdecl CTxdStore::RemoveTxd(int32 slot)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    const uint32 tlsIdx = Core::StaticRef<uint32>(0x17237B0);
    auto** tlsArray = reinterpret_cast<uint32**>(__readfsdword(0x2C));
    uint32* tlsBlock = tlsArray[tlsIdx];

    ++tlsBlock[4];
    pgDictionaryBase* dict = def->m_pDictionary;
    if (--dict->m_nRefCount == 0)
        dict->DeletingDtor(1);
    --tlsBlock[4];

    def->m_pDictionary = nullptr;
    return 0;
}

// 0x820540
int32 __cdecl CTxdStore::UnloadTxd(int32 slot)
{
    if (Core::Hook::Call<uint8>(0x832E60, slot, ms_nStreamingModuleId))
        return Core::Hook::Call<int32>(0x832C70, slot, ms_nStreamingModuleId);

    if (GetTxd(slot))
        return RemoveTxd(slot);

    return 0;
}

// 0x820A50
void __cdecl CTxdStore::RemoveUnusedTxds(bool force)
{
    CPool<TxdDef>* pool = ms_pTxdPool;
    for (int32 i = 0; i < pool->m_nSize; ++i)
    {
        if (pool->IsSlotFree(i))
            continue;

        TxdDef* def = pool->GetAt(i);
        if (!def || !def->m_pDictionary)
            continue;

        if (!force)
        {
            if (GetNumRefs(i) != 0)
                continue;

            const uint8 flags = Core::Hook::Call<uint8>(0x832C90, i, ms_nStreamingModuleId);
            if ((flags & 0xC6u) != 0)
                continue;
        }

        UnloadTxd(i);
    }
}

// 0x820AD0
void __cdecl CTxdStore::Shutdown()
{
    RemoveUnusedTxds(false);
    RemoveUnusedTxds(false);
}

// 0x820AF0
void __cdecl CTxdStore::GarbageCollect()
{
    RemoveUnusedTxds(false);
}

// 0x8204F0
pgDictionaryBase* __cdecl CTxdStore::PushCurrentTxd()
{
    pgDictionaryBase* prev = ms_pCurrentTxd;
    ++ms_nTxdStackDepth;
    ms_pStoredTxd = prev;
    if (prev)
        ++prev->m_nRefCount;
    return prev;
}

// 0x820900
void __cdecl CTxdStore::SetCurrentTxd(int32 slot)
{
    pgDictionaryBase* prev = ms_pCurrentTxd;
    if (slot == -1)
    {
        ms_pCurrentTxd = nullptr;
    }
    else
    {
        pgDictionaryBase* next = ms_pTxdPool->GetAt(slot)->m_pDictionary;
        ms_pCurrentTxd = next;
        if (next)
            ++next->m_nRefCount;
    }

    if (prev && --prev->m_nRefCount == 0)
        prev->DeletingDtor(1);
}

// 0x820960
pgDictionaryBase* __cdecl CTxdStore::PopCurrentTxd()
{
    pgDictionaryBase* stored = ms_pStoredTxd;
    --ms_nTxdStackDepth;

    pgDictionaryBase* prevCurrent = ms_pCurrentTxd;
    ms_pCurrentTxd = stored;
    if (stored)
        ++stored->m_nRefCount;

    if (prevCurrent && --prevCurrent->m_nRefCount == 0)
        prevCurrent->DeletingDtor(1);

    pgDictionaryBase* toRelease = ms_pStoredTxd;
    if (toRelease && --toRelease->m_nRefCount == 0)
        toRelease->DeletingDtor(1);

    ms_pStoredTxd = nullptr;
    return toRelease;
}

// 0x8209C0
int32 __cdecl CTxdStore::SetDictionaryParent(pgDictionaryBase* dict, pgDictionaryBase* parent)
{
    if (parent)
        ++parent->m_nRefCount;

    pgDictionaryBase* oldParent = dict->m_pParent;
    if (oldParent && --oldParent->m_nRefCount == 0)
        oldParent->DeletingDtor(1);

    dict->m_pParent = parent;
    return 0;
}

// 0x820B00
int32 __cdecl CTxdStore::SetupTxdParent(int32 slot)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    if (def->m_nParent != -1)
    {
        TxdDef* parentDef = ms_pTxdPool->GetAt(def->m_nParent);
        return SetDictionaryParent(def->m_pDictionary, parentDef->m_pDictionary);
    }
    return 0;
}

// 0x820BB0
int32 __cdecl CTxdStore::SetTxdDictionary(int32 slot, pgDictionaryBase* dict)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    def->m_pDictionary = dict;
    if (dict)
        SetupTxdParent(slot);

    return (def->m_pDictionary != nullptr) ? 1 : 0;
}

// 0x8204A0
int32 __cdecl CTxdStore::GetDependencies(int32 slot, int32* outDeps)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    if (def->m_nParent != -1)
    {
        auto* streamingMgr = Core::Hook::Call<uint8*>(0x832F00);
        const int32 baseIdx = *reinterpret_cast<int32*>(streamingMgr + 100 * ms_nStreamingModuleId + 88);
        *outDeps = def->m_nParent + baseIdx;
        return 1;
    }
    return 0;
}

// 0x820D40
int32 __cdecl CTxdStore::LoadTxd(int32 slot, const char* filename)
{
    TxdDef* def = ms_pTxdPool->GetAt(slot);
    Core::Hook::Call<void>(0x5BBB60);

    const uint8 loaded = Core::Hook::CallMethod<uint8>(
        0xAC2130,
        reinterpret_cast<void*>(Core::Rebase(0x11DC448)),
        filename,
        reinterpret_cast<const void*>(Core::Rebase(0xD5B3D0)),
        8);

    if (loaded)
    {
        Core::Hook::Call<void>(0x636300, def, filename, reinterpret_cast<const void*>(Core::Rebase(0xD5B3D4)), 8, 0);
        if (def->m_pDictionary)
            SetupTxdParent(slot);
    }

    return (def->m_pDictionary != nullptr) ? 1 : 0;
}
