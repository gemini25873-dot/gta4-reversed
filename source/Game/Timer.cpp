#include "StdInc.h"
#include "Timer.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace {
    struct TimerChannel {
        uint32 timeInMs;
        uint32 prevTimeInMs;
        float  timeStep;
        float  invTimeStep;

        void Advance(float step) {
            prevTimeInMs = timeInMs;
            timeStep     = step;
            invTimeStep  = 1.0f / step;
            timeInMs    += static_cast<uint32>(static_cast<int64>(step * 1000.0f));
        }

        void AdvanceNoTimeMs(float step) {
            prevTimeInMs = timeInMs;
            timeStep     = step;
            invTimeStep  = 1.0f / step;
        }
    };
    static_assert(sizeof(TimerChannel) == 16);

    TimerChannel& ChannelAt(uintptr_t idaAddr) {
        return *reinterpret_cast<TimerChannel*>(Core::Rebase(idaAddr));
    }

    float ClampStep(float step, bool bypassMax) {
        const float minStep = Core::StaticRef<float>(0xF1D9E4);
        const float maxStep = Core::StaticRef<float>(0xF1D9E8);
        float       clamped = (step > minStep) ? step : minStep;
        if (!bypassMax && clamped >= maxStep) {
            clamped = maxStep;
        }
        return clamped;
    }
}

void CTimer::InjectHooks() {
    Core::Hook::Install(0x444370, &CTimer::GetIsPaused);
    Core::Hook::Install(0x404DF0, &CTimer::GetTimeStepInMilliseconds);
    Core::Hook::Install(0x848770, &CTimer::StartUserPause);
    Core::Hook::Install(0x848780, &CTimer::EndUserPause);
    Core::Hook::Install(0x848790, &CTimer::GetIsUserPaused);
    Core::Hook::Install(0x8487A0, &CTimer::StartCodePause);
    Core::Hook::Install(0x8487B0, &CTimer::EndCodePause);
    Core::Hook::Install(0x8487C0, &CTimer::GetIsCodePaused);
    Core::Hook::Install(0x8487D0, &CTimer::Stop);
    Core::Hook::Install(0x848A10, &CTimer::Update);
    Core::Log::Info("CTimer hooks installed.");
}

// 0x444370
bool __cdecl CTimer::GetIsPaused() {
    const HWND hwnd = Core::StaticRef<HWND>(0x1849DDC);
    const bool b1   = Core::StaticRef<bool>(0xFC7E79);
    const bool b2   = Core::StaticRef<bool>(0x18A8607);

    const uint8 paused = static_cast<uint8>(m_CodePause) | static_cast<uint8>(m_UserPause);
    if (IsIconic(hwnd) || (b1 && b2)) {
        return (paused | 1) != 0;
    }
    return paused != 0;
}

// 0x404DF0
int32 __cdecl CTimer::GetTimeStepInMilliseconds() {
    return static_cast<int32>(ms_fTimeStep * 1000.0f);
}

// 0x848770
void __cdecl CTimer::StartUserPause() {
    m_UserPause = true;
}

// 0x848780
void __cdecl CTimer::EndUserPause() {
    m_UserPause = false;
}

// 0x848790
bool __cdecl CTimer::GetIsUserPaused() {
    return m_UserPause;
}

// 0x8487A0
void __cdecl CTimer::StartCodePause() {
    m_CodePause = true;
}

// 0x8487B0
void __cdecl CTimer::EndCodePause() {
    m_CodePause = false;
}

// 0x8487C0
bool __cdecl CTimer::GetIsCodePaused() {
    return m_CodePause;
}

// 0x8487D0
bool __cdecl CTimer::Stop() {
    ms_fTimeScale                    = 1.0f;
    Core::StaticRef<float>(0xF1D9DC) = 1.0f;
    Core::StaticRef<float>(0xF1D9E0) = 1.0f;
    Core::StaticRef<bool>(0x11DDF40) = false;
    m_UserPause                      = false;
    m_CodePause                      = false;
    return false;
}

