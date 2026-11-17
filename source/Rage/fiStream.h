#pragma once

namespace rage
{
    class fiDevice
    {
    public:
        int32 Read(void* handle, void* buffer, int32 size)
        {
            using Fn = int32(__thiscall*)(fiDevice*, void*, void*, int32);
            return reinterpret_cast<Fn>((*reinterpret_cast<void***>(this))[5])(this, handle, buffer, size);
        }

        int32 Write(void* handle, const void* buffer, int32 size)
        {
            using Fn = int32(__thiscall*)(fiDevice*, void*, const void*, int32);
            return reinterpret_cast<Fn>((*reinterpret_cast<void***>(this))[8])(this, handle, buffer, size);
        }

        int32 Seek(void* handle, int32 offset, int32 whence)
        {
            using Fn = int32(__thiscall*)(fiDevice*, void*, int32, int32);
            return reinterpret_cast<Fn>((*reinterpret_cast<void***>(this))[9])(this, handle, offset, whence);
        }

        int32 Close(void* handle)
        {
            using Fn = int32(__thiscall*)(fiDevice*, void*);
            return reinterpret_cast<Fn>((*reinterpret_cast<void***>(this))[10])(this, handle);
        }

        int32 Size(void* handle)
        {
            using Fn = int32(__thiscall*)(fiDevice*, void*);
            return reinterpret_cast<Fn>((*reinterpret_cast<void***>(this))[12])(this, handle);
        }
    };

    class fiStream
    {
    public:
        fiDevice* m_pDevice = nullptr;        // +0x00
        void*     m_hHandle = nullptr;        // +0x04
        uint8*    m_pBuffer = nullptr;        // +0x08
        int32     m_nFileOffset = 0;          // +0x0C
        int32     m_nBufferOffset = 0;        // +0x10
        int32     m_nBufferSize = 0;          // +0x14
        int32     m_nBufferCapacity = 0;      // +0x18

        static void InjectHooks();

        int32 __thiscall Read(void* dst, int32 size);
        int32 __thiscall Write(const void* src, int32 size);
        int32 __thiscall Seek(int32 offset);
        int32 __thiscall Tell() const;
        int32 __thiscall Close();
        int32 __thiscall Size();

        static int32 __cdecl ReadLineTrimmed(char* dst, int32 maxChars, fiStream* stream);
    };

    VALIDATE_OFFSET(fiStream, m_pDevice, 0x00);
    VALIDATE_OFFSET(fiStream, m_hHandle, 0x04);
    VALIDATE_OFFSET(fiStream, m_pBuffer, 0x08);
    VALIDATE_OFFSET(fiStream, m_nFileOffset, 0x0C);
    VALIDATE_OFFSET(fiStream, m_nBufferOffset, 0x10);
    VALIDATE_OFFSET(fiStream, m_nBufferSize, 0x14);
    VALIDATE_OFFSET(fiStream, m_nBufferCapacity, 0x18);
}
