#include "StdInc.h"
#include "Game/Streaming.h"
#include "Game/ModelInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    void  (__cdecl* const kSetArchiveMounted)(int32) = &CStreaming::SetArchiveMounted;
    void  (__cdecl* const kDisableStreaming)()       = &CStreaming::DisableStreaming;
    void  (__cdecl* const kEnableStreaming)()        = &CStreaming::EnableStreaming;
    uint8 (__cdecl* const kIsStreamingDisabled)()    = &CStreaming::IsStreamingDisabled;
    void* (__cdecl* const kGetModuleManager)()       = &CStreaming::GetModuleManager;

    // LTCG leaf wrappers preserving caller-expected registers
    void __declspec(naked) HookedSetArchiveMounted()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kSetArchiveMounted
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedDisableStreaming()
    {
        __asm {
            push eax
            push ecx
            push edx
            mov eax, kDisableStreaming
            call eax
            pop edx
            pop ecx
            pop eax
            ret
        }
    }

    void __declspec(naked) HookedEnableStreaming()
    {
        __asm {
            push eax
            push ecx
            push edx
            mov eax, kEnableStreaming
            call eax
            pop edx
            pop ecx
            pop eax
            ret
        }
    }

    void __declspec(naked) HookedIsStreamingDisabled()
    {
        __asm {
            push ecx
            push edx
            mov eax, kIsStreamingDisabled
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModuleManager()
    {
        __asm {
            push ecx
            push edx
            mov eax, kGetModuleManager
            call eax
            pop edx
            pop ecx
            ret
        }
    }
}

void CStreaming::InjectHooks()
{
    Core::Hook::Install(0x832C00, &HookedSetArchiveMounted);
    Core::Hook::Install(0x832C20, &CStreaming::LoadAllRequestedModels);
    Core::Hook::Install(0x832C40, &CStreaming::RequestModel);
    Core::Hook::Install(0x832C70, &CStreaming::RemoveModel);
    Core::Hook::Install(0x832C90, &CStreaming::GetModelFlags);
    Core::Hook::Install(0x832CB0, &CStreaming::GetModelArchiveIndex);
    Core::Hook::Install(0x832CD0, &CStreaming::GetModelAndDependenciesSize);
    Core::Hook::Install(0x832DD0, &CStreaming::HasModelLoaded);
    Core::Hook::Install(0x832E00, &CStreaming::IsModelRequested);
    Core::Hook::Install(0x832E30, &CStreaming::IsModelLoading);
    Core::Hook::Install(0x832E60, &CStreaming::IsObjectInStreaming);
    Core::Hook::Install(0x832E80, &CStreaming::IsModelArchiveMounted);
    Core::Hook::Install(0x832EA0, &CStreaming::RegisterStreamingEntry);
    Core::Hook::Install(0x832EC0, &CStreaming::ClearAllStreamingFlags);
    Core::Hook::Install(0x832ED0, &HookedDisableStreaming);
    Core::Hook::Install(0x832EE0, &HookedEnableStreaming);
    Core::Hook::Install(0x832EF0, &HookedIsStreamingDisabled);
    Core::Hook::Install(0x832F00, &HookedGetModuleManager);
    Core::Hook::Install(0xB6C3D0, &CStreaming::ScriptRequestModel);

    Core::Log::Info("CStreaming hooks installed (19 functions)");
}

// 0x832C00
void __cdecl CStreaming::SetArchiveMounted(int32 archiveIndex)
{
    auto* archiveMounted = reinterpret_cast<uint8*>(Core::Rebase(0x121E077));
    archiveMounted[160 * archiveIndex] = 1;
}

// 0x832C20
int32 __cdecl CStreaming::LoadAllRequestedModels(int32 priorityOnly)
{
    void* mgr = ms_pStreamingMgr;
    void** vtbl = *reinterpret_cast<void***>(mgr);
    using LoadFn = int32(__thiscall*)(void*, int32);
    return reinterpret_cast<LoadFn>(vtbl[6])(mgr, priorityOnly);
}

// 0x832C40
int32 __cdecl CStreaming::RequestModel(int32 localIndex, int32 moduleId, int32 flags)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return Core::Hook::CallMethod<int32>(0xBCBEB0, &ms_pStreamingInfo, globalIndex, flags);
}

// 0x832C70
int32 __cdecl CStreaming::RemoveModel(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return Core::Hook::CallMethod<int32>(0xBCCB20, &ms_pStreamingInfo, globalIndex);
}

// 0x832C90
uint16 __cdecl CStreaming::GetModelFlags(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return ms_pStreamingInfo[globalIndex].m_nFlags;
}

