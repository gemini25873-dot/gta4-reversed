#pragma once
#include "Game/Physical.h"

class CPlayerInfo;
class CVehicle;

class CPed : public CPhysical
{
public:
    uint8  m_bIsDead;                      // +0x210
    uint8  m_bIsInjured;                   // +0x211
    uint8  m_bIsCriticallyInjured;         // +0x212
    uint8  pad213;                         // +0x213
    float  m_fPedHealth;                   // +0x214
    uint8  m_nPedUnk218;                   // +0x218
    uint8  m_nPedUnk219;                   // +0x219
    uint8  pad21A[2];                      // +0x21A..+0x21B
    void*  m_pPedUnk21C;                   // +0x21C
    uint8  m_bPedUnk220;                   // +0x220
    uint8  pad221[3];                      // +0x221..+0x223
    void*  m_pPedUnk224;                   // +0x224
    CPlayerInfo* m_pPlayerInfo;            // +0x228
    void*  m_pPedUnk22C;                   // +0x22C
    uint8  pad230[8];                      // +0x230..+0x237
    uint8  m_pedSubObj238[0x28];           // +0x238..+0x25F
    uint32 m_nPedFlags;                    // +0x260
    uint8  pad264[8];                      // +0x264..+0x26B
    uint32 m_nPedFlags2;                   // +0x26C
    uint8  pad270[0x40];                   // +0x270..+0x2AF
    uint8  m_pedSubObj2B0[0x120];          // +0x2B0..+0x3CF
    uint8  m_pedSubObj3D0[0x1B0];          // +0x3D0..+0x57F
    uint8  m_pedSpeechAudio[0x210];        // +0x580..+0x78F
    uint8  m_audPlaceableTracker[0x30];    // +0x790..+0x7BF
    void*  m_pPedUnk7C0;                   // +0x7C0
    uint8  pad7C4[4];                      // +0x7C4..+0x7C7
    int32  m_nPedUnk7C8;                   // +0x7C8
    uint8  pad7CC[0x2B4];                  // +0x7CC..+0xA7F
    int32  m_nUnkA80;                      // +0xA80
    int32  m_nPedState;                    // +0xA84
    uint8  padA88[8];                      // +0xA88..+0xA8F
    void*  m_pPedIntelligence;             // +0xA90
    uint8  padA94[0xAC];                   // +0xA94..+0xB3F
    CVehicle* m_pMyVehicle;                // +0xB40
    uint32 m_nPedUnkB44[18];               // +0xB44..+0xB8B
    uint8  padB8C[8];                      // +0xB8C..+0xB93
    uint32 m_nPedUnkB94;                   // +0xB94
    uint32 m_nPedUnkB98;                   // +0xB98
    uint8  padB9C[0x24];                   // +0xB9C..+0xBBF
    uint8  m_pedSubObjBC0[0x1D4];          // +0xBC0..+0xD93
    int16  m_nPedUnkD94;                   // +0xD94
    uint8  padD96[2];                      // +0xD96..+0xD97
    uint8  m_pedSubObjD98[0x7C];           // +0xD98..+0xE13
    void*  m_pPedUnkE14;                   // +0xE14
    float  m_fPedUnkE18;                   // +0xE18
    float  m_fPedUnkE1C;                   // +0xE1C
    uint8  m_nPedUnkE20;                   // +0xE20
    uint8  padE21[0xF];                    // +0xE21..+0xE2F
    CVector m_vecPedUnkE30;                // +0xE30..+0xE3B
    uint8  padE3C[4];                      // +0xE3C..+0xE3F
    CVector m_vecPedUnkE40;                // +0xE40..+0xE4B
    uint8  padE4C[4];                      // +0xE4C..+0xE4F
    float  m_fPedUnkE50;                   // +0xE50
    int32  m_nPedUnkE54;                   // +0xE54
    int32  m_nPedUnkE58;                   // +0xE58
    int32  m_nPedUnkE5C;                   // +0xE5C
    uint8  padE60[0xA0];                   // +0xE60..+0xEFF

    static void InjectHooks();

