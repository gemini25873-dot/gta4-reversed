#include "StdInc.h"
#include "Game/Radar.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstdio>
#include <intrin.h>

void CRadar::InjectHooks()
{
    Core::Hook::Install(0x80AA30, &CRadar::AssignBlipHandle);
    Core::Hook::Install(0x80AA80, &CRadar::ConvertUniqueBlipToActualBlip);
    Core::Hook::Install(0x80E600, &CRadar::IsBlipIdInUse);
    Core::Hook::Install(0x80E620, &CRadar::RemoveBlip);
    Core::Hook::Install(0x80E720, &CRadar::SetBlipParameterFloat);
    Core::Hook::Install(0x80E770, &CRadar::SetBlipVectorParam);
    Core::Hook::Install(0x810DC0, &CRadar::SetBlipParameter);

    Core::Hook::Install(0xBB1DB0, &CRadar::GetSimpleBlipHandle);
    Core::Hook::Install(0xBB1EF0, &CRadar::GetBlipCategory);
    Core::Hook::Install(0xBB1F30, &CRadar::GetBlipType);
    Core::Hook::Install(0xBB1F70, &CRadar::GetBlipSprite);
    Core::Hook::Install(0xBB20C0, &CRadar::SetBlipCoordinates);
    Core::Hook::Install(0xBB2420, &CRadar::SetBlipColour);
    Core::Hook::Install(0xBB2440, &CRadar::SetBlipDisplay);
    Core::Hook::Install(0xBB2540, &CRadar::IsBlipFlag8Set);
    Core::Hook::Install(0xBB2580, &CRadar::SetBlipFlashes);
    Core::Hook::Install(0xBB25C0, &CRadar::SetBlipShortRange);
    Core::Hook::Install(0xBB25E0, &CRadar::SetBlipFriendly);
    Core::Hook::Install(0xBB2620, &CRadar::SetBlipScale);
    Core::Hook::Install(0xBB2640, &CRadar::SetBlipPriority);
    Core::Hook::Install(0xBB2660, &CRadar::SetBlipDisplayMode);
    Core::Hook::Install(0xBB2680, &CRadar::SetBlipSprite);
    Core::Hook::Install(0xBB2710, &CRadar::SetBlipRoute);
    Core::Hook::Install(0xBB2750, &CRadar::SetBlipMinimalClean);
    Core::Hook::Install(0xBB2770, &CRadar::DoesBlipExist);

    Core::Log::Info("CRadar hooks installed (25 functions)");
}

// 0x80AA30
int32 __cdecl CRadar::AssignBlipHandle(int32 actualIndex)
{
    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (!trace)
        return -1;

    const uint16 nextRef = (ms_nNextBlipRef >= 0xFFFEu) ? 1u : static_cast<uint16>(ms_nNextBlipRef + 1u);
    ms_nNextBlipRef = nextRef;
    trace->m_nReferenceIndex = nextRef;
    return actualIndex | (static_cast<int32>(trace->m_nReferenceIndex) << 16);
}

// 0x80AA80
int32 __cdecl CRadar::ConvertUniqueBlipToActualBlip(int32 blipHandle)
{
    if (blipHandle == -1)
        return -1;

    const uint16 actualIndex = static_cast<uint16>(blipHandle & 0xFFFF);
    if (actualIndex > 0x5DBu)
        return -1;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (!trace || static_cast<uint16>(static_cast<uint32>(blipHandle) >> 16) != trace->m_nReferenceIndex)
        return -1;

    return actualIndex;
}

// 0x80E600
bool __cdecl CRadar::IsBlipIdInUse(int32 blipHandle)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    return actualIndex >= 0 && ms_RadarTrace[actualIndex] != nullptr;
}

// 0x80E620
uint8 __cdecl CRadar::RemoveBlip(int32 index, uint8 isScriptHandle)
{
    const int32 actualIndex = isScriptHandle ? ConvertUniqueBlipToActualBlip(index) : index;
    if (actualIndex < 0)
        return 0;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (!trace)
        return 0;

    trace->m_nDisplay = 0;
    Core::Hook::Call<void>(0x86EE20, actualIndex);

    const uint32 tlsIdx = Core::StaticRef<uint32>(0x17237B0);
    auto** tlsArray = reinterpret_cast<void***>(__readfsdword(0x2C));
    if (tlsArray[tlsIdx][2])
        Core::Hook::Call<void>(0x5B1C10, ms_RadarTrace[actualIndex]);

    ms_RadarTrace[actualIndex] = nullptr;
    return 1;
}

