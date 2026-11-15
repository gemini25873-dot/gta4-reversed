#pragma once

class CWeather {
public:
    static inline float&  ForcedWind             = *reinterpret_cast<float*>(Core::Rebase(0x16394CC));
    static inline float&  WindClipped            = *reinterpret_cast<float*>(Core::Rebase(0x16394D0));
    static inline float&  NightFactor            = *reinterpret_cast<float*>(Core::Rebase(0x16394D4));
    static inline int32&  NewWeatherType         = *reinterpret_cast<int32*>(Core::Rebase(0x16394D8));
    static inline uint32& LightningFlashLastTime = *reinterpret_cast<uint32*>(Core::Rebase(0x16394DC));
    static inline float&  WetRoads               = *reinterpret_cast<float*>(Core::Rebase(0x16394E4));
    static inline uint32& LightningStartFrame    = *reinterpret_cast<uint32*>(Core::Rebase(0x16394EC));
    static inline float&  StreetLightIntensity   = *reinterpret_cast<float*>(Core::Rebase(0x16394F0));
    static inline float&  Wind                   = *reinterpret_cast<float*>(Core::Rebase(0x16394F8));
    static inline float&  Thunder                = *reinterpret_cast<float*>(Core::Rebase(0x16394FC));
    static inline int32&  ForcedWeatherType      = *reinterpret_cast<int32*>(Core::Rebase(0x1639500));
    static inline int32&  Unused504              = *reinterpret_cast<int32*>(Core::Rebase(0x1639504));
    static inline float&  CloudCoverage          = *reinterpret_cast<float*>(Core::Rebase(0x1639508));
    static inline float&  InterpolationValue     = *reinterpret_cast<float*>(Core::Rebase(0x163950C));
    static inline float&  Sun                    = *reinterpret_cast<float*>(Core::Rebase(0x1639514));
    static inline bool&   LightningFlash         = *reinterpret_cast<bool*>(Core::Rebase(0x163951C));
    static inline bool&   LightningBurst         = *reinterpret_cast<bool*>(Core::Rebase(0x163951E));
    static inline int32&  LightningDuration      = *reinterpret_cast<int32*>(Core::Rebase(0x1639520));
    static inline int32&  OldWeatherType         = *reinterpret_cast<int32*>(Core::Rebase(0x1639524));
    static inline float&  Rain                   = *reinterpret_cast<float*>(Core::Rebase(0x163952C));
    static inline int32&  WeatherTypeList        = *reinterpret_cast<int32*>(Core::Rebase(0x1639530));
    static inline float&  Foggyness              = *reinterpret_cast<float*>(Core::Rebase(0x1639534));
    static inline uint32& NextThunderTime        = *reinterpret_cast<uint32*>(Core::Rebase(0x1639538));
    static inline float&  UnderWaterness         = *reinterpret_cast<float*>(Core::Rebase(0x163953C));

    static void InjectHooks();

    static float       __cdecl CalcNightFactor();
    static const int8* __cdecl GetWeatherCycle();
    static void        __cdecl ProcessPlayerWetness();
    static void        __cdecl Update();
};
