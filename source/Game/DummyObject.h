#pragma once
#include "Game/Entity.h"

class CDummyObject : public CEntity
{
public:
    void* m_pFragLink = nullptr;           // +0x70
    uint8 m_bSavedVisibleFlag = 0;         // +0x74
    uint8 m_pad75[0x0B]{};                 // +0x75..+0x7F

    static void InjectHooks();

    CDummyObject* __thiscall Ctor();
    int32         __thiscall Dtor();
    CDummyObject* __thiscall DeletingDtor(uint8 flags);
    void          __thiscall CleanupLinks();
    bool          __thiscall SaveVisibleFlag();
    int32         __thiscall RestoreVisibleFlag();

    static int32  __cdecl UpdateAllDummyVisibility(uint8 restore);
    static void*  __cdecl InitPool();
};

VALIDATE_SIZE(CDummyObject, 0x80);
VALIDATE_OFFSET(CDummyObject, m_pFragLink, 0x70);
VALIDATE_OFFSET(CDummyObject, m_bSavedVisibleFlag, 0x74);
