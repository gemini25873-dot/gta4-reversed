#pragma once
#include "Game/Entity.h"

class CBuilding : public CEntity
{
public:
    static void InjectHooks();

    CBuilding* __thiscall Ctor();
    void       __thiscall Dtor();
    CBuilding* __thiscall DeletingDtor(uint8 flags);
    int16      __thiscall ReplaceWithNewModel(int16 modelIndex);

    static void* __cdecl InitPool();
};

VALIDATE_SIZE(CBuilding, 0x70);