    CPed* __thiscall Ctor(const uint8* initParams, int32 modelIndex);
    CPed* __thiscall DeletingDtor(uint8 flags);
    bool  __thiscall IsValidOrPlayer();
    void  __thiscall SetPedState(int32 state);
    int32 __thiscall UpdateHealthFlags(float health);
    int32 __thiscall InitHealth(float health);
    int32 __thiscall SetHealth(float health, int32 arg);
    int32 __thiscall AddHealth(float delta);

// --- Slice 1 ---
    // =========================================================================
    // Slice 1 (0x89C100 .. 0x89D8D0) - 58 functions analyzed, 23 hooked
    //
    // Discovered CPed field notes (offsets relative to CPed*):
    //   +0x270 (uint32)       : flags (bits 0x100/0x300, 0x400/0xC00) [0x89D5C0]
    //   +0x29C (uint32)       : flags (bit 0x4000000, 0x300000) [0x89C2C0, 0x89CA40]
    //   +0x2A0 (uint32)       : flags (bit 0x8000) [0x89CC30]
    //   +0x7B0 (float)        : speech/audio timer 1 (decayed by frame delta) [0x89D640]
    //   +0x7B4 (float)        : speech/audio timer 2 (decayed by frame delta) [0x89D640]
    //   +0x7C0 (void*)        : m_pPedUnk7C0 (back-pointer at +0x0C, vtbl+0x08) [0x89C350, 0x89C560]
    //   +0x7C4 (void*)        : m_pPedUnk7C4 (back-pointer at +0x0C, vtbl+0x08/+0x10, word +0x08) [0x89C2C0, 0x89C370, 0x89C560]
    //   +0x7C8 (int32)        : m_nPedUnk7C8 [0x89C390]
    //   +0x7CC (uint32)       : timestamp (game timer ms) when +0x7C8 was set [0x89C390]
    //   +0xA70 (uint8)        : state byte (checked == 2) [0x89C460]
    //   +0xA74 (int32/float)  : value + timestamped blend state [0x89D040, 0x89D0E0]
    //   +0xA78 (uint32)       : timer start ms for +0xA7C [0x89D040, 0x89D070]
    //   +0xA7C (uint8)        : active flag for 500ms timer [0x89D040, 0x89D070]
    //   +0xAB0 (float/int32)  : heading/angle field [0x89D0F0, 0x89D7C0]
    //   +0xAB4 (float/int32)  : target heading/angle [0x89D110, 0x89D130]
    //   +0xAB8 (float)        : speed/weight value (reset to 11.0f) [0x89D180, 0x89D8D0]
    //   +0xAC0 (uint32)       : flag/ptr checked in ragdoll/fall check [0x89D3A0]
    //   +0xB8C (int32)        : unk [0x89C8B0]
    //   +0xB90 (int32)        : counter/threshold (checked > 2, > 3) [0x89C7A0, 0x89CA40, 0x89CC30]
    //   +0xBA4 (int32)        : unk handle/index [0x89C8C0]
    //   +0xBAC (int32)        : speech/audio voice id [0x89CD90, 0x89CE30, 0x89CF00]
    //   +0xBB0 (int32)        : unk (reset to 438) [0x89CE20]
    //   +0xD30 (void*)        : entity/ped ref (cleared/set with ref list) [0x89C900, 0x89C980, 0x89C9D0]
    //   +0xD34 (void*)        : secondary entity/ped ref [0x89C980, 0x89C9D0]
    //   +0xD68 (void*[4])     : array of 4 entity pointers [0x89C200]
    // =========================================================================

    static void InjectHooks_Slice1();

    int32 __thiscall SetPedUnk7C8(int32 value, int32 unused);                    // 0x89C390
    int32 __thiscall UpdateUnk7C0And7C4(int32 arg);                              // 0x89C560
    static uint8 __cdecl IsAllowedAnimId(const int32* animId);                   // 0x89C5B0
    int32 __thiscall SetUnkB8C(int32 value);                                     // 0x89C8B0
    void  __thiscall ResetUnkBB0();                                              // 0x89CE20
    float __thiscall GetPedIntelligenceUnk08();                                  // 0x89CF80
    int32 __thiscall SetUnkVtbl52_322(uint8 value);                              // 0x89CFA0
    uint8 __thiscall GetUnkVtbl53_322();                                         // 0x89CFC0
    int32 __thiscall SetUnkVtbl52_324(int32 value);                              // 0x89CFE0
    float __thiscall GetUnkVtbl53_324();                                         // 0x89D000
    uint32 __thiscall StartUnkA74Timer(int32 value);                             // 0x89D040
    float __thiscall GetUnkA74TimerProgress();                                   // 0x89D070
    float __thiscall GetUnkA74Float();                                           // 0x89D0E0
    void  __thiscall SetUnkAB0(int32 value);                                     // 0x89D0F0
    void  __thiscall SetUnkAB4(int32 value);                                     // 0x89D110
    void  __thiscall SetHeadingToPoint2D(const float* targetPos);                // 0x89D130
    float __thiscall GetUnkAB8();                                                // 0x89D180
    int32 __thiscall CallIntelligenceVtbl9();                                    // 0x89D190
    int32 __thiscall CallIntelligenceVtbl10(int32 arg);                          // 0x89D1B0
    void  __thiscall DecayAudioTimers();                                         // 0x89D640
    uint8 __thiscall GetMaxFromPedUnk224(float* outMax);                         // 0x89D840
    CVector* __thiscall GetVecPedUnkE30IfSet();                                  // 0x89D8B0
    void  __thiscall ResetUnkAB8();                                              // 0x89D8D0

    // Unhooked in Slice 1 (35 functions):
    //   0x89C5A0: pure 5-byte thunk to CDynamicEntity/CEntity 0x9E6A80 (skipped)
    //   0x89D020, 0x89D030: 11-byte __stdcall wrappers to 0xB3C710 / 0xBC9870
    //   0x89C900, 0x89C980, 0x89C9D0: reference TlsIndex (address not in dump)
    //   0x89C100, 0x89C200, 0x89C2C0, 0x89C350, 0x89C370, 0x89C3B0, 0x89C460,
    //   0x89C520, 0x89C630, 0x89C780, 0x89C7A0, 0x89C810, 0x89C870, 0x89C890,
    //   0x89C8C0, 0x89CA40, 0x89CC30, 0x89CD90, 0x89CE30, 0x89CF00, 0x89D1C0,
    //   0x89D200, 0x89D260, 0x89D270, 0x89D360, 0x89D3A0, 0x89D5C0, 0x89D6C0,
    //   0x89D7C0: external callees require IDA disasm to verify __thiscall vs __cdecl
// --- Slice 2 ---
    // =========================================================================
    // Slice 2 (0x89D8F0 .. 0x89F530) - 57 functions analyzed, 21 hooked
    // =========================================================================
    // Newly discovered CPed fields / offsets from Slice 2:
    //   +0x270 (uint32)       : extra ped flags (bit 1 = 0x2, bit 20 = 0x100000)
    //   +0x7C4 (CEntity*)     : target/last vehicle or entity (inside pad7C4)
    //   +0x7CC (uint32)       : timestamp (inside pad7CC)
    //   +0x7E4 (float)        : anim/blend weight fallback (inside pad7CC)
    //   +0x8C4 (float)        : anim/blend weight (inside pad7CC)
    //   +0xA70 (uint8)        : ped state/mode byte (inside pad7CC)
    //   +0xAB0 (float)        : current heading/yaw in radians (inside padA94)
    //   +0xB38 (float)        : threshold float (inside padA94)
    //   +0xD58..+0xD64 (int32[4]) : sub-object BC0 fields
    //   +0xD68..+0xD77 (CEntity*[4]) : 4-slot entity ref array (inside m_pedSubObjBC0)
    //   +0xD78 (int32)        : sub-object BC0 handle/state
    //   +0xD80..+0xD8B        : sub-object BC0 cleared fields
    //   +0xE10 (CEntity*)     : entity reference (inside m_pedSubObjD98)

