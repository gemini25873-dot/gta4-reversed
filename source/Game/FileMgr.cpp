#include "StdInc.h"
#include "Game/FileMgr.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstring>

namespace
{
    const char* (__cdecl* const kGetRootDir)() = &CFileMgr::GetRootDir;

    void __declspec(naked) HookedGetRootDir()
    {
        __asm {
            push ecx
            push edx
            mov eax, kGetRootDir
            call eax
            pop edx
            pop ecx
            ret
        }
    }
}

void CFileMgr::InjectHooks()
{
    Core::Hook::Install(0x7B20B0, &HookedGetRootDir);
    Core::Hook::Install(0x7B20C0, &CFileMgr::IsRelativePath);
    Core::Hook::Install(0x7B26F0, &CFileMgr::LoadFile);
    Core::Hook::Install(0x7B2740, &CFileMgr::OpenFile);
    Core::Hook::Install(0x7B2850, &CFileMgr::OpenFileForWriting);
    Core::Hook::Install(0x7B2870, &CFileMgr::OpenFileForAppending);
    Core::Hook::Install(0x7B28A0, &CFileMgr::Read);
    Core::Hook::Install(0x7B28C0, &CFileMgr::Write);
    Core::Hook::Install(0x7B28E0, &CFileMgr::Seek);
    Core::Hook::Install(0x7B28F0, &CFileMgr::ReadLine);
    Core::Hook::Install(0x7B2960, &CFileMgr::LoadLine);
    Core::Hook::Install(0x7B29E0, &CFileMgr::CloseFile);
    Core::Hook::Install(0x7B29F0, &CFileMgr::GetFileLength);
    Core::Hook::Install(0x7B2A00, &CFileMgr::Tell);

    Core::Log::Info("CFileMgr hooks installed (14 functions)");
}

// 0x7B20B0
const char* __cdecl CFileMgr::GetRootDir()
{
    return reinterpret_cast<const char*>(Core::Rebase(0xD533CF));
}

// 0x7B20C0
bool __cdecl CFileMgr::IsRelativePath(const char* path)
{
    if (!path)
        return true;

    const char ch = *path;
    if (ch == '/' || ch == '\\' || ch == '\0' || std::strchr(path, ':') != nullptr)
        return false;

    return true;
}

// 0x7B26F0
int32 __cdecl CFileMgr::LoadFile(const char* path, uint8* outBuffer)
{
    void* device = reinterpret_cast<void*>(Core::Rebase(0x108A3C0));
    auto* stream = Core::Hook::CallMethod<rage::fiStream*>(
        0x5B2740, device, path, reinterpret_cast<const void*>(Core::Rebase(0xD53517)), 0, 1);
    if (!stream)
        return -1;

    const int32 size = stream->Size();
    stream->Read(outBuffer, size);
    outBuffer[size] = 0;
    stream->Close();
    return size;
}

// 0x7B2740
rage::fiStream* __cdecl CFileMgr::OpenFile(const char* path, const char* mode)
{
    void* device = reinterpret_cast<void*>(Core::Rebase(0x108A3C0));
    uint8 readOnly = 1;
    bool isWrite = false;

    if (mode && *mode)
    {
        for (const char* p = mode; *p; ++p)
        {
            if (*p == 'a')
                readOnly = 0;
            else if (*p == 'w')
                isWrite = true;
        }
        if (isWrite)
        {
            return Core::Hook::CallMethod<rage::fiStream*>(
                0x5B27B0, device, path, reinterpret_cast<const void*>(Core::Rebase(0xD53521)), 0);
        }
    }

    char fullPath[256];
    const char* curDir = GetCurrentDir();
    size_t i = 0;
    do
    {
        fullPath[i] = curDir[i];
    }
    while (curDir[i++] != '\0');

    const size_t pathLen = std::strlen(path) + 1;
    std::memcpy(fullPath + i - 1, path, pathLen);
    _strlwr(fullPath);

    return Core::Hook::CallMethod<rage::fiStream*>(
        0x5B2740, device, path, reinterpret_cast<const void*>(Core::Rebase(0xD53522)), 0, static_cast<int32>(readOnly));
}

// 0x7B2850
rage::fiStream* __cdecl CFileMgr::OpenFileForWriting(const char* path)
{
    return OpenFile(path, reinterpret_cast<const char*>(Core::Rebase(0xD53BA0)));
}

// 0x7B2870
rage::fiStream* __cdecl CFileMgr::OpenFileForAppending(const char* path)
{
    rage::fiStream* stream = OpenFile(path, reinterpret_cast<const char*>(Core::Rebase(0xD53BA4)));
    if (stream)
    {
        const int32 size = stream->Size();
        stream->Seek(size);
    }
    return stream;
}

// 0x7B28A0
int32 __cdecl CFileMgr::Read(rage::fiStream* file, void* buffer, int32 size)
{
    return file->Read(buffer, size);
}

// 0x7B28C0
int32 __cdecl CFileMgr::Write(rage::fiStream* file, const void* buffer, int32 size)
{
    return file->Write(buffer, size);
}

// 0x7B28E0
int32 __cdecl CFileMgr::Seek(rage::fiStream* file, int32 offset)
{
    return file->Seek(offset);
}

// 0x7B28F0
uint8 __cdecl CFileMgr::ReadLine(rage::fiStream* file, char* buffer, int32 maxLen)
{
    const int32 len = rage::fiStream::ReadLineTrimmed(buffer, maxLen - 1, file);
    if (!len)
    {
        if (file->Tell() >= file->Size())
            return 0;
    }

    int32 i = 0;
    if (len > 0)
    {
        while (i < len)
        {
            const char ch = buffer[i];
            if (ch == '\0')
                return 1;
            if (ch == '\n')
                break;
            if (ch == '\r')
                buffer[i] = '\0';
            ++i;
        }
    }
    buffer[i] = '\0';
    return 1;
}

// 0x7B2960
char* __cdecl CFileMgr::LoadLine(rage::fiStream* file, uint8 replaceControlChars)
{
    char* buf = GetLineBuffer();
    if (!ReadLine(file, buf, 1024))
        return nullptr;

    if (buf[0] != '\0' && replaceControlChars)
    {
        for (char* p = buf; *p != '\0'; ++p)
        {
            if ((*p < ' ' && *p >= 0) || *p == ',')
                *p = ' ';
        }
    }

    while (*buf == ' ')
        ++buf;

    return buf;
}

// 0x7B29E0
int32 __cdecl CFileMgr::CloseFile(rage::fiStream* file)
{
    return file->Close();
}

// 0x7B29F0
int32 __cdecl CFileMgr::GetFileLength(rage::fiStream* file)
{
    return file->Size();
}

// 0x7B2A00
int32 __cdecl CFileMgr::Tell(rage::fiStream* file)
{
    return file->Tell();
}
