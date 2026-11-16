#include "StdInc.h"
#include "Weather.h"
#include "Clock.h"
#include "Core/Hook.h"
#include "Core/Log.h"

#include <cmath>

void CWeather::InjectHooks() {
    Core::Hook::Install(0x9E4100, &CWeather::CalcNightFactor);
    Core::Hook::Install(0x9E4250, &CWeather::GetWeatherCycle);
    Core::Hook::Install(0x9E4270, &CWeather::ProcessPlayerWetness);
    Core::Hook::Install(0x9E4490, &CWeather::Update);
    Core::Hook::Install(0x9E42A0, &CWeather::AdvanceWeather);
    Core::Log::Info("CWeather hooks installed (5 functions).");
}

// 0x9E4100
float __cdecl CWeather::CalcNightFactor() {
    const float minutes = static_cast<float>(
        CClock::ms_nGameClockMinutes + 60 * static_cast<uint16>(CClock::ms_nGameClockHours)
    ) + static_cast<float>(CClock::ms_nGameClockSeconds) * 0.016666668f;

    if (minutes < 360.0f) {
        return 1.0f;
    }
    if (minutes < 420.0f) {
        return (420.0f - minutes) * 0.016666668f;
    }
    if (minutes < 1140.0f) {
        return 0.0f;
    }
    if (minutes >= 1200.0f) {
        return 1.0f;
    }
    return 1.0f - (1200.0f - minutes) * 0.016666668f;
}

// 0x9E4250
const int8* __cdecl CWeather::GetWeatherCycle() {
    if (Core::StaticRef<int32>(0x10F8044) != 2) {
        return reinterpret_cast<const int8*>(Core::Rebase(0xF34488));
    }
    return reinterpret_cast<const int8*>(Core::Rebase(0xF34508));
}

// 0x9E4270
void __cdecl CWeather::ProcessPlayerWetness() {
    if (UnderWaterness <= 0.0f) {
        if (void* ped = Core::Hook::Call<void*>(0x878890, 0)) {
            Core::Hook::CallMethod<bool>(0x9C5FD0, ped);
        }
    }
}

// 0x9E42A0
int32 __cdecl CWeather::AdvanceWeather() {
    const int32 current = (InterpolationValue >= 0.5f) ? NewWeatherType : OldWeatherType;
    const int32 target  = (current + 1) % 8;
    const auto* cycle   = reinterpret_cast<const int8*>(Core::Rebase(0xF34488));

    int32 idx   = WeatherTypeList;
    int32 steps = 0;
    while (true) {
        if (cycle[(idx + 127) % 128] == target && cycle[idx] == target) {
            break;
        }
        idx = (idx + 1) % 128;
        ++steps;
        if (steps > 128) {
            break;
        }
    }
    WeatherTypeList = idx;

    if (steps > 128) {
        const int32 offset = (InterpolationValue < 0.5f) ? -1 : 0;
        for (int32 i = 0; i <= 128; ++i) {
            if (cycle[(offset + idx + 128) % 128] == target) {
                break;
            }
            idx = (idx + 1) % 128;
        }
        WeatherTypeList = idx;
    }

    const int32 prevIdx = (idx + 127) % 128;
    const int32 result  = cycle[idx];
    OldWeatherType      = cycle[prevIdx];
    NewWeatherType      = result;
    return result;
}

