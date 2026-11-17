#pragma once
#include "Core/Pool.h"

struct IplDef
{
    void*  m_pData = nullptr;              // +0x00
    uint32 m_nUnk04 = 0;                   // +0x04
    uint32 m_nHash = 0;                    // +0x08
    uint8  m_pad0C[0x24]{};                // +0x0C..+0x2F
    float  m_fBounds[4]{};                 // +0x30..+0x3F (CRect)
    uint8  m_pad40[0x10]{};                // +0x40..+0x4F
    int32  m_nParentIpl = -1;              // +0x50
    uint8  m_bDisableWhenFar = 0;          // +0x54
    uint8  m_bUnk55 = 0;                   // +0x55
    uint8  m_bLargeBounds = 0;             // +0x56
    uint8  m_pad57[0x09]{};                // +0x57..+0x5F
};

VALIDATE_SIZE(IplDef, 0x60);
VALIDATE_OFFSET(IplDef, m_nHash, 0x08);
VALIDATE_OFFSET(IplDef, m_fBounds, 0x30);
VALIDATE_OFFSET(IplDef, m_nParentIpl, 0x50);
VALIDATE_OFFSET(IplDef, m_bDisableWhenFar, 0x54);
VALIDATE_OFFSET(IplDef, m_bLargeBounds, 0x56);

class CIplStore
{
public:
    static inline CPool<IplDef>*& ms_pPool              = Core::StaticRef<CPool<IplDef>*>(0x168FF98);
    static inline void*&          ms_pQuadTree          = Core::StaticRef<void*>(0x168FF9C);
    static inline uint8&          ms_bIplsNeededAtPosn  = Core::StaticRef<uint8>(0x168FFA0);
    static inline float&          ms_fLoadRadius        = Core::StaticRef<float>(0x168FFA4);
    static inline uint8&          ms_bAllIplsLoaded     = Core::StaticRef<uint8>(0x168FFA8);
    static inline uint8*&         ms_pRequiredFlags     = Core::StaticRef<uint8*>(0x168FFAC);
    static inline int32&          ms_nDefArrayCount     = Core::StaticRef<int32>(0x168FFB4);
    static inline int32&          ms_nStreamingModuleId = Core::StaticRef<int32>(0xF3BE40);

    static void** GetDefArrays() { return reinterpret_cast<void**>(Core::Rebase(0x168FFB8)); }

    static void InjectHooks();

    static CPool<IplDef>* __cdecl CreatePool(int32 size, const char* name);
    static int32          __cdecl AllocateDefArray(uint32 count);
    static void*          __cdecl GetDefArray(int32 index);
    static int32          __cdecl RequestIplIfInStreaming(int32 slot, int32 flags);
    static int32          __cdecl FindIplSlotByHash(uint32 hash);
    static int32          __cdecl FindIplSlot(const char* name);
    static void*          __cdecl AllocateIplSlotData(int32 slot);
    static int32          __cdecl SetIplDisabled(int32 slot, uint8 enabled);
    static void           __cdecl RemoveRelatedIpls(int32 parentIndex);
    static int32          __cdecl RequestIplAndMark(int32 slot);
    static int32          __cdecl RemoveIplAndMark(int32 slot);
    static int32          __cdecl SetIfIplIsRequired(const void* pos2D, IplDef* def);
    static int32          __cdecl SetIfIplIsRequiredReducedBB(const void* pos2D, IplDef* def);
    static void           __cdecl CheckIfIplLoadedAtPos(const void* pos2D, IplDef* def);
    static void           __cdecl RequestIplAtPosImmediate(const void* pos2D, IplDef* def);
};