// 0x832CB0
uint8 __cdecl CStreaming::GetModelArchiveIndex(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return ms_pStreamingInfo[globalIndex].m_nArchiveIndex;
}

// 0x832CD0
uint8 __cdecl CStreaming::GetModelAndDependenciesSize(int32 globalIndex, uint32* outMainSize, uint32* outVramSize, const int32* visitedIndices, int32 numVisited)
{
    for (int32 i = 0; i < numVisited; ++i)
    {
        if (visitedIndices[i] == globalIndex)
            return 0;
    }

    if (!IsEntryActive(globalIndex))
        return 0;

    int32 deps[6]{};
    int32 depCount = Core::Hook::CallMethod<int32>(0xBCD6E0, GetModuleManager(), globalIndex, deps);

    const CStreamingInfo& entry = ms_pStreamingInfo[globalIndex];
    const bool packed = (entry.m_nFlags & 0x2000u) != 0;

    const uint32 mainSize = packed
        ? ((entry.m_nBlockInfo & 0x7FFu) << (((entry.m_nBlockInfo >> 11) & 0xFu) + 8u))
        : entry.m_nBlockInfo;
    *outMainSize += mainSize;

    const uint32 vramSize = packed
        ? (((entry.m_nBlockInfo >> 15) & 0x7FFu) << (((entry.m_nBlockInfo >> 26) & 0xFu) + 8u))
        : 0u;
    *outVramSize += vramSize;

    uint8 result = 0;
    while (depCount > 0)
    {
        --depCount;
        result = GetModelAndDependenciesSize(deps[depCount], outMainSize, outVramSize, visitedIndices, numVisited);
    }
    return result;
}

// 0x832DD0
int32 __cdecl CStreaming::HasModelLoaded(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return ((ms_pStreamingInfo[globalIndex].m_nStatusFlags & 3u) == 1u) ? 1 : 0;
}

// 0x832E00
bool __cdecl CStreaming::IsModelRequested(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return (ms_pStreamingInfo[globalIndex].m_nStatusFlags & 3u) == 2u;
}

// 0x832E30
bool __cdecl CStreaming::IsModelLoading(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    return (ms_pStreamingInfo[globalIndex].m_nStatusFlags & 3u) == 3u;
}

// 0x832E60
bool __cdecl CStreaming::IsObjectInStreaming(int32 localIndex, int32 moduleId)
{
    return IsEntryActive(GetGlobalIndex(localIndex, moduleId));
}

// 0x832E80
uint8 __cdecl CStreaming::IsModelArchiveMounted(int32 localIndex, int32 moduleId)
{
    const int32 globalIndex = GetGlobalIndex(localIndex, moduleId);
    const auto* archiveMounted = reinterpret_cast<const uint8*>(Core::Rebase(0x121E077));
    return archiveMounted[160 * ms_pStreamingInfo[globalIndex].m_nArchiveIndex];
}

// 0x832EA0
int32 __cdecl CStreaming::RegisterStreamingEntry(int32 unk, int16 localIndex, int32 moduleId)
{
    return Core::Hook::CallMethod<int32>(0xBCC530, &ms_pStreamingInfo, unk, static_cast<int32>(localIndex), moduleId);
}

// 0x832EC0
void __cdecl CStreaming::ClearAllStreamingFlags(uint16 flagMask)
{
    Core::Hook::CallMethod<void>(0xBCC830, &ms_pStreamingInfo, static_cast<int32>(flagMask));
}

// 0x832ED0
void __cdecl CStreaming::DisableStreaming()
{
    ms_bDisableStreaming = 1;
}

// 0x832EE0
void __cdecl CStreaming::EnableStreaming()
{
    ms_bDisableStreaming = 0;
}

// 0x832EF0
uint8 __cdecl CStreaming::IsStreamingDisabled()
{
    return ms_bDisableStreaming;
}

// 0x832F00
void* __cdecl CStreaming::GetModuleManager()
{
    return reinterpret_cast<void*>(Core::Rebase(0x1227F40));
}

// 0xB6C3D0
int32 __cdecl CStreaming::ScriptRequestModel(uint32 hash, int32 checkFlag)
{
    int32 modelIndex = -1;
    CBaseModelInfo* info = CModelInfo::GetModelInfo(hash, &modelIndex);
    if (checkFlag)
        Core::Hook::Call<int32>(0x5A4CA0);

    int32 result = 0;
    if (info && info->GetModelType() == 2)
    {
        result = Core::Hook::CallMethod<int32>(0xC81370, info, "limbo", ms_nScriptModelFlags | 0xC);
    }
    else
    {
        result = RequestModel(modelIndex, CModelInfo::ms_nStreamingModuleId, ms_nScriptModelFlags | 0xC);
    }

    ms_nScriptModelFlags = 0;
    return result;
}
