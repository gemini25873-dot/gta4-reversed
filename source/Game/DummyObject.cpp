#include "StdInc.h"
#include "Game/DummyObject.h"
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

    bool __cdecl DoSaveVisibleFlag(CDummyObject* obj) { return obj->SaveVisibleFlag(); }
    bool (__cdecl* const kDoSaveVisibleFlag)(CDummyObject*) = &DoSaveVisibleFlag;

    // Preserve ECX and EDX for LTCG callers (e.g. 0xB25740)
    void __declspec(naked) HookedSaveVisibleFlag()
    {
        __asm {
            push edx
            push ecx
            push ecx
            mov eax, kDoSaveVisibleFlag
            call eax
            add esp, 4
            pop ecx
            pop edx
            ret
        }
    }
}

void CDummyObject::InjectHooks()
{
    Core::Hook::Install(0xB25590, &CDummyObject::Ctor);
    Core::Hook::Install(0xB255C0, &CDummyObject::Dtor);
    Core::Hook::Install(0xB25A60, &CDummyObject::DeletingDtor);
    Core::Hook::Install(0xB25AC0, &CDummyObject::CleanupLinks);
    Core::Hook::Install(0xB25640, &HookedSaveVisibleFlag);
    Core::Hook::Install(0xB25650, &CDummyObject::RestoreVisibleFlag);
    Core::Hook::Install(0xB25740, &CDummyObject::UpdateAllDummyVisibility);
    Core::Hook::Install(0xB257A0, &CDummyObject::InitPool);

    Core::Log::Info("CDummyObject hooks installed (8 functions)");
}

// 0xB25590
CDummyObject* __thiscall CDummyObject::Ctor()
{
    CEntity::Ctor();
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDA5994));
    m_pFragLink = nullptr;
    m_bSavedVisibleFlag = 0;
    m_nFlags2 = (m_nFlags2 & 0xFFFFFC3Fu) | 0x140u;
    return this;
}

// 0xB255C0
int32 __thiscall CDummyObject::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDA5994));
    CleanupLinks();
    return Core::Hook::CallMethod<int32>(0x9EA4E0, this);
}

// 0xB25A60
CDummyObject* __thiscall CDummyObject::DeletingDtor(uint8 flags)
{
    Dtor();
    if ((flags & 1) != 0)
        CPools::ms_pDummyObjectPool->Delete(this);
    return this;
}

// 0xB25AC0
void __thiscall CDummyObject::CleanupLinks()
{
    auto*& poolA = Core::StaticRef<CPoolBase*>(0x16B775C);
    auto*& poolB = Core::StaticRef<CPoolBase*>(0x1706E98);

    uint32* node = static_cast<uint32*>(m_pFragLink);
    while (node)
    {
        uint32* inner = reinterpret_cast<uint32*>(node[1]);
        uint32** headPtr = reinterpret_cast<uint32**>(node[0]);
        if (*headPtr == inner)
            *headPtr = reinterpret_cast<uint32*>(inner[1]);

        uint32* prevInner = reinterpret_cast<uint32*>(inner[2]);
        if (prevInner)
            prevInner[1] = inner[1];

        uint32* nextInner = reinterpret_cast<uint32*>(inner[1]);
        if (nextInner)
            nextInner[2] = inner[2];

        poolA->Delete(inner);

        uint32* curNode = node;
        const bool isHead = (m_pFragLink == curNode);
        uint32* nextNode = reinterpret_cast<uint32*>(curNode[4]);
        node = nextNode;
        if (isHead)
            m_pFragLink = nextNode;

        uint32* prevNode = reinterpret_cast<uint32*>(curNode[3]);
        if (prevNode)
            prevNode[4] = reinterpret_cast<uint32>(nextNode);
        if (nextNode)
            nextNode[3] = curNode[3];

        poolB->Delete(curNode);
    }
}

// 0xB25640
bool __thiscall CDummyObject::SaveVisibleFlag()
{
    const bool visible = (m_nFlags & 0x20u) != 0;
    m_bSavedVisibleFlag = visible ? 1 : 0;
    return visible;
}

// 0xB25650
int32 __thiscall CDummyObject::RestoreVisibleFlag()
{
    using SetVisibleFn = int32(__thiscall*)(CDummyObject*, uint32);
    return reinterpret_cast<SetVisibleFn>(m_vftable[12])(this, m_bSavedVisibleFlag);
}

// 0xB25740
int32 __cdecl CDummyObject::UpdateAllDummyVisibility(uint8 restore)
{
    CPool<CDummyObject>* pool = CPools::ms_pDummyObjectPool;
    int32 result = 0;
    for (int32 i = 0; i < pool->m_nSize; ++i)
    {
        CDummyObject* dummy = pool->GetAt(i);
        if (!dummy)
            continue;

        if (restore)
        {
            result = dummy->RestoreVisibleFlag();
        }
        else
        {
            dummy->SaveVisibleFlag();
            using SetVisibleFn = int32(__thiscall*)(CDummyObject*, uint32);
            result = reinterpret_cast<SetVisibleFn>(dummy->m_vftable[12])(dummy, 0);
        }
    }
    return result;
}

// 0xB257A0
void* __cdecl CDummyObject::InitPool()
{
    auto* pool = static_cast<CPoolBase*>(RageAlloc(sizeof(CPoolBase)));
    if (pool)
    {
        pool->Init(14000, "Dummy Object", sizeof(CDummyObject));
        CPools::ms_pDummyObjectPool = reinterpret_cast<CPool<CDummyObject>*>(pool);
    }
    else
    {
        CPools::ms_pDummyObjectPool = nullptr;
    }
    return CPools::ms_pDummyObjectPool;
}
