#pragma once
#include "Rage/fiStream.h"

class CFileMgr
{
public:
    static char* GetCurrentDir() { return reinterpret_cast<char*>(Core::Rebase(0x10F13A0)); }
    static char* GetLineBuffer() { return reinterpret_cast<char*>(Core::Rebase(0x10F4348)); }

    static void InjectHooks();

    static const char*     __cdecl GetRootDir();
    static bool            __cdecl IsRelativePath(const char* path);
    static int32           __cdecl LoadFile(const char* path, uint8* outBuffer);
    static rage::fiStream* __cdecl OpenFile(const char* path, const char* mode);
    static rage::fiStream* __cdecl OpenFileForWriting(const char* path);
    static rage::fiStream* __cdecl OpenFileForAppending(const char* path);
    static int32           __cdecl Read(rage::fiStream* file, void* buffer, int32 size);
    static int32           __cdecl Write(rage::fiStream* file, const void* buffer, int32 size);
    static int32           __cdecl Seek(rage::fiStream* file, int32 offset);
    static uint8           __cdecl ReadLine(rage::fiStream* file, char* buffer, int32 maxLen);
    static char*           __cdecl LoadLine(rage::fiStream* file, uint8 replaceControlChars);
    static int32           __cdecl CloseFile(rage::fiStream* file);
    static int32           __cdecl GetFileLength(rage::fiStream* file);
    static int32           __cdecl Tell(rage::fiStream* file);
};
