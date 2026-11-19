#pragma once

#include "Core/Vector.h"

class CEntity;
class CPed;

struct CVector4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};
VALIDATE_SIZE(CVector4, 0x10);

struct CPedIntelligenceSub10
{
    CVector4 m_vecMin;      // +0x00..+0x0F
    CVector4 m_vecMax;      // +0x10..+0x1F
    float    m_fRadius;     // +0x20
    void*    m_pEntity;     // +0x24
    uint32   m_nFlags;      // +0x28

    void* __thiscall InitSub0();                                                                                        // 0x9C2DD0
    void  __thiscall ClearSub0_24();                                                                                    // 0x9C2E10
    void __thiscall Dtor();                                                                                             // 0x9C3660
    bool __thiscall GetBounds(CVector4* outMin, CVector4* outMax, float* outRadius);                                    // 0x9C3670
    bool __thiscall IsPointInside(const CVector4* point, uint32 unused);                                                // 0x9C3930
    void __thiscall SetBox(const CVector4* minPos, const CVector4* maxPos, float width, void* entity, bool isLocal);    // 0x9C3CD0
    void __thiscall SetSphere(const CVector4* center, float radius, void* entity);                                      // 0x9C3DB0
    void __thiscall GetCenterAndRadiusSq(CVector4* outCenter, float* outRadiusSq);                                      // 0x9C3E20
};
VALIDATE_SIZE(CPedIntelligenceSub10, 0x2C);

class CPedIntelligence
{
public:
    static void InjectHooks();

    // Slice 1 (0x9C22C0..0x9C2FFF)
    void  __thiscall SetUnkD0(uint32 val);                                                      // 0x9C22C0
    void  __thiscall SetUnkD4(uint32 val);                                                      // 0x9C22D0
    void  __thiscall SetUnkDC(uint32 val);                                                      // 0x9C22E0
    void  __thiscall SetUnkE0(float val);                                                       // 0x9C22F0
    void  __thiscall SetUnkE4(float val);                                                       // 0x9C2310
    float __thiscall ComputeUnk9C2330(CEntity* target, float unusedArg);                        // 0x9C2330
    void* __thiscall UpdateAndGetUnk1C4();                                                      // 0x9C2450
    bool  __thiscall CheckUnk9C24A0(int32 id);                                                  // 0x9C24A0
    void  __thiscall ClearUnk25C();                                                             // 0x9C24B0
    bool  __thiscall CheckAndUpdateUnk25C();                                                    // 0x9C24E0
    void* __thiscall GetFromUnk25C();                                                           // 0x9C2520
    void  __thiscall SetUnk260(uint32 val);                                                     // 0x9C2540
    void  __thiscall SetTaskSlot0(void* task, uint8 arg2);                                      // 0x9C2580
    void  __thiscall SetTaskSlot1(void* task, uint8 arg2);                                      // 0x9C25A0
    void  __thiscall SetTaskSlot2(void* task, uint8 arg2);                                      // 0x9C25C0
    void* __thiscall FindTaskInPrimarySlots(int32 taskType);                                    // 0x9C25E0
    void* __thiscall GetTask431();                                                              // 0x9C2620
    void* __thiscall GetTask433();                                                              // 0x9C2630
    void* __thiscall GetPrimaryTaskIf1016();                                                    // 0x9C2640
    bool  __thiscall HasEventF1();                                                              // 0x9C2680
    bool  __thiscall HasEventFE();                                                              // 0x9C26C0
    bool  __thiscall CheckEvent41EAndVehicle(int32* outVal, bool* outFlag);                     // 0x9C2720
    bool  __thiscall HasEvent120Or11F();                                                        // 0x9C28B0
    void  __thiscall ClearUnk84AndSub20();                                                      // 0x9C2930
    void  __thiscall ProcessTasks9C2960(float arg);                                             // 0x9C2960
    void* __thiscall GetEvent2DEOrVehicle();                                                    // 0x9C29E0
    bool  __thiscall HasAnyCombatTask();                                                        // 0x9C2A60
    bool  __thiscall ProcessCombatTask(void* arg);                                              // 0x9C2AA0
    float __thiscall GetMoveSpeed();                                                            // 0x9C2B10
    void  __thiscall SetUnkF0F1(uint8 valF0, uint8 valF1);                                      // 0x9C2B80
    bool  __thiscall HasAnyUnk2D0();                                                            // 0x9C2BB0
    bool  __thiscall HasUnk2D0(void* ptr);                                                      // 0x9C2BD0
    void  __thiscall UpdateUnk2D0Nearby();                                                      // 0x9C2C00
    static void __cdecl ResetUnk152EBE0();                                                      // 0x9C2D00
    void  __thiscall AddSubUnk2C8(float delta);                                                 // 0x9C2D10
    void  __thiscall SetUnk2C8(float val);                                                      // 0x9C2D50
    void  __thiscall SetUnk2C4_2CC(bool flag, float val);                                       // 0x9C2D90
    void* __thiscall GetTask141_144Unk20(uint32* outVal);                                       // 0x9C2E30
    bool  __thiscall IsTargetInViewCone(const CVector* targetPos, float minDist, float maxScore, float maxDist, float minScore); // 0x9C2E80
    void* __thiscall FindTaskInAllSlots(int32 taskType);                                        // 0x9C2FD0

