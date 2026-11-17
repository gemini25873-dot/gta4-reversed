#pragma once
#include "Core/Pool.h"

class pgDictionaryBase
{
public:
    void**            m_vftable = nullptr;     // +0x00
    void*             m_pBlockMap = nullptr;   // +0x04
    pgDictionaryBase* m_pParent = nullptr;     // +0x08
    int32             m_nRefCount = 0;         // +0x0C

    void DeletingDtor(uint8 flags)
    {
        using Fn = void(__thiscall*)(pgDictionaryBase*, uint8);
        reinterpret_cast<Fn>(m_vftable[0])(this, flags);
    }
};

VALIDATE_OFFSET(pgDictionaryBase, m_pParent, 0x08);
VALIDATE_OFFSET(pgDictionaryBase, m_nRefCount, 0x0C);

struct TxdDef
{
    pgDictionaryBase* m_pDictionary = nullptr; // +0x00
    int32             m_nRefCount = 0;         // +0x04
    uint32            m_nHash = 0;             // +0x08
    int32             m_nParent = -1;          // +0x0C
};

VALIDATE_SIZE(TxdDef, 0x10);
VALIDATE_OFFSET(TxdDef, m_pDictionary, 0x00);
VALIDATE_OFFSET(TxdDef, m_nRefCount, 0x04);
VALIDATE_OFFSET(TxdDef, m_nHash, 0x08);
VALIDATE_OFFSET(TxdDef, m_nParent, 0x0C);

class CTxdStore
{
public:
    static inline CPool<TxdDef>*&    ms_pTxdPool           = Core::StaticRef<CPool<TxdDef>*>(0x11D52D4);
    static inline pgDictionaryBase*& ms_pCurrentTxd        = Core::StaticRef<pgDictionaryBase*>(0x19ACACC);
    static inline pgDictionaryBase*& ms_pStoredTxd         = Core::StaticRef<pgDictionaryBase*>(0x11D52E8);
    static inline int32&             ms_nTxdStackDepth     = Core::StaticRef<int32>(0x11D52EC);
    static inline int32&             ms_nStreamingModuleId = Core::StaticRef<int32>(0xF1CD84);

    static void InjectHooks();

    static CPool<TxdDef>*    __cdecl CreateTxdPool(int32 size, const char* name);
    static int32             __cdecl AddTxdSlot(const char* name);
    static int32             __cdecl RemoveTxdSlot(int32 slot);
    static int32             __cdecl FindTxdSlot(const char* name);
    static int32             __cdecl FindTxdSlotByHash(uint32 hash);
    static int32             __cdecl FindOrAddTxdSlot(const char* name);

    static int32             __cdecl AddRef(int32 slot);
    static int32             __cdecl RemoveRef(int32 slot);
    static int32             __cdecl GetNumRefs(int32 slot);
    static pgDictionaryBase* __cdecl GetTxd(int32 slot);

    static int32             __cdecl RemoveTxd(int32 slot);
    static int32             __cdecl UnloadTxd(int32 slot);
    static void              __cdecl RemoveUnusedTxds(bool force);
    static void              __cdecl Shutdown();
    static void              __cdecl GarbageCollect();

    static pgDictionaryBase* __cdecl PushCurrentTxd();
    static void              __cdecl SetCurrentTxd(int32 slot);
    static pgDictionaryBase* __cdecl PopCurrentTxd();

    static int32             __cdecl SetDictionaryParent(pgDictionaryBase* dict, pgDictionaryBase* parent);
    static int32             __cdecl SetupTxdParent(int32 slot);
    static int32             __cdecl SetTxdDictionary(int32 slot, pgDictionaryBase* dict);
    static int32             __cdecl GetDependencies(int32 slot, int32* outDeps);
    static int32             __cdecl LoadTxd(int32 slot, const char* filename);
};
