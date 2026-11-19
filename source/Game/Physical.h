#pragma once
#include "Game/DynamicEntity.h"

class CPhysical : public CDynamicEntity
{
public:
    float  m_fHealth = 100.0f;           // +0x110 (this[68])
    uint32 m_nPhysPad114 = 0;            // +0x114
    uint32 m_nPhysFlags118 = 0;          // +0x118 (this[70])
    uint16 m_nPhysUnk11C = 0;            // +0x11C
    uint16 m_nPhysPad11E = 0;            // +0x11E
    float  m_fPhysUnk120 = 1.1f;         // +0x120 (this[72])
    uint8  m_physAnimArray[0x28]{};      // +0x124..+0x14B (this[73..82])
    uint8  m_nPhysUnk14C = 0;            // +0x14C (byte +336 is +0x150)
    uint8  m_nPhysPad14D[3]{};           // +0x14D..+0x14F
    uint8  m_nPhysUnk150 = 0;            // +0x150
    uint8  m_nPhysPad151[3]{};           // +0x151..+0x153
    CVector m_vecMoveSpeed{};            // +0x154..+0x15F (this[85..87])
    CVector m_vecTurnSpeed{};            // +0x160..+0x16B (this[88..90])
    void*  m_pPhysUnk16C = nullptr;      // +0x16C (this[91])
    uint32 m_nPhysUnk170 = 0;            // +0x170 (this[92])
    uint32 m_nPhysUnk174 = 0;            // +0x174 (this[93])
    uint32 m_nPhysPad178 = 0;            // +0x178
    uint32 m_nPhysPad17C = 0;            // +0x17C
    CVector m_vecPhysUnk180{};           // +0x180..+0x18B (this[96..98])
    uint32 m_nPhysPad18C = 0;            // +0x18C
    CVector m_vecPhysUnk190{};           // +0x190..+0x19B (this[100..102])
    uint32 m_nPhysPad19C = 0;            // +0x19C
    CVector m_vecPhysUnk1A0{};           // +0x1A0..+0x1AB (this[104..106])
    uint32 m_nPhysUnk1AC = 0;            // +0x1AC (this[107])
    uint32 m_nPhysUnk1B0 = 0;            // +0x1B0 (this[108])
    uint32 m_nPhysUnk1B4 = 0;            // +0x1B4 (this[109])
    uint32 m_nPhysUnk1B8 = 0;            // +0x1B8 (this[110])
    void*  m_pAttachedTo = nullptr;      // +0x1BC (this[111])
    CVector m_vecAttachOffset{};         // +0x1C0..+0x1CB (this[112..114])
    uint32 m_nPhysPad1CC = 0;            // +0x1CC
    float  m_vecAttachQuat[4]{0,0,0,1};  // +0x1D0..+0x1DF (this[116..119])
    uint16 m_nPhysUnk1E0 = 0;            // +0x1E0
    uint16 m_nPhysUnk1E2 = 0;            // +0x1E2
    void*  m_pPhysUnk1E4 = nullptr;      // +0x1E4 (this[121])
    uint32 m_nPhysUnk1E8 = 0;            // +0x1E8 (this[122])
    int8   m_nPhysUnk1EC = -1;           // +0x1EC (byte +492)
    uint8  m_nPhysPad1ED[3]{};           // +0x1ED..+0x1EF
    float  m_fMaxHealth = 100.0f;        // +0x1F0 (this[124]; CPhysical health)
    uint8  m_nPhysUnk1F4 = 0;            // +0x1F4 (byte +500)
    uint8  m_nPhysPad1F5[3]{};           // +0x1F5..+0x1F7
    void*  m_pPhysUnk1F8 = nullptr;      // +0x1F8 (this[126])
    uint32 m_nPhysPad1FC = 0;            // +0x1FC
    CVector m_vecPhysUnk200{};           // +0x200..+0x20B (this[128..130])
    uint32 m_nPhysPad20C = 0;            // +0x20C

    static void InjectHooks();

    CPhysical* __thiscall Ctor();
    int32 __thiscall Dtor();
    CPhysical* __thiscall DeletingDtor(uint8 flags);
    void* __thiscall SetPosition(const float* pos, bool skipPhysics, int32 arg);
    void __thiscall SetHeading(float heading);
    void  __thiscall InitHealth(float health);
    void  __thiscall SetHealth(float health, int32 arg);
    void  __thiscall AddHealth(float delta);
    float __thiscall GetHealth() const;

