#pragma once

class CPoolBase
{
public:
    void*  m_pObjects = nullptr;     // +0x00
    uint8* m_byteMap = nullptr;      // +0x04 (bit 7 = free, bits 0..6 = ref)
    int32  m_nSize = 0;              // +0x08
    int32  m_nElementSize = 0;       // +0x0C
    int32  m_nFirstFree = -1;        // +0x10
    int32  m_nUsed = 0;              // +0x14
    bool   m_bOwnsMemory = false;    // +0x18
    uint8  pad19[3]{};               // +0x19..+0x1B

    static void InjectHooks();

    CPoolBase* __thiscall Init(int32 size, const char* name, int32 elemSize);
    void* __thiscall New();
    int32 __thiscall SetRef(int32 index, uint8 ref);
    void* __thiscall InitSlot(int32 handle);
    int32 __thiscall GetIndex(const void* obj) const;

    bool IsSlotFree(int32 index) const { return (m_byteMap[index] & 0x80) != 0; }
    uint8 GetRef(int32 index) const { return m_byteMap[index] & 0x7F; }
    int32 GetHandle(int32 index) const { return (index << 8) | m_byteMap[index]; }

    void* GetAtRaw(int32 index) const
    {
        if (index < 0 || index >= m_nSize || IsSlotFree(index))
            return nullptr;
        return static_cast<uint8*>(m_pObjects) + index * m_nElementSize;
    }
};

VALIDATE_SIZE(CPoolBase, 0x1C);
VALIDATE_OFFSET(CPoolBase, m_pObjects, 0x00);
VALIDATE_OFFSET(CPoolBase, m_byteMap, 0x04);
VALIDATE_OFFSET(CPoolBase, m_nSize, 0x08);
VALIDATE_OFFSET(CPoolBase, m_nElementSize, 0x0C);
VALIDATE_OFFSET(CPoolBase, m_nFirstFree, 0x10);
VALIDATE_OFFSET(CPoolBase, m_nUsed, 0x14);
VALIDATE_OFFSET(CPoolBase, m_bOwnsMemory, 0x18);

template<typename T>
class CPool : public CPoolBase
{
public:
    T* GetAt(int32 index) const
    {
        return static_cast<T*>(GetAtRaw(index));
    }

    T* GetAtHandle(int32 handle) const
    {
        const int32 index = handle >> 8;
        if (index < 0 || index >= m_nSize)
            return nullptr;
        if (m_byteMap[index] != static_cast<uint8>(handle & 0xFF))
            return nullptr;
        return static_cast<T*>(static_cast<void*>(static_cast<uint8*>(m_pObjects) + index * m_nElementSize));
    }
};
