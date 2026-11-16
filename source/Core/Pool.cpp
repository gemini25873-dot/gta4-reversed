#include "StdInc.h"
#include "Core/Pool.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CPoolBase::InjectHooks()
{
    Core::Hook::Install(0xC72F10, &CPoolBase::Init);
    Core::Hook::Install(0x439CB0, &CPoolBase::New);
    Core::Hook::Install(0x41F180, &CPoolBase::SetRef);
    Core::Hook::Install(0x96C6C0, &CPoolBase::InitSlot);
    Core::Hook::Install(0x96C730, &CPoolBase::GetIndex);

    Core::Log::Info("CPool hooks installed (5 functions)");
}

CPoolBase* __thiscall CPoolBase::Init(int32 size, const char* /*name*/, int32 elemSize)
{
    m_nElementSize = elemSize;
    m_pObjects = Core::Hook::Call<void*>(0x4011D0, elemSize * size);
    m_byteMap = static_cast<uint8*>(Core::Hook::Call<void*>(0x4011D0, size));
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
        if (++idx == m_nSize)
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
    while (static_cast<int8>(m_byteMap[m_nFirstFree]) >= 0)
        ++m_nFirstFree;

    return &m_byteMap[m_nFirstFree];
}

int32 __thiscall CPoolBase::GetIndex(const void* obj) const
{
    const uintptr_t diff = reinterpret_cast<uintptr_t>(obj) - reinterpret_cast<uintptr_t>(m_pObjects);
    return static_cast<int32>(diff) / m_nElementSize;
}