// 0x80E720
int32 __cdecl CRadar::SetBlipParameterFloat(int32 paramId, int32 blipHandle, float value)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex < 0)
        return actualIndex;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (!trace)
        return 0;

    if (paramId == 0)
    {
        if (trace->m_bNotSimple)
            trace->m_fScale = value;
    }
    else if (paramId == 16)
    {
        if (trace->m_bNotSimple)
            trace->m_fRotation = value;
    }

    return reinterpret_cast<int32>(trace);
}

// 0x80E770
int32 __cdecl CRadar::SetBlipVectorParam(int32 paramId, int32 blipHandle, const CVector* vec)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex >= 0 && ms_RadarTrace[actualIndex] && paramId == 2)
        return Core::Hook::Call<int32>(0x442100, actualIndex, vec);

    return actualIndex;
}

// 0x810DC0
uint8 __cdecl CRadar::SetBlipParameter(int32 paramId, int32 blipHandle, int32 value)
{
    const int32 idx = ConvertUniqueBlipToActualBlip(blipHandle);
    if (idx < 0 || !ms_RadarTrace[idx])
        return static_cast<uint8>(idx);

    uint8 result = static_cast<uint8>(paramId - 1);
    switch (paramId)
    {
    case 1:
        return Core::Hook::Call<uint8>(0x80A220, idx, value);

    case 3:
    {
        Core::Hook::Call<void>(0x4420E0, idx, ms_RadarBlipSprites[value]);
        if (value < ms_nEpisodeBlipStart || value > ms_nEpisodeBlipStart + 11)
            Core::Hook::Call<void>(0x80B9D0, idx, 128);
        else
            Core::Hook::Call<void>(0x80A150, idx, 128);

        Core::Hook::Call<void>(0x80A200, idx, (value == ms_nActiveLevel) ? 2 : 1);

        const uint32 tlsIdx = Core::StaticRef<uint32>(0x17237B0);
        auto** tlsArray = reinterpret_cast<uint8***>(__readfsdword(0x2C));
        char* scratch = reinterpret_cast<char*>(tlsArray[tlsIdx]) + 1232;
        std::sprintf(scratch, "BLIP_%d", value);
        Core::Hook::Call<void>(0x80F980, idx, scratch);

        const int32* info = Core::Hook::Call<const int32*>(0x441FC0, idx);
        if (info[1] > ms_nActiveLevel + 4)
        {
            result = Core::Hook::Call<uint8>(0x441F60, idx, 16);
            if (!result)
                return Core::Hook::Call<uint8>(0x80A1E0, idx, 0);
        }
        return result;
    }

    case 4:
        return RemoveBlip(idx, 0);

    case 5:
    {
        Core::Hook::Call<void>(0x80A1E0, idx, value);
        if (Core::Hook::Call<uint8>(0x441F60, idx, 16))
        {
            const uint8 f2 = Core::Hook::Call<uint8>(0x441F60, idx, 2);
            const int32 c = Core::Hook::Call<int32>(0x80A180, idx);
            int32 colOut = 0;
            Core::Hook::Call<void>(0x80E7B0, &colOut, c, static_cast<int32>(f2), 0);
            Core::Hook::Call<void>(0x86EE50, idx, colOut);
        }
        const int32 t = Core::Hook::Call<int32>(0x442030, idx);
        result = static_cast<uint8>(t);
        if (t == 2 && value == 3)
            result = Core::Hook::Call<uint8>(0x80F980, idx, "BLIP_FRIEND");
        return result;
    }

    case 6:
        return Core::Hook::Call<uint8>(0x80A240, idx, value);

    case 7:
    {
        Core::Hook::Call<void>(0x80A1E0, idx, 42);
        if (value)
        {
            Core::Hook::Call<void>(0x80A150, idx, 2);
            if (Core::Hook::Call<int32>(0x442030, idx) == 2)
                Core::Hook::Call<void>(0x80F980, idx, "BLIP_FRIEND");
        }
        else
        {
            Core::Hook::Call<void>(0x80B9D0, idx, 2);
            if (Core::Hook::Call<int32>(0x442030, idx) == 2)
                Core::Hook::Call<void>(0x80F980, idx, "BLIP_ENEMY");
        }
        result = Core::Hook::Call<uint8>(0x441F60, idx, 16);
        if (result)
        {
            const uint8 f2 = Core::Hook::Call<uint8>(0x441F60, idx, 2);
            const int32 c = Core::Hook::Call<int32>(0x80A180, idx);
            int32 colOut = 0;
            Core::Hook::Call<void>(0x80E7B0, &colOut, c, static_cast<int32>(f2), 0);
            result = Core::Hook::Call<uint8>(0x86EE50, idx, colOut);
        }
        return result;
    }

    case 8:
    {
        if (value)
        {
            for (int32 i = 0; i < 1500; ++i)
            {
                if (i != ms_iSimpleBlip && ms_RadarTrace[i])
                    Core::Hook::Call<void>(0x80B9D0, i, 16);
            }
            Core::Hook::Call<void>(0x80A150, idx, 16);
        }
        else
        {
            Core::Hook::Call<void>(0x80B9D0, idx, 16);
        }

        if (Core::Hook::Call<uint8>(0x441F60, idx, 16))
        {
            const uint8 f2 = Core::Hook::Call<uint8>(0x441F60, idx, 2);
            const int32 c = Core::Hook::Call<int32>(0x80A180, idx);
            int32 colOut = 0;
            Core::Hook::Call<void>(0x80E7B0, &colOut, c, static_cast<int32>(f2), 0);
            uint8 nameBuf[16]{};
            Core::Hook::Call<void>(0x441FF0, nameBuf, idx);
            const int32 strPtr = Core::Hook::Call<int32>(0x80EAF0, idx);
            return Core::Hook::Call<uint8>(0x86F600, nameBuf, strPtr, idx, colOut);
        }

        Core::Hook::Call<void>(0x86EE20, idx);
        const int32* info = Core::Hook::Call<const int32*>(0x441FC0, idx);
        if (info[1] > ms_nActiveLevel + 4)
            return Core::Hook::Call<uint8>(0x80A1E0, idx, 0);
        return static_cast<uint8>(reinterpret_cast<uintptr_t>(info));
    }

    case 9:
        return value ? Core::Hook::Call<uint8>(0x80A150, idx, 2) : Core::Hook::Call<uint8>(0x80B9D0, idx, 2);

    case 11:
        if (!value)
        {
            Core::Hook::Call<void>(0x80B9D0, idx, 4);
            return Core::Hook::Call<uint8>(0x80B9D0, idx, 256);
        }
        Core::Hook::Call<void>(0x80A150, idx, 4);
        return Core::Hook::Call<uint8>(0x80B9D0, idx, 256);

    case 12:
        if (value)
        {
            Core::Hook::Call<void>(0x80A150, idx, 256);
            return Core::Hook::Call<uint8>(0x80B9D0, idx, 4);
        }
        Core::Hook::Call<void>(0x80B9D0, idx, 4);
        return Core::Hook::Call<uint8>(0x80B9D0, idx, 256);

    case 13:
        return value ? Core::Hook::Call<uint8>(0x80A150, idx, 8) : Core::Hook::Call<uint8>(0x80B9D0, idx, 8);

    case 14:
        return value ? Core::Hook::Call<uint8>(0x80A150, idx, 512) : Core::Hook::Call<uint8>(0x80B9D0, idx, 512);

    case 15:
        return Core::Hook::Call<uint8>(0x80A200, idx, value);

    case 19:
        return value ? Core::Hook::Call<uint8>(0x80A150, idx, 1024) : Core::Hook::Call<uint8>(0x80B9D0, idx, 1024);

    default:
        return result;
    }
}

