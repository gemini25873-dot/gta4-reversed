#include "StdInc.h"
#include "Game/Camera.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    int32 __fastcall DestroyAllPooledCams(CPoolBase* pool)
    {
        int32 idx = pool->m_nSize;
        int32 result = 0;
        while (idx > 0)
        {
            --idx;
            if (!pool->IsSlotFree(idx))
            {
                auto* cam = static_cast<CCam*>(
                    static_cast<void*>(static_cast<uint8*>(pool->m_pObjects) + idx * pool->m_nElementSize));
                if (cam && cam->m_vftable)
                {
                    using DtorFn = int32(__thiscall*)(CCam*, int32);
                    result = reinterpret_cast<DtorFn>(cam->m_vftable[0])(cam, 1);
                }
            }
        }
        return result;
    }
}

void CCamera::InjectHooks()
{
    Core::Hook::Install(0x91F200, &CCamera::GetTopActiveCam);
    Core::Hook::Install(0x91F250, &CCamera::DestroyCam);
    Core::Hook::Install(0x91F280, &CCamera::GetScriptCam);
    Core::Hook::Install(0x91F2A0, &CCamera::GetCutsceneCam);
    Core::Hook::Install(0x91F2C0, &CCamera::GetAimWeaponCam);
    Core::Hook::Install(0x91F2E0, &CCamera::GetViewportsCam);
    Core::Hook::Install(0x91F300, &CCamera::GetFpsWeaponCam);
    Core::Hook::Install(0x91F320, &CCamera::GetIntermezzoCam);
    Core::Hook::Install(0x91F340, &CCamera::GetCinematicCam);
    Core::Hook::Install(0x91F360, &CCamera::GetReplayCam);
    Core::Hook::Install(0x91F380, &CCamera::GetDebugCam);
    Core::Hook::Install(0x91F3A0, &CCamera::GetFreeCam);
    Core::Hook::Install(0x91F3C0, &CCamera::DestroyUnusedCams);
    Core::Hook::Install(0x91F470, &DestroyAllPooledCams);
    Core::Hook::Install(0x91FB40, &CCamera::Shutdown);

    Core::Log::Info("CCamera hooks installed (15 functions)");
}

// 0x91F200
CCam* __thiscall CCamera::GetTopActiveCam()
{
    CCam* cams[60]{};
    int32 idx = Core::Hook::CallMethod<int32>(0x9ACB70, m_pFinalCam, cams) - 1;
    while (idx >= 0)
    {
        if (Core::Hook::CallMethod<uint8>(0x9ACB20, cams[idx]))
            return cams[idx];
        --idx;
    }
    return nullptr;
}

// 0x91F250
uint8 __thiscall CCamera::DestroyCam(CCam* cam)
{
    --m_nActiveCamCount;
    if (cam == m_pGameCam)
        m_pGameCam = nullptr;

    if (cam)
    {
        using DtorFn = void(__thiscall*)(CCam*, int32);
        reinterpret_cast<DtorFn>(cam->m_vftable[0])(cam, 1);
    }
    return 1;
}

// 0x91F280
CCam* __thiscall CCamera::GetScriptCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(6, 0) : nullptr;
}

// 0x91F2A0
CCam* __thiscall CCamera::GetCutsceneCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(15, 0) : nullptr;
}

// 0x91F2C0
CCam* __thiscall CCamera::GetAimWeaponCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(9, 0) : nullptr;
}

// 0x91F2E0
CCam* __thiscall CCamera::GetViewportsCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(20, 0) : nullptr;
}

// 0x91F300
CCam* __thiscall CCamera::GetFpsWeaponCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(27, 0) : nullptr;
}

// 0x91F320
CCam* __thiscall CCamera::GetIntermezzoCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(32, 0) : nullptr;
}

// 0x91F340
CCam* __thiscall CCamera::GetCinematicCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(22, 0) : nullptr;
}

// 0x91F360
CCam* __thiscall CCamera::GetReplayCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(37, 0) : nullptr;
}

// 0x91F380
CCam* __thiscall CCamera::GetDebugCam()
{
    return m_pFinalCam ? m_pFinalCam->GetCamOfType(39, 0) : nullptr;
}

// 0x91F3A0
CCam* __thiscall CCamera::GetFreeCam()
{
    CCam* debugCam = GetDebugCam();
    return debugCam ? debugCam->GetCamOfType(38, 0) : nullptr;
}

// 0x91F3C0
uint8 __thiscall CCamera::DestroyUnusedCams(uint8 onlyUnmarked)
{
    CCam* cams[60]{};
    if (onlyUnmarked)
    {
        for (int32 pass = 0; pass < 2; ++pass)
        {
            const int32 count = Core::Hook::CallMethod<int32>(0x9ACB70, m_pFinalCam, cams);
            for (int32 i = count - 1; i >= 0; --i)
            {
                CCam* cam = cams[i];
                if ((cam->m_nFlags & 2u) != 0 && cam->m_nUnk13B == 0)
                    DestroyCam(cam);
            }
        }
        return 1;
    }

    const int32 count = Core::Hook::CallMethod<int32>(0x9ACB70, m_pFinalCam, cams);
    for (int32 i = count - 1; i >= 0; --i)
    {
        CCam* cam = cams[i];
        if ((cam->m_nFlags & 2u) != 0)
            DestroyCam(cam);
    }
    return 1;
}

// 0x91FB40
uint8 __thiscall CCamera::Shutdown()
{
    CCam* cams[60]{};
    const int32 count = Core::Hook::CallMethod<int32>(0x9ACB70, m_pFinalCam, cams);
    for (int32 i = count - 1; i >= 0; --i)
        Core::Hook::CallMethod<void>(0x9AC930, cams[i]);

    DestroyUnusedCams(0);
    DestroyAllPooledCams(ms_pCamPool);
    m_nActiveCamCount = 0;
    m_pFinalCam = nullptr;
    return 1;
}
