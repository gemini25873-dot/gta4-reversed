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