// 0xBB1DB0
int32 __cdecl CRadar::GetSimpleBlipHandle()
{
    return AssignBlipHandle(ms_iSimpleBlip);
}

// 0xBB1EF0
int32 __cdecl CRadar::GetBlipCategory(int32 blipHandle)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex == -1)
        return 0;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (trace->m_bNotSimple)
        return trace->m_nBlipCategory;

    return ms_RadarTrace[ms_iSimpleBlip]->m_nBlipCategory;
}

// 0xBB1F30
int32 __cdecl CRadar::GetBlipType(int32 blipHandle)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex == -1)
        return 0;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (trace->m_bNotSimple)
        return trace->m_nBlipType;

    return ms_RadarTrace[ms_iSimpleBlip]->m_nBlipType;
}

// 0xBB1F70
int32 __cdecl CRadar::GetBlipSprite(int32 blipHandle)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex == -1)
        return -1;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (!trace->m_bNotSimple)
        return -1;

    return trace->m_nSprite;
}

// 0xBB20C0
int32 __cdecl CRadar::SetBlipCoordinates(int32 blipHandle, float x, float y, float z)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex < 0)
        return actualIndex;

    const int32 blipType = GetBlipType(blipHandle);
    if (blipType == 4 || blipType == 5 || blipType == 7)
    {
        CVector pos{ x, y, z };
        return SetBlipVectorParam(2, blipHandle, &pos);
    }
    return blipType;
}

