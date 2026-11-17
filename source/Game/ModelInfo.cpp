#include "StdInc.h"
#include "Game/ModelInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstdlib>

namespace
{
    int16           (__cdecl* const kSortModelHashTable)()          = &CModelInfo::SortModelHashTable;
    CBaseModelInfo* (__cdecl* const kGetModelInfo)(uint32, int32*)  = &CModelInfo::GetModelInfo;
    int32           (__cdecl* const kGetModelIndex)(uint32)         = &CModelInfo::GetModelIndex;
    int32           (__cdecl* const kGetModelIndexFromName)(const char*) = &CModelInfo::GetModelIndexFromName;
    int32           (__cdecl* const kGetModelRefCount)(int32)       = &CModelInfo::GetModelRefCount;
    void*           (__cdecl* const kGetModelDrawData)(int32)       = &CModelInfo::GetModelDrawData;
    void*           (__cdecl* const kGetModelFragType)(int32)       = &CModelInfo::GetModelFragType;

    // Preserve ECX and EDX across short __cdecl leaf hooks for LTCG callers in GTAIV.exe
    void __declspec(naked) HookedSortModelHashTable()
    {
        __asm {
            push ecx
            push edx
            mov eax, kSortModelHashTable
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelInfo()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 16]
            push dword ptr [esp + 16]
            mov eax, kGetModelInfo
            call eax
            add esp, 8
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelIndex()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetModelIndex
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelIndexFromName()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetModelIndexFromName
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelRefCount()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetModelRefCount
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelDrawData()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetModelDrawData
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetModelFragType()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetModelFragType
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }
}

void CModelInfo::InjectHooks()
{
    Core::Hook::Install(0x988C00, &CModelInfo::CompareHashEntries);
    Core::Hook::Install(0x98AB90, &HookedSortModelHashTable);
    Core::Hook::Install(0x98AAE0, &HookedGetModelInfo);
    Core::Hook::Install(0x98AB40, &HookedGetModelIndex);
    Core::Hook::Install(0x98AC40, &HookedGetModelIndexFromName);
    Core::Hook::Install(0x9884C0, &HookedGetModelRefCount);
    Core::Hook::Install(0x988500, &HookedGetModelDrawData);
    Core::Hook::Install(0x9885B0, &HookedGetModelFragType);
    Core::Hook::Install(0x98EE30, &CBaseModelInfo::SetTexDictionary);
    Core::Hook::Install(0x98F3F0, &CBaseModelInfo::SetAnimGroup);
    Core::Hook::Install(0xBE6A60, &CVehicleModelInfo::GetNumberOfSeats);

    Core::Log::Info("CModelInfo hooks installed (11 functions)");
}

// 0x5B1C30 (atStringHash)
uint32 CModelInfo::HashString(const char* str, uint32 seed)
{
    const char* p = str;
    if (*str == '"')
        p = str + 1;

    uint32 hash = seed;
    for (char ch = *p; ch != '\0'; ch = *p)
    {
        if (*str == '"' && ch == '"')
            break;
        ++p;

        if (static_cast<uint8>(ch - 'A') <= 25u)
            ch += 32;
        else if (ch == '\\')
            ch = '/';

        const uint32 acc = 1025u * (hash + static_cast<uint32>(ch));
        hash = (acc >> 6) ^ acc;
    }

    return 32769u * ((9u * hash) ^ ((9u * hash) >> 11));
}

// 0x988C00
int32 __cdecl CModelInfo::CompareHashEntries(const void* a, const void* b)
{
    const uint32 ha = *static_cast<const uint32*>(a);
    const uint32 hb = *static_cast<const uint32*>(b);
    if (ha < hb)
        return -1;
    return (hb < ha) ? 1 : 0;
}

// 0x98AB90
int16 __cdecl CModelInfo::SortModelHashTable()
{
    int16 result = 0;
    if (!ms_bHashTableSorted)
    {
        result = static_cast<int16>(ms_nHashTableCount);
        if (ms_nHashTableCount)
            std::qsort(ms_pHashTable, ms_nHashTableCount, sizeof(ModelHashEntry), &CompareHashEntries);
        ms_bHashTableSorted = 1;
    }
    return result;
}