    static void InjectHooks_Slice2();

    int32        __thiscall RemoveFromVehicle(uint8 arg);                              // 0x89D8F0
    bool         __thiscall CanUseUnkB38(float val) const;                            // 0x89DCB0
    bool         __thiscall IsSpecialPedType() const;                                 // 0x89DE90
    int32        __thiscall SetUnkE10(CEntity* entity);                               // 0x89DFD0
    CEntity*     __thiscall GetUnkE10() const;                                        // 0x89E000
    int32        __thiscall ClearUnkD68Refs();                                        // 0x89E070
    int32        __thiscall AddUnkD68Ref(CEntity* entity);                            // 0x89E0A0
    int32        __thiscall RemoveUnkD68Ref(CEntity* entity);                         // 0x89E0E0
    bool         __thiscall IsPedType2NotModel() const;                               // 0x89E120
    uint8        __thiscall HasUnkTask1310() const;                                   // 0x89E150
    static int32 __cdecl    GetSpecialWeaponType(const CPed* ped);                    // 0x89E1B0
    int32        __thiscall GetModelUnkB0B1(int32* outA, int32* outB) const;           // 0x89E2D0
    bool         __thiscall CheckRandomSeedThreshold() const;                         // 0x89E450
    int32        __thiscall SetUnkD78(int32 val);                                     // 0x89E480
    int32        __thiscall SetUnkFlag20AndNotifyD68(uint8 flag);                     // 0x89E4B0
    double       __thiscall GetUnkAnimWeight() const;                                 // 0x89E570
    uint32*      __thiscall SetUnkB98(int32 value);                                   // 0x89E610
    uint32*      __thiscall AddUnkB98(int32 delta);                                   // 0x89E620
    int32        __thiscall SetPositionRaw(const float* pos, int32 arg2, int32 arg3); // 0x89E930
    const float* __thiscall GetBoneMatrix(int32 boneIndex);                           // 0x89E980
    static bool  __cdecl    CollectNearbyEntityCallback(float* entity, float* sphere);// 0x89F450
// --- Slice 3 ---
    // Slice 3 (0x89F7C0 .. 0x8A64F0, 58 functions analyzed)
    static void InjectHooks_Slice3();

    int32        __thiscall UpdateVehicularWindExposure();                           // 0x8A2CF0
    bool         __thiscall IsValidForSomething8A3390();                             // 0x8A3390
    const float* __thiscall SetCoverPositions8A3770(const float* posA, const float* posB); // 0x8A3770
    int32        __thiscall Unk8A3EC0();                                             // 0x8A3EC0
    bool         __thiscall Unk8A5030(CVector* outVec, int32 arg);                   // 0x8A5030
    float*       __thiscall GetLocalBoundBox8A55C0(float* outBox);                   // 0x8A55C0

