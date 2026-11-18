#pragma once
#include "Core/Vector.h"

class CEntity;

struct tExplosionInfo
{
    float  m_fDamageAtCentre = 0.0f;         // +0x00
    float  m_fDamageAtEdge = 0.0f;           // +0x04
    float  m_fNetworkPlayerModifier = 0.0f;  // +0x08
    float  m_fNetworkPedModifier = 0.0f;     // +0x0C
    float  m_fEndRadius = 0.0f;              // +0x10
    float  m_fInitSpeed = 0.0f;              // +0x14
    float  m_fDecayFactor = 0.0f;            // +0x18
    float  m_fForceFactor = 0.0f;            // +0x1C
    float  m_fRagdollModifier = 0.0f;        // +0x20
    uint32 m_nFxHash = 0;                    // +0x24
    uint32 m_nFxAirHash = 0;                 // +0x28
    float  m_fFxScale = 0.0f;                // +0x2C
    float  m_fDirectedWidth = 0.0f;          // +0x30
    float  m_fDirectedLife = 0.0f;           // +0x34
    uint32 m_nNumFiresMin = 0;               // +0x38
    uint32 m_nNumFiresMax = 0;               // +0x3C
    float  m_fFireRangeMin = 0.0f;           // +0x40
    float  m_fFireRangeMax = 0.0f;           // +0x44
    float  m_fCameraShake = 0.0f;            // +0x48
    uint32 m_nWeaponType = 0;                // +0x4C
    uint32 m_nInfinite = 0;                  // +0x50
    float  m_fLightsR = 0.0f;                // +0x54
    float  m_fLightsG = 0.0f;                // +0x58
    float  m_fLightsB = 0.0f;                // +0x5C
    float  m_fLightsScale = 0.0f;            // +0x60
    float  m_fLightsRange = 0.0f;            // +0x64
    float  m_fLightsTime = 0.0f;             // +0x68
    uint32 m_nType = 0;                      // +0x6C
};

VALIDATE_SIZE(tExplosionInfo, 0x70);
VALIDATE_OFFSET(tExplosionInfo, m_fEndRadius, 0x10);
VALIDATE_OFFSET(tExplosionInfo, m_nFxHash, 0x24);
VALIDATE_OFFSET(tExplosionInfo, m_fDirectedWidth, 0x30);
VALIDATE_OFFSET(tExplosionInfo, m_nWeaponType, 0x4C);
VALIDATE_OFFSET(tExplosionInfo, m_nInfinite, 0x50);
VALIDATE_OFFSET(tExplosionInfo, m_nType, 0x6C);

struct CExplosionDamageList
{
    CEntity* m_apEntities[8]{};              // +0x00..+0x1F

    CExplosionDamageList* __thiscall Ctor();
    void                  __thiscall Clear();
};

VALIDATE_SIZE(CExplosionDamageList, 0x20);
VALIDATE_OFFSET(CExplosionDamageList, m_apEntities, 0x00);

struct CExplosionData
{
    uint8   m_pad00[0x40]{};                 // +0x00..+0x3F
    int32   m_nType = 0;                     // +0x40
    uint8   m_pad44[0x0C]{};                 // +0x44..+0x4F
    CVector m_vecPos{};                      // +0x50..+0x5B
    uint8   m_pad5C[0x04]{};                 // +0x5C..+0x5F
    float   m_fScale = 0.0f;                 // +0x60
};

VALIDATE_OFFSET(CExplosionData, m_nType, 0x40);
VALIDATE_OFFSET(CExplosionData, m_vecPos, 0x50);
VALIDATE_OFFSET(CExplosionData, m_fScale, 0x60);

class CExplosion
{
public:
    static constexpr int32 MAX_EXPLOSION_INFOS = 25;
    static constexpr int32 MAX_ACTIVE_EXPLOSIONS = 16;
    static constexpr int32 MAX_EXPLOSION_PAIRS = 128;

    static inline int32& ms_nNumPairs = Core::StaticRef<int32>(0x15C1734);

    static tExplosionInfo* GetExplosionInfo()
    {
        return reinterpret_cast<tExplosionInfo*>(Core::Rebase(0x15C0778));
    }

    static CExplosionData** GetActiveExplosions()
    {
        return reinterpret_cast<CExplosionData**>(Core::Rebase(0x15C12A8));
    }

    static CEntity** GetPairArray()
    {
        return reinterpret_cast<CEntity**>(Core::Rebase(0x15C1328));
    }

    static void InjectHooks();

    static bool  __cdecl IsSpecialExplodableModel(int32 modelIndex);
    static bool  __cdecl IsInstSpecialExplodable(void* inst);
    static int32 __cdecl RegisterExplosionPair(CEntity* entity1, CEntity* entity2);
    static uint8 __cdecl TestForExplosionInArea(int32 type, const CVector* minPos, const CVector* maxPos);
    static uint8 __cdecl TestForExplosionInSphere(int32 type, const CVector* pos, float radius);
    static bool  __cdecl IsDirected(int32 type);
    static uint8 __cdecl RemoveExplosionsInArea(int32 type, const CVector* pos, float radius);
    static void  __cdecl RemoveAllExplosions();
};
