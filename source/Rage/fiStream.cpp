#include "StdInc.h"
#include "Rage/fiStream.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstring>

namespace
{
    int32 CallReadLineRaw(char* dst, int32 maxChars, rage::fiStream* stream)
    {
        int32 result = 0;
        const uintptr_t target = Core::Rebase(0x5B53B0);
        __asm {
            push stream
            mov eax, maxChars
            mov ecx, dst
            call target
            add esp, 4
            mov result, eax
        }
        return result;
    }
}

namespace rage
{
    void fiStream::InjectHooks()
    {
        Core::Hook::Install(0x5B5080, &fiStream::Read);
        Core::Hook::Install(0x5B5180, &fiStream::Write);
        Core::Hook::Install(0x5B5260, &fiStream::Seek);
        Core::Hook::Install(0x5B5290, &fiStream::Tell);
        Core::Hook::Install(0x5B52A0, &fiStream::Close);
        Core::Hook::Install(0x5B52E0, &fiStream::Size);
        Core::Hook::Install(0x5B5420, &fiStream::ReadLineTrimmed);

        Core::Log::Info("rage::fiStream hooks installed (7 functions)");
    }

    // 0x5B5080
    int32 __thiscall fiStream::Read(void* dst, int32 size)
    {
        int32 totalFromBuf = 0;
        if (m_nBufferSize == 0 && m_nBufferOffset != 0)
        {
            if (Core::Hook::CallMethod<int32>(0x5B52F0, this) < 0)
                return -1;
        }

        const int32 bufAvail = m_nBufferSize - m_nBufferOffset;
        if (size > bufAvail)
        {
            if (m_nBufferSize != m_nBufferOffset)
            {
                std::memcpy(dst, m_pBuffer + m_nBufferOffset, static_cast<size_t>(bufAvail));
                dst = static_cast<uint8*>(dst) + bufAvail;
                totalFromBuf = bufAvail;
                size -= bufAvail;
                m_nBufferOffset = m_nBufferSize;
            }

            m_nFileOffset += m_nBufferOffset;
            if (size >= m_nBufferCapacity)
            {
                const int32 bytesRead = m_pDevice->Read(m_hHandle, dst, size);
                m_nBufferOffset = 0;
                m_nBufferSize = 0;
                if (bytesRead < 0)
                    return -1;

                m_nFileOffset += bytesRead;
                return totalFromBuf + bytesRead;
            }

            const int32 bytesRead = m_pDevice->Read(m_hHandle, m_pBuffer, m_nBufferCapacity);
            m_nBufferSize = bytesRead;
            m_nBufferOffset = 0;
            if (bytesRead < 0)
            {
                m_nBufferOffset = 0;
                m_nBufferSize = 0;
                return -1;
            }
        }

        const int32 rem = m_nBufferSize - m_nBufferOffset;
        int32 chunk = size;
        if (chunk > rem)
            chunk = rem;

        std::memcpy(dst, m_pBuffer + m_nBufferOffset, static_cast<size_t>(chunk));
        m_nBufferOffset += chunk;
        return totalFromBuf + chunk;
    }

    // 0x5B5180
    int32 __thiscall fiStream::Write(const void* src, int32 size)
    {
        const int32 origSize = size;
        if (m_nBufferSize != 0)
        {
            if (Core::Hook::CallMethod<int32>(0x5B52F0, this) < 0)
                return -1;
        }

        if (size >= m_nBufferCapacity)
        {
            if (Core::Hook::CallMethod<int32>(0x5B52F0, this) < 0)
                return -1;

            const int32 written = m_pDevice->Write(m_hHandle, src, size);
            if (written < 0)
                return -1;

            m_nFileOffset += written;
            return written;
        }

        const int32 space = m_nBufferCapacity - m_nBufferOffset;
        if (size >= space)
        {
            std::memcpy(m_pBuffer + m_nBufferOffset, src, static_cast<size_t>(space));
            m_nBufferOffset = m_nBufferCapacity;
            src = static_cast<const uint8*>(src) + space;
            size -= space;
            if (Core::Hook::CallMethod<int32>(0x5B52F0, this) < 0)
                return -1;
        }

        std::memcpy(m_pBuffer + m_nBufferOffset, src, static_cast<size_t>(size));
        m_nBufferOffset += size;
        return origSize;
    }

    // 0x5B5260
    int32 __thiscall fiStream::Seek(int32 offset)
    {
        if (Core::Hook::CallMethod<int32>(0x5B52F0, this) < 0)
            return -1;

        m_nFileOffset = offset;
        return m_pDevice->Seek(m_hHandle, offset, 0);
    }

    // 0x5B5290
    int32 __thiscall fiStream::Tell() const
    {
        return m_nFileOffset + m_nBufferOffset;
    }

    // 0x5B52A0
    int32 __thiscall fiStream::Close()
    {
        if (m_nBufferSize == 0 && m_nBufferOffset != 0)
            Core::Hook::CallMethod<int32>(0x5B52F0, this);

        m_pDevice->Close(m_hHandle);
        m_hHandle = reinterpret_cast<void*>(-1);
        m_pDevice = nullptr;
        return 0;
    }

    // 0x5B52E0
    int32 __thiscall fiStream::Size()
    {
        return m_pDevice->Size(m_hHandle);
    }

    // 0x5B5420
    int32 __cdecl fiStream::ReadLineTrimmed(char* dst, int32 maxChars, fiStream* stream)
    {
        int32 len = CallReadLineRaw(dst, maxChars, stream);
        while (len > 0 && static_cast<int8>(dst[len - 1]) <= 32)
            --len;

        dst[len] = '\0';
        return len;
    }
}
