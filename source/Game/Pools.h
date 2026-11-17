#pragma once
#include "Core/Pool.h"

class CPed;
class CVehicle;
class CObject;
class CBuilding;
class CDummyObject;

class CPools
{
public:
    static inline CPool<CPed>*&         ms_pPedPool           = Core::StaticRef<CPool<CPed>*>(0x18A82AC);
    static inline CPool<CVehicle>*&     ms_pVehiclePool       = Core::StaticRef<CPool<CVehicle>*>(0x1619240);
    static inline CPool<CObject>*&      ms_pObjectPool        = Core::StaticRef<CPool<CObject>*>(0x1350CE0);
    static inline CPool<CDummyObject>*& ms_pDummyObjectPool = Core::StaticRef<CPool<CDummyObject>*>(0x168FF90);
    static inline CPool<CBuilding>*&    ms_pBuildingPool      = Core::StaticRef<CPool<CBuilding>*>(0x168FED0);
    static inline CPool<CDummyObject>*& ms_pDummyPool         = Core::StaticRef<CPool<CDummyObject>*>(0x168FF90);
    static inline CPoolBase*&           ms_pTaskPool          = Core::StaticRef<CPoolBase*>(0x164B01C);
    static inline CPoolBase*&           ms_pPtrNodeSinglePool = Core::StaticRef<CPoolBase*>(0x16B7758);
    static inline CPoolBase*&           ms_pPtrNodeDoublePool = Core::StaticRef<CPoolBase*>(0x16B775C);
    static inline CPoolBase*&           ms_pInteriorInstPool  = Core::StaticRef<CPoolBase*>(0x165D2E4);
    static inline CPoolBase*&           ms_pEventPool         = Core::StaticRef<CPoolBase*>(0x152F4B4);
    static inline CPoolBase*&           ms_pQuadTreeNodePool  = Core::StaticRef<CPoolBase*>(0x13504D0);
};
