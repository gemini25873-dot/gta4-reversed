#include "StdInc.h"
#include "Game/Explosion.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CExplosion::InjectHooks()
{
    Core::Hook::Install(0x95C300, &CExplosionDamageList::Ctor);
    Core::Hook::Install(0x95C410, &CExplosionDamageList::Clear);
    Core::Hook::Install(0x95C1E0, &CExplosion::IsSpecialExplodableModel);
    Core::Hook::Install(0x95C230, &CExplosion::IsInstSpecialExplodable);
    Core::Hook::Install(0x95C4B0, &CExplosion::RegisterExplosionPair);
    Core::Hook::Install(0x95C4F0, &CExplosion::TestForExplosionInArea);
    Core::Hook::Install(0x95C580, &CExplosion::TestForExplosionInSphere);
    Core::Hook::Install(0x95C610, &CExplosion::IsDirected);
    Core::Hook::Install(0x961520, &CExplosion::RemoveExplosionsInArea);
    Core::Hook::Install(0x9618C0, &CExplosion::RemoveAllExplosions);

    Core::Log::Info("CExplosion hooks installed (10 functions)");
}

// 0x95C300
CExplosionDamageList* __thiscall CExplosionDamageList::Ctor()
{
    for (int32 i = 0; i < 8; ++i)
    {
        m_apEntities[i] = nullptr;
    }
    return this;
}

// 0x95C410
void __thiscall CExplosionDamageList::Clear()
{
    for (int32 i = 0; i < 8; ++i)
    {
        if (m_apEntities[i])
        {
            Core::Hook::CallMethod<void>(0x9E6580, m_apEntities[i], &m_apEntities[i]);
            m_apEntities[i] = nullptr;
        }
    }
}

// 0x95C1E0
bool __cdecl CExplosion::IsSpecialExplodableModel(int32 modelIndex)
{
    if (modelIndex == -1)
    {
        return false;
    }

    return modelIndex == Core::StaticRef<int32>(0x1230AFC)
        || modelIndex == Core::StaticRef<int32>(0x123079C)
        || modelIndex == Core::StaticRef<int32>(0x1230838)
        || modelIndex == Core::StaticRef<int32>(0x1230940)
        || modelIndex == Core::StaticRef<int32>(0x1230268)
        || modelIndex == Core::StaticRef<int32>(0x1230658);
}

// 0x95C230
bool __cdecl CExplosion::IsInstSpecialExplodable(void* inst)
{
    const uint16 levelIndex = *reinterpret_cast<const uint16*>(reinterpret_cast<const uint8*>(inst) + 8);
    const uint8* phLevel = Core::StaticRef<const uint8*>(0x18A87D8);
    const uint8* entryBase = *reinterpret_cast<const uint8* const*>(phLevel + 0x70);
    const uint32 flags = *reinterpret_cast<const uint32*>(entryBase + static_cast<uint32>(levelIndex) * 8 + 4);

    if ((flags & 3u) != 2u)
    {
        return false;
    }

    void* entity = Core::Hook::Call<void*>(0xA72220, inst);
    if (!entity)
    {
        return false;
    }

    const int16 modelIndex = *reinterpret_cast<const int16*>(reinterpret_cast<const uint8*>(entity) + 0x2E);
    return !IsSpecialExplodableModel(modelIndex);
}

// 0x95C4B0
int32 __cdecl CExplosion::RegisterExplosionPair(CEntity* entity1, CEntity* entity2)
{
    int32 count = ms_nNumPairs;
    if (count < MAX_EXPLOSION_PAIRS)
    {
        CEntity** pairs = GetPairArray();

        CEntity** slot1 = &pairs[count * 2];
        *slot1 = entity1;
        Core::Hook::CallMethod<void>(0x9E6550, entity1, slot1);

        CEntity** slot2 = &pairs[ms_nNumPairs * 2 + 1];
        *slot2 = entity2;
        const int32 result = Core::Hook::CallMethod<int32>(0x9E6550, entity2, slot2);

        ++ms_nNumPairs;
        return result;
    }
    return count;
}

// 0x95C4F0
uint8 __cdecl CExplosion::TestForExplosionInArea(int32 type, const CVector* minPos, const CVector* maxPos)
{
    CExplosionData** explosions = GetActiveExplosions();
    for (int32 i = 0; i < MAX_ACTIVE_EXPLOSIONS; ++i)
    {
        CExplosionData* exp = explosions[i];
        if (exp && (exp->m_nType == type || type == -1))
        {
            const CVector& pos = exp->m_vecPos;
            if (pos.x >= minPos->x && maxPos->x >= pos.x &&
                pos.y >= minPos->y && maxPos->y >= pos.y &&
                pos.z >= minPos->z && maxPos->z >= pos.z)
            {
                return 1;
            }
        }
    }
    return 0;
}

// 0x95C580
uint8 __cdecl CExplosion::TestForExplosionInSphere(int32 type, const CVector* pos, float radius)
{
    const float radiusSq = radius * radius;
    CExplosionData** explosions = GetActiveExplosions();
    for (int32 i = 0; i < MAX_ACTIVE_EXPLOSIONS; ++i)
    {
        CExplosionData* exp = explosions[i];
        if (exp && (exp->m_nType == type || type == -1))
        {
            const float dx = exp->m_vecPos.x - pos->x;
            const float dy = exp->m_vecPos.y - pos->y;
            const float dz = exp->m_vecPos.z - pos->z;
            if (radiusSq > (dx * dx + dy * dy + dz * dz))
            {
                return 1;
            }
        }
    }
    return 0;
}

// 0x95C610
bool __cdecl CExplosion::IsDirected(int32 type)
{
    return GetExplosionInfo()[type].m_fDirectedWidth > 0.0f;
}

// 0x961520
uint8 __cdecl CExplosion::RemoveExplosionsInArea(int32 type, const CVector* pos, float radius)
{
    const float radiusSq = radius * radius;
    CExplosionData** explosions = GetActiveExplosions();
    uint8 result = 0;

    for (int32 i = 0; i < MAX_ACTIVE_EXPLOSIONS; ++i)
    {
        CExplosionData* exp = explosions[i];
        if (exp && (exp->m_nType == type || type == -1))
        {
            const float dx = exp->m_vecPos.x - pos->x;
            const float dy = exp->m_vecPos.y - pos->y;
            const float dz = exp->m_vecPos.z - pos->z;
            if (radiusSq > (dx * dx + dy * dy + dz * dz))
            {
                Core::Hook::Call<void>(0x961470, i);
                result = 1;
            }
        }
    }
    return result;
}

// 0x9618C0
void __cdecl CExplosion::RemoveAllExplosions()
{
    CExplosionData** explosions = GetActiveExplosions();
    for (int32 i = 0; i < MAX_ACTIVE_EXPLOSIONS; ++i)
    {
        if (explosions[i])
        {
            Core::Hook::Call<void>(0x961470, i);
        }
    }
}