    // Newly discovered CPed field notes from Slice 3 (0x89F7C0 .. 0x8A64F0):
    //   +0x264 (uint32) : secondary ped flags (& 0x8000, & 0x4000000)
    //   +0x288 (float)  : blend/state float (pad270)
    //   +0x294 (float)  : scale divisor (pad270)
    //   +0x298 (uint8)  : blend counter (pad270)
    //   +0x29C (uint32) : flags (& 0x20000000, & 0x4, |= 0x1000000) (pad270)
    //   +0x29F (uint8)  : bone cache valid flag (& 1) (pad270)
    //   +0x2A0 (uint32) : flags (& 0x20, & 0x40, & 0x80, & 0x100, & 0x200, & 0x400, & 0x8000) (pad270)
    //   +0x2EC (struct[10], stride 12) : weapon/item slots (m_pedSubObj2B0)
    //   +0x44C (int32)  : state/type checked == 15 in 0x8A2CF0 (m_pedSubObj3D0)
    //   +0x450 (int32)  : state/type checked == 15 in 0x8A2CF0 (m_pedSubObj3D0)
    //   +0x7C4 (CEntity*) : attached/mount/ragdoll entity (pad7C4)
    //   +0xA70 (uint8)  : ped sub-type / category (1, 2, etc.) (pad7CC)
    //   +0xAB0 (float)  : cached heading/angle (padA94)
    //   +0xAB4 (float)  : cached target heading/angle (padA94)
    //   +0xAC0 (CEntity*) : ground/standing entity ref-counted via 0x9E6550/0x9E6580 (padA94)
    //   +0xAD0 (float[4]) : ground contact position (padA94)
    //   +0xAE0 (CVector)  : previous ground/ped position (padA94)
    //   +0xB10 (float[4]) : ground contact normal (padA94)
    //   +0xB20 (float)  : ground Z (padA94)
    //   +0xB24 (float)  : ground delta Z (padA94)
    //   +0xB30 (float)  : breath/idle timer (padA94)
    //   +0xB34 (float)  : breath/idle interval (padA94)
    //   +0xB8C (int32)  : override anim ID (-1 = none) (padB8C)
    //   +0xB90 (int32)  : move state (3/4 = sprint/run) (padB8C)
    //   +0xBA8 (int32)  : motion state (69/70/71) (padB9C)
    //   +0xBAC (int32)  : anim group ID (padB9C)
    //   +0xBB0 (int32)  : current anim ID (padB9C)
    //   +0xC08 (float)  : facial/audio float (m_pedSubObjBC0)
    //   +0xD00 (uint8)  : disable ground snap flag (m_pedSubObjBC0)
    //   +0xD38 (float)  : shock/damage timer (m_pedSubObjBC0)
    //   +0xD3C (uint32) : timestamp updated in 0x8A56D0 (m_pedSubObjBC0)
    //   +0xD60 (int32)  : vehicle enter/exit seat data (m_pedSubObjD98)
    //   +0xD64 (int32)  : vehicle seat index (-1 = none) (m_pedSubObjD98)
    //   +0xE08 (float)  : lean/IK float X (m_pedSubObjD98)
    //   +0xE0C (float)  : lean/IK float Y (m_pedSubObjD98)
    //   +0xEA8 (void*)  : render/shader data ptr (padE60)
    //
    // Non-CPed / unhooked functions in 0x89F7C0 .. 0x8A64F0 (52 functions):
    //   - Non-CPed: 0x8A13B0 (static ground probe helper), 0x8A5190 (CPedSaveStructure::Save),
    //     0x8A53C0 (global save helper), 0x8A5690 (non-CPed vtable-less struct).
    //   - Unhooked (external callees require disasm to verify __thiscall/__usercall ECX):
    //     0x89F7C0, 0x89F9C0, 0x89FE30, 0x89FFB0, 0x8A02D0, 0x8A0540, 0x8A0680, 0x8A0720,
    //     0x8A07E0, 0x8A0850, 0x8A08A0, 0x8A09E0, 0x8A15E0, 0x8A2050, 0x8A23C0, 0x8A28B0,
    //     0x8A2980, 0x8A2F00, 0x8A3100, 0x8A3250, 0x8A33F0, 0x8A3480, 0x8A3680, 0x8A3850,
    //     0x8A3990, 0x8A39E0, 0x8A3B30, 0x8A3ED0, 0x8A4020, 0x8A4730, 0x8A4AB0, 0x8A4BE0,
    //     0x8A4C40, 0x8A4D00, 0x8A4E30, 0x8A4E70, 0x8A4EB0, 0x8A4F50, 0x8A5060, 0x8A54B0,
    //     0x8A56D0, 0x8A5700, 0x8A5740, 0x8A5780, 0x8A57E0, 0x8A5830, 0x8A63A0, 0x8A64F0.
// --- Slice 4 ---
// Slice 4 (0x8A6530 .. 0x8ACCC0): 55 functions total
// - 7 already hooked in Ped.cpp (skipped): 0x8A9030, 0x8A92C0, 0x8ABF10, 0x8AC150, 0x8AC180, 0x8AC1B0, 0x8ACAC0
// - 48 analyzed from Hex-Rays dump:
//   * 10 CPed methods hooked in InjectHooks_Slice4():
//     - 0x8A6E00: IsEntityWithinRange
//     - 0x8A7710: SetVoiceGroup
//     - 0x8A7740: InitVoiceByPedType
//     - 0x8AA480: ClearWeaponObject
//     - 0x8AAC30: ResetRagdollIfActiveSimple
//     - 0x8AAC50: ResetRagdollIfActive
//     - 0x8AACD0: UpdateRagdollTimeout
//     - 0x8AC500: SetModelIndexDefault
//     - 0x8AC520: ProcessRagdollOrPhysical
//     - 0x8AC9D0: AttachWeaponOrProp
//   * 25 CPed / static-helper functions analyzed & documented (deferred due to unverified external __thiscall/__usercall callee conventions or unknown vtable addresses):
//     - 0x8A6580 (__stdcall CreateWanderTask), 0x8A65E0 (__stdcall CreateDefaultTask),
//       0x8A6780 (InitPhysics), 0x8A6910 (ResetRagdollToAnim), 0x8A6E40 (__cdecl SelectAnimOrVoice),
//       0x8A70C0 (UpdateSpeechState), 0x8A71D0 (TriggerSpeech), 0x8A74E0 (ProcessConversation / LookAtSpeaker),
//       0x8A7890 (ProcessHeading), 0x8A7D70 (ProcessControlPhysics), 0x8A8340 (PreRenderPed),
//       0x8A8CC0 (Teleport), 0x8A9070 (OnDetach), 0x8A90F0 (ComputeVehicleAttachWeights),
//       0x8A9590 (ProcessVehicleCollision), 0x8AA500 (IsPerformingUnkAction), 0x8AA570 (InitPedFields),
//       0x8AAA80 (SetModelIndex), 0x8AAD00 (__stdcall SetTaskBit96_A), 0x8AAD60 (__stdcall SetTaskBit96_B),
//       0x8AADC0 (ProcessControl), 0x8AB200 (ProcessAttachedToVehicle), 0x8AB720 (__cdecl LoadPedFromSave),
//       0x8AB880 (__cdecl UpdatePedBuckets), 0x8AB9C0 (ComputeWeaponBoneTransform),
//       0x8AC1E0 (Dtor), 0x8AC540 (ProcessAttachment)
//   * 13 non-CPed functions skipped:
//     - 0x8A6530 (m_pPedUnk224 / CPedIntelligence method)
//     - 0x8A6660 (task/intelligence method)
//     - 0x8A6750 (global ped subsystem shutdown)
//     - 0x8A9310 (CPedSaveData::ApplyToPed, `this` is 192-byte save struct)
//     - 0x8AA540 (CTaskSimpleMoveDoNothing::DeletingDtor)
//     - 0x8ACAF0, 0x8ACB10 (dword_12301AC linked-list node ctor & updater)
//     - 0x8ACB40, 0x8ACBD0, 0x8ACC90, 0x8ACCC0 (global game/subsystem init & shutdown)
//
// Discovered CPed field offsets in Slice 4:
//   +0x219 (m_nPedUnk219)  : uint8  m_bIsPlayer (checked throughout Slice 4)
//   +0x21C (m_pPedUnk21C)  : +0x12C = int32 m_nPedType (used in 0x8A7740)
//   +0x224 (m_pPedUnk224)  : CPedIntelligence/task manager (+0x44 = 5 task slots)
//   +0x264 (pad264[0])     : uint32 flags (bit 0x8000 = weapon/prop attached; bit 0x40, 0x20000)
//   +0x7BC                 : void*  ragdoll/frag helper
//   +0x7C0 (m_pPedUnk7C0)  : void*  animated phInst
//   +0x7C4                 : void*  ragdoll phInst
//   +0x7C8 (m_nPedUnk7C8)  : int32  ragdoll state (1, 2, 3, 6)
//   +0x7CC                 : uint32 ragdoll state timestamp (ms)
//   +0xAB0                 : float  m_fCurrentHeading
//   +0xAB4                 : float  m_fDesiredHeading
//   +0xBB4                 : int32  m_nVoiceGroup
//   +0xD30                 : CPed*  conversation speaker partner
//   +0xD58..+0xD64         : attachment angle/bone state

static void InjectHooks_Slice4();

bool   __thiscall IsEntityWithinRange(const CEntity* entity, float range);
int32  __thiscall SetVoiceGroup(int32 voiceGroup);
int32  __thiscall InitVoiceByPedType();
int32  __thiscall ClearWeaponObject(bool bKeepUnk);
int32  __thiscall ResetRagdollIfActiveSimple();
int32  __thiscall ResetRagdollIfActive();
uint32 __thiscall UpdateRagdollTimeout();
int32  __thiscall SetModelIndexDefault(int32 modelIndex);
int32  __thiscall ProcessRagdollOrPhysical();
bool   __thiscall AttachWeaponOrProp(int32 arg);
// --- Slice 5 ---
    // =========================================================================
    // Slice 5 (0x89C100 .. 0x89DC70) - 41 functions analyzed, 36 hooked, 5 skipped
    // =========================================================================

