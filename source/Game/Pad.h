#pragma once

enum ePadControls
{
    INPUT_NEXT_CAMERA,
    INPUT_SPRINT,
    INPUT_JUMP,
    INPUT_ENTER,
    INPUT_ATTACK,
    INPUT_ATTACK2,
    INPUT_AIM,
    INPUT_LOOK_BEHIND,
    INPUT_NEXT_WEAPON,
    INPUT_PREV_WEAPON,
    INPUT_NEXT_TARGET_LEFT,
    INPUT_NEXT_TARGET_RIGHT,
    INPUT_MOVE_LEFT,
    INPUT_MOVE_RIGHT,
    INPUT_MOVE_UP,
    INPUT_MOVE_DOWN,
    INPUT_LOOK_LEFT,
    INPUT_LOOK_RIGHT,
    INPUT_LOOK_UP,
    INPUT_LOOK_DOWN,
    INPUT_DUCK,
    INPUT_PHONE_TAKE_OUT,
    INPUT_PHONE_PUT_AWAY,
    INPUT_PICKUP,
    INPUT_SNIPER_ZOOM_IN,
    INPUT_SNIPER_ZOOM_OUT,
    INPUT_SNIPER_ZOOM_IN_ALTERNATE,
    INPUT_SNIPER_ZOOM_OUT_ALTERNATE,
    INPUT_COVER,
    INPUT_RELOAD,
    INPUT_VEH_MOVE_LEFT,
    INPUT_VEH_MOVE_RIGHT,
    INPUT_VEH_MOVE_UP,
    INPUT_VEH_MOVE_DOWN,
    INPUT_VEH_GUN_LEFT,
    INPUT_VEH_GUN_RIGHT,
    INPUT_VEH_GUN_UP,
    INPUT_VEH_GUN_DOWN,
    INPUT_VEH_ATTACK,
    INPUT_VEH_ATTACK2,
    INPUT_VEH_ACCELERATE,
    INPUT_VEH_BRAKE,
    INPUT_VEH_HEADLIGHT,
    INPUT_VEH_EXIT,
    INPUT_VEH_HANDBRAKE,
    INPUT_VEH_HANDBRAKE_ALT,
    INPUT_VEH_HOTWIRE_LEFT,
    INPUT_VEH_HOTWIRE_RIGHT,
    INPUT_VEH_LOOK_LEFT,
    INPUT_VEH_LOOK_RIGHT,
    INPUT_VEH_LOOK_BEHIND,
    INPUT_VEH_CIN_CAM,
    INPUT_VEH_NEXT_RADIO,
    INPUT_VEH_PREV_RADIO,
    INPUT_VEH_HORN,
    INPUT_VEH_FLY_THROTTLE_UP,
    INPUT_VEH_FLY_THROTTLE_DOWN,
    INPUT_VEH_FLY_YAW_LEFT,
    INPUT_VEH_FLY_YAW_RIGHT,
    INPUT_MELEE_ATTACK1,
    INPUT_MELEE_ATTACK2,
    INPUT_MELEE_ATTACK3,
    INPUT_MELEE_KICK,
    INPUT_MELEE_BLOCK,
    INPUT_FRONTEND_DOWN,
    INPUT_FRONTEND_UP,
    INPUT_FRONTEND_LEFT,
    INPUT_FRONTEND_RIGHT,
    INPUT_FRONTEND_RDOWN,
    INPUT_FRONTEND_RUP,
    INPUT_FRONTEND_RLEFT,
    INPUT_FRONTEND_RRIGHT,
    INPUT_FRONTEND_AXIS_X,
    INPUT_FRONTEND_AXIS_Y,
    INPUT_FRONTEND_RIGHT_AXIS_X,
    INPUT_FRONTEND_RIGHT_AXIS_Y,
    INPUT_FRONTEND_PAUSE,
    INPUT_FRONTEND_ACCEPT,
    INPUT_FRONTEND_CANCEL,
    INPUT_FRONTEND_X,
    INPUT_FRONTEND_Y,
    INPUT_FRONTEND_LB,
    INPUT_FRONTEND_RB,
    INPUT_FRONTEND_LT,
    INPUT_FRONTEND_RT,
    INPUT_MELEE_ATTACK4,
    INPUT_ZOOM_RADAR,
    INPUT_FREE_AIM,
    INPUT_MOUSE_UD,
    INPUT_MOUSE_LR,
    INPUT_MOVE_KEY_FB,
    INPUT_MOVE_KEY_LR,
    INPUT_MOVE_KEY_STUNTJUMP,
    INPUT_MOUSE_WHEEL_UP,
    INPUT_MOUSE_WHEEL_DOWN,
    INPUT_FRONTEND_AXIS_UD,
    INPUT_FRONTEND_AXIS_LR,
    INPUT_FRONTEND_LEGEND,
    INPUT_FRONTEND_APPLY,
    INPUT_FRONTEND_SHOWSCORE,
    INPUT_WEAPON_UNARMED,
    INPUT_WEAPON_MELEE,
    INPUT_WEAPON_HANDGUN,
    INPUT_WEAPON_SHOTGUN,
    INPUT_WEAPON_SMG,
    INPUT_WEAPON_RIFLE,
    INPUT_WEAPON_SNIPER,
    INPUT_WEAPON_HEAVY,
    INPUT_WEAPON_THROWN,
    INPUT_WEAPON_SPECIAL,
    INPUT_VEH_KEY_UD,
    INPUT_VEH_KEY_LR,
    INPUT_VEH_NEXT_WEAPON,
    INPUT_VEH_PREV_WEAPON,
    INPUT_TURN_OFF_RADIO,
    INPUT_FRONTEND_MINIGAME_1,
    INPUT_FRONTEND_MINIGAME_2,
    INPUT_FRONTEND_MINIGAME_3,
    INPUT_FRONTEND_MINIGAME_4,
    INPUT_FRONTEND_MODEL_SCREEN,
    INPUT_FRONTEND_VEHICLE_TOGGLE,
    INPUT_FRONTEND_DELETE_FILTER,
    INPUT_FRONTEND_REFRESH,
    INPUT_FRONTEND_PLAYER_LIST,
    INPUT_TEXT_CHAT_TOGGLE,
    INPUT_TEXT_CHAT_TEAM,
    INPUT_TEXT_CHAT_UNIVERSAL,
    INPUT_TEXT_CHAT_RESULTS_TOGGLE,
    INPUT_TEXT_CHAT_Y,
    INPUT_FRONTEND_LOCK_LOBBY,
    INPUT_KB_UP,
    INPUT_KB_DOWN,
    INPUT_KB_LEFT,
    INPUT_KB_RIGHT,
    INPUT_NEXT_TRACK,
    INPUT_PREV_TRACK,
    INPUT_DROP_WEAPON,
    INPUT_MOUSE_LMB,
    INPUT_MOUSE_RMB,
    INPUT_FE_MOUSE_UD,
    INPUT_FE_MOUSE_LR,
    INPUT_VEH_FLY_PITCH_UP,
    INPUT_VEH_FLY_PITCH_DOWN,
    INPUT_VEH_FLY_ROLL_LEFT,
    INPUT_VEH_FLY_ROLL_RIGHT,
    INPUT_REPLAY_PLAY,
    INPUT_REPLAY_SAVE_TO_HDD,
    INPUT_REPLAY_LOAD_FROM_HDD,
    INPUT_REPLAY_EXIT,
    INPUT_FRONTEND_MARKER_DELETE,
    INPUT_FRONTEND_REPLAY_PAUSE,
    INPUT_FRONTEND_REPLAY_REWIND,
    INPUT_FRONTEND_REPLAY_FFWD,
    INPUT_FRONTEND_REPLAY_NEWMARKER,
    INPUT_FRONTEND_REPLAY_RECORD,
    INPUT_FRONTEND_REPLAY_SCREENSHOT,
    INPUT_FRONTEND_REPLAY_HIDEHUD,
    INPUT_FRONTEND_REPLAY_STARTPOINT,
    INPUT_FRONTEND_REPLAY_ENDPOINT,
    INPUT_FRONTEND_REPLAY_ADVANCE,
    INPUT_FRONTEND_REPLAY_BACK,
    INPUT_FRONTEND_REPLAY_SHOWHOTKEY,
    INPUT_FRONTEND_REPLAY_TOOLS,
    INPUT_FRONTEND_REPLAY_RESTART,
    INPUT_FRONTEND_REPLAY_CYCLEMARKERLEFT,
    INPUT_FRONTEND_REPLAY_CYCLEMARKERRIGHT,
    INPUT_FRONTEND_AXIS_FWD,
    INPUT_FRONTEND_AXIS_BACK,
    INPUT_FRONTEND_AXIS_LEFT,
    INPUT_FRONTEND_AXIS_RIGHT,
    INPUT_FRONTEND_DEFINITION,
    INPUT_FRONTEND_F6,
    INPUT_FE_REPLAY_TOGGLETIME,
    INPUT_FE_REPLAY_TOGGLETIPS,
    INPUT_FE_REPLAY_PREVIEW,
    INPUT_FRONTEND_PASTE,
    INPUT_FRONTEND_CUT,
    INPUT_FRONTEND_COPY,
    INPUT_VOICE_CHAT,
    INPUT_SNIPER_ZOOM_IN_KB,
    INPUT_SNIPER_ZOOM_OUT_KB,
    INPUT_KB_PHONE_ACCEPT,
    INPUT_KB_PHONE_CANCEL,
    INPUT_VEH_ATTACK3_KB,
    INPUT_DETONATE,
    INPUT_VEH_MOVE_LEFT_2,
    INPUT_VEH_MOVE_RIGHT_2,
    NUM_INPUTS
};

