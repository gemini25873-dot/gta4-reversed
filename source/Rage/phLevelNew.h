#pragma once

namespace rage
{
    class phLevelBase
    {
    public:
        void**           m_vftable = nullptr;          // +0x00 (rage::phLevelBase::`vftable' @ 0x00EBC560)
        CRITICAL_SECTION m_csLock{};                   // +0x04..+0x1B
        uint8            m_pad1C[0x08]{};              // +0x1C..+0x23
        uint8            m_bUnk24 = 0;                 // +0x24
        uint8            m_pad25 = 0;                  // +0x25
        uint16           m_nMaxInstances = 0;          // +0x26
        uint16           m_nUnk28 = 0;                 // +0x28
        uint16           m_nUnk2A = 0;                 // +0x2A
        uint16           m_nUnk2C = 0;                 // +0x2C
        uint16           m_nUnk2E = 0;                 // +0x2E
        uint16           m_nUnk30 = 0;                 // +0x30
        uint8            m_pad32[0x02]{};              // +0x32..+0x33
        uint8*           m_pInstanceStates = nullptr;  // +0x34
        uint8            m_pad38[0x08]{};              // +0x38..+0x3F
        int32            m_nUnk40 = -1;                // +0x40
        uint8            m_bUnk44 = 0;                 // +0x44
        uint8            m_pad45[0x03]{};              // +0x45..+0x47

        static void InjectHooks();

        // 0x6D75A0 (__thiscall, retn 4) - vftable[0]
        phLevelBase* __thiscall DeletingDtor(uint8 flags);

        // 0x6D75E0 (__usercall: esi = this) - saf C++ (hooklanmaz)
        int32 AllocInstanceStates();
    };

    VALIDATE_SIZE(phLevelBase, 0x48);
    VALIDATE_OFFSET(phLevelBase, m_vftable, 0x00);
    VALIDATE_OFFSET(phLevelBase, m_csLock, 0x04);
    VALIDATE_OFFSET(phLevelBase, m_bUnk24, 0x24);
    VALIDATE_OFFSET(phLevelBase, m_nMaxInstances, 0x26);
    VALIDATE_OFFSET(phLevelBase, m_nUnk28, 0x28);
    VALIDATE_OFFSET(phLevelBase, m_nUnk2A, 0x2A);
    VALIDATE_OFFSET(phLevelBase, m_nUnk2C, 0x2C);
    VALIDATE_OFFSET(phLevelBase, m_nUnk2E, 0x2E);
    VALIDATE_OFFSET(phLevelBase, m_nUnk30, 0x30);
    VALIDATE_OFFSET(phLevelBase, m_pInstanceStates, 0x34);
    VALIDATE_OFFSET(phLevelBase, m_nUnk40, 0x40);
    VALIDATE_OFFSET(phLevelBase, m_bUnk44, 0x44);

    class phLevelNew : public phLevelBase
    {
    public:
        uint8  m_pad48[0x08]{};              // +0x48..+0x4F
        uint32 m_nUnk50 = 0;                 // +0x50
        uint32 m_nUnk54 = 0;                 // +0x54
        uint32 m_nUnk58 = 0;                 // +0x58
        uint32 m_nUnk5C = 0;                 // +0x5C
        uint32 m_nUnk60 = 0;                 // +0x60
        uint32 m_nMaxOctreeNodes = 10240;    // +0x64
        uint32 m_nUnk68 = 5;                 // +0x68
        uint32 m_nUnk6C = 4;                 // +0x6C
        void*  m_pUnk70 = nullptr;           // +0x70
        void*  m_pUnk74 = nullptr;           // +0x74
        uint32 m_nUnk78 = 0;                 // +0x78
        uint32 m_nUnk7C = 0;                 // +0x7C
        void*  m_pUnk80 = nullptr;           // +0x80
        void*  m_pUnk84 = nullptr;           // +0x84
        uint16 m_nUnk88 = 0;                 // +0x88
        uint16 m_nUnk8A = 0;                 // +0x8A
        void*  m_pUnk8C = nullptr;           // +0x8C
        int16  m_nUnk90 = -1;                // +0x90
        uint8  m_pad92[0x02]{};              // +0x92..+0x93
        void*  m_pUnk94 = nullptr;           // +0x94
        uint32 m_nUnk98 = 0;                 // +0x98
        void*  m_pUnk9C = nullptr;           // +0x9C
        void*  m_pUnkA0 = nullptr;           // +0xA0

        static void InjectHooks();

        // 0x6A2480 (__thiscall, retn)
        phLevelNew* __thiscall Ctor();

        // 0x6A2AF0 (__thiscall, retn)
        int32 __thiscall Shutdown();
    };

    VALIDATE_SIZE(phLevelNew, 0xA4);
    VALIDATE_OFFSET(phLevelNew, m_nUnk50, 0x50);
    VALIDATE_OFFSET(phLevelNew, m_nUnk54, 0x54);
    VALIDATE_OFFSET(phLevelNew, m_nUnk60, 0x60);
    VALIDATE_OFFSET(phLevelNew, m_nMaxOctreeNodes, 0x64);
    VALIDATE_OFFSET(phLevelNew, m_nUnk68, 0x68);
    VALIDATE_OFFSET(phLevelNew, m_nUnk6C, 0x6C);
    VALIDATE_OFFSET(phLevelNew, m_pUnk70, 0x70);
    VALIDATE_OFFSET(phLevelNew, m_pUnk74, 0x74);
    VALIDATE_OFFSET(phLevelNew, m_nUnk78, 0x78);
    VALIDATE_OFFSET(phLevelNew, m_nUnk7C, 0x7C);
    VALIDATE_OFFSET(phLevelNew, m_pUnk80, 0x80);
    VALIDATE_OFFSET(phLevelNew, m_pUnk84, 0x84);
    VALIDATE_OFFSET(phLevelNew, m_nUnk88, 0x88);
    VALIDATE_OFFSET(phLevelNew, m_nUnk8A, 0x8A);
    VALIDATE_OFFSET(phLevelNew, m_pUnk8C, 0x8C);
    VALIDATE_OFFSET(phLevelNew, m_nUnk90, 0x90);
    VALIDATE_OFFSET(phLevelNew, m_pUnk94, 0x94);
    VALIDATE_OFFSET(phLevelNew, m_nUnk98, 0x98);
    VALIDATE_OFFSET(phLevelNew, m_pUnk9C, 0x9C);
    VALIDATE_OFFSET(phLevelNew, m_pUnkA0, 0xA0);
}
