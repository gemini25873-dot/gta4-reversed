#include "StdInc.h"
#include "Game/Pad.h"
#include "Game/PlayerInfo.h"
#include "Game/Ped.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <intrin.h>

namespace
{
    void* RageAlloc(size_t size)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using AllocFn = void*(__thiscall*)(void*, size_t, size_t, uint32);
        return reinterpret_cast<AllocFn>(vtable[2])(allocator, size, 16, 0);
    }

    CPad*  (__cdecl* const kGetPad)()                   = &CPad::GetPad;
    CPad*  (__cdecl* const kGetPadChecked)(uint8)       = &CPad::GetPadChecked;
    CPad*  (__cdecl* const kGetPlayerPad)()             = &CPad::GetPlayerPad;
    CPad*  (__cdecl* const kGetDebugPad)()              = &CPad::GetDebugPad;

    int32  __cdecl DoSetPadNumber(CPad* pad, int32 idx) { return pad->SetPadNumber(idx); }
    uint32 __cdecl DoTouchLastUpdateTime(CPad* pad)     { return pad->TouchLastUpdateTime(); }
    uint32 __cdecl DoGetTimeSinceLastUpdate(CPad* pad)  { return pad->GetTimeSinceLastUpdate(); }
    void*  __cdecl DoGetJoystick(CPad* pad)             { return pad->GetJoystick(); }

    int32  (__cdecl* const kDoSetPadNumber)(CPad*, int32) = &DoSetPadNumber;
    uint32 (__cdecl* const kDoTouchLastUpdate)(CPad*)     = &DoTouchLastUpdateTime;
    uint32 (__cdecl* const kDoGetTimeSinceUpdate)(CPad*)  = &DoGetTimeSinceLastUpdate;
    void*  (__cdecl* const kDoGetJoystick)(CPad*)         = &DoGetJoystick;

    void __declspec(naked) HookedGetPad()
    {
        __asm {
            push ecx
            push edx
            mov eax, kGetPad
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetPadChecked()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kGetPadChecked
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetPlayerPad()
    {
        __asm {
            push ecx
            push edx
            mov eax, kGetPlayerPad
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedGetDebugPad()
    {
        __asm {
            push ecx
            push edx
            mov eax, kGetDebugPad
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedSetPadNumber()
    {
        __asm {
            push edx
            push ecx
            push dword ptr [esp + 12]
            push ecx
            mov eax, kDoSetPadNumber
            call eax
            add esp, 8
            pop ecx
            pop edx
            ret 4
        }
    }

    void __declspec(naked) HookedTouchLastUpdateTime()
    {
        __asm {
            push edx
            push ecx
            push ecx
            mov eax, kDoTouchLastUpdate
            call eax
            add esp, 4
            pop ecx
            pop edx
            ret
        }
    }

    void __declspec(naked) HookedGetTimeSinceLastUpdate()
    {
        __asm {
            push edx
            push ecx
            push ecx
            mov eax, kDoGetTimeSinceUpdate
            call eax
            add esp, 4
            pop ecx
            pop edx
            ret
        }
    }

    void __declspec(naked) HookedGetJoystick()
    {
        __asm {
            push edx
            push ecx
            push ecx
            mov eax, kDoGetJoystick
            call eax
            add esp, 4
            pop ecx
            pop edx
            ret
        }
    }
}

void CPad::InjectHooks()
{
    Core::Hook::Install(0x8340F0, &tPadValues::AllocHistory);
    Core::Hook::Install(0x834130, &tPadValues::GetAnalogValue);
    Core::Hook::Install(0x834300, &HookedSetPadNumber);
    Core::Hook::Install(0x834630, &CPad::CopySettings);
    Core::Hook::Install(0x834670, &HookedTouchLastUpdateTime);
    Core::Hook::Install(0x834680, &HookedGetTimeSinceLastUpdate);
    Core::Hook::Install(0x834690, &HookedGetJoystick);
    Core::Hook::Install(0x8346B0, &CPad::GetFrontendAxisLR);
    Core::Hook::Install(0x8346F0, &CPad::GetFrontendAxisUD);
    Core::Hook::Install(0x8348F0, &CPad::FindKeyMapping);
    Core::Hook::Install(0x7C3900, &HookedGetPad);
    Core::Hook::Install(0x7C3950, &HookedGetPadChecked);
    Core::Hook::Install(0x7C3990, &HookedGetPlayerPad);
    Core::Hook::Install(0x7C39A0, &HookedGetDebugPad);

    Core::Log::Info("CPad hooks installed (14 functions)");
}

// 0x8340F0
void __thiscall tPadValues::AllocHistory()
{
    if (!m_pHistory)
    {
        m_pHistory = Core::Hook::Call<tValueHistory*>(0x4011D0, sizeof(tValueHistory));
        for (int32 i = 0; i < 64; ++i)
        {
            m_pHistory->m_aItems[i].m_nValue = 0;
            m_pHistory->m_aItems[i].m_nLastUpdateTime = 0;
        }
    }
}

// 0x834130
double __thiscall tPadValues::GetAnalogValue() const
{
    const uint8 raw = m_nUnknown4 ^ m_nCurrentValue;
    const float val = (static_cast<float>(raw) - 127.5f) * 0.0078431377f;
    if (val < -1.0f)
        return -1.0f;
    if (val > 1.0f)
        return 1.0f;
    return val;
}

// 0x834300
int32 __thiscall CPad::SetPadNumber(int32 padIndex)
{
    m_nPadNumber = padIndex;
    m_mapper0.m_nPadIndex = padIndex;
    m_mapper1.m_nPadIndex = padIndex;
    m_mapper2.m_nPadIndex = padIndex;
    m_mapper3.m_nPadIndex = padIndex;
    m_mapper4.m_nPadIndex = padIndex;
    m_mapper5.m_nPadIndex = padIndex;
    return padIndex;
}

// 0x834630
uint8 __thiscall CPad::CopySettings(const CPad* src)
{
    m_bIsUsingKeyboardForAim = src->m_bIsUsingKeyboardForAim;
    m_bUnk328B = src->m_bUnk328B;
    m_bIsUsingKeyboardForHeli = src->m_bIsUsingKeyboardForHeli;
    m_bUnk328C = src->m_bUnk328C;
    return m_bUnk328C;
}

// 0x834670
uint32 __thiscall CPad::TouchLastUpdateTime()
{
    const uint32 now = Core::StaticRef<uint32>(0x11DDE74);
    m_nLastUpdateTime = now;
    return now;
}

// 0x834680
uint32 __thiscall CPad::GetTimeSinceLastUpdate() const
{
    return Core::StaticRef<uint32>(0x11DDE74) - m_nLastUpdateTime;
}

// 0x834690
void* __thiscall CPad::GetJoystick() const
{
    return reinterpret_cast<uint8*>(Core::Rebase(0x110A590)) + 188 * m_nPadNumber;
}

// 0x8346B0
tPadValues* __thiscall CPad::GetFrontendAxisLR(uint8 allowMouse)
{
    if (allowMouse && Core::StaticRef<int32>(0x10FB7F4))
        return &m_aValues[INPUT_FE_MOUSE_LR];

    tPadValues& axis = m_aValues[INPUT_FRONTEND_AXIS_LR];
    if ((axis.m_nCurrentValue ^ axis.m_nUnknown4) == 0x80u)
        return &m_aValues[INPUT_FRONTEND_AXIS_X];

    return &axis;
}

// 0x8346F0
tPadValues* __thiscall CPad::GetFrontendAxisUD(uint8 allowMouse)
{
    if (allowMouse && Core::StaticRef<int32>(0x10FB7F8))
        return &m_aValues[INPUT_FE_MOUSE_UD];

    tPadValues& axis = m_aValues[INPUT_FRONTEND_AXIS_UD];
    if ((axis.m_nCurrentValue ^ axis.m_nUnknown4) == 0x80u)
        return &m_aValues[INPUT_FRONTEND_AXIS_Y];

    return &axis;
}

// 0x8348F0
int32 __thiscall CPad::FindKeyMapping(int32 mapperIndex, int32 controlIndex, uint32* outKey, uint32* outDevice, int32 startIndex)
{
    ioMapper* mapper = nullptr;
    switch (mapperIndex)
    {
    case 0: mapper = &m_mapper0; break;
    case 1: mapper = &m_mapper2; break;
    case 2: mapper = &m_mapper1; break;
    case 3: mapper = &m_mapper4; break;
    case 4: mapper = &m_mapper3; break;
    case 5: mapper = &m_mapper5; break;
    default: return -1;
    }

    if (startIndex >= mapper->m_nCount)
        return -1;

    tPadValues* target = &m_aValues[controlIndex];
    int32 idx = startIndex;
    while (mapper->m_apValues[idx] != target)
    {
        if (++idx >= mapper->m_nCount)
            return -1;
    }

    if (idx == -1)
        return -1;

    *outKey = mapper->m_aKeys[idx] & 0x00FFFFFFu;
    *outDevice = mapper->m_aDevices[idx];
    return idx;
}

// 0x7C3900
CPad* __cdecl CPad::GetPad()
{
    CPad* pads = GetPadsArray();
    const int32 idx = CPlayerInfo::ms_nLocalPlayerIndex;
    if (idx == -1)
        return &pads[2];

    CPlayerInfo* info = CPlayerInfo::ms_apPlayers[idx];
    if (!info || info->m_nFlags != 0 || !info->m_pPed || info->m_pPed->m_nUnkA80 == 1)
        return &pads[2];

    return &pads[0];
}

// 0x7C3950
CPad* __cdecl CPad::GetPadChecked(uint8 checkDisabled)
{
    CPad* pads = GetPadsArray();
    const int32 idx = CPlayerInfo::ms_nLocalPlayerIndex;
    if (idx == -1)
        return &pads[2];

    CPlayerInfo* info = CPlayerInfo::ms_apPlayers[idx];
    if (!info || (checkDisabled && info->m_nFlags != 0))
        return &pads[2];

    return &pads[0];
}

// 0x7C3990
CPad* __cdecl CPad::GetPlayerPad()
{
    return &GetPadsArray()[0];
}

// 0x7C39A0
CPad* __cdecl CPad::GetDebugPad()
{
    return &GetPadsArray()[3];
}