// 0x848A10
void __cdecl CTimer::Update() {
    auto& justPaused   = Core::StaticRef<bool>(0x11DDF43);
    auto& pendingPause = Core::StaticRef<bool>(0x11DDF44);
    justPaused = false;
    if (pendingPause) {
        StartUserPause();
        justPaused   = true;
        pendingPause = false;
    }

    const float ts1 = Core::StaticRef<float>(0xF1D9DC);
    const float ts2 = Core::StaticRef<float>(0xF1D9E0);
    if (ts1 < 1.0f) {
        ms_fTimeScale = ts1;
    } else if (ts2 < 1.0f) {
        ms_fTimeScale = ts2;
    }
    const float scale = ms_fTimeScale;

    auto&       lastTick = Core::StaticRef<uint64>(0x11DDF48);
    const float invFreq  = Core::StaticRef<float>(0x18A85D8);
    const uint64 now     = Core::Hook::Call<uint64>(0x5AFB70);
    const float  rawDt   = static_cast<float>(static_cast<double>(static_cast<int64>(now - lastTick)) * invFreq);
    lastTick             = Core::Hook::Call<uint64>(0x5AFB70);

    if (!GetIsPaused() || justPaused) {
        const float scaledDt = rawDt * scale;
        ChannelAt(0x11DDE74).Advance(ClampStep(scaledDt, Core::Hook::Call<bool>(0x85F210)));
        ChannelAt(0x11DDEB4).Advance(scaledDt);
        ChannelAt(0x11DDE54).Advance(ClampStep(rawDt, Core::Hook::Call<bool>(0x85F210)));
        ChannelAt(0x11DDEF4).Advance(ClampStep(rawDt, Core::Hook::Call<bool>(0x85F210)));
    }

    ++m_FrameCounter;
    ChannelAt(0x11DDE84).Advance(rawDt);

    const float unpausedDt = ClampStep(rawDt, Core::Hook::Call<bool>(0x85F210));
    auto&       chUnpaused = ChannelAt(0x11DDEE4);
    if (!m_UserPause || justPaused) {
        chUnpaused.Advance(unpausedDt);
    } else {
        chUnpaused.AdvanceNoTimeMs(unpausedDt);
    }

    Core::Hook::Call<bool>(0x85F210);
    ChannelAt(0x11DDF24).Advance(ClampStep(rawDt, false));

    auto& chGame = ChannelAt(0x11DDE74);
    auto& chReal = ChannelAt(0x11DDE54);

    if (Core::Hook::Call<bool>(0x462F80)
        && (Core::Hook::Call<bool>(0x463000) || Core::Hook::CallMethod<bool>(0x767A40, reinterpret_cast<void*>(Core::Rebase(0x1968EE8))))) {
        m_snTimeInMilliseconds = Core::Hook::CallMethod<uint32>(0x7679F0, reinterpret_cast<void*>(Core::Rebase(0x1968EE8)));
    } else {
        m_snTimeInMilliseconds = chGame.timeInMs;
    }

    Core::StaticRef<float>(0x19A5AC4)  = 1.0f / chGame.timeStep;
    Core::StaticRef<float>(0x19A5AC0)  = chGame.timeStep;
    Core::StaticRef<uint32>(0x19A5AE0) = m_FrameCounter;
    Core::StaticRef<float>(0x19A5B08)  = chReal.timeStep;
    Core::StaticRef<float>(0x19A5ACC)  = static_cast<float>(static_cast<double>(chGame.timeInMs) * 0.001);

    if (!GetIsPaused() && Core::StaticRef<int32>(0x15C4A90) && !Core::Hook::Call<bool>(0x462F90)) {
        float dtMs = chReal.timeStep * 1000.0f;
        if (dtMs < 0.0f) {
            dtMs = 0.0f;
        } else if (dtMs > 1000.0f) {
            dtMs = 1000.0f;
        }

        auto& statTimeMs = Core::StaticRef<uint32>(0x11DDF34);
        statTimeMs += static_cast<uint32>(static_cast<int64>(dtMs));
        Core::Hook::Call<void>(0x9655C0, 118, static_cast<float>(statTimeMs));
        Core::Hook::Call<void>(0x962B90, 44, dtMs, scale);

        const uint32 remoteStatMs = static_cast<uint32>(static_cast<int64>(Core::Hook::Call<double>(0x9633E0, 44)));
        if (remoteStatMs < statTimeMs) {
            statTimeMs = remoteStatMs;
            Core::Hook::Call<void>(0x963D00, 118, static_cast<float>(remoteStatMs));
        }
    }
}
