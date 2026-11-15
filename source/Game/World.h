#pragma once
#include "Rage/phInst.h"

class CPhysicsWorld : public phInstGta
{
public:
    uint8  m_pad54[0x08]{};          // +0x54..+0x5B
    int32* m_pUnk5C = nullptr;       // +0x5C (this[23])
    uint32 m_nFragFlags = 0;         // +0x60 (this[24])
    void*  m_pCacheEntry = nullptr;  // +0x64 (this[25])
    uint32 m_nUnk68 = 0;             // +0x68 (this[26])
    void*  m_pFragType = nullptr;    // +0x6C (this[27])

    static void InjectHooks();

    // 0x518A50 (__thiscall, retn 10h)
    int32 __thiscall UpdateCacheEntryContext(int32 a2, int32 a3, int32 a4, int32 a5);

    // 0x518A70 (__thiscall, retn 10h)
    int32 __thiscall ApplyImpulse(float* impulse, int32* pos, int32 a4, float scale);

    // 0x518C10 (__thiscall, retn 10h)
    int32 __thiscall ApplyScaledImpulse(float* impulse, int32 a3, int32 a4, float a5);

    // 0x51A0E0 (__thiscall, retn 4)
    int32 __thiscall GetComponentData(int32 a2);

    // Atlanan __thiscall fonksiyonlar (guvensiz / belirsiz callee imzalari):
    // - 0x518180: 0x6A4470 (__thiscall, retn 0Ch) cagrisinda implicit ECX (this) bilinmiyor
    // - 0x51A090: 0x536140 (__thiscall, retn 4) cagrisinda a2 parametresi IDA'da eksik
    // - 0x51A390: 0x53B2C0 (__userpurge) cagiriyor
    // - 0x51A700: 0x51A720 (__usercall) cagiriyor
    // - 0x51A960: 0x63DD40 (__thiscall, retn 8) ve 0x51B0B0 (__thiscall, retn 0Ch) cagrilarinda ECX (this) bilinmiyor
};

VALIDATE_SIZE(CPhysicsWorld, 0x70);
VALIDATE_OFFSET(CPhysicsWorld, m_pUnk5C, 0x5C);
VALIDATE_OFFSET(CPhysicsWorld, m_nFragFlags, 0x60);
VALIDATE_OFFSET(CPhysicsWorld, m_pCacheEntry, 0x64);
VALIDATE_OFFSET(CPhysicsWorld, m_nUnk68, 0x68);
VALIDATE_OFFSET(CPhysicsWorld, m_pFragType, 0x6C);

class CWorld
{
public:
    static inline void*& ms_pFragManager = Core::StaticRef<void*>(0x18A8408);
    static inline void*& ms_pImpulseScaleData = Core::StaticRef<void*>(0x19AC950);

    static void InjectHooks();
};
