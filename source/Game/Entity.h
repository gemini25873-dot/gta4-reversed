#pragma once
#include "Core/Vector.h"
#include "Core/Matrix.h"

class CEntity
{
public:
    void**   m_vftable = nullptr;          // +0x00
    uint32   m_nUnk04 = 0;                 // +0x04
    uint32   m_nUnk08 = 0;                 // +0x08
    uint32   m_nUnk0C = 0;                 // +0x0C
    CVector  m_vecPos{};                   // +0x10
    float    m_fPad1C = 0.0f;              // +0x1C
    CMatrix* m_pMatrix = nullptr;          // +0x20
    uint32   m_nFlags = 0;                 // +0x24
    uint32   m_nFlags2 = 0;                // +0x28
    uint16   m_nRandomSeed = 0;            // +0x2C
    int16    m_nModelIndex = -1;           // +0x2E
    uint32   m_pReferences = 0;            // +0x30
    void*    m_pDrawHandler = nullptr;     // +0x34
    void*    m_pPhysicsInst = nullptr;     // +0x38
    uint32   m_nUnk3C = 0;                 // +0x3C
    int8     m_nInteriorIndex = 63;        // +0x40
    uint8    m_nUnk41 = 0;                 // +0x41
    int16    m_nUnk42 = -1;                // +0x42
    int16    m_nUnk44 = -1;                // +0x44
    int16    m_nPad46 = 0;                 // +0x46
    int32    m_nInteriorHash = -1;         // +0x48
    void*    m_pLod = nullptr;             // +0x4C
    uint32   m_nUnk50 = 0;                 // +0x50
    uint32   m_nUnk54 = 0;                 // +0x54
    uint32   m_nUnk58 = 0;                 // +0x58 (byte +0x5B = alpha)
    uint16   m_nUnk5C = 0;                 // +0x5C
    uint16   m_nUnk5E = 0;                 // +0x5E
    int8     m_nUnk60 = -1;                // +0x60
    uint8    m_nUnk61 = 0;                 // +0x61
    int8     m_nUnk62 = -1;                // +0x62
    int8     m_nUnk63 = -1;                // +0x63
    int32    m_nUnk64 = -1;                // +0x64
    int32    m_nUnk68 = -1;                // +0x68
    uint32   m_nUnk6C = 0;                 // +0x6C

    static inline CEntity*& ms_pCurrentEntity = Core::StaticRef<CEntity*>(0x1639574);

    static void InjectHooks();

    CEntity* __thiscall Ctor();
    int32 __thiscall PreRender(int32 arg);
    void __thiscall UpdatePhysics(int32 arg);
    int32 __thiscall UpdateTransform();
    double __thiscall GetBoundRadius() const;
    const CVector* __thiscall GetBoundCentre() const;
    int32 __thiscall AddToInterior();
    int32 __thiscall RemoveFromInterior();

    const CVector& GetPosition() const { return m_pMatrix ? m_pMatrix->pos : m_vecPos; }

    struct CEntityReference
    {
    CEntityReference* pNext;
    CEntity**         ppEntity;
    };

    static inline CEntityReference*& ms_pRefFreeList  = Core::StaticRef<CEntityReference*>(0x1639550);
    static inline CEntityReference*& ms_pRefPool      = Core::StaticRef<CEntityReference*>(0x1639554);
    static inline int32&             ms_nMaxRefs      = Core::StaticRef<int32>(0x1639558);
    static inline int32&             ms_nRefLockCount = Core::StaticRef<int32>(0x163955C);


    static void __cdecl LockReferences();
    static void __cdecl UnlockReferences();
    static void __cdecl InitReferences();
    static void __cdecl ShutdownReferences();
    void __thiscall RegisterReference(CEntity** ppRef);
    void __thiscall CleanUpOldReference(CEntity** ppRef);
    void __thiscall ResolveReferences();
    void __thiscall PruneReferences();
    static void __cdecl PruneAllImportantReferences();
    static void __cdecl PruneReferencesInPools();
    void __thiscall CopyInteriorInfo(const CEntity* src, bool copyUnk44);
    void* __thiscall GetPortalTracker();
    void __thiscall Init2dEffects(int32 modelIndex);
    void __thiscall Remove2dEffects();
    int32 __thiscall SetModelByName(const char* modelName);
    void __thiscall DeleteDrawHandler();
    const CVector* __thiscall GetBoundMax() const;
    bool __thiscall HasDrawableOrEffects() const;
    bool __thiscall HasIdentityMatrix() const;
    bool __thiscall IsOnScreen(bool checkShadowOrReflection);
    bool __thiscall IsVisibleInFrustum(void* viewport);

