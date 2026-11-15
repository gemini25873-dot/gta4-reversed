#include "StdInc.h"
#include "Game/World.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CPhysicsWorld::InjectHooks()
{
    Core::Hook::Install(0x518A50, &CPhysicsWorld::UpdateCacheEntryContext);
    Core::Hook::Install(0x518A70, &CPhysicsWorld::ApplyImpulse);
    Core::Hook::Install(0x518C10, &CPhysicsWorld::ApplyScaledImpulse);
    Core::Hook::Install(0x51A0E0, &CPhysicsWorld::GetComponentData);

    Core::Log::Info("CPhysicsWorld hooks yuklendi (4 fonksiyon; 5 fonksiyon guvensiz callee nedeniyle atlandi)");
}

void CWorld::InjectHooks()
{
    // Izolasyon testi: kirilabilir/hareketli nesnelerin duvar gibi davranmasi CPhysicsWorld'den mi kontrol ediliyor
    // CPhysicsWorld::InjectHooks();
}

// 0x518A50 (retn 10h)
int32 __thiscall CPhysicsWorld::UpdateCacheEntryContext(int32 a2, int32 a3, int32 a4, int32 a5)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;

    auto* cacheEntry = static_cast<uint8*>(m_pCacheEntry);
    if (cacheEntry)
    {
        auto* fragMgr = static_cast<uint8*>(CWorld::ms_pFragManager);
        auto* mgrObj = *reinterpret_cast<uint8**>(fragMgr + 752);
        *reinterpret_cast<uint32*>(cacheEntry + 12) = *reinterpret_cast<uint32*>(mgrObj + 8);
    }
    return static_cast<int32>(reinterpret_cast<uintptr_t>(cacheEntry));
}

// 0x518A70 (retn 10h)
int32 __thiscall CPhysicsWorld::ApplyImpulse(float* impulse, int32* pos, int32 a4, float scale)
{
    if (m_nLevelIndex == 0xFFFFu)
    {
        return 0;
    }

    struct ImpulseEvent
    {
        int32 pos[4];
        float quat[4];
        float dir[4];
        float magnitude;
        float scaledForce;
        int32 arg4;
        int32 unk60;
        int32 unk64;
        int32 unk68;
        int32 unk6C;
        uint8 flags;
    } ev{};

    ev.flags |= 1u;
    ev.pos[0] = pos[0];
    ev.pos[1] = pos[1];
    ev.pos[2] = pos[2];

    const float ix = impulse[0];
    const float iy = impulse[1];
    const float iz = impulse[2];
    const float iw = impulse[3];

    ev.quat[0] = ix;
    ev.quat[1] = iy;
    ev.quat[2] = iz;
    ev.quat[3] = iw;
    ev.arg4 = a4;

    Core::Hook::CallMethod<void>(0x42F5D0, ev.quat);

    ev.dir[0] = ix;
    ev.dir[1] = iy;
    ev.dir[2] = iz;
    ev.dir[3] = iw;

    const float lenSq = (iz * iz) + (iy * iy) + (ix * ix);
    ev.magnitude = static_cast<float>(Core::Hook::Call<double>(0x404B80, lenSq));

    auto* scaleData = static_cast<uint8*>(CWorld::ms_pImpulseScaleData);
    float minThresh = 1.0f;
    float maxThresh = 16.0f;
    float multiplier = 1.0f;
    ev.scaledForce = 0.0f;

    if (scaleData)
    {
        multiplier = *reinterpret_cast<float*>(scaleData + 40);
        minThresh = *reinterpret_cast<float*>(scaleData + 44);
        maxThresh = *reinterpret_cast<float*>(scaleData + 48);
    }

    if (ev.magnitude > minThresh)
    {
        float clamped = ev.magnitude * scale;
        if (minThresh <= clamped)
        {
            if (clamped > maxThresh)
            {
                clamped = maxThresh;
            }
        }
        else
        {
            clamped = minThresh;
        }

        ev.scaledForce = ((clamped - minThresh) / (maxThresh - minThresh)) * multiplier;
        ev.unk64 = 0;

        using DispatchFn = int32(__thiscall*)(CPhysicsWorld*, void*, int32);
        return reinterpret_cast<DispatchFn>(m_vftable[60])(this, ev.pos, 0);
    }

    return static_cast<int32>(reinterpret_cast<uintptr_t>(scaleData));
}

// 0x518C10 (retn 10h)
int32 __thiscall CPhysicsWorld::ApplyScaledImpulse(float* impulse, int32 a3, int32 a4, float a5)
{
    float scaled[4]{};
    scaled[0] = impulse[0] * 0.033333335f;
    scaled[1] = impulse[1] * 0.033333335f;
    scaled[2] = impulse[2] * 0.033333335f;

    using Fn = int32(__thiscall*)(CPhysicsWorld*, float*, int32, int32, float);
    return reinterpret_cast<Fn>(m_vftable[17])(this, scaled, a3, a4, a5);
}

// 0x51A0E0 (retn 4)
int32 __thiscall CPhysicsWorld::GetComponentData(int32 a2)
{
    uint8* ptr = nullptr;
    if (m_pUnk5C)
    {
        if ((m_pUnk5C[2] & 0x30000000) == 0x30000000)
        {
            ptr = reinterpret_cast<uint8*>(m_pUnk5C[0]);
        }
    }
    else
    {
        ptr = static_cast<uint8*>(m_pFragType);
    }

    if (m_pCacheEntry)
    {
        return *reinterpret_cast<int32*>(static_cast<uint8*>(m_pCacheEntry) + 444);
    }

    const int32 result = *reinterpret_cast<int32*>(ptr + 516);
    if (!result)
    {
        uint32 buf[256]{};
        Core::Hook::CallStdcall<void>(0x521920, reinterpret_cast<int32>(ptr), reinterpret_cast<int32>(buf), nullptr);
        return a2;
    }

    return result;
}