// 0x98AAE0
CBaseModelInfo* __cdecl CModelInfo::GetModelInfo(uint32 hash, int32* outIndex)
{
    if (!ms_nHashTableCount)
        return nullptr;

    ModelHashEntry key{ hash, 0 };
    const auto* found = static_cast<const ModelHashEntry*>(
        std::bsearch(&key, ms_pHashTable, ms_nHashTableCount, sizeof(ModelHashEntry), &CompareHashEntries));
    if (!found)
        return nullptr;

    if (outIndex)
        *outIndex = found->index;

    return ms_modelInfoPtrs[found->index];
}

// 0x98AB40
int32 __cdecl CModelInfo::GetModelIndex(uint32 hash)
{
    if (!ms_nHashTableCount)
        return -1;

    ModelHashEntry key{ hash, 0 };
    const auto* found = static_cast<const ModelHashEntry*>(
        std::bsearch(&key, ms_pHashTable, ms_nHashTableCount, sizeof(ModelHashEntry), &CompareHashEntries));
    if (!found)
        return -1;

    return found->index;
}

// 0x98AC40
int32 __cdecl CModelInfo::GetModelIndexFromName(const char* name)
{
    return GetModelIndex(HashString(name, 0));
}

// 0x9884C0
int32 __cdecl CModelInfo::GetModelRefCount(int32 modelIndex)
{
    return ms_modelInfoPtrs[modelIndex]->m_nRefCount;
}

// 0x988500
void* __cdecl CModelInfo::GetModelDrawData(int32 modelIndex)
{
    CBaseModelInfo* info = ms_modelInfoPtrs[modelIndex];
    if (info->m_pFragType)
        return *reinterpret_cast<void**>(static_cast<uint8*>(info->m_pFragType) + 180);

    void** drawable = static_cast<void**>(info->m_pDrawableStruct);
    return drawable ? *drawable : nullptr;
}

// 0x9885B0
void* __cdecl CModelInfo::GetModelFragType(int32 modelIndex)
{
    return ms_modelInfoPtrs[modelIndex]->m_pFragType;
}

// 0x98EE30
int32 __thiscall CBaseModelInfo::SetTexDictionary(const char* txdName)
{
    const int32 cmp = _stricmp(txdName, "NULL");
    if (!cmp)
    {
        m_nTexDictionary = -1;
        return 0;
    }

    int32 txdSlot = Core::Hook::Call<int32>(0x41ECE0, txdName);
    m_nTexDictionary = static_cast<int16>(txdSlot);
    if (static_cast<uint16>(txdSlot) == 0xFFFFu)
    {
        txdSlot = Core::Hook::Call<int32>(0x41EC20, txdName);
        m_nTexDictionary = static_cast<int16>(txdSlot);
    }
    return txdSlot;
}

// 0x98F3F0
int32 __thiscall CBaseModelInfo::SetAnimGroup(const char* animGroup)
{
    const int32 cmp = _stricmp(animGroup, "null");
    if (!cmp)
    {
        m_nAnimIndex = -1;
        return 0;
    }

    const int32 animIdx = Core::Hook::Call<int32>(0x909520, animGroup);
    m_nAnimIndex = static_cast<int16>(animIdx);
    return animIdx;
}

// 0xBE6A60
int32 __cdecl CVehicleModelInfo::GetNumberOfSeats(int32 modelIndex)
{
    auto* vehModel = static_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[modelIndex]);
    const uint32* bones = vehModel->m_pVehicleStruct->m_nBones;

    int32 seats = (bones[3] != 0xFFFFFFFFu) ? 1 : 0;
    if (bones[4] != 0xFFFFFFFFu)
        ++seats;
    if (bones[2] != 0xFFFFFFFFu)
        ++seats;
    return seats;
}