    // Slice 1 (0xA47600..0xA483D0)
    static void __stdcall RemoveInstConstraints(void* phInst);
    void __thiscall ActivatePhysics();
    void __thiscall DeactivatePhysics();
    void __thiscall TryDeactivatePhysics();
    void __thiscall SetMoveSpeed(const CVector* speed);
    CVector* __thiscall GetMoveSpeed(CVector* out);
    void __thiscall SetTurnSpeed(const CVector* speed);
    CVector* __thiscall GetTurnSpeed(CVector* out);
    void __thiscall SetPhysUnk1E4(CPhysical* target);
    void __thiscall ClearPhysUnk16C(bool keepCount);
    bool __thiscall HasPhysUnk154Value(uint32 value);
    void __thiscall ApplyForce(const CVector* force, const CVector* offset, int32 component);
    void __thiscall ApplyForceCenter(const CVector* force);
    void __thiscall ApplyForceWithTorque(const CVector* force, const CVector* offset);
    void __thiscall ApplyTorque(const CVector* torque);
    void __thiscall ApplyImpulse(const CVector* impulse, const CVector* offset, int32 component);
    void __thiscall ApplyImpulseCenter(const CVector* impulse);
    void __thiscall ApplyImpulseToLevel(const CVector* impulse, const CVector* offset, int32 component, void* phInst);
    void __thiscall ApplyImpulseToLevelScaled(const CVector* impulse, const CVector* offset, int32 component, void* phInst, float scale);
    CVector* __thiscall GetVelocityAtPoint(CVector* out, const CVector* offset, bool isWorldPos, int32 component);
    void __thiscall SetPhysFlag4();
    void __thiscall RemovePhysUnk154Value(uint32 value);
    void __thiscall SetPhysUnk1F8(void* entity, uint8 flags);
    void __thiscall UpdatePhysUnk1F8();
    void __thiscall AttachToEntitySimple(CPhysical* target, int16 targetBone, int16 myBone, uint16 attachFlags, const CVector* offset, float breakForce);
    void __thiscall AttachToWorld(int16 myBone, uint16 attachFlags, const CVector* worldPos, const CVector* offset, float breakForce);

    // Slice 2 (0xA48550..0xA49A60)
    void __thiscall DetachFromEntity();
    bool __thiscall CheckPhysUnk11C(uint16 value) const;
    double __thiscall GetModelBoundRadiusNeg() const;
    void __thiscall AddToPhysicsWorld();
    void __thiscall RemoveFromPhysicsWorld();
    void __thiscall SyncPhysicsMatrix(bool forceColliderUpdate);
    void __thiscall SetPhysFlag(bool enable, bool isFlag10);
    void __thiscall SetColliderScale(float scale);
    void __thiscall UpdateTransformFromPhysics(void* phInst, bool skipUnk);
    void __thiscall SetMatrixAndWarp(const CMatrix* mat, bool skipPhysics, int32 warpArg);
    void __thiscall ApplyImpactImpulse(CPhysical* attacker, const CVector* impulse, const float* unk180, const float* unk190, uint32 unk1A0, uint32 unk1A4, uint32 unk1A8);
    void __thiscall ProcessImpactEffect();
    double __thiscall CalcEffectiveMass(const CVector* dir, const CVector* pos);
    void __thiscall UpdatePhysicsActiveState();
    static void __cdecl CopyAttachTransform(CEntity* src, CEntity* dst, const CVector* offset);
    void __thiscall MarkPhysUnk1F4Bit7();
    bool __thiscall CheckPhysCollisionFilter(void* arg0, bool arg1);
    void __thiscall AttachToEntityQuat(CEntity* target, int16 boneTag, uint16 flags, const float* offset, const float* quat);
    void __thiscall AttachToWorldQuat(uint16 flags, const float* offset, const float* quat);

    // Slice 3 (0xA49A90..0xA4AF20, 0x447B20, 0x447B50)
    void __thiscall AttachToEntity(CEntity* target, int16 targetBoneId, int16 selfBoneId, uint16 attachFlags, const float* offset, const float* quat, const CVector* velOffset, float breakStrength);
    void __thiscall UpdateAttachRotation(const CVector* pos, const CVector* rot);
    void __thiscall ProcessAttach(uint32 flags);
    void __thiscall UpdateAttachment();
    bool __thiscall CanBeDamaged(CEntity* inflictor, int32 weaponType, bool checkUnk) const;
    void __thiscall UpdateSubmersion();
    bool __thiscall IsPhysFlag118Bit3Clear() const;
    void __thiscall SetPhysVec200(const float* vec4);
};

VALIDATE_SIZE(CPhysical, 0x210);
VALIDATE_OFFSET(CPhysical, m_fHealth, 0x110);
VALIDATE_OFFSET(CPhysical, m_vecMoveSpeed, 0x154);
VALIDATE_OFFSET(CPhysical, m_vecTurnSpeed, 0x160);
VALIDATE_OFFSET(CPhysical, m_pAttachedTo, 0x1BC);
VALIDATE_OFFSET(CPhysical, m_fMaxHealth, 0x1F0);
VALIDATE_OFFSET(CPhysical, m_vecPhysUnk200, 0x200);
