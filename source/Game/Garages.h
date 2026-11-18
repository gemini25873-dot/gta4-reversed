#pragma once
#include "Core/Vector.h"

class CVehicle;

class CStoredCar
{
public:
    CVector  m_vecPos{};                // +0x00..+0x0B
    uint32   m_nFlagsLocal = 0;         // +0x0C
    uint16   m_nModelIndex = 0;         // +0x10
    uint8    m_pad12[0x1E]{};           // +0x12..+0x2F
    uint16   m_nBits = 0;               // +0x30
    uint8    m_nColour1 = 0;            // +0x32
    uint8    m_nColour2 = 0;            // +0x33
    uint8    m_nColour3 = 0;            // +0x34
    uint8    m_nColour4 = 0;            // +0x35
    uint8    m_pad36[0x02]{};           // +0x36..+0x37
    uint32   m_nExtras = 0;             // +0x38
    uint32   m_nPaintJob = 0;           // +0x3C
    uint8    m_pad40[0x02]{};           // +0x40..+0x41
    uint8    m_nUnk42 = 0;              // +0x42
    int8     m_nFrontX = 0;             // +0x43
    int8     m_nFrontY = 0;             // +0x44
    int8     m_nFrontZ = 0;             // +0x45
    uint8    m_pad46[0x02]{};           // +0x46..+0x47
};

VALIDATE_SIZE(CStoredCar, 0x48);
VALIDATE_OFFSET(CStoredCar, m_vecPos, 0x00);
VALIDATE_OFFSET(CStoredCar, m_nFlagsLocal, 0x0C);
VALIDATE_OFFSET(CStoredCar, m_nModelIndex, 0x10);
VALIDATE_OFFSET(CStoredCar, m_nBits, 0x30);
VALIDATE_OFFSET(CStoredCar, m_nColour1, 0x32);
VALIDATE_OFFSET(CStoredCar, m_nExtras, 0x38);
VALIDATE_OFFSET(CStoredCar, m_nPaintJob, 0x3C);
VALIDATE_OFFSET(CStoredCar, m_nUnk42, 0x42);
VALIDATE_OFFSET(CStoredCar, m_nFrontX, 0x43);
VALIDATE_OFFSET(CStoredCar, m_nFrontZ, 0x45);

class CGarage
{
public:
    float     m_fBaseX = 0.0f;          // +0x00
    float     m_fBaseY = 0.0f;          // +0x04
    float     m_fBaseZ = 0.0f;          // +0x08
    float     m_fDelta1X = 0.0f;        // +0x0C
    float     m_fDelta1Y = 0.0f;        // +0x10
    float     m_fDelta2X = 0.0f;        // +0x14
    float     m_fDelta2Y = 0.0f;        // +0x18
    float     m_fCeilingZ = 0.0f;       // +0x1C
    float     m_fDelta1Length = 0.0f;   // +0x20
    float     m_fDelta2Length = 0.0f;   // +0x24
    float     m_fMinX = 0.0f;           // +0x28
    float     m_fMaxX = 0.0f;           // +0x2C
    float     m_fMinY = 0.0f;           // +0x30
    float     m_fMaxY = 0.0f;           // +0x34
    uint32    m_nTimeOfNextEvent = 0;   // +0x38
    CVehicle* m_pCarToCollect = nullptr;// +0x3C
    uint32    m_nNameHash = 0;          // +0x40
    uint8     m_pad44[0x04]{};          // +0x44..+0x47
    uint8     m_nType = 0;              // +0x48
    uint8     m_nState = 0;             // +0x49
    uint8     m_nOriginalType = 0;      // +0x4A
    uint8     m_nSafehouseIndex = 0;    // +0x4B
    uint8     m_nFlags = 0;             // +0x4C
    uint8     m_pad4D[0x03]{};          // +0x4D..+0x4F
    uint32    m_nUnk50 = 0;             // +0x50
    float     m_fUnk54 = 0.0f;          // +0x54
    uint32    m_nUnk58 = 0;             // +0x58
    uint8     m_unk5C[0x10]{};          // +0x5C..+0x6B

    void __thiscall PlayerArrestedOrDied();
    bool __thiscall IsPointInsideGarage(const CVector* point) const;
};

VALIDATE_SIZE(CGarage, 0x6C);
VALIDATE_OFFSET(CGarage, m_fBaseZ, 0x08);
VALIDATE_OFFSET(CGarage, m_fCeilingZ, 0x1C);
VALIDATE_OFFSET(CGarage, m_fDelta1Length, 0x20);
VALIDATE_OFFSET(CGarage, m_fDelta2Length, 0x24);
VALIDATE_OFFSET(CGarage, m_fMinX, 0x28);
VALIDATE_OFFSET(CGarage, m_nTimeOfNextEvent, 0x38);
VALIDATE_OFFSET(CGarage, m_pCarToCollect, 0x3C);
VALIDATE_OFFSET(CGarage, m_nNameHash, 0x40);
VALIDATE_OFFSET(CGarage, m_nType, 0x48);
VALIDATE_OFFSET(CGarage, m_nState, 0x49);
VALIDATE_OFFSET(CGarage, m_nOriginalType, 0x4A);
VALIDATE_OFFSET(CGarage, m_nSafehouseIndex, 0x4B);
VALIDATE_OFFSET(CGarage, m_nFlags, 0x4C);
VALIDATE_OFFSET(CGarage, m_nUnk50, 0x50);
VALIDATE_OFFSET(CGarage, m_fUnk54, 0x54);
VALIDATE_OFFSET(CGarage, m_nUnk58, 0x58);
VALIDATE_OFFSET(CGarage, m_unk5C, 0x5C);