    static void InjectHooks_Slice5();

    void   __thiscall ResetPhysicsCollider89C100();
    int32  __thiscall CollectEntities89C200(int32 outArray, int32* outCount, int32 maxCount, int32 arg5);
    void   __thiscall UpdateUnk7C4Matrix89C2C0(int8 arg);
    int32  __thiscall SetPedUnk7C0(void* ptr);
    int32  __thiscall SetPedUnk7C4(void* ptr);
    int32  __thiscall UpdateModelPedFlags89C3B0();
    int32  __thiscall UpdateState89C460();
    int8   __thiscall ResetSubObj89C520();
    static int8 __cdecl CanUseAnim89C630(int32 a1, const int32* animId, const CPed* ped);
    int32  __thiscall GetIntelligenceUnk3C();
    bool   __thiscall IsWeaponAllowed89C7A0();
    static int8 __cdecl HasActiveTask89C810();
    static bool __cdecl CheckTaskFlag80000();
    static bool __cdecl CheckTaskFlag20000000();
    int32  __thiscall GetUnkBA4State();
    bool   __thiscall CheckCondition89CA40(int32 a2);
    int8   __thiscall CheckCondition89CC30();
    void   __thiscall PlayAnim89CD90(int32 a2, uint32 a3, int32 a4, int32 a5);
    void   __thiscall PlayAngryAnim89CE30(uint32 a2);
    void   __thiscall PlayAnim445_89CF00(uint32 a2);
    static int32 __stdcall Helper89D020(int32 a1);
    static int32 __stdcall Helper89D030(int8 a1);
    int32  __thiscall SetTaskTimer89D1C0(float a2);
    int32  __thiscall ClearZeroTasks89D200();
    static int32 __cdecl Helper89D260();
    void   __thiscall ApplyMovement89D270(float a2);
    int32  __thiscall UpdateUnk137_89D360();
    int8   __thiscall ProcessFall89D3A0(int8 a2);
    int32  __thiscall SetFlags156_89D5C0(int8 a2, int32 a3);
    int32  __thiscall ProcessSpeech89D6C0();
    int32  __thiscall SetTask89D950(int32 a2, int16 a3, int32 a4, int32 a5, int32 a6, int32 a7);
    int32  __thiscall SetTask89D9C0(int32 a2, int32 a3, int32 a4, int32 a5);
    int32  __thiscall SetTaskWithVehicleCheck89DA20(int32 a2, int32 a3, int32 a4, int32 a5);
    int32  __thiscall GetModelProp89DB00(uint8 a2);
    void   __thiscall UpdateAudio89DB40();
    int32  __thiscall ClearUnkD78_89DC70();

