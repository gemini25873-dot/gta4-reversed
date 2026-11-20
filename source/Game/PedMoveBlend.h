#pragma once

#include "Core/Vector.h"
#include "Core/Matrix.h"

struct CVector4;

class CPedMoveBlend
{
public:
    static void InjectHooks();

    // Slice 1: 0xA35050 .. 0xA35E32
    static bool __cdecl IsAnimGroup53_59(int32 groupId);                                                                 // 0xA35050
    void __thiscall ResetBlendFlagsAndSaveTargetHeading();                                                              // 0xA35070
    void __thiscall ApplyPedVelocityToTask();                                                                           // 0xA350D0
    void __thiscall GetEntityPositionByIndex(CVector4* outPos, int32 unused);                                           // 0xA35180
    static float __cdecl ComputeBlendSegment(const float* thresholds, float speed);                                     // 0xA351D0
    void* __thiscall HasActiveMoveAnimInSlot2(void* animPlayer);                                                        // 0xA35260
    bool __thiscall HasActiveLocomotionAnimInSlot2(void* animPlayer);                                                   // 0xA352C0
    bool __thiscall IsMovingOrTurning(void* pedInfo);                                                                   // 0xA35310
    void __thiscall ClampMoveSpeeds();                                                                                  // 0xA35370
    // 0xA353E0 (4 bytes < 5, skipped)
    void __thiscall SetUnk6C(float val);                                                                                // 0xA353F0
    static float __cdecl GetDefaultFloat_F359A4();                                                                      // 0xA35400
    CPedMoveBlend* __thiscall Ctor(void* arg);                                                                          // 0xA35410
    static void __cdecl StartIdleAnim(void* ped, uint32* animData);                                                     // 0xA35430
    static void __cdecl StartIdleAnimFromPed(uint8* ped);                                                               // 0xA35470
    bool __thiscall ProcessStopBlend(uint32* arg0, void* arg1, float arg2, bool arg3);                                  // 0xA354B0
    void* __thiscall ProcessStartTurnBlend(uint32* arg0);                                                               // 0xA35900

    // Slice 2: 0xA35E40 .. 0xA38608
    void __thiscall BuildBlendWeights(float* outWeights);                                                               // 0xA35E40
    bool __thiscall CanTriggerQuickTurn(uint8* arg0);                                                                   // 0xA35EE0
    void __thiscall UpdateSlopeAndObstacleCheck(uint8* arg0);                                                           // 0xA35F70
    void __thiscall UpdateHeadingAndSpeed(uint8* arg0);                                                                 // 0xA362E0
    void __thiscall UpdateHeadingOnly();                                                                                // 0xA36820
    bool __thiscall Blend4WayStrafe(uint32* arg0, float* arg1, float arg2, bool arg3);                                  // 0xA36AA0
    bool __thiscall Blend8WayStrafe(uint32* arg0, float* arg1, float arg2, bool arg3);                                  // 0xA37540
    bool __thiscall ProcessTurnAndMovement(uint32* arg0);                                                               // 0xA38020
    bool __thiscall ProcessJumpOrVaultBlend(uint32* arg0);                                                              // 0xA38330

    // Slice 3: 0xA38610 .. 0xA3B448
    CVector* __thiscall ComputeDesiredVelocity(CVector* outVel, float dt);                                              // 0xA38610
    CVector* __thiscall TransformMoveVector(CVector* outVec, const CMatrix* mat);                                       // 0xA38B20
    void __thiscall StopMovement(bool bForceStop);                                                                      // 0xA38DA0
    void __thiscall UpdateAimingBlend(float pitch, float yaw);                                                          // 0xA38EB0
    static void* __cdecl AllocFromPool();                                                                               // 0xA39120
    bool __thiscall UpdateStrafeMovement(uint8* arg0, void* arg1);                                                      // 0xA39140
    bool __thiscall UpdateNormalMovement(uint8* arg0);                                                                  // 0xA39EB0
    uint8 __thiscall ProcessMovement();                                                                                  // 0xA3A200
    void* __thiscall ScalarDeletingDtor(uint8 flags);                                                                   // 0xA3B0A0
    void __thiscall ClearSubArray();                                                                                    // 0xA3B0D0
    void __thiscall UpdateSubArraySlot4();                                                                              // 0xA3B120
    void __thiscall UpdateSubArraySlot8();                                                                              // 0xA3B150
    void __thiscall SetSubArrayEntry(int32 index, void* val);                                                           // 0xA3B180
    void __thiscall ProcessSubArraySlot10(void* arg);                                                                   // 0xA3B190
    void __thiscall DestroySubObject(int32* arg);                                                                       // 0xA3B1D0
    void __thiscall ClearSubArrayThunk();                                                                               // 0xA3B220
    void* __thiscall Alloc16AndStore(void* val);                                                                          // 0xA3B230
    static void __cdecl InvokeCallback_11DE7F8();                                                                       // 0xA3B250
    void* __thiscall CtorSubA3B260(void* arg);                                                                          // 0xA3B260
    static void __cdecl SetupDebugRenderStates();                                                                       // 0xA3B290
    void __thiscall RenderDebugOverlay(uint8* arg);                                                                     // 0xA3B2D0
};
