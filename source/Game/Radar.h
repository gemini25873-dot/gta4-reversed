#pragma once
#include "Core/Vector.h"

enum eRadarSprite
{
    SPRITE_HIGHER,
    SPRITE_LEVEL,
    SPRITE_LOWER,
    SPRITE_OBJECTIVE_HIGHER,
    SPRITE_OBJECTIVE,
    SPRITE_OBJECTIVE_LOWER,
    SPRITE_CENTRE,
    SPRITE_NORTH,
    SPRITE_WAYPOINT,
    SPRITE_WEAPON_PISTOL,
    SPRITE_WEAPON_SHOTGUN,
    SPRITE_WEAPON_SMG,
    SPRITE_WEAPON_ASSAULT_RIFLE,
    SPRITE_WEAPON_ROCKET,
    SPRITE_WEAPON_GRENADES,
    SPRITE_WEAPON_MOLOTOV,
    SPRITE_WEAPON_SNIPER,
    SPRITE_WEAPON_BAT,
    SPRITE_WEAPON_KNIFE,
    SPRITE_WEAPON_HEALTH,
    SPRITE_WEAPON_ARMOUR,
    SPRITE_BURGERSHOT,
    SPRITE_CHICKEN,
    SPRITE_VLAD,
    SPRITE_INTERNET,
    SPRITE_MANNY,
    SPRITE_LITTLEJACOB,
    SPRITE_ROMAN,
    SPRITE_FAUSTIN,
    SPRITE_SAFEHOUSE,
    SPRITE_TAXI,
    SPRITE_BERNIECRANE,
    SPRITE_BRUCIE,
    SPRITE_CIA,
    SPRITE_DWAYNE,
    SPRITE_ELIZABETA,
    SPRITE_GAMBETTI,
    SPRITE_JIMMY,
    SPRITE_MCDERRICK,
    SPRITE_MCFRANCIS,
    SPRITE_MCGERRY,
    SPRITE_MCKATIE,
    SPRITE_MCPACKIE,
    SPRITE_PHILBELL,
    SPRITE_PLAYBOYX,
    SPRITE_RAYBOCCINO,
    SPRITE_8BALL,
    SPRITE_BAR,
    SPRITE_BOATTOUR,
    SPRITE_BOWLING,
    SPRITE_CLOTHES,
    SPRITE_CLUB,
    SPRITE_DARTS,
    SPRITE_DWAYNE_RED,
    SPRITE_GIRLFRIEND,
    SPRITE_PLAYBOYX_RED,
    SPRITE_HELITOUR,
    SPRITE_RESTAURANT,
    SPRITE_STATION,
    SPRITE_WEAPONS,
    SPRITE_POLICE,
    SPRITE_FIRE,
    SPRITE_HOSPITAL,
    SPRITE_MALE,
    SPRITE_FEMALE,
    SPRITE_RACEFLAG,
    SPRITE_STRIPCLUB,
    SPRITE_EMPTY,
    SPRITE_POLICE_CHASE,
    SPRITE_DIMITRI,
    SPRITE_COMEDY,
    SPRITE_CABARET,
    SPRITE_RANSOM,
    SPRITE_POLICE_HELI,
    SPRITE_MICHELLE,
    SPRITE_SPRAYSHOP,
    SPRITE_ASSASSIN,
    SPRITE_SNAKE,
    SPRITE_DEAL,
    SPRITE_CAR,
    SPRITE_LAWYER,
    SPRITE_CUP,
    SPRITE_MULTIPLAYER,
    SPRITE_STATION_3,
    SPRITE_STATION_8,
    SPRITE_STATION_A,
    SPRITE_STATION_B,
    SPRITE_STATION_C,
    SPRITE_STATION_E,
    SPRITE_STATION_J,
    SPRITE_STATION_K,
    SPRITE_CARWASH,
    SPRITE_UNITED_LIBERTY_PAPER,
    SPRITE_WEAPON_UP,
    SPRITE_WEAPON_DOWN,
    SPRITE_EPISODE_BLIP_1,
    SPRITE_EPISODE_BLIP_2,
    SPRITE_EPISODE_BLIP_3,
    SPRITE_EPISODE_BLIP_4,
    SPRITE_EPISODE_BLIP_5,
    SPRITE_EPISODE_BLIP_6,
    SPRITE_EPISODE_BLIP_7,
    SPRITE_EPISODE_BLIP_8,
    SPRITE_EPISODE_BLIP_9,
    SPRITE_EPISODE_BLIP_10,
    SPRITE_EPISODE_BLIP_11,
    SPRITE_EPISODE_BLIP_12,
    SPRITE_EPISODE_BLIP_13,
    SPRITE_EPISODE_BLIP_14,
    SPRITE_EPISODE_BLIP_15,
    SPRITE_ARMWRESTLING,
    SPRITE_ROADRAGE,
    SPRITE_CARDGAME,
    SPRITE_AIRHOCKEY,
    SPRITE_GANGWAR,
    SPRITE_WEAPON_POOLCUE,
    SPRITE_WEAPON_PIPEBOMB,
    SPRITE_WEAPON_GRENADELAUNCHER,
    SPRITE_TERRY,
    SPRITE_CLAY,
    SPRITE_TERRITORY_1,
    SPRITE_TERRITORY_2,
    SPRITE_TERRITORY_3,
    SPRITE_TERRITORY_4,
    SPRITE_TERRITORY_5,
    SPRITE_TERRITORY_6,
    SPRITE_MOTORCYCLE,
    SPRITE_JIMSWEAPON,
    SPRITE_WEAPON_STICKYBOMB
};