    // Slice 2 (0x9C3000..0x9C41FF)
    void* __thiscall GetActiveTask518();                                                        // 0x9C3040
    bool  __thiscall IsClimbingOrVaulting();                                                    // 0x9C3090
    void* __thiscall GetActiveTaskFE();                                                         // 0x9C30C0
    void* __thiscall GetActiveTask120();                                                        // 0x9C3110
    void  __thiscall ProcessFirst();                                                            // 0x9C3160
    void  __thiscall ProcessSecond();                                                           // 0x9C31D0
    void  __thiscall UpdateStuckTimer();                                                        // 0x9C3260
    void  __thiscall SetVec290(const CVector4* vec);                                            // 0x9C34D0
    void  __thiscall UpdatePosCheck2B0(const CVector4* vec);                                    // 0x9C3510
    int32 __thiscall GetAlertnessLevel();                                                       // 0x9C3640
    CPedIntelligence* __thiscall Ctor(void* ped);                                               // 0x9C3F40
    void  __thiscall Dtor();                                                                    // 0x9C4160
    void  __thiscall SetDecisionMakerType(int32 type);                                          // 0x9C41F0

    // Slice 3 (0x9C4200..0x9C515F)
    void* __thiscall GetOrCreateSub25C(bool createIfMissing);                                   // 0x9C4300
    void* __thiscall UpdateSub25C();                                                            // 0x9C4350
    void  __thiscall SetSub25CTarget(void* target, uint32 arg2);                                // 0x9C43A0
    void  __thiscall Reset(bool bResetEvents, bool bUpdateTasks);                               // 0x9C4400
    void  __thiscall Process();                                                                 // 0x9C4590
    void  __thiscall UpdateTaskMotionFlags();                                                   // 0x9C4910
    void  __thiscall ProcessTasks();                                                            // 0x9C4940
    static void __cdecl ShutdownPoolAndClearEvents();                                           // 0x9C4B10
    void  __thiscall AdjustSub2C8(float scale);                                                 // 0x9C4B60
    void* __thiscall CreateDefaultTask200(uint32 unused);                                       // 0x9C4C50
    bool  __thiscall CheckRelationshipBit(int32 relType, int32 otherGroup);                     // 0x9C4C70 (ECX = CPed*)
    void* __thiscall ScalarDeletingDtor(uint8 flags);                                           // 0x9C4D10
    bool  __thiscall IsAnySeenPedHatedInRange();                                                // 0x9C4D40
    void  __thiscall ProcessDeath(bool bCreateDeadTask);                                        // 0x9C4DF0
    bool  __thiscall IsFriendlyWithPed(void* otherPed);                                         // 0x9C4EF0
    bool  __thiscall IsPedThreatening(void* otherPed, bool bCheckDislike);                      // 0x9C4FC0
    static bool __cdecl ArePedsFriendly(void* pedA, void* pedB);                              // 0x9C5120
};