class CSaveGarage
{
public:
    uint8  m_nType = 0;                 // +0x00
    uint8  m_nState = 0;                // +0x01
    uint8  m_nFlags = 0;                // +0x02
    uint8  m_pad03 = 0;                 // +0x03
    float  m_fBaseX = 0.0f;             // +0x04
    float  m_fBaseY = 0.0f;             // +0x08
    float  m_fBaseZ = 0.0f;             // +0x0C
    float  m_fDelta1X = 0.0f;           // +0x10
    float  m_fDelta1Y = 0.0f;           // +0x14
    float  m_fDelta2X = 0.0f;           // +0x18
    float  m_fDelta2Y = 0.0f;           // +0x1C
    float  m_fCeilingZ = 0.0f;          // +0x20
    float  m_fDelta1Length = 0.0f;      // +0x24
    float  m_fDelta2Length = 0.0f;      // +0x28
    float  m_fMinX = 0.0f;              // +0x2C
    float  m_fMaxX = 0.0f;              // +0x30
    float  m_fMinY = 0.0f;              // +0x34
    float  m_fMaxY = 0.0f;              // +0x38
    uint32 m_nTimeOfNextEvent = 0;      // +0x3C
    uint32 m_nNameHash = 0;             // +0x40
    uint8  m_nOriginalType = 0;         // +0x44
    uint8  m_nSafehouseIndex = 0;       // +0x45
    uint8  m_pad46[0x02]{};             // +0x46..+0x47

    void     __thiscall CopyGarageIntoSaveGarage(const CGarage* garage);
    CGarage* __thiscall CopyGarageOutOfSaveGarage(CGarage* garage) const;
};

VALIDATE_SIZE(CSaveGarage, 0x48);
VALIDATE_OFFSET(CSaveGarage, m_nFlags, 0x02);
VALIDATE_OFFSET(CSaveGarage, m_fBaseX, 0x04);
VALIDATE_OFFSET(CSaveGarage, m_nTimeOfNextEvent, 0x3C);
VALIDATE_OFFSET(CSaveGarage, m_nNameHash, 0x40);
VALIDATE_OFFSET(CSaveGarage, m_nOriginalType, 0x44);
VALIDATE_OFFSET(CSaveGarage, m_nSafehouseIndex, 0x45);

class CGarages
{
public:
    static constexpr int32 MAX_NUM_GARAGES = 40;
    static constexpr int32 MAX_NUM_STORED_CARS = 20;

    static inline uint32& ms_nNumSafehouseGarages   = Core::StaticRef<uint32>(0x1354A50);
    static inline uint32& ms_nMessageStartTime      = Core::StaticRef<uint32>(0x1354A54);
    static inline uint8&  ms_bBombsAreFree          = Core::StaticRef<uint8>(0x1354A60);
    static inline uint8&  ms_bNoResprays            = Core::StaticRef<uint8>(0x1354A61);
    static inline uint8&  ms_bHasResprayHappened    = Core::StaticRef<uint8>(0x1354A62);
    static inline uint8&  ms_bRespraysAreFree       = Core::StaticRef<uint8>(0x1354A63);
    static inline uint32& ms_nNumGarages            = Core::StaticRef<uint32>(0x1354A64);
    static inline uint32& ms_nLastTimeHelpMessage   = Core::StaticRef<uint32>(0x1354A68);
    static inline int32&  ms_nLastGaragePlayerWasIn = Core::StaticRef<int32>(0x1354A70);
    static inline uint32& ms_nMessageEndTime        = Core::StaticRef<uint32>(0x1354A74);
    static inline int32&  ms_bCamShouldBeOutside    = Core::StaticRef<int32>(0x1354A7C);

    static CStoredCar* GetStoredCarsArray()
    {
        return reinterpret_cast<CStoredCar*>(Core::Rebase(0x1354A88));
    }

    static CGarage* GetArray()
    {
        return reinterpret_cast<CGarage*>(Core::Rebase(0x1355030));
    }

    static void InjectHooks();

    static int32 __cdecl Init();
    static void  __cdecl Shutdown();
    static void  __cdecl Reset();
    static int32 __cdecl CameraShouldBeOutside();
    static void  __cdecl TidyUpGaragesAfterSave();
    static bool  __cdecl IsPointInAnyHideOutGarage(const CVector* point);
    static bool  __cdecl IsPointInAnyGarage(const CVector* point);
    static void  __cdecl PlayerArrestedOrDied();
    static void  __cdecl CloseHideOutGaragesBeforeSave(bool unk);
};