struct tValueHistoryItem
{
    uint8  m_nValue = 0;           // +0x00
    uint8  m_pad01[3]{};           // +0x01..+0x03
    uint32 m_nLastUpdateTime = 0;  // +0x04
};
VALIDATE_SIZE(tValueHistoryItem, 0x08);

class tValueHistory
{
public:
    tValueHistoryItem m_aItems[64]{};
};
VALIDATE_SIZE(tValueHistory, 0x200);

struct tPadValues
{
    void**         m_vftable = nullptr;       // +0x00
    uint8          m_nUnknown4 = 0;           // +0x04
    uint8          m_nContext = 0;            // +0x05
    uint8          m_nCurrentValue = 0;       // +0x06
    uint8          m_nLastValue = 0;          // +0x07
    uint8          m_nHistoryIndex = 0;       // +0x08
    uint8          m_pad09[3]{};              // +0x09..+0x0B
    tValueHistory* m_pHistory = nullptr;      // +0x0C

    void   __thiscall AllocHistory();
    double __thiscall GetAnalogValue() const;
};
VALIDATE_SIZE(tPadValues, 0x10);

struct ioMapper
{
    int32       m_nCount = 0;                 // +0x000
    int32       m_nPadIndex = 0;              // +0x004
    uint32      m_aDevices[164]{};            // +0x008
    uint32      m_aKeys[164]{};               // +0x298
    tPadValues* m_apValues[164]{};            // +0x528
};
VALIDATE_SIZE(ioMapper, 0x7B8);

