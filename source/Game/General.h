#pragma once

class CGeneral
{
public:
    static void InjectHooks();

    static int32 __cdecl GetRandomNumberInRange(int32 min, int32 max);
    static double __cdecl GetRandomNumberInRange(float min, float max);
    static double __cdecl Sqrt(float val);
    static double __cdecl InvSqrt(float val);
};
