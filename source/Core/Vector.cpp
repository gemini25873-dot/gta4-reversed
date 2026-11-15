#include "StdInc.h"
#include "Core/Vector.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>

void CVector::InjectHooks()
{
    Core::Hook::Install(0x42F570, &CVector::InvMagnitude);
    Core::Hook::Install(0x42F5D0, &CVector::Normalise);

    Core::Log::Info("CVector hooks yuklendi (2 fonksiyon)");
}

double __thiscall CVector::InvMagnitude()
{
    const float magSq = x * x + y * y + z * z;
    if (magSq == 0.0f)
        return 0.0;

    return 1.0 / std::sqrt(magSq);
}

void __thiscall CVector::Normalise()
{
    const float magSq = x * x + y * y + z * z;
    const float invMag = (magSq == 0.0f) ? 0.0f : static_cast<float>(1.0 / std::sqrt(magSq));
    x *= invMag;
    y *= invMag;
    z *= invMag;
}