    // Skipped in Slice 5 (5 functions):
    //   0x89C5A0: pure 5-byte thunk to 0x9E6A80
    //   0x89C900, 0x89C980, 0x89C9D0: reference TlsIndex / NtCurrentTeb()->ThreadLocalStoragePointer (TlsIndex address not in dump)
    //   0x89D7C0: calls CRT __m128 atan2 (0xD123AC) with custom-register arguments dropped by decompiler
// --- Slice 6 ---
    // =========================================================================
    // Slice 6 (0x89DCE0 .. 0x8A08A0) - 41 functions analyzed, 31 hooked
    //
    // Discovered CPed field notes (offsets relative to CPed*):
    //   +0x266 (int16)        : flags (bit 1 checked in 0x89E1F0)
    //   +0x2A0 (uint32)       : flags (bit 0x8000, bit 0x100/0x200/0x400/0x80/0x40/0x20)
    //   +0x7C0 (void*)        : m_pPedUnk7C0
    //   +0x7C4 (void*)        : m_pPedUnk7C4
    //   +0xA70 (uint8)        : state byte (2, 3) [0x89E1F0, 0x89EC20, 0x89F9C0]
    //   +0xB90 (int32)        : state/counter [0x89FE30, 0x8A07E0, 0x8A08A0]
    //   +0xB94 (uint32)       : m_nPedUnkB94 [0x89E660]
    //   +0xBAC (int32)        : speech/audio id [0x89F9C0, 0x89FFB0, 0x8A0680]
    //   +0xBB0 (int32)        : speech/audio sub-id [0x89FFB0]
    //   +0xBE0..+0xBEC (16B)  : vector/color [0x89FE30]
    //   +0xD38 (float)        : anim event timestamp [0x8A0720]
    // =========================================================================

    static void InjectHooks_Slice6();

    int32 __thiscall SyncVehicleFlags();                                         // 0x89DCE0
    static int32 __stdcall GetDominantAxisDir(const float* mat);                 // 0x89DD20
    int32 __thiscall CallUnkA66ED0();                                            // 0x89DD80
    int32 __thiscall CountNearbyPassengers(float radius);                        // 0x89DD90
    static int32 __cdecl RegisterPedGridEntry(uint8* entry);                     // 0x89DED0
    static int32* __cdecl UnregisterPedGridEntry(uint8* entry);                  // 0x89DF30
    static int32 __cdecl AdvancePedGridTick();                                   // 0x89DF90
    int8  __thiscall CheckVehicleUnk9C5D60();                                    // 0x89DFB0
    static bool __stdcall TryTriggerPedAction(int32 arg1, float chance, int8 arg3, int8 arg4); // 0x89E010
    bool  __thiscall CanUseUnkE1F0();                                            // 0x89E1F0
    int32 __thiscall GetUnk9E27D0Minus1();                                       // 0x89E250
    int32 __thiscall GetVehicleSeatOrTypeState();                                // 0x89E300
    int8  __thiscall UpdateVehicleSeatState();                                   // 0x89E3E0
    static int32 __stdcall CallUnk447610(float value);                           // 0x89E640
    int32 __thiscall AddToUnkB94AndCall(float delta);                            // 0x89E660
    static int32 __cdecl ShutdownPedSubsystems();                                // 0x89EBF0
    int8  __thiscall SetUnkA70State(int8 state);                                 // 0x89EC20
    void  __thiscall OnRagdollModelCheck(int32 inst, int32 step);                // 0x89EFE0
    void  __thiscall UpdateRagdollDistanceCheck(int8 forceReset);                // 0x89F050
    int8  __thiscall FindNearbyCoverPoint();                                     // 0x89F530
    bool  __thiscall HasLineOfSightToEntity(const CEntity* target, int8 skipAngleAndRangeCheck); // 0x89F7C0
    int32 __thiscall ProcessPedSpeechAndState();                                 // 0x89F9C0
    int32 __thiscall UpdatePedFlagsAndAudio();                                   // 0x89FE30
    int32 __thiscall UpdatePedMoodAudio();                                       // 0x89FFB0
    void  __thiscall ProcessBoneAttachmentEvents();                              // 0x8A02D0
    int32 __thiscall ProcessAnimEventTrack(const uint8* track, float timeSec, int32 unused); // 0x8A0540
    void  __thiscall TriggerShockedSpeech();                                     // 0x8A0680
    void  __thiscall UpdateAnimEvents();                                         // 0x8A0720
    int32 __thiscall SetRagdollUnkB90(int32 value);                              // 0x8A07E0
    int32 __thiscall UpdateRagdollUnkBC9B00(float value);                        // 0x8A0850
    int8  __thiscall UpdateMovementAndSpeech();                                  // 0x8A08A0