class CPad
{
public:
    static inline CPad* const Pads = reinterpret_cast<CPad*>(0x10FB818); // rebased in getters

    ioMapper   m_mapper0{};                   // +0x0000..+0x07B7
    ioMapper   m_mapper1{};                   // +0x07B8..+0x0F6F
    ioMapper   m_mapper2{};                   // +0x0F70..+0x1727
    ioMapper   m_mapper3{};                   // +0x1728..+0x1EDF
    ioMapper   m_mapper4{};                   // +0x1EE0..+0x2697
    tPadValues m_aValues[NUM_INPUTS]{};       // +0x2698..+0x3247
    tPadValues m_extraValues[4]{};            // +0x3248..+0x3287
    uint8      m_bUnk3288 = 0;                // +0x3288
    uint8      m_bIsUsingKeyboardForAim = 0;  // +0x3289
    uint8      m_bUnk328A = 0;                // +0x328A
    uint8      m_bUnk328B = 0;                // +0x328B
    uint8      m_bUnk328C = 0;                // +0x328C
    uint8      m_bIsUsingKeyboardForHeli = 0; // +0x328D
    uint8      m_pad328E[2]{};                // +0x328E..+0x328F
    tPadValues m_extraValue5{};               // +0x3290..+0x329F
    uint32     m_nUnk32A0 = 0;                // +0x32A0
    int32      m_nPadNumber = 0;              // +0x32A4
    int32      m_nUnk32A8 = -1;               // +0x32A8
    ioMapper   m_mapper5{};                   // +0x32AC..+0x3A63
    uint8      m_pad3A64[8]{};                // +0x3A64..+0x3A6B
    uint32     m_nLastUpdateTime = 0;         // +0x3A6C
    uint8      m_pad3A70[0x14]{};             // +0x3A70..+0x3A83

    static CPad* GetPadsArray()
    {
        return reinterpret_cast<CPad*>(Core::Rebase(0x10FB818));
    }

    static void InjectHooks();

    int32       __thiscall SetPadNumber(int32 padIndex);
    uint8       __thiscall CopySettings(const CPad* src);
    uint32      __thiscall TouchLastUpdateTime();
    uint32      __thiscall GetTimeSinceLastUpdate() const;
    void*       __thiscall GetJoystick() const;
    tPadValues* __thiscall GetFrontendAxisLR(uint8 allowMouse);
    tPadValues* __thiscall GetFrontendAxisUD(uint8 allowMouse);
    int32       __thiscall FindKeyMapping(int32 mapperIndex, int32 controlIndex, uint32* outKey, uint32* outDevice, int32 startIndex);

    static CPad* __cdecl GetPad();
    static CPad* __cdecl GetPadChecked(uint8 checkDisabled);
    static CPad* __cdecl GetPlayerPad();
    static CPad* __cdecl GetDebugPad();
};

VALIDATE_SIZE(CPad, 0x3A84);
VALIDATE_OFFSET(CPad, m_aValues, 0x2698);
VALIDATE_OFFSET(CPad, m_bIsUsingKeyboardForAim, 0x3289);
VALIDATE_OFFSET(CPad, m_bIsUsingKeyboardForHeli, 0x328D);
VALIDATE_OFFSET(CPad, m_nPadNumber, 0x32A4);
VALIDATE_OFFSET(CPad, m_mapper5, 0x32AC);
VALIDATE_OFFSET(CPad, m_nLastUpdateTime, 0x3A6C);
