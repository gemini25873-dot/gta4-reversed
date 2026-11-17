#pragma once
#include "Core/Vector.h"

struct ModelHashEntry
{
    uint32 hash;
    int32  index;
};
VALIDATE_SIZE(ModelHashEntry, 8);

class CBaseModelInfo
{
public:
    void**  m_vftable = nullptr;           // +0x00
    void*   m_pArchetype = nullptr;        // +0x04
    void*   m_pFragType = nullptr;         // +0x08
    void*   m_pDrawableStruct = nullptr;   // +0x0C
    uint8   m_pad10[0x0C]{};               // +0x10..+0x1B
    float   m_fBoundRadius = 0.0f;         // +0x1C
    CVector m_vBoundCentre{};              // +0x20..+0x2B
    float   m_fDrawDistance = 0.0f;        // +0x2C
    CVector m_vMaxBounds{};                // +0x30..+0x3B
    uint32  m_nHash = 0;                   // +0x3C
    uint32  m_nIDEFlags = 0;               // +0x40
    int32   m_nRefCount = 0;               // +0x44
    int16   m_nTexDictionary = -1;         // +0x48
    uint8   m_pad4A[0x0C]{};               // +0x4A..+0x55
    int16   m_nAnimIndex = -1;             // +0x56
    uint8   m_pad58[0x08]{};               // +0x58..+0x5F

    int32 __thiscall SetTexDictionary(const char* txdName);
    int32 __thiscall SetAnimGroup(const char* animGroup);
};

VALIDATE_SIZE(CBaseModelInfo, 0x60);
VALIDATE_OFFSET(CBaseModelInfo, m_pFragType, 0x08);
VALIDATE_OFFSET(CBaseModelInfo, m_pDrawableStruct, 0x0C);
VALIDATE_OFFSET(CBaseModelInfo, m_fBoundRadius, 0x1C);
VALIDATE_OFFSET(CBaseModelInfo, m_vBoundCentre, 0x20);
VALIDATE_OFFSET(CBaseModelInfo, m_fDrawDistance, 0x2C);
VALIDATE_OFFSET(CBaseModelInfo, m_vMaxBounds, 0x30);
VALIDATE_OFFSET(CBaseModelInfo, m_nHash, 0x3C);
VALIDATE_OFFSET(CBaseModelInfo, m_nIDEFlags, 0x40);
VALIDATE_OFFSET(CBaseModelInfo, m_nRefCount, 0x44);
VALIDATE_OFFSET(CBaseModelInfo, m_nTexDictionary, 0x48);
VALIDATE_OFFSET(CBaseModelInfo, m_nAnimIndex, 0x56);

class CPedModelInfo : public CBaseModelInfo
{
public:
    uint8  m_pad60[0x20]{};                // +0x60..+0x7F
    int32  m_nGestureAnimIndex = -1;       // +0x80
    int32  m_nFacialAnimIndex = -1;        // +0x84
    int32  m_nVisemesAnimIndex = -1;       // +0x88
    uint8  m_bStreamedPed = 0;             // +0x8C
    uint8  m_pad8D[3]{};                   // +0x8D..+0x8F
    uint32 m_nPedType = 0;                 // +0x90
    uint8  m_pad94[0x28]{};                // +0x94..+0xBB
    uint32 m_nVoiceHash = 0;               // +0xBC
    uint8  m_padC0[0xA0]{};                // +0xC0..+0x15F
};

VALIDATE_SIZE(CPedModelInfo, 0x160);
VALIDATE_OFFSET(CPedModelInfo, m_nGestureAnimIndex, 0x80);
VALIDATE_OFFSET(CPedModelInfo, m_nFacialAnimIndex, 0x84);
VALIDATE_OFFSET(CPedModelInfo, m_nVisemesAnimIndex, 0x88);
VALIDATE_OFFSET(CPedModelInfo, m_bStreamedPed, 0x8C);
VALIDATE_OFFSET(CPedModelInfo, m_nPedType, 0x90);
VALIDATE_OFFSET(CPedModelInfo, m_nVoiceHash, 0xBC);

class CVehicleStructure
{
public:
    uint32 m_nBones[102]{};                // +0x000..+0x197
    uint8  m_pad198[0x74]{};               // +0x198..+0x20B
};

VALIDATE_SIZE(CVehicleStructure, 0x20C);

class CVehicleModelInfo : public CBaseModelInfo
{
public:
    char               m_sGameName[12]{};         // +0x060..+0x06B
    int32              m_nVehicleType = 0;        // +0x06C
    uint8              m_pad70[0x1C]{};           // +0x070..+0x08B
    uint32             m_nHandlingId = 0;         // +0x08C
    uint8              m_pad90[4]{};              // +0x090..+0x093
    uint32             m_nVehicleFlags = 0;       // +0x094
    uint8              m_pad98[0x28]{};           // +0x098..+0x0BF
    int16              m_nAnimIndex2 = -1;        // +0x0C0
    uint8              m_padC2[0x0A]{};           // +0x0C2..+0x0CB
    CVehicleStructure* m_pVehicleStruct = nullptr; // +0x0CC
    uint8              m_padD0[0x6C]{};           // +0x0D0..+0x13B
    uint32             m_nLiveryHashes[4]{};      // +0x13C..+0x14B
    uint8              m_pad14C[0x284]{};         // +0x14C..+0x3CF

    static int32 __cdecl GetNumberOfSeats(int32 modelIndex);
};

VALIDATE_SIZE(CVehicleModelInfo, 0x3D0);
VALIDATE_OFFSET(CVehicleModelInfo, m_sGameName, 0x60);
VALIDATE_OFFSET(CVehicleModelInfo, m_nVehicleType, 0x6C);
VALIDATE_OFFSET(CVehicleModelInfo, m_nHandlingId, 0x8C);
VALIDATE_OFFSET(CVehicleModelInfo, m_nVehicleFlags, 0x94);
VALIDATE_OFFSET(CVehicleModelInfo, m_nAnimIndex2, 0xC0);
VALIDATE_OFFSET(CVehicleModelInfo, m_pVehicleStruct, 0xCC);
VALIDATE_OFFSET(CVehicleModelInfo, m_nLiveryHashes, 0x13C);

class CModelInfo
{
public:
    static inline CBaseModelInfo**& ms_apModelInfoPtrs   = Core::StaticRef<CBaseModelInfo**>(0x15F73B0);
    static inline auto&             ms_modelInfoPtrs     = Core::StaticRef<CBaseModelInfo*[31000]>(0x15F73B0);
    static inline int32&            ms_nModelInfoCount   = Core::StaticRef<int32>(0x15F73A4);
    static inline ModelHashEntry*&  ms_pHashTable        = Core::StaticRef<ModelHashEntry*>(0xF2C240);
    static inline uint16&           ms_nHashTableCount   = Core::StaticRef<uint16>(0xF2C244);
    static inline uint8&            ms_bHashTableSorted  = Core::StaticRef<uint8>(0xF2C23C);

    static void InjectHooks();

    static uint32          HashString(const char* str, uint32 seed = 0);
    static int32           __cdecl CompareHashEntries(const void* a, const void* b);
    static int16           __cdecl SortModelHashTable();
    static CBaseModelInfo* __cdecl GetModelInfo(uint32 hash, int32* outIndex);
    static int32           __cdecl GetModelIndex(uint32 hash);
    static int32           __cdecl GetModelIndexFromName(const char* name);
    static int32           __cdecl GetModelRefCount(int32 modelIndex);
    static void*           __cdecl GetModelDrawData(int32 modelIndex);
    static void*           __cdecl GetModelFragType(int32 modelIndex);
};