    // Skipped in Slice 6 (10 functions):
    //   0x89DEB0: non-CPed (__thiscall on 12-byte ped grid entry struct)
    //   0x89E270, 0x89F140: unsafe stack-allocated CEvent (uninitialized v5[36]/v13[36]; 0xA9CFB0 takes hidden ecx)
    //   0x89E690: non-CPed (this[2] - this[5] on small helper struct)
    //   0x89E6A0: 8-byte __stdcall wrapper to CTaskSimpleMoveDoNothing::DeletingDtor (0x8AA540)
    //   0x89E6B0: non-CPed (CMatrix orthonormalization, `this` is float* matrix)
    //   0x89EA80, 0x89EAB0: non-CPed (CPedIntelligence / m_pPedUnk224 methods)
    //   0x89ED00: custom-register CRT SSE2 calls (0xD112E3, 0xD1112B taking xmm0) & stack-matrix ecx call (0x441ED0)
    //   0x89F390: lost x87 st7 return assignment from 0xA4E780 in decompilation
// --- Slice 7 ---
// Slice 7 (0x8A09E0 .. 0x8A64F0): 41 functions total
// - 28 functions hooked in InjectHooks_Slice7() (27 CPed methods + 1 CPed static helper)
// - 13 functions skipped:
//   * 10 CPed methods deferred due to unsafe Hex-Rays decompilation / hidden sret / RTTI / unknown vtables:
//     - 0x8A09E0: uninitialized v77..v79 (missed sret/usercall on sub_441ED0/sub_534930), sub_42F5D0 missing ecx
//     - 0x8A2980: uninitialized v21..v23 (missed CMatrix sret on sub_42F680/sub_513710)
//     - 0x8A3480: sub_49F460/sub_49F400 likely __thiscall missing ecx, uninitialized upper bits of v10
//     - 0x8A3680: uses __RTDynamicCast (sub_D0F75A) with unknown RTTI type descriptor addresses
//     - 0x8A4730: missed sret on sub_441ED0 (v21..v23 uninitialized) and implicit xmm0 flow (sub_D123AC/sub_D112E3/sub_D1112B)
//     - 0x8A4AB0: uninitialized v11/v12 before sub_4A12F0 (missed sret on sub_5BE050)
//     - 0x8A4EB0: hidden sret/a2 arg to sub_441ED0
//     - 0x8A4F50: hidden sret/a2 arg to sub_441ED0/sub_513430
//     - 0x8A63A0: uninitialized v11/v12 before sub_50DC90 (missed sret on sub_9ED3C0/sub_50DC90)
//     - 0x8A64F0: references unknown CTaskSimpleMoveDoNothing::vftable addresses
//   * 3 non-CPed functions skipped:
//     - 0x8A5190: CPedSaveStructure::ExtractFromPed(CPed*)
//     - 0x8A53C0: CPedSaveStructure global save helper
//     - 0x8A5690: non-CPed handle/struct (*this != -1, no vtable)

static void InjectHooks_Slice7();

static bool __cdecl ProbeGroundCircle(float* origin, float* outMinZ, float* outMaxZ, float radius, float steps, float depth, int32* outNormal, uint32* outMaterial);

void   __thiscall UpdateGroundCollision(int32 entity);
int32  __thiscall ProcessContactPoints(int32 contacts, int32 phInst, int32 unused, int32 count);
int32  __thiscall ApplyGroundSlopeVelocity();
void   __thiscall ProcessPhysicsStep();
int32  __thiscall UpdateSpeechAndAudio();
void   __thiscall TriggerFootstepAudio(int32 isLeftFoot);
int32  __thiscall UpdatePositionOrHeadLook();
bool   __thiscall CanConvertToDummy(bool allowFlag4, bool checkStreamed);
int32  __thiscall PreRenderPedInternal();
void*  __thiscall PostRenderPedInternal();
void   __thiscall SyncRagdollToMatrix();
int32  __thiscall RenderPed(int32 pass, int32 arg3, uint8 alpha, int32 arg5);
bool   __thiscall TestLineOfSightToEntity(int32 entityA, int32 entityB);
bool   __thiscall FindVehicleExitPosition(int32 exitType, int32 refMatrix, float* outMatrix);
int32* __thiscall GetBonePosPtr(int32 boneId);
bool   __thiscall GetBonePosition(void* outPos, int32 boneId);
bool   __thiscall GetBoneMatrixOut(float* outMatrix, int32 boneId);
bool   __thiscall GetBonePositionUpdated(void* outPos, int32 boneId, bool forceUpdate);
bool   __thiscall GetBoneMatrixUpdated(float* outMatrix, int32 boneId, bool forceUpdate);
void*  __thiscall GetHeadOrCenterPosition(void* outPos);
int32  __thiscall GetClimbOrStepHeight();
int32  __thiscall UpdateTaskTimerD3C(int8 flag);
bool   __thiscall HasHeldWeaponObject();
void   __thiscall SetWeaponVariation(int32 variation);
int32  __thiscall GetWeaponVariation(int32 slot);
int32  __thiscall SetWeaponDrawable(int32 drawable, int32 texture);
bool   __thiscall ProcessAnimMoveAndTurn(int32 animData, float* outWeight1, float* outWeight2, float* outWeight3, float* outWeight4, int32 outDeltaPos, float* outHeading, uint8* outFlag1, uint8* outFlag2, uint8* outFlag3, uint8* outFlag4);
// --- Slice 8 ---
// Slice 8 (cped_rem_slice4, 0x8A6530 .. 0x8ACCC0): 38 remaining functions analyzed
// - 6 CPed methods / static helpers hooked in InjectHooks_Slice8():
//   - 0x8A6750: ShutdownSubsystems (static __cdecl)
//   - 0x8A6E40: SelectAnimOrVoice (static __cdecl)
//   - 0x8A70C0: UpdateSpeechState (__thiscall)
//   - 0x8AA500: IsPerformingUnkAction (__thiscall)
//   - 0x8AAD00: SetTaskBit96_A (static __stdcall)
//   - 0x8AAD60: SetTaskBit96_B (static __stdcall)
//
// - 21 CPed / static-helper functions skipped (unsafe patterns):
//   - 0x8A6580 (CreateWanderTask: 0xA18C70 omits implicit `this` register + CTaskComplexWanderStandard::vftable)
//   - 0x8A65E0 (CreateDefaultTask: 0x9B8120/0x9B7AE0 omit implicit `this` register from 0x439CB0)
//   - 0x8A6780 (InitPhysics: 0x686A20/0x686AD0/0x69AD80/0x6AC410 omit implicit `this` register)
//   - 0x8A6910 (ResetRagdollToAnim: SSE2 math 0xD123AC/0xD112E3/0xD1112B omit xmm args; RAGE callees omit ecx)
//   - 0x8A71D0 (TriggerSpeech: 0x9EFF90 omits implicit `this`/object register)
//   - 0x8A74E0 (ProcessConversation: 0xA5DC20/0xA5DBF0/0x959CC0 omit implicit `this` register)
//   - 0x8A7890 (ProcessHeading: SSE2 math 0xD112E3/0xD1112B omit xmm0 argument)
//   - 0x8A7D70 (ProcessControlPhysics: 0x5D3260/0x5D34D0/0x5D32F0/0x5D36C0/0x5D3390 omit implicit `v46`/`ecx` register)
//   - 0x8A8340 (PreRenderPed: 0x688F70/0x6A3960/0x6A4470 omit implicit `ecx` register)
//   - 0x8A8CC0 (Teleport: __userpurge with custom ebx/edi/esi register parameters)
//   - 0x8A9070 (OnDetach: 0xA4A320 and 0x686AD0 omit implicit `this`/`ecx` register)
//   - 0x8A90F0 (ComputeVehicleAttachWeights: 0x9EC300 omits implicit `this`/`ecx` register)
//   - 0x8A9590 (ProcessVehicleCollision: 0xA47750/0xA47B80/0xAA80D0 omit implicit `this`/`ecx` register)
//   - 0x8AA570 (InitPedFields: 0xA28B30/0x9E58B0/0x9E5E20 omit implicit `this`/`ecx` + CTaskSimpleMoveDoNothing::vftable)
//   - 0x8AAA80 (SetModelIndex: 0x9E7C20/0x9297D0/0x9EFDC0/0xA4C470/0xA4D6E0 omit implicit `this`/`ecx` register)
//   - 0x8AADC0 (ProcessControl: 0x9E6BD0 omits implicit `this`/`ecx` register)
//   - 0x8AB200 (ProcessAttachedToVehicle: SSE2 math 0xD112E3/0xD1112B omit xmm0; 0xA47750 omits ecx)
//   - 0x8AB720 (LoadPedFromSave: uninitialized stack args v7/v8 passed to callees + off_75EE00)
//   - 0x8AB880 (UpdatePedBuckets: 0xABDDF0 omits implicit `this` = &dword_12300D8 in ecx)
//   - 0x8AB9C0 (ComputeWeaponBoneTransform: 0xA4C8F0/0x513AA0/0x513710/0x506DB0 omit implicit `this`/`ecx` register)
//   - 0x8AC1E0 (Dtor: requires CPed::vftable / audPlaceableTracker::vftable + 0xCF0E90 omits ecx)
//   - 0x8AC540 (ProcessAttachment: SSE2 math omits xmm args; 0x5BE050/0x513430/0xA47750 omit ecx)
//
// - 11 non-CPed functions skipped:
//   - 0x8A6530 (CPedIntelligence / m_pPedUnk224 method)
//   - 0x8A6660 (task/intelligence method)
//   - 0x8A9310 (CPedSaveStructure::ApplyToPed)
//   - 0x8AA540 (CTaskSimpleMoveDoNothing::DeletingDtor)
//   - 0x8ACAF0, 0x8ACB10 (dword_12301AC linked-list node ctor & updater)
//   - 0x8ACB40, 0x8ACBD0, 0x8ACC90, 0x8ACCC0 (global game/subsystem init & shutdown)
// (Note: 22 unsafe + 10 non-CPed + 6 hooked = 38 total; 0x8AC540 counted in unsafe list above.)

static void InjectHooks_Slice8();

static int32 __cdecl ShutdownSubsystems();
static int32 __cdecl SelectAnimOrVoice(int32 a1, int32* a2, int32 a3, int16* a4, int32 a5, int8 a6, int8 a7, int8 a8);
void  __thiscall UpdateSpeechState(int8 a2, int32 a3);
bool  __thiscall IsPerformingUnkAction();
static int32 __stdcall SetTaskBit96_A(int32 a1, int32 bit);
static int32 __stdcall SetTaskBit96_B(int32 a1, int32 bit);
};

VALIDATE_SIZE(CPed, 0xF00);
VALIDATE_OFFSET(CPed, m_bIsDead, 0x210);
VALIDATE_OFFSET(CPed, m_fPedHealth, 0x214);
VALIDATE_OFFSET(CPed, m_pPedUnk21C, 0x21C);
VALIDATE_OFFSET(CPed, m_nPedFlags, 0x260);
VALIDATE_OFFSET(CPed, m_nPedFlags2, 0x26C);
VALIDATE_OFFSET(CPed, m_pedSpeechAudio, 0x580);
VALIDATE_OFFSET(CPed, m_audPlaceableTracker, 0x790);
VALIDATE_OFFSET(CPed, m_nUnkA80, 0xA80);
VALIDATE_OFFSET(CPed, m_nPedState, 0xA84);
VALIDATE_OFFSET(CPed, m_pPedIntelligence, 0xA90);
VALIDATE_OFFSET(CPed, m_pPlayerInfo, 0x228);
VALIDATE_OFFSET(CPed, m_pMyVehicle, 0xB40);
VALIDATE_OFFSET(CPed, m_nPedUnkD94, 0xD94);
VALIDATE_OFFSET(CPed, m_fPedUnkE50, 0xE50);
