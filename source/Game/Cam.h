#pragma once

class CCam
{
public:
    void** m_vftable = nullptr;          // +0x00
    uint8  m_pad04[0x0C]{};              // +0x04
    uint8  m_pad10[0x50]{};              // +0x10 (sub-struct at +0x10, 0x80 bytes)
    float  m_fFov = 0.0f;                // +0x60
    uint8  m_pad64[0x08]{};              // +0x64
    float  m_fNearClip = 0.0f;           // +0x6C
    float  m_fFarClip = 0.0f;            // +0x70
    float  m_fUnk74 = 0.0f;              // +0x74
    uint8  m_pad78[0x08]{};              // +0x78
    uint8  m_nUnk80 = 0;                 // +0x80
    uint8  m_pad81[0x0F]{};              // +0x81
    uint8  m_pad90[0x80]{};              // +0x90 (sub-struct at +0x90, 0x80 bytes)
    void*  m_pUnk110 = nullptr;          // +0x110
    uint32 m_nUnk114 = 0;                // +0x114
    CCam*  m_pParent = nullptr;          // +0x118
    CCam*  m_pNext = nullptr;            // +0x11C
    CCam*  m_pPrev = nullptr;            // +0x120
    CCam*  m_pChild = nullptr;           // +0x124
    void*  m_pUnk128 = nullptr;          // +0x128
    void*  m_pUnk12C = nullptr;          // +0x12C
    uint32 m_nUnk130 = 0;                // +0x130
    int32  m_nUnk134 = 0;                // +0x134
    int16  m_nUnk138 = 0;                // +0x138
    uint8  m_nUnk13A = 0;                // +0x13A
    uint8  m_nUnk13B = 0;                // +0x13B
    uint32 m_nFlags = 0;                 // +0x13C

    static void InjectHooks();

    static void* __cdecl InitPool();

    CCam* __thiscall Ctor();
    int32 __thiscall InitFields();
    int32 __thiscall Dtor();
    CCam* __thiscall ScalarDeletingDtor(uint8 flags);
    void  __thiscall ClearUnk128();
    void  __thiscall SyncBlend();
    int32 __thiscall CleanupUnk12C();
    int32 __thiscall CleanupUnk110();
    CCam* __thiscall AttachChild(CCam* child);
    CCam* __thiscall FindCamOfType(int32 type, int32* indexPtr);
    CCam* __thiscall GetCamOfType(int32 type, int32 index);
    int32 __thiscall UpdateChildren();
    void  __thiscall UpdateNearFarClip();
};

VALIDATE_SIZE(CCam, 0x140);
VALIDATE_OFFSET(CCam, m_vftable, 0x00);
VALIDATE_OFFSET(CCam, m_fFov, 0x60);
VALIDATE_OFFSET(CCam, m_fNearClip, 0x6C);
VALIDATE_OFFSET(CCam, m_fFarClip, 0x70);
VALIDATE_OFFSET(CCam, m_fUnk74, 0x74);
VALIDATE_OFFSET(CCam, m_nUnk80, 0x80);
VALIDATE_OFFSET(CCam, m_pUnk110, 0x110);
VALIDATE_OFFSET(CCam, m_pParent, 0x118);
VALIDATE_OFFSET(CCam, m_pNext, 0x11C);
VALIDATE_OFFSET(CCam, m_pPrev, 0x120);
VALIDATE_OFFSET(CCam, m_pChild, 0x124);
VALIDATE_OFFSET(CCam, m_pUnk128, 0x128);
VALIDATE_OFFSET(CCam, m_pUnk12C, 0x12C);
VALIDATE_OFFSET(CCam, m_nUnk130, 0x130);
VALIDATE_OFFSET(CCam, m_nUnk134, 0x134);
VALIDATE_OFFSET(CCam, m_nUnk138, 0x138);
VALIDATE_OFFSET(CCam, m_nFlags, 0x13C);