// 0xBB2420
int32 __cdecl CRadar::SetBlipColour(int32 blipHandle, int32 colour)
{
    return SetBlipParameter(5, blipHandle, colour);
}

// 0xBB2440
int32 __cdecl CRadar::SetBlipDisplay(int32 blipHandle, int32 display)
{
    return SetBlipParameter(6, blipHandle, display);
}

// 0xBB2540
bool __cdecl CRadar::IsBlipFlag8Set(int32 blipHandle)
{
    const int32 actualIndex = ConvertUniqueBlipToActualBlip(blipHandle);
    if (actualIndex < 0)
        return false;

    sRadarTrace* trace = ms_RadarTrace[actualIndex];
    if (trace->m_bNotSimple)
        return (trace->m_nFlags & 8u) != 0;

    return (ms_RadarTrace[ms_iSimpleBlip]->m_nFlags & 8u) != 0;
}

// 0xBB2580
int32 __cdecl CRadar::SetBlipFlashes(int32 blipHandle, uint8 on)
{
    return SetBlipParameter(11, blipHandle, on);
}

// 0xBB25C0
int32 __cdecl CRadar::SetBlipShortRange(int32 blipHandle, uint8 on)
{
    return SetBlipParameter(12, blipHandle, on);
}

// 0xBB25E0
int32 __cdecl CRadar::SetBlipFriendly(int32 blipHandle, uint8 on)
{
    return SetBlipParameter(13, blipHandle, on);
}

// 0xBB2620
int32 __cdecl CRadar::SetBlipScale(int32 blipHandle, float scale)
{
    ConvertUniqueBlipToActualBlip(blipHandle);
    return SetBlipParameterFloat(0, blipHandle, scale);
}

// 0xBB2640
int32 __cdecl CRadar::SetBlipPriority(int32 blipHandle, int32 priority)
{
    return SetBlipParameter(1, blipHandle, priority);
}

// 0xBB2660
int32 __cdecl CRadar::SetBlipDisplayMode(int32 blipHandle, int32 mode)
{
    return SetBlipParameter(15, blipHandle, (mode == 3) ? 2 : mode);
}

// 0xBB2680
int32 __cdecl CRadar::SetBlipSprite(int32 blipHandle, int32 sprite)
{
    return SetBlipParameter(3, blipHandle, sprite);
}

// 0xBB2710
int32 __cdecl CRadar::SetBlipRoute(int32 blipHandle, uint8 on)
{
    return SetBlipParameter(7, blipHandle, on);
}

// 0xBB2750
int32 __cdecl CRadar::SetBlipMinimalClean(int32 blipHandle, int32 value)
{
    return SetBlipParameter(10, blipHandle, value);
}

// 0xBB2770
int32 __cdecl CRadar::DoesBlipExist(int32 blipHandle)
{
    return IsBlipIdInUse(blipHandle) ? 1 : 0;
}