struct sRadarTrace
{
    uint16  m_nReferenceIndex = 0;       // +0x00
    uint8   m_pad02[6]{};                // +0x02..+0x07
    uint8   m_bNotSimple = 0;            // +0x08
    uint8   m_pad09[3]{};                // +0x09..+0x0B
    uint32  m_nDisplay = 0;              // +0x0C
    uint8   m_pad10[8]{};                // +0x10..+0x17
    float   m_fScaleActual = 0.0f;       // +0x18
    uint8   m_pad1C[4]{};                // +0x1C..+0x1F
    uint32  m_nFlags = 0;                // +0x20
    int32   m_nSprite = -1;              // +0x24
    uint8   m_pad28[8]{};                // +0x28..+0x2F
    CVector m_vPos{};                    // +0x30..+0x3B
    uint8   m_pad3C[4]{};                // +0x3C..+0x3F
    float   m_fRotation = 0.0f;          // +0x40
    uint8   m_pad44[4]{};                // +0x44..+0x47
    int32   m_nBlipType = 0;             // +0x48
    int32   m_nBlipCategory = 0;         // +0x4C
    float   m_fScale = 0.0f;             // +0x50
    uint32  m_nColour = 0;               // +0x54
    uint8   m_nAlpha = 0;                // +0x58
    uint8   m_pad59[3]{};                // +0x59..+0x5B
    void*   m_pProperties = nullptr;     // +0x5C
};

VALIDATE_SIZE(sRadarTrace, 0x60);
VALIDATE_OFFSET(sRadarTrace, m_nReferenceIndex, 0x00);
VALIDATE_OFFSET(sRadarTrace, m_bNotSimple, 0x08);
VALIDATE_OFFSET(sRadarTrace, m_nDisplay, 0x0C);
VALIDATE_OFFSET(sRadarTrace, m_fScaleActual, 0x18);
VALIDATE_OFFSET(sRadarTrace, m_nFlags, 0x20);
VALIDATE_OFFSET(sRadarTrace, m_nSprite, 0x24);
VALIDATE_OFFSET(sRadarTrace, m_vPos, 0x30);
VALIDATE_OFFSET(sRadarTrace, m_fRotation, 0x40);
VALIDATE_OFFSET(sRadarTrace, m_nBlipType, 0x48);
VALIDATE_OFFSET(sRadarTrace, m_nBlipCategory, 0x4C);
VALIDATE_OFFSET(sRadarTrace, m_fScale, 0x50);
VALIDATE_OFFSET(sRadarTrace, m_nAlpha, 0x58);
VALIDATE_OFFSET(sRadarTrace, m_pProperties, 0x5C);

class CRadar
{
public:
    static inline auto&   ms_RadarTrace          = Core::StaticRef<sRadarTrace*[1500]>(0x119ED50);
    static inline auto&   ms_RadarBlipSprites    = Core::StaticRef<void*[128]>(0x119EB20);
    static inline uint16& ms_nNextBlipRef        = Core::StaticRef<uint16>(0x119ED38);
    static inline int32&  ms_iSimpleBlip         = Core::StaticRef<int32>(0xF1C97C);
    static inline int32&  ms_nActiveLevel        = Core::StaticRef<int32>(0xF1C978);
    static inline int32&  ms_nEpisodeBlipStart   = Core::StaticRef<int32>(0xF1C988);

    static void InjectHooks();

    static int32 __cdecl AssignBlipHandle(int32 actualIndex);
    static int32 __cdecl ConvertUniqueBlipToActualBlip(int32 blipHandle);
    static bool  __cdecl IsBlipIdInUse(int32 blipHandle);
    static uint8 __cdecl RemoveBlip(int32 index, uint8 isScriptHandle);
    static int32 __cdecl SetBlipParameterFloat(int32 paramId, int32 blipHandle, float value);
    static int32 __cdecl SetBlipVectorParam(int32 paramId, int32 blipHandle, const CVector* vec);
    static uint8 __cdecl SetBlipParameter(int32 paramId, int32 blipHandle, int32 value);

    // Script blip helpers (0xBB1DB0..0xBB2770)
    static int32 __cdecl GetSimpleBlipHandle();
    static int32 __cdecl GetBlipCategory(int32 blipHandle);
    static int32 __cdecl GetBlipType(int32 blipHandle);
    static int32 __cdecl GetBlipSprite(int32 blipHandle);
    static int32 __cdecl SetBlipCoordinates(int32 blipHandle, float x, float y, float z);
    static int32 __cdecl SetBlipColour(int32 blipHandle, int32 colour);
    static int32 __cdecl SetBlipDisplay(int32 blipHandle, int32 display);
    static bool  __cdecl IsBlipFlag8Set(int32 blipHandle);
    static int32 __cdecl SetBlipFlashes(int32 blipHandle, uint8 on);
    static int32 __cdecl SetBlipShortRange(int32 blipHandle, uint8 on);
    static int32 __cdecl SetBlipFriendly(int32 blipHandle, uint8 on);
    static int32 __cdecl SetBlipScale(int32 blipHandle, float scale);
    static int32 __cdecl SetBlipPriority(int32 blipHandle, int32 priority);
    static int32 __cdecl SetBlipDisplayMode(int32 blipHandle, int32 mode);
    static int32 __cdecl SetBlipSprite(int32 blipHandle, int32 sprite);
    static int32 __cdecl SetBlipRoute(int32 blipHandle, uint8 on);
    static int32 __cdecl SetBlipMinimalClean(int32 blipHandle, int32 value);
    static int32 __cdecl DoesBlipExist(int32 blipHandle);
};
