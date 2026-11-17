#pragma once
#include "Game/Cam.h"
#include "Core/Pool.h"

class CCamera
{
public:
    static inline CCamera&    TheCamera     = Core::StaticRef<CCamera>(0xF21A6C);
    static inline CCam*&      ms_pFinalCam  = Core::StaticRef<CCam*>(0xF21A70);
    static inline CPoolBase*& ms_pCamPool   = Core::StaticRef<CPoolBase*>(0x1618020);

    int32 m_nActiveCamCount = 0;   // +0x00
    CCam* m_pFinalCam = nullptr;   // +0x04
    uint32 m_nUnk08 = 0;           // +0x08
    CCam* m_pGameCam = nullptr;    // +0x0C
    CCam* m_pVehicleCam = nullptr; // +0x10
    CCam* m_pOnFootCam = nullptr;  // +0x14

    static void InjectHooks();

    CCam* __thiscall GetTopActiveCam();
    uint8 __thiscall DestroyCam(CCam* cam);
    CCam* __thiscall GetScriptCam();
    CCam* __thiscall GetCutsceneCam();
    CCam* __thiscall GetAimWeaponCam();
    CCam* __thiscall GetViewportsCam();
    CCam* __thiscall GetFpsWeaponCam();
    CCam* __thiscall GetIntermezzoCam();
    CCam* __thiscall GetCinematicCam();
    CCam* __thiscall GetReplayCam();
    CCam* __thiscall GetDebugCam();
    CCam* __thiscall GetFreeCam();
    uint8 __thiscall DestroyUnusedCams(uint8 onlyUnmarked);
    uint8 __thiscall Shutdown();
};

VALIDATE_OFFSET(CCamera, m_pFinalCam, 0x04);
VALIDATE_OFFSET(CCamera, m_pGameCam, 0x0C);
VALIDATE_OFFSET(CCamera, m_pVehicleCam, 0x10);
VALIDATE_OFFSET(CCamera, m_pOnFootCam, 0x14);
