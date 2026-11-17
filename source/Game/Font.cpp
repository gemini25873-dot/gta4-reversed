#include "StdInc.h"
#include "Game/Font.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstring>

namespace
{
    bool  (__cdecl* const kIsWideScreen)()           = &CFont::IsWideScreen;
    bool  (__cdecl* const kIsHiDef)()                = &CFont::IsHiDef;
    uint8 (__cdecl* const kIsHudPrefEnabled)()       = &CFont::IsHudPreferenceEnabled;
    int32 (__cdecl* const kSetRenderIndex)(int32)    = &CFont::SetRenderIndex;

    void __declspec(naked) HookedIsWideScreen()
    {
        __asm {
            push ecx
            push edx
            mov eax, kIsWideScreen
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedIsHiDef()
    {
        __asm {
            push ecx
            push edx
            mov eax, kIsHiDef
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedIsHudPreferenceEnabled()
    {
        __asm {
            push ecx
            push edx
            mov eax, kIsHudPrefEnabled
            call eax
            pop edx
            pop ecx
            ret
        }
    }

    void __declspec(naked) HookedSetRenderIndex()
    {
        __asm {
            push ecx
            push edx
            push dword ptr [esp + 12]
            mov eax, kSetRenderIndex
            call eax
            add esp, 4
            pop edx
            pop ecx
            ret
        }
    }
}

void CFont::InjectHooks()
{
    Core::Hook::Install(0xBB0130, &CFont::SafeStrlen);
    Core::Hook::Install(0xBB0210, &HookedIsWideScreen);
    Core::Hook::Install(0xBB0220, &HookedIsHiDef);
    Core::Hook::Install(0xBB29C0, &HookedIsHudPreferenceEnabled);
    Core::Hook::Install(0xBB0260, &CFont::SetSubtitles);
    Core::Hook::Install(0xBB03D0, &CFont::PrintStringAscii);
    Core::Hook::Install(0xBB06A0, &HookedSetRenderIndex);
    Core::Hook::Install(0xBB06B0, &CFont::SetScale);
    Core::Hook::Install(0xBB06E0, &CFont::SetFontStyle);
    Core::Hook::Install(0xBB0780, &CFont::SetDropShadowCount);
    Core::Hook::Install(0xBB07A0, &CFont::SetWrapEnd);
    Core::Hook::Install(0xBB07C0, &CFont::SetWrap);
    Core::Hook::Install(0xBB3F60, &CFont::SetColor);
    Core::Hook::Install(0xBB3F90, &CFont::SetDropColor);
    Core::Hook::Install(0xBB3FD0, &CFont::SetEdge);
    Core::Hook::Install(0xBB0880, &CFont::Thunk7FA570);
    Core::Hook::Install(0xBB0890, &CFont::Thunk7F9260);
    Core::Hook::Install(0xBB08A0, &CFont::Thunk7E7190);
    Core::Hook::Install(0xBB08B0, &CFont::Thunk7E71B0);

    Core::Hook::Install(0xBB4530, &CFont::Native_BB4530);
    Core::Hook::Install(0xBB4580, &CFont::Native_BB4580);
    Core::Hook::Install(0xBB45C0, &CFont::Native_IsWideScreen);
    Core::Hook::Install(0xBB45E0, &CFont::Native_IsHiDef);
    Core::Hook::Install(0xBB4600, &CFont::Native_SetFlag118034C);
    Core::Hook::Install(0xBB46D0, &CFont::Native_SetSubtitles);
    Core::Hook::Install(0xBB4700, &CFont::Native_SetFlag118BB3B);
    Core::Hook::Install(0xBB4740, &CFont::Native_BB4740);
    Core::Hook::Install(0xBB4760, &CFont::Native_SetScale);
    Core::Hook::Install(0xBB47A0, &CFont::Native_SetColor);
    Core::Hook::Install(0xBB47D0, &CFont::Native_SetCentre);
    Core::Hook::Install(0xBB4850, &CFont::Native_SetWrap);
    Core::Hook::Install(0xBB4890, &CFont::Native_SetBackground);
    Core::Hook::Install(0xBB48F0, &CFont::Native_BB48F0);
    Core::Hook::Install(0xBB4910, &CFont::Native_BB4910);
    Core::Hook::Install(0xBB4970, &CFont::Native_SetFlag118F0D0);
    Core::Hook::Install(0xBB49E0, &CFont::Native_SetEdge);
    Core::Hook::Install(0xBB4A10, &CFont::Native_BB4A10);
    Core::Hook::Install(0xBB4A30, &CFont::Native_BB4A30);
    Core::Hook::Install(0xBB4A50, &CFont::Native_BB4A50);
    Core::Hook::Install(0xBB4A90, &CFont::Native_BB4A90);
    Core::Hook::Install(0xBB4AD0, &CFont::Native_BB4AD0);
    Core::Hook::Install(0xBB4AF0, &CFont::Native_BB4AF0);
    Core::Hook::Install(0xBB4B30, &CFont::Native_BB4B30);
    Core::Hook::Install(0xBB4B50, &CFont::Native_BB4B50);
    Core::Hook::Install(0xBB4B70, &CFont::Native_BB4B70);
    Core::Hook::Install(0xBB4B90, &CFont::Native_BB4B90);
    Core::Hook::Install(0xBB4BC0, &CFont::Native_BB4BC0);
    Core::Hook::Install(0xBB4BE0, &CFont::Native_BB4BE0);
    Core::Hook::Install(0xBB4C00, &CFont::Native_BB4C00);
    Core::Hook::Install(0xBB4C50, &CFont::Native_BB4C50);
    Core::Hook::Install(0xBB4C80, &CFont::Native_BB4C80);
    Core::Hook::Install(0xBB4CA0, &CFont::Native_BB4CA0);
    Core::Hook::Install(0xBB4CC0, &CFont::Native_BB4CC0);
    Core::Hook::Install(0xBB4CE0, &CFont::Native_BB4CE0);
    Core::Hook::Install(0xBB4D00, &CFont::Native_BB4D00);
    Core::Hook::Install(0xBB4D20, &CFont::Native_BB4D20);
    Core::Hook::Install(0xBB4D90, &CFont::Native_BB4D90);
    Core::Hook::Install(0xBB4DB0, &CFont::Native_BB4DB0);
    Core::Hook::Install(0xBB4DD0, &CFont::Native_BB4DD0);
    Core::Hook::Install(0xBB4E20, &CFont::Native_BB4E20);
    Core::Hook::Install(0xBB4E50, &CFont::Native_BB4E50);
    Core::Hook::Install(0xBB4E80, &CFont::Native_BB4E80);
    Core::Hook::Install(0xBB4EB0, &CFont::Native_BB4EB0);
    Core::Hook::Install(0xBB4ED0, &CFont::Native_BB4ED0);
    Core::Hook::Install(0xBB4EF0, &CFont::Native_BB4EF0);
    Core::Hook::Install(0xBB4F10, &CFont::Native_BB4F10);
    Core::Hook::Install(0xBB4F30, &CFont::Native_BB4F30);
    Core::Hook::Install(0xBB4F50, &CFont::Native_BB4F50);
    Core::Hook::Install(0xBB4F80, &CFont::Native_BB4F80);
    Core::Hook::Install(0xBB4FC0, &CFont::Native_BB4FC0);
    Core::Hook::Install(0xBB4FE0, &CFont::Native_BB4FE0);

    Core::Log::Info("CFont hooks installed (71 functions)");
}

// 0xBB0130
uint32 __cdecl CFont::SafeStrlen(const char* str)
{
    return str ? static_cast<uint32>(std::strlen(str)) : 0u;
}

// 0xBB0210
bool __cdecl CFont::IsWideScreen()
{
    return Core::StaticRef<int32>(0x10C7A2C) != 0;
}

// 0xBB0220
bool __cdecl CFont::IsHiDef()
{
    return Core::StaticRef<int32>(0x10C7A30) != 0;
}

// 0xBB29C0
uint8 __cdecl CFont::IsHudPreferenceEnabled()
{
    const int32 mode = Core::StaticRef<int32>(0x10C7F80);
    if (mode == 2 || mode == 6)
        return 0;
    return Core::StaticRef<uint8>(0x10C7F6F);
}

// 0xBB0260
bool __cdecl CFont::SetSubtitles(uint8 enable, int16 a2, int16 a3)
{
    const bool b = (enable != 0);
    Core::StaticRef<uint8>(0x118034D) = b ? 1 : 0;
    Core::StaticRef<int16>(0x118BB3C) = a2;
    Core::StaticRef<int16>(0x118D5D8) = a3;
    return b;
}

// 0xBB03D0
int32 __cdecl CFont::PrintStringAscii(float x, float y, const char* text)
{
    const int32 wide = Core::Hook::Call<int32>(0xBAF1C0, text, 0);
    return Core::Hook::Call<int32>(0xBB02A0, x, y, wide);
}

// 0xBB06A0
int32 __cdecl CFont::SetRenderIndex(int32 idx)
{
    ms_nRenderIndex = idx;
    return idx;
}

// 0xBB06B0
int32 __cdecl CFont::SetScale(float x, float y)
{
    uint8* slot = GetSlotBase();
    *reinterpret_cast<float*>(slot + 0x04) = x;
    *reinterpret_cast<float*>(slot + 0x08) = y;
    return 124 * ms_nCurrentSlot;
}

// 0xBB06E0
int32 __cdecl CFont::SetFontStyle(uint8 style, uint8 unk)
{
    const int32 off = 124 * ms_nCurrentSlot;
    uint8* slot = GetSlotBase();
    slot[0x10] = style;
    slot[0x11] = unk;
    return off;
}

// 0xBB0780
int32 __cdecl CFont::SetDropShadowCount(int32 count)
{
    const int32 off = 124 * ms_nCurrentSlot;
    *reinterpret_cast<int32*>(GetSlotBase() + 0x28) = count;
    return off;
}

// 0xBB07A0
int32 __cdecl CFont::SetWrapEnd(float endX)
{
    const int32 off = 124 * ms_nCurrentSlot;
    *reinterpret_cast<float*>(GetSlotBase() + 0x24) = endX;
    return off;
}

// 0xBB07C0
int32 __cdecl CFont::SetWrap(float startX, float endX)
{
    uint8* slot = GetSlotBase();
    *reinterpret_cast<float*>(slot + 0x20) = startX;
    *reinterpret_cast<float*>(slot + 0x24) = endX;
    return 124 * ms_nCurrentSlot;
}

// 0xBB3F60
uint32 __cdecl CFont::SetColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    const uint32 packed = (static_cast<uint32>(a) << 24)
        | (static_cast<uint32>(r) << 16)
        | (static_cast<uint32>(g) << 8)
        | static_cast<uint32>(b);
    *reinterpret_cast<uint32*>(GetSlotBase() + 0x14) = packed;
    return packed;
}

// 0xBB3F90
uint32 __cdecl CFont::SetDropColor(uint8 count, uint8 r, uint8 g, uint8 b, uint8 a)
{
    const int32 off = 124 * ms_nCurrentSlot;
    const uint32 packed = (static_cast<uint32>(a) << 24)
        | (static_cast<uint32>(r) << 16)
        | (static_cast<uint32>(g) << 8)
        | static_cast<uint32>(b);
    uint8* slot = GetSlotBase();
    *reinterpret_cast<uint32*>(slot + 0x34) = packed;
    slot[0x38] = count;
    return static_cast<uint32>(off);
}

// 0xBB3FD0
uint32 __cdecl CFont::SetEdge(uint8 size, uint8 r, uint8 g, uint8 b, uint8 a)
{
    const int32 off = 124 * ms_nCurrentSlot;
    const uint32 packed = (static_cast<uint32>(a) << 24)
        | (static_cast<uint32>(r) << 16)
        | (static_cast<uint32>(g) << 8)
        | static_cast<uint32>(b);
    uint8* slot = GetSlotBase();
    *reinterpret_cast<uint32*>(slot + 0x34) = packed;
    slot[0x39] = size;
    return static_cast<uint32>(off);
}

// 0xBB0880
int32 __cdecl CFont::Thunk7FA570(int32 a1) { return Core::Hook::Call<int32>(0x7FA570, a1); }
// 0xBB0890
int32 __cdecl CFont::Thunk7F9260()         { return Core::Hook::Call<int32>(0x7F9260); }
// 0xBB08A0
int32 __cdecl CFont::Thunk7E7190(int32 a1) { return Core::Hook::Call<int32>(0x7E7190, a1); }
// 0xBB08B0
int32 __cdecl CFont::Thunk7E71B0(int32 a1) { return Core::Hook::Call<int32>(0x7E71B0, a1); }

// 0xBB4530
int32 __cdecl CFont::Native_BB4530(scrNativeCallContext* ctx)
{
    const uint8 res = Core::Hook::Call<uint8>(0xBAFE80, ctx->GetArg<int32>(0));
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4580
int32 __cdecl CFont::Native_BB4580(scrNativeCallContext* ctx)
{
    const uint8 res = Core::Hook::Call<uint8>(0xBAFEA0, ctx->GetArg<int32>(0));
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB45C0
int32 __cdecl CFont::Native_IsWideScreen(scrNativeCallContext* ctx)
{
    const int32 res = IsWideScreen() ? 1 : 0;
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB45E0
int32 __cdecl CFont::Native_IsHiDef(scrNativeCallContext* ctx)
{
    const int32 res = IsHiDef() ? 1 : 0;
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4600
bool __cdecl CFont::Native_SetFlag118034C(scrNativeCallContext* ctx)
{
    const bool b = (ctx->GetArg<int32>(0) != 0);
    Core::StaticRef<uint8>(0x118034C) = b ? 1 : 0;
    return b;
}

// 0xBB46D0
bool __cdecl CFont::Native_SetSubtitles(scrNativeCallContext* ctx)
{
    return SetSubtitles(
        ctx->GetArg<int32>(0) != 0 ? 1 : 0,
        static_cast<int16>(ctx->GetArg<int32>(1)),
        static_cast<int16>(ctx->GetArg<int32>(2)));
}

// 0xBB4700
bool __cdecl CFont::Native_SetFlag118BB3B(scrNativeCallContext* ctx)
{
    const bool b = (ctx->GetArg<int32>(0) != 0);
    Core::StaticRef<uint8>(0x118BB3B) = b ? 1 : 0;
    return b;
}

// 0xBB4740
int32 __cdecl CFont::Native_BB4740(scrNativeCallContext* ctx)
{
    const uint8 res = IsHudPreferenceEnabled();
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4760
int32 __cdecl CFont::Native_SetScale(scrNativeCallContext* ctx)
{
    return SetScale(ctx->GetArg<float>(0), ctx->GetArg<float>(1));
}

// 0xBB47A0
uint32 __cdecl CFont::Native_SetColor(scrNativeCallContext* ctx)
{
    return SetColor(
        static_cast<uint8>(ctx->GetArg<uint32>(0)),
        static_cast<uint8>(ctx->GetArg<uint32>(1)),
        static_cast<uint8>(ctx->GetArg<uint32>(2)),
        static_cast<uint8>(ctx->GetArg<uint32>(3)));
}

// 0xBB47D0
int32 __cdecl CFont::Native_SetCentre(scrNativeCallContext* ctx)
{
    const bool on = (ctx->GetArg<int32>(0) != 0);
    GetSlotBase()[0x18] = on ? 1 : 0;
    return 124 * ms_nCurrentSlot;
}

// 0xBB4850
int32 __cdecl CFont::Native_SetWrap(scrNativeCallContext* ctx)
{
    return SetWrap(ctx->GetArg<float>(0), ctx->GetArg<float>(1));
}

// 0xBB4890
int32 __cdecl CFont::Native_SetBackground(scrNativeCallContext* ctx)
{
    const bool on = (ctx->GetArg<int32>(0) != 0);
    GetSlotBase()[0x1D] = on ? 1 : 0;
    return 124 * ms_nCurrentSlot;
}

// 0xBB48F0
int32 __cdecl CFont::Native_BB48F0(scrNativeCallContext* ctx)
{
    return Thunk7FA570(ctx->GetArg<int32>(0));
}

// 0xBB4910
int32 __cdecl CFont::Native_BB4910(scrNativeCallContext* ctx)
{
    const uint8 res = static_cast<uint8>(Thunk7E7190(ctx->GetArg<int32>(0)));
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4970
bool __cdecl CFont::Native_SetFlag118F0D0(scrNativeCallContext* ctx)
{
    const bool b = (ctx->GetArg<int32>(0) != 0);
    Core::StaticRef<uint8>(0x118F0D0) = b ? 1 : 0;
    return b;
}

// 0xBB49E0
uint32 __cdecl CFont::Native_SetEdge(scrNativeCallContext* ctx)
{
    return SetEdge(
        static_cast<uint8>(ctx->GetArg<uint32>(0)),
        static_cast<uint8>(ctx->GetArg<uint32>(1)),
        static_cast<uint8>(ctx->GetArg<uint32>(2)),
        static_cast<uint8>(ctx->GetArg<uint32>(3)),
        static_cast<uint8>(ctx->GetArg<uint32>(4)));
}

// 0xBB4A10
int32 __cdecl CFont::Native_BB4A10(scrNativeCallContext* ctx)
{
    const int32 target = ctx->GetArg<int32>(0);
    const uint16 count = Core::StaticRef<uint16>(0x10F47E4);
    int32** list = Core::StaticRef<int32**>(0x10F47E0);

    for (uint16 i = 0; i < count; ++i)
    {
        int32* entry = list[i];
        if (entry && entry[335] == target)
        {
            const int32 val = entry[336];
            Core::StaticRef<int32>(0x1180320) = val;
            ms_nRenderIndex = 0;
            return val;
        }
    }

    ms_nRenderIndex = 0;
    return 0;
}

// 0xBB4A30
int32 __cdecl CFont::Native_BB4A30(scrNativeCallContext* ctx)
{
    const int32 val = ctx->GetArg<int32>(0);
    Core::StaticRef<int32>(0x1180320) = val;
    void* mgr = reinterpret_cast<void*>(Core::Rebase(0x1615844));
    const int32 res = Core::Hook::CallMethod<int32>(0x98E310, mgr, val);
    ms_nRenderIndex = 0;
    return res;
}

// 0xBB4A50
int32 __cdecl CFont::Native_BB4A50(scrNativeCallContext* ctx)
{
    const uint8 res = Core::Hook::Call<uint8>(0xBB15A0);
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4A90
int32 __cdecl CFont::Native_BB4A90(scrNativeCallContext* ctx)
{
    const bool enable = (ctx->GetArg<int32>(0) != 0);
    uint8* thread = Core::Hook::Call<uint8*>(0x804230);
    if (thread[153])
    {
        thread = Core::Hook::Call<uint8*>(0x804230);
        if (enable)
        {
            if (!thread[154])
            {
                ++Core::StaticRef<int32>(0x118BAEC);
                thread = Core::Hook::Call<uint8*>(0x804230);
                thread[154] = 1;
            }
        }
        else if (thread[154])
        {
            if (Core::StaticRef<int32>(0x118BAEC) > 0)
                --Core::StaticRef<int32>(0x118BAEC);
            thread = Core::Hook::Call<uint8*>(0x804230);
            thread[154] = 0;
        }
    }
    return reinterpret_cast<int32>(thread);
}

// 0xBB4AD0
int32 __cdecl CFont::Native_BB4AD0(scrNativeCallContext* ctx)
{
    const uint8 enable = (ctx->GetArg<int32>(0) != 0) ? 1 : 0;
    uint8* thread = Core::Hook::Call<uint8*>(0x804230);
    thread[152] = enable;
    return reinterpret_cast<int32>(thread);
}

// 0xBB4AF0
int32 __cdecl CFont::Native_BB4AF0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(
        0xBB4080,
        ctx->GetArg<float>(0),
        ctx->GetArg<float>(1),
        ctx->GetArg<float>(2),
        ctx->GetArg<float>(3),
        ctx->GetArg<int32>(4));
}

// 0xBB4B30
int32 __cdecl CFont::Native_BB4B30(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB1940, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1), ctx->GetArg<int32>(2));
}

// 0xBB4B50
int32 __cdecl CFont::Native_BB4B50(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1950, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4B70
int32 __cdecl CFont::Native_BB4B70(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1960, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4B90
int32 __cdecl CFont::Native_BB4B90(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(
        0xBB1970,
        ctx->GetArg<int32>(0),
        ctx->GetArg<int32>(1),
        ctx->GetArg<int32>(2) != 0);
}

// 0xBB4BC0
int32 __cdecl CFont::Native_BB4BC0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB1980, ctx->GetArg<int32>(0));
}

// 0xBB4BE0
int32 __cdecl CFont::Native_BB4BE0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB1D30, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4C00
int32 __cdecl CFont::Native_BB4C00(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB1D80, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1), ctx->GetArg<float>(2));
}

// 0xBB4C50
int32 __cdecl CFont::Native_BB4C50(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1DB0);
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4C80
int32 __cdecl CFont::Native_BB4C80(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1EE0, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4CA0
int32 __cdecl CFont::Native_BB4CA0(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1EF0, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4CC0
int32 __cdecl CFont::Native_BB4CC0(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1F30, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4CE0
int32 __cdecl CFont::Native_BB4CE0(scrNativeCallContext* ctx)
{
    const int32 res = Core::Hook::Call<int32>(0xBB1F70, ctx->GetArg<int32>(0));
    ctx->SetReturn<int32>(res);
    return res;
}

// 0xBB4D00
int32 __cdecl CFont::Native_BB4D00(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2060, ctx->GetArg<int32>(0));
}

// 0xBB4D20
int32 __cdecl CFont::Native_BB4D20(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(
        0xBB20C0,
        ctx->GetArg<int32>(0),
        ctx->GetArg<int32>(1),
        ctx->GetArg<int32>(2),
        ctx->GetArg<int32>(3));
}

// 0xBB4D90
int32 __cdecl CFont::Native_BB4D90(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2440, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4DB0
int32 __cdecl CFont::Native_BB4DB0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2420, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4DD0
int32 __cdecl CFont::Native_BB4DD0(scrNativeCallContext* ctx)
{
    const uint8 res = Core::Hook::Call<uint8>(0xBB2540, ctx->GetArg<int32>(0));
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4E20
int32 __cdecl CFont::Native_BB4E20(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2580, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1) != 0);
}

// 0xBB4E50
int32 __cdecl CFont::Native_BB4E50(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB25C0, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1) != 0);
}

// 0xBB4E80
int32 __cdecl CFont::Native_BB4E80(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB25E0, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1) != 0);
}

// 0xBB4EB0
int32 __cdecl CFont::Native_BB4EB0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2620, ctx->GetArg<int32>(0), ctx->GetArg<float>(1));
}

// 0xBB4ED0
int32 __cdecl CFont::Native_BB4ED0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2660, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4EF0
int32 __cdecl CFont::Native_BB4EF0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2640, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4F10
int32 __cdecl CFont::Native_BB4F10(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2680, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4F30
int32 __cdecl CFont::Native_BB4F30(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB26A0, ctx->GetArg<int32>(0));
}

// 0xBB4F50
int32 __cdecl CFont::Native_BB4F50(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2710, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1) != 0);
}

// 0xBB4F80
int32 __cdecl CFont::Native_BB4F80(scrNativeCallContext* ctx)
{
    const uint8 res = Core::Hook::Call<uint8>(0xBB2770, ctx->GetArg<int32>(0));
    ctx->SetReturn<uint32>(res);
    return res;
}

// 0xBB4FC0
int32 __cdecl CFont::Native_BB4FC0(scrNativeCallContext* ctx)
{
    return Core::Hook::Call<int32>(0xBB2750, ctx->GetArg<int32>(0), ctx->GetArg<int32>(1));
}

// 0xBB4FE0
bool __cdecl CFont::Native_BB4FE0(scrNativeCallContext* ctx)
{
    const bool b = (ctx->GetArg<int32>(0) != 0);
    Core::StaticRef<uint8>(0x11DE0D5) = b ? 1 : 0;
    return b;
}
