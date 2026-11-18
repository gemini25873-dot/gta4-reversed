#include "StdInc.h"
#include "Game/General.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>
#include <cstdlib>

void CGeneral::InjectHooks()
{
    std::srand(GetTickCount());
    Core::Hook::Install(0x401950, static_cast<int32(__cdecl*)(int32, int32)>(&CGeneral::GetRandomNumberInRange));
    Core::Hook::Install(0x438960, static_cast<double(__cdecl*)(float, float)>(&CGeneral::GetRandomNumberInRange));
    Core::Hook::Install(0x404B80, &CGeneral::Sqrt);
    Core::Hook::Install(0x42F540, &CGeneral::InvSqrt);

    Core::Log::Info("CGeneral hooks installed (4 functions)");
}

int32 __cdecl CGeneral::GetRandomNumberInRange(int32 min, int32 max)
{
    const float r = static_cast<float>(static_cast<uint16>(std::rand())) * (1.0f / 32768.0f);
    return min + static_cast<int32>(r * static_cast<float>(max - min));
}

double __cdecl CGeneral::GetRandomNumberInRange(float min, float max)
{
    const double r = static_cast<double>(std::rand()) * (1.0 / 32767.0);
    return min + (max - min) * r;
}

double __cdecl CGeneral::Sqrt(float val)
{
    return std::sqrt(val);
}

double __cdecl CGeneral::InvSqrt(float val)
{
    if (val == 0.0f)
        return 0.0;

    return 1.0 / std::sqrt(val);
}