    void __thiscall SetInteriorState(int32 interiorHash, int32 roomIndex, int32 portalIndex);
    void __thiscall ClearInteriorState();
    void __thiscall MarkModelUsed();
    CVector* __thiscall GetOffsetVec(CVector* outVec, int32 arg) const;
    void* __thiscall FindClosest2dEffect(int32 effectType, const CVector* pos) const;
    bool __thiscall HasNetworkIdBelow200() const;
    bool __thiscall IsNetworkIdFFFE() const;
    void __thiscall SetPhysicsInst(void* inst, bool addModelRef);
    void __thiscall DeletePhysicsInst(int32 unused);
    void* __thiscall GetPhysicsCollider() const;
    bool __thiscall IsNotInPhysicsLevel() const;
    void __thiscall RemoveFromPhysicsLevel();
    static void __stdcall AddEntityToList(void** array, int32* count, int32 maxCount, void* item);
    void __thiscall UpdateUnk6C(int32 arg);
    int32 __thiscall GetNum2dEffects() const;
    void __thiscall Process2dEffects(float arg);
    void __thiscall Create2dEffectAudio();
    void __thiscall Destroy2dEffectAudio();
    void __thiscall Process2dEffectsPtfx();



    static const CVector* __cdecl TransformPoint2D(CVector* out, const float* posHeading, const CVector* offset);

    bool __thiscall IsInArea2D(float x1, float y1, float x2, float y2) const;
    bool __thiscall IsInArea3D(float x1, float y1, float z1, float x2, float y2, float z2) const;
    void* __thiscall GetTypeSpecificPtr();
    bool __thiscall CheckAndClearLodFlag();
    void __thiscall SetFlag5EBit1(bool enable);
    void __thiscall SetModelIndex(int32 modelIndex);
    bool __thiscall CreateDrawHandler();
    float* __thiscall GetBoundRect(float* outRect) const;
    float* __thiscall GetBoundCentreAndRadius(float* out) const;
    float* __thiscall CalcBoundCentreAndRadius(float* out) const;
    double __thiscall GetBoundSphere(float* out) const;
    bool __thiscall CheckDrawableAndUpdate();
    bool __thiscall IsInFrustum(int32 index);
    bool __thiscall CanAddToInterior() const;
    void* __thiscall GetPhArchetype() const;
    void __thiscall RemoveFromPhysicsWorld();
    void __thiscall InitModelEffects();
    void __thiscall TriggerModelEffectsType1(int32 arg0, int32 arg1, int32 boneIndex, float arg3);
    void __thiscall TriggerModelEffectsType2(int32 arg0, int32 arg1, int32 boneIndex);
    void __thiscall TriggerModelEffectsAtMatrix(int32 boneTag, const CMatrix* mat);


    void __thiscall RemoveMatrix();
    float* __thiscall GetWorldBoundBox(float* outBox);
    int32 __thiscall UpdateRenderAlpha();
    void __thiscall SetInteriorByName(void* interiorNameOrPtr, int32 interiorIndex, int32 arg3);
    void __thiscall AddToGrid(const float* boundRect);
    void __thiscall AllocateMatrix();
    void __thiscall AllocateStaticMatrix();
    void __thiscall ReleaseMatrix();
    void __thiscall Dtor();
    int32 __thiscall Render(int32 arg0, int32 arg1, int32 arg2, int32 arg3);
    void __thiscall RemoveFromGrid();
    void __thiscall CreatePhysicsInst();
    void __thiscall UpdatePhysicsMatrix(int32 arg);
    void __thiscall Process2dEffects(int32 arg0, int32 arg1, int32 arg2);
    void __thiscall SetMatrix(const CMatrix* mat, int32 arg1, int32 arg2);
    CEntity* __thiscall DeletingDtor(uint8 flags);
};

VALIDATE_SIZE(CEntity, 0x70);
VALIDATE_OFFSET(CEntity, m_vecPos, 0x10);
VALIDATE_OFFSET(CEntity, m_pMatrix, 0x20);
VALIDATE_OFFSET(CEntity, m_nFlags, 0x24);
VALIDATE_OFFSET(CEntity, m_nFlags2, 0x28);
VALIDATE_OFFSET(CEntity, m_nRandomSeed, 0x2C);
VALIDATE_OFFSET(CEntity, m_nModelIndex, 0x2E);
VALIDATE_OFFSET(CEntity, m_pDrawHandler, 0x34);
VALIDATE_OFFSET(CEntity, m_pPhysicsInst, 0x38);
VALIDATE_OFFSET(CEntity, m_nInteriorIndex, 0x40);
VALIDATE_OFFSET(CEntity, m_nInteriorHash, 0x48);
VALIDATE_OFFSET(CEntity, m_pLod, 0x4C);