// 0x9E4490
void __cdecl CWeather::Update() {
    const int32 minutes = CClock::GetGameClockMinutes();
    const float interp  = (static_cast<float>(CClock::ms_nGameClockSeconds) * 0.016666668f + static_cast<float>(minutes)) * 0.016666668f;

    if (!Core::Hook::Call<bool>(0x462F90) || Core::Hook::Call<bool>(0x463000)) {
        if (InterpolationValue > interp) {
            OldWeatherType = NewWeatherType;
            if (ForcedWeatherType >= 0) {
                NewWeatherType = ForcedWeatherType;
            } else {
                WeatherTypeList = (WeatherTypeList + 1) % 128;
                NewWeatherType  = GetWeatherCycle()[WeatherTypeList];
            }
        }
        InterpolationValue = interp;
    } else if (interp > InterpolationValue) {
        InterpolationValue = interp;
    }

    const uint32 timeMs       = Core::StaticRef<uint32>(0x11DDE74);
    const uint32 frameCounter = Core::StaticRef<uint32>(0x11DDF38);

    Thunder = 0.0f;
    if (NewWeatherType == 7) {
        Thunder = InterpolationValue;
    }
    if (OldWeatherType == 7) {
        Thunder += 1.0f - InterpolationValue;
    }

    if (Thunder <= 0.99000001f || UnderWaterness > 0.0f) {
        LightningBurst = false;
        LightningFlash = false;
    } else if (!LightningFlash) {
        if (static_cast<uint16>(Core::Hook::Call<int32>(0xD0D21A)) >= 0x14u || Core::StaticRef<uint8>(0x1690C11) != 0) {
            LightningBurst = false;
        } else {
            LightningStartFrame    = frameCounter;
            LightningFlash         = true;
            LightningBurst         = true;
            LightningFlashLastTime = timeMs;
        }
    } else if (static_cast<uint8>(Core::Hook::Call<int32>(0xD0D21A)) < 0x10u) {
        LightningFlash = false;
        uint32 dur = 20;
        if (frameCounter - LightningStartFrame <= 0x14u) {
            dur = frameCounter - LightningStartFrame;
        }
        LightningDuration = static_cast<int32>(dur);
        NextThunderTime   = timeMs + 150u * (20u - dur);
        LightningBurst    = false;
    } else if (timeMs - LightningFlashLastTime > 0x32u) {
        const bool prev = LightningBurst;
        const bool next = (Core::Hook::Call<int32>(0xD0D21A) & 1) != 0;
        LightningBurst  = next;
        if (prev != next) {
            LightningFlashLastTime = timeMs;
        }
    }

    if (NextThunderTime != 0 && timeMs > NextThunderTime) {
        Core::Hook::Call<int32>(0xAE9580);
        Core::Hook::Call<void>(0x7C3B90, 40 * LightningDuration + 70, static_cast<float>(LightningDuration) * 0.2f + 0.1f, 0);
        NextThunderTime = 0;
    }

    const auto isRainy = [](int32 weather) {
        return weather == 4 || weather == 5 || weather == 7;
    };

    if (isRainy(OldWeatherType)) {
        Rain = isRainy(NewWeatherType) ? 1.0f : (1.0f - InterpolationValue);
    } else {
        Rain = isRainy(NewWeatherType) ? InterpolationValue : 0.0f;
    }

    const double invInterp      = 1.0 - static_cast<double>(InterpolationValue);
    const float  invInterpFloat = static_cast<float>(invInterp);
    const float  step           = Core::StaticRef<float>(0x11DDE7C) * 0.1f;

    const auto* wetTable  = reinterpret_cast<const float*>(Core::Rebase(0xF34430));
    const auto* fogTable  = reinterpret_cast<const float*>(Core::Rebase(0xF3445C));
    const auto* windTable = reinterpret_cast<const float*>(Core::Rebase(0xF34404));

    const float targetWet = static_cast<float>(
        (static_cast<double>(InterpolationValue * wetTable[NewWeatherType]) + invInterp * static_cast<double>(wetTable[OldWeatherType]))
        * (static_cast<double>((timeMs >> 13) & 3u) * 0.1 + 0.69999999)
    );
    const float wetDiff = targetWet - WetRoads;
    if (step <= std::fabs(wetDiff)) {
        WetRoads += (wetDiff <= 0.0f) ? -step : step;
    } else {
        WetRoads = targetWet;
    }

    Unused504     = 0;
    CloudCoverage = (OldWeatherType >= 2 ? invInterpFloat : 0.0f) + (NewWeatherType >= 2 ? InterpolationValue : 0.0f);
    Foggyness     = fogTable[NewWeatherType] * InterpolationValue + fogTable[OldWeatherType] * invInterpFloat;
    Sun           = (OldWeatherType <= 2 ? invInterpFloat : 0.0f) + (NewWeatherType <= 2 ? InterpolationValue : 0.0f);
    Wind          = windTable[NewWeatherType] * InterpolationValue + windTable[OldWeatherType] * invInterpFloat;

    if (ForcedWind >= 0.0f) {
        Wind = ForcedWind;
    }

    const uintptr_t camBase   = Core::Hook::Call<uintptr_t>(0x817F40);
    const float     camHeight = *reinterpret_cast<const float*>(*reinterpret_cast<const uintptr_t*>(camBase + 32) + 56);
    if (camHeight > 50.0f) {
        if (camHeight <= 200.0f) {
            Wind *= ((camHeight - 50.0f) * 0.0066666668f + 1.0f);
        } else {
            Wind *= 2.0f;
        }
    }

    if (Wind >= 1.0f) {
        Wind = 1.0f;
    }
    WindClipped = Core::StaticRef<float>(0xF3C5D4) * Wind;

    if (1.0f - UnderWaterness <= WetRoads) {
        WetRoads = 1.0f - UnderWaterness;
    }

    const int32 hours    = CClock::GetGameClockHours();
    float       darkness = 1.0f;
    if (hours > 20) {
        darkness = 1.0f;
    } else if (hours > 19) {
        darkness = static_cast<float>(minutes) * 0.016666668f;
    } else if (hours > 6) {
        darkness = 0.0f;
    } else if (hours > 5) {
        darkness = 1.0f - static_cast<float>(minutes) * 0.016666668f;
    }

    if (darkness <= Rain) {
        darkness = Rain;
    }
    if (darkness <= Foggyness) {
        darkness = Foggyness;
    }
    if (darkness <= WetRoads) {
        darkness = WetRoads;
    }
    StreetLightIntensity = darkness;

    ProcessPlayerWetness();
    NightFactor = CalcNightFactor();
}
