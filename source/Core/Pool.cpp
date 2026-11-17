#include "StdInc.h"
#include "Core/Pool.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <intrin.h>

namespace
{
    // 0x4011D0 (__cdecl) - Rage sysMemAllocator::Allocate(size, 16, 0)
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

void CPoolBase::InjectHooks()
{
    Core::Hook::Install(0xC72F10, &CPoolBase::Init);
    Core::Hook::Install(0x439CB0, &CPoolBase::New);
    Core::Hook::Install(0x41F180, &CPoolBase::SetRef);
    Core::Hook::Install(0x96C6C0, &CPoolBase::InitSlot);
    Core::Hook::Install(0x96C730, &CPoolBase::GetIndex);
    Core::Hook::Install(0xC79660, &CPoolBase::Delete);
    Core::Hook::Install(0xB26820, &CPoolBase::NewAt);
    Core::Hook::Install(0xA4C560, &CPoolBase::GetAtHandleRegistered);
    Core::Hook::Install(0xB25790, &CPoolBase::GetIndexThunk);

    Core::Log::Info("CPool hooks installed (9 functions)");
}

CPoolBase* __thiscall CPoolBase::Init(int32 size, const char* /*name*/, int32 elemSize)
{
    m_nElementSize = elemSize;
    m_pObjects = RageAlloc(static_cast<size_t>(elemSize) * static_cast<size_t>(size));
    m_byteMap = static_cast<uint8*>(RageAlloc(static_cast<size_t>(size)));
    m_bOwnsMemory = true;
    m_nSize = size;
    m_nFirstFree = -1;
    m_nUsed = 0;

    for (int32 i = 0; i < size; ++i)
    {
        m_byteMap[i] |= 0x80;
        m_byteMap[i] = (m_byteMap[i] & 0x80) | 1;
    }

    return this;
}

void* __thiscall CPoolBase::New()
{
    if (m_nSize <= 0)
        return nullptr;

    int32 idx = m_nFirstFree;
    bool wrapped = false;

    while (true)
    {
        if (++idx >= m_nSize)
        {
            if (wrapped)
                return nullptr;
            idx = 0;
            wrapped = true;
        }

        if (static_cast<int8>(m_byteMap[idx]) < 0)
        {
            m_byteMap[idx] &= ~0x80;
            SetRef(idx, static_cast<uint8>((m_byteMap[idx] & 0x7F) + 1));
            ++m_nUsed;
            m_nFirstFree = idx;
            return static_cast<uint8*>(m_pObjects) + idx * m_nElementSize;
        }
    }
}

int32 __thiscall CPoolBase::SetRef(int32 index, uint8 ref)
{
    const uint8 masked = ref & 0x7F;
    const int32 result = (masked <= 1) ? 1 : masked;
    m_byteMap[index] = (m_byteMap[index] & 0x80) | static_cast<uint8>(result);
    return result;
}

void* __thiscall CPoolBase::InitSlot(int32 handle)
{
    const int32 index = handle >> 8;
    m_byteMap[index] &= ~0x80;

    const uint8 ref = static_cast<uint8>(handle & 0x7F);
    const uint8 clamped = (ref <= 1) ? 1 : ref;
    m_byteMap[index] = (m_byteMap[index] & 0x80) | clamped;

    m_nFirstFree = 0;
    while (m_nFirstFree < m_nSize && static_cast<int8>(m_byteMap[m_nFirstFree]) >= 0)
        ++m_nFirstFree;

    if (m_nFirstFree >= m_nSize)
    {
        m_nFirstFree = -1;
        return nullptr;
    }

    return &m_byteMap[m_nFirstFree];
}

int32 __thiscall CPoolBase::GetIndex(const void* obj) const
{
    const uintptr_t diff = reinterpret_cast<uintptr_t>(obj) - reinterpret_cast<uintptr_t>(m_pObjects);
    return static_cast<int32>(diff) / m_nElementSize;
}

// 0xC79660
int32 __thiscall CPoolBase::Delete(void* obj)
{
    const int32 index = GetIndex(obj);
    m_byteMap[index] |= 0x80u;
    --m_nUsed;
    if (index < m_nFirstFree)
        m_nFirstFree = index;
    return index;
}

// 0xB26820
void* __thiscall CPoolBase::NewAt(int32 handle)
{
    void* obj = static_cast<uint8*>(m_pObjects) + (handle >> 8) * m_nElementSize;
    InitSlot(handle);
    ++m_nUsed;
    return obj;
}

// 0xA4C560
void* __thiscall CPoolBase::GetAtHandleRegistered(int32 handle) const
{
    const int32 index = handle >> 8;
    if (m_byteMap[index] == static_cast<uint8>(handle))
        return static_cast<uint8*>(m_pObjects) + index * m_nElementSize;
    return nullptr;
}

// 0xB25790
int32 __thiscall CPoolBase::GetIndexThunk(const void* obj) const
{
    return GetIndex(obj);
}