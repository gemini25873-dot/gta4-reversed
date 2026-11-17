#pragma once

struct scrNativeCallContext
{
    uint32* m_pReturn = nullptr;   // +0x00
    uint32  m_nArgCount = 0;       // +0x04
    uint32* m_pArgs = nullptr;     // +0x08

    template<typename T>
    T GetArg(uint32 idx) const
    {
        return *reinterpret_cast<const T*>(&m_pArgs[idx]);
    }

    template<typename T>
    void SetReturn(T val)
    {
        *reinterpret_cast<T*>(m_pReturn) = val;
    }
};

class CFont
{
public:
    static inline int32& ms_nCurrentSlot = Core::StaticRef<int32>(0x118D5A0);
    static inline int32& ms_nRenderIndex = Core::StaticRef<int32>(0x118F0CC);

    static uint8* GetSlotBase()
    {
        return reinterpret_cast<uint8*>(Core::Rebase(0x1196F00)) + 124 * ms_nCurrentSlot;
    }

    static void InjectHooks();

    // Core CFont state & helpers (19)
    static uint32 __cdecl SafeStrlen(const char* str);
    static bool   __cdecl IsWideScreen();
    static bool   __cdecl IsHiDef();
    static uint8  __cdecl IsHudPreferenceEnabled();
    static bool   __cdecl SetSubtitles(uint8 enable, int16 a2, int16 a3);
    static int32  __cdecl PrintStringAscii(float x, float y, const char* text);
    static int32  __cdecl SetRenderIndex(int32 idx);
    static int32  __cdecl SetScale(float x, float y);
    static int32  __cdecl SetFontStyle(uint8 style, uint8 unk);
    static int32  __cdecl SetDropShadowCount(int32 count);
    static int32  __cdecl SetWrapEnd(float endX);
    static int32  __cdecl SetWrap(float startX, float endX);
    static uint32 __cdecl SetColor(uint8 r, uint8 g, uint8 b, uint8 a);
    static uint32 __cdecl SetDropColor(uint8 count, uint8 r, uint8 g, uint8 b, uint8 a);
    static uint32 __cdecl SetEdge(uint8 size, uint8 r, uint8 g, uint8 b, uint8 a);
    static int32  __cdecl Thunk7FA570(int32 a1);
    static int32  __cdecl Thunk7F9260();
    static int32  __cdecl Thunk7E7190(int32 a1);
    static int32  __cdecl Thunk7E71B0(int32 a1);

    // Native wrappers 0xBB4530..0xBB4FE0 (52)
    static int32  __cdecl Native_BB4530(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4580(scrNativeCallContext* ctx);
    static int32  __cdecl Native_IsWideScreen(scrNativeCallContext* ctx);
    static int32  __cdecl Native_IsHiDef(scrNativeCallContext* ctx);
    static bool   __cdecl Native_SetFlag118034C(scrNativeCallContext* ctx);
    static bool   __cdecl Native_SetSubtitles(scrNativeCallContext* ctx);
    static bool   __cdecl Native_SetFlag118BB3B(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4740(scrNativeCallContext* ctx);
    static int32  __cdecl Native_SetScale(scrNativeCallContext* ctx);
    static uint32 __cdecl Native_SetColor(scrNativeCallContext* ctx);
    static int32  __cdecl Native_SetCentre(scrNativeCallContext* ctx);
    static int32  __cdecl Native_SetWrap(scrNativeCallContext* ctx);
    static int32  __cdecl Native_SetBackground(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB48F0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4910(scrNativeCallContext* ctx);
    static bool   __cdecl Native_SetFlag118F0D0(scrNativeCallContext* ctx);
    static uint32 __cdecl Native_SetEdge(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4A10(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4A30(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4A50(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4A90(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4AD0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4AF0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4B30(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4B50(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4B70(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4B90(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4BC0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4BE0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4C00(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4C50(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4C80(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4CA0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4CC0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4CE0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4D00(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4D20(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4D90(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4DB0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4DD0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4E20(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4E50(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4E80(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4EB0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4ED0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4EF0(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4F10(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4F30(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4F50(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4F80(scrNativeCallContext* ctx);
    static int32  __cdecl Native_BB4FC0(scrNativeCallContext* ctx);
    static bool   __cdecl Native_BB4FE0(scrNativeCallContext* ctx);
};
