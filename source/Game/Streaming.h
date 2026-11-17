#pragma once

struct CStreamingInfo
{
    uint32 m_nBlockInfo = 0;        // +0x00 (size / compressed chunk bits)
    uint8  m_nArchiveIndex = 0;     // +0x04
    uint8  m_pad05[3]{};            // +0x05..+0x07
    uint32 m_nStatusFlags = 0;      // +0x08 (bits 0..1: 0=unloaded, 1=loaded, 2=requested, 3=loading)
    uint16 m_nUnk0C = 0;            // +0x0C
    uint16 m_nFlags = 0;            // +0x0E
    uint16 m_nPrevIndex = 0xFFFF;   // +0x10
    uint16 m_nUnk12 = 0;            // +0x12
    uint16 m_nUnk14 = 0;            // +0x14
    uint8  m_nModuleIdCopy = 0;     // +0x16
    uint8  m_nModuleId = 0;         // +0x17
};

VALIDATE_SIZE(CStreamingInfo, 0x18);
VALIDATE_OFFSET(CStreamingInfo, m_nArchiveIndex, 0x04);
VALIDATE_OFFSET(CStreamingInfo, m_nStatusFlags, 0x08);
VALIDATE_OFFSET(CStreamingInfo, m_nFlags, 0x0E);
VALIDATE_OFFSET(CStreamingInfo, m_nModuleId, 0x17);

struct CStreamingModule
{
    uint8 m_pad00[0x58]{};          // +0x00..+0x57
    int32 m_nBaseIndex = 0;         // +0x58
    uint8 m_pad5C[0x08]{};          // +0x5C..+0x63
};

VALIDATE_SIZE(CStreamingModule, 0x64);
VALIDATE_OFFSET(CStreamingModule, m_nBaseIndex, 0x58);

class CStreaming
{
public:
    static inline CStreamingInfo*& ms_pStreamingInfo    = Core::StaticRef<CStreamingInfo*>(0xF21C60);
    static inline void*&           ms_pStreamingMgr     = Core::StaticRef<void*>(0x121DFA4);
    static inline uint8&           ms_bDisableStreaming = Core::StaticRef<uint8>(0x121DFAA);
    static inline int32&           ms_nScriptModelFlags = Core::StaticRef<int32>(0x16C38A8);

    static CStreamingModule* GetModuleManagerArray()
    {
        return reinterpret_cast<CStreamingModule*>(Core::Rebase(0x1227F40));
    }

    static int32 GetGlobalIndex(int32 localIndex, int32 moduleId)
    {
        return localIndex + GetModuleManagerArray()[moduleId].m_nBaseIndex;
    }

    static bool IsEntryActive(int32 globalIndex)
    {
        const CStreamingInfo* info = ms_pStreamingInfo;
        if (!info)
            return false;
        const CStreamingInfo& entry = info[globalIndex];
        return (entry.m_nStatusFlags & 0xFFFFFFFCu) != 0 || (entry.m_nFlags & 0x800u) != 0;
    }

    static void InjectHooks();

    static void    __cdecl SetArchiveMounted(int32 archiveIndex);
    static int32   __cdecl LoadAllRequestedModels(int32 priorityOnly);
    static int32   __cdecl RequestModel(int32 localIndex, int32 moduleId, int32 flags);
    static int32   __cdecl RemoveModel(int32 localIndex, int32 moduleId);
    static uint16  __cdecl GetModelFlags(int32 localIndex, int32 moduleId);
    static uint8   __cdecl GetModelArchiveIndex(int32 localIndex, int32 moduleId);
    static uint8   __cdecl GetModelAndDependenciesSize(int32 globalIndex, uint32* outMainSize, uint32* outVramSize, const int32* visitedIndices, int32 numVisited);
    static int32   __cdecl HasModelLoaded(int32 localIndex, int32 moduleId);
    static bool    __cdecl IsModelRequested(int32 localIndex, int32 moduleId);
    static bool    __cdecl IsModelLoading(int32 localIndex, int32 moduleId);
    static bool    __cdecl IsObjectInStreaming(int32 localIndex, int32 moduleId);
    static uint8   __cdecl IsModelArchiveMounted(int32 localIndex, int32 moduleId);
    static int32   __cdecl RegisterStreamingEntry(int32 unk, int16 localIndex, int32 moduleId);
    static void    __cdecl ClearAllStreamingFlags(uint16 flagMask);
    static void    __cdecl DisableStreaming();
    static void    __cdecl EnableStreaming();
    static uint8   __cdecl IsStreamingDisabled();
    static void*   __cdecl GetModuleManager();
    static int32   __cdecl ScriptRequestModel(uint32 hash, int32 checkFlag);
};
