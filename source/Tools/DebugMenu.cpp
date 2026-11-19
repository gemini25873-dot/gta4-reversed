#include "StdInc.h"
#include "Tools/DebugMenu.h"
#include "Core/Log.h"
#include "Game/Clock.h"
#include "Game/Explosion.h"
#include "Game/Game.h"
#include "Game/Pickups.h"
#include "Game/PlayerInfo.h"
#include "Game/Weather.h"
#include "Game/Zones.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Tools::DebugMenu {

namespace {
    using EndSceneFn       = HRESULT(__stdcall*)(IDirect3DDevice9*);
    using ResetFn          = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    using GetDeviceStateFn = HRESULT(__stdcall*)(IDirectInputDevice8A*, DWORD, LPVOID);
    using GetDeviceDataFn  = HRESULT(__stdcall*)(IDirectInputDevice8A*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
    using SetCursorPosFn   = BOOL(WINAPI*)(int, int);
    using ClipCursorFn     = BOOL(WINAPI*)(const RECT*);

    EndSceneFn       oEndScene       = nullptr;
    ResetFn          oReset          = nullptr;
    GetDeviceStateFn oGetDeviceState = nullptr;
    GetDeviceDataFn  oGetDeviceData  = nullptr;
    SetCursorPosFn   oSetCursorPos   = nullptr;
    ClipCursorFn     oClipCursor     = nullptr;

    struct InlineHookBackup {
        uintptr_t addr = 0;
        uint8     bytes[7]{}; // up to 5 before + 2 at addr (for hotpatch) or 5 at addr
        int32     offsetFromAddr = 0;
        size_t    size = 0;
        void*     trampoline = nullptr;
        bool      installed = false;
    };

    InlineHookBackup g_EndSceneHook{};
    InlineHookBackup g_ResetHook{};
    InlineHookBackup g_DiStateHook{};
    InlineHookBackup g_DiDataHook{};
    InlineHookBackup g_SetCursorPosHook{};
    InlineHookBackup g_ClipCursorHook{};

    HWND    g_GameHwnd   = nullptr;
    WNDPROC g_OrigWndProc = nullptr;
    HANDLE  g_HookThread = nullptr;

    bool g_ImGuiReady = false;
    bool g_MenuOpen   = false;
    bool g_Running    = true;
    int  g_ActiveTab  = 0;

    bool InstallInlineHook(void* target, void* detour, void** outOrig, InlineHookBackup& bk, const char* name) {
        if (!target || !detour || !outOrig) return false;
        uint8* p = reinterpret_cast<uint8*>(target);

        Core::Log::Info("[DebugMenu] %s @ 0x%p bytes: [-5:%02X %02X %02X %02X %02X] [%02X %02X %02X %02X %02X %02X]",
            name, p, p[-5], p[-4], p[-3], p[-2], p[-1], p[0], p[1], p[2], p[3], p[4], p[5]);

        auto isPad = [](uint8 b) { return b == 0x90 || b == 0xCC; };

        // Case 1: Standard Windows /hotpatch prologue (5x NOP/INT3 before func, mov edi, edi at +0)
        if (p[0] == 0x8B && p[1] == 0xFF && isPad(p[-5]) && isPad(p[-4]) && isPad(p[-3]) && isPad(p[-2]) && isPad(p[-1])) {
            DWORD oldProt = 0;
            if (!VirtualProtect(p - 5, 7, PAGE_EXECUTE_READWRITE, &oldProt)) return false;
            bk.addr = reinterpret_cast<uintptr_t>(p);
            bk.offsetFromAddr = -5;
            bk.size = 7;
            std::memcpy(bk.bytes, p - 5, 7);

            const int32 rel = static_cast<int32>(reinterpret_cast<uintptr_t>(detour) - reinterpret_cast<uintptr_t>(p));
            p[-5] = 0xE9;
            std::memcpy(p - 4, &rel, 4);
            p[0] = 0xEB; // jmp short -7 (to p - 5)
            p[1] = 0xF9;

            DWORD dummy = 0;
            VirtualProtect(p - 5, 7, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), p - 5, 7);

            *outOrig = p + 2;
            bk.installed = true;
            return true;
        }

        // Case 2: 5-byte prologue "8B FF 55 8B EC" without hotpatch padding
        if (p[0] == 0x8B && p[1] == 0xFF && p[2] == 0x55 && p[3] == 0x8B && p[4] == 0xEC) {
            uint8* tramp = static_cast<uint8*>(VirtualAlloc(nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
            if (!tramp) return false;
            std::memcpy(tramp, p, 5);
            tramp[5] = 0xE9;
            const int32 backRel = static_cast<int32>((reinterpret_cast<uintptr_t>(p) + 5) - (reinterpret_cast<uintptr_t>(tramp) + 10));
            std::memcpy(tramp + 6, &backRel, 4);

            DWORD oldProt = 0;
            if (!VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &oldProt)) {
                VirtualFree(tramp, 0, MEM_RELEASE);
                return false;
            }
            bk.addr = reinterpret_cast<uintptr_t>(p);
            bk.offsetFromAddr = 0;
            bk.size = 5;
            std::memcpy(bk.bytes, p, 5);
            bk.trampoline = tramp;

            p[0] = 0xE9;
            const int32 rel = static_cast<int32>(reinterpret_cast<uintptr_t>(detour) - (reinterpret_cast<uintptr_t>(p) + 5));
            std::memcpy(p + 1, &rel, 4);

            DWORD dummy = 0;
            VirtualProtect(p, 5, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), p, 5);

            *outOrig = tramp;
            bk.installed = true;
            return true;
        }

        // Case 3: Already hooked with E9 rel32 (chain onto existing detour)
        if (p[0] == 0xE9) {
            int32 existingRel = 0;
            std::memcpy(&existingRel, p + 1, 4);
            void* existingTarget = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + 5 + existingRel);

            DWORD oldProt = 0;
            if (!VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &oldProt)) return false;
            bk.addr = reinterpret_cast<uintptr_t>(p);
            bk.offsetFromAddr = 0;
            bk.size = 5;
            std::memcpy(bk.bytes, p, 5);

            const int32 rel = static_cast<int32>(reinterpret_cast<uintptr_t>(detour) - (reinterpret_cast<uintptr_t>(p) + 5));
            std::memcpy(p + 1, &rel, 4);

            DWORD dummy = 0;
            VirtualProtect(p, 5, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), p, 5);

            *outOrig = existingTarget;
            bk.installed = true;
            return true;
        }

        // Case 4: MSVC SEH4 prologue "6A <imm8> B8 <imm32>" (7 bytes: push imm8; mov eax, imm32)
        if (p[0] == 0x6A && p[2] == 0xB8) {
            uint8* tramp = static_cast<uint8*>(VirtualAlloc(nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
            if (!tramp) return false;
            std::memcpy(tramp, p, 7);
            tramp[7] = 0xE9;
            const int32 backRel = static_cast<int32>((reinterpret_cast<uintptr_t>(p) + 7) - (reinterpret_cast<uintptr_t>(tramp) + 12));
            std::memcpy(tramp + 8, &backRel, 4);

            DWORD oldProt = 0;
            if (!VirtualProtect(p, 7, PAGE_EXECUTE_READWRITE, &oldProt)) {
                VirtualFree(tramp, 0, MEM_RELEASE);
                return false;
            }
            bk.addr = reinterpret_cast<uintptr_t>(p);
            bk.offsetFromAddr = 0;
            bk.size = 7;
            std::memcpy(bk.bytes, p, 7);
            bk.trampoline = tramp;

            p[0] = 0xE9;
            const int32 rel = static_cast<int32>(reinterpret_cast<uintptr_t>(detour) - (reinterpret_cast<uintptr_t>(p) + 5));
            std::memcpy(p + 1, &rel, 4);
            p[5] = 0x90;
            p[6] = 0x90;

            DWORD dummy = 0;
            VirtualProtect(p, 7, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), p, 7);

            *outOrig = tramp;
            bk.installed = true;
            return true;
        }

        Core::Log::Error("[DebugMenu] Unsupported prologue for %s at 0x%p", name, p);
        return false;
    }

    void RemoveInlineHook(InlineHookBackup& bk) {
        if (!bk.installed || !bk.addr || bk.size == 0) return;
        uint8* p = reinterpret_cast<uint8*>(bk.addr) + bk.offsetFromAddr;
        DWORD oldProt = 0;
        if (VirtualProtect(p, bk.size, PAGE_EXECUTE_READWRITE, &oldProt)) {
            std::memcpy(p, bk.bytes, bk.size);
            DWORD dummy = 0;
            VirtualProtect(p, bk.size, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), p, bk.size);
        }
        if (bk.trampoline) {
            VirtualFree(bk.trampoline, 0, MEM_RELEASE);
            bk.trampoline = nullptr;
        }
        bk.installed = false;
    }

    HRESULT __stdcall HookedGetDeviceState(IDirectInputDevice8A* dev, DWORD cbData, LPVOID lpvData) {
        if (g_MenuOpen && lpvData && cbData > 0) {
            std::memset(lpvData, 0, cbData);
            return DI_OK;
        }
        return oGetDeviceState(dev, cbData, lpvData);
    }

    HRESULT __stdcall HookedGetDeviceData(IDirectInputDevice8A* dev, DWORD cbObjectData,
        LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        if (g_MenuOpen) {
            const HRESULT hr = oGetDeviceData(dev, cbObjectData, rgdod, pdwInOut, dwFlags);
            if (pdwInOut) {
                *pdwInOut = 0;
            }
            return SUCCEEDED(hr) ? DI_OK : hr;
        }
        return oGetDeviceData(dev, cbObjectData, rgdod, pdwInOut, dwFlags);
    }

    BOOL WINAPI HookedSetCursorPos(int X, int Y) {
        if (g_MenuOpen) {
            return TRUE;
        }
        return oSetCursorPos(X, Y);
    }

    BOOL WINAPI HookedClipCursor(const RECT* lpRect) {
        if (g_MenuOpen) {
            return TRUE;
        }
        return oClipCursor(lpRect);
    }

    LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_KEYDOWN && (wParam == VK_F10 || wParam == VK_INSERT) && ((lParam & (1 << 30)) == 0)) {
            g_MenuOpen = !g_MenuOpen;
            if (g_MenuOpen) {
                if (oClipCursor) oClipCursor(nullptr);
                else ClipCursor(nullptr);
            }
            Core::Log::InfoOnce("debug_menu_toggled", "[TEST] DebugMenu toggled via hotkey (%s)",
                g_MenuOpen ? "OPEN" : "CLOSED");
            return 0;
        }

        if (g_MenuOpen && g_ImGuiReady) {
            if (msg == WM_SETCURSOR) {
                SetCursor(nullptr);
                return TRUE;
            }
            if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
                return 1;
            }
            switch (msg) {
            case WM_INPUT:
            case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
            case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
            case WM_MOUSEWHEEL:  case WM_MOUSEHWHEEL: case WM_MOUSEMOVE:
            case WM_KEYDOWN:     case WM_KEYUP:     case WM_CHAR:
            case WM_SYSKEYDOWN:  case WM_SYSKEYUP:  case WM_SYSCHAR:
                return 0;
            default:
                break;
            }
        }

        return CallWindowProcA(g_OrigWndProc, hwnd, msg, wParam, lParam);
    }

    void InitImGui(IDirect3DDevice9* device) {
        D3DDEVICE_CREATION_PARAMETERS cp{};
        const HRESULT cpHr = device->GetCreationParameters(&cp);
        if (FAILED(cpHr) || !cp.hFocusWindow) {
            Core::Log::InfoOnce("debug_menu_cp_fail",
                "[DebugMenu] GetCreationParameters failed (hr=0x%08X, hwnd=0x%p)",
                static_cast<uint32>(cpHr), cp.hFocusWindow);
            return;
        }

        g_GameHwnd = cp.hFocusWindow;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        // Mouse-only menu navigation: no keyboard nav flags
        io.IniFilename = nullptr; // Don't clutter game dir with imgui.ini

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 4.0f;
        style.FrameRounding = 3.0f;

        ImGui_ImplWin32_Init(g_GameHwnd);
        ImGui_ImplDX9_Init(device);

        g_OrigWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrA(g_GameHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HookedWndProc)));

        g_ImGuiReady = true;
        Core::Log::InfoOnce("debug_menu_init", "[TEST] ImGui D3D9 Test Menu initialized (hwnd=0x%p, F10/Insert to toggle)", g_GameHwnd);
    }

    void RenderDebugMenu() {
        // Always show a slim gta-reversed style top bar
        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "gta4-reversed");
            ImGui::Separator();
            if (ImGui::MenuItem(g_MenuOpen ? "Test Menu [OPEN] (F10/INS)" : "Test Menu (F10/INS)")) {
                g_MenuOpen = !g_MenuOpen;
            }
            ImGui::Separator();
            ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("Clock: %02d:%02d", CClock::ms_nGameClockHours, CClock::ms_nGameClockMinutes);
            ImGui::Separator();
            ImGui::Text("Ep: %d | Zones: %d/%d",
                CGame::ms_nCurrentEpisode,
                CTheZones::ms_nZonesRevealed,
                CTheZones::ms_nTotalNumberOfMapZones);
            ImGui::EndMainMenuBar();
        }

        if (!g_MenuOpen) {
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(480, 390), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(20, 32), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("gta4-reversed | In-Game Test Menu (F10 / INS)", &g_MenuOpen)) {
            if (ImGui::BeginTabBar("TestTabs")) {
                if (ImGui::BeginTabItem("World & Time")) {
                    ImGui::Text("CClock (%02d:%02d, Day %d):",
                        CClock::ms_nGameClockHours, CClock::ms_nGameClockMinutes, CClock::CurrentDay);
                    if (ImGui::Button("06:00")) CClock::SetGameClock(6, 0, 0, CClock::CurrentDay);
                    ImGui::SameLine();
                    if (ImGui::Button("12:00")) CClock::SetGameClock(12, 0, 0, CClock::CurrentDay);
                    ImGui::SameLine();
                    if (ImGui::Button("18:00")) CClock::SetGameClock(18, 0, 0, CClock::CurrentDay);
                    ImGui::SameLine();
                    if (ImGui::Button("00:00")) CClock::SetGameClock(0, 0, 0, CClock::CurrentDay);

                    ImGui::Separator();
                    ImGui::Text("CWeather (Old=%d, New=%d, Forced=%d):",
                        CWeather::OldWeatherType, CWeather::NewWeatherType, CWeather::ForcedWeatherType);
                    auto setWeather = [](int32 w) {
                        CWeather::OldWeatherType = w;
                        CWeather::NewWeatherType = w;
                        CWeather::ForcedWeatherType = w;
                    };
                    if (ImGui::Button("ExtraSunny (0)")) setWeather(0);
                    ImGui::SameLine();
                    if (ImGui::Button("Sunny (1)")) setWeather(1);
                    ImGui::SameLine();
                    if (ImGui::Button("Rain (4)")) setWeather(4);
                    ImGui::SameLine();
                    if (ImGui::Button("Clear Force")) CWeather::ForcedWeatherType = -1;

                    ImGui::Separator();
                    ImGui::Text("CTheZones: Total=%d | Revealed=%d",
                        CTheZones::ms_nTotalNumberOfMapZones, CTheZones::ms_nZonesRevealed);
                    ImGui::Text("CGame: CurrentEp=%d | TargetEp=%d | Paused=%d",
                        CGame::ms_nCurrentEpisode, CGame::ms_nTargetEpisode, CGame::ms_bGamePaused ? 1 : 0);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Player & Gameplay")) {
                    const int32 localIdx = CPlayerInfo::ms_nLocalPlayerIndex;
                    CPlayerInfo* player = (localIdx >= 0 && localIdx < 32) ? CPlayerInfo::ms_apPlayers[localIdx] : nullptr;
                    if (player) {
                        ImGui::Text("CWanted (Level=%d):", player->m_Wanted.GetWantedLevel());
                        for (int32 lvl = 0; lvl <= 6; ++lvl) {
                            if (lvl > 0) ImGui::SameLine();
                            char buf[8];
                            std::snprintf(buf, sizeof(buf), "%d*", lvl);
                            if (ImGui::Button(buf)) {
                                player->m_Wanted.SetWantedLevel(lvl);
                            }
                        }
                    } else {
                        ImGui::TextDisabled("Local CPlayerInfo not active yet.");
                    }

                    ImGui::Separator();
                    int32 activePickups = 0;
                    CPickup* arr = CPickups::GetArray();
                    for (int32 i = 0; i < CPickups::MAX_NUM_PICKUPS; ++i) {
                        if (arr[i].m_nType != 0) ++activePickups;
                    }
                    ImGui::Text("CPickups: %d / %d active", activePickups, CPickups::MAX_NUM_PICKUPS);
                    if (ImGui::Button("Remove Temporary Pickups")) {
                        CPickups::RemoveTemporaryPickups();
                    }

                    ImGui::Separator();
                    if (ImGui::Button("CExplosion::RemoveAllExplosions()")) {
                        CExplosion::RemoveAllExplosions();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Test Events Log")) {
                    const auto events = Core::Log::GetRecentEvents();
                    ImGui::Text("Recorded unique [TEST] / [ERR] events: %zu", events.size());
                    ImGui::Separator();
                    ImGui::BeginChild("EventScroll", ImVec2(0, 0), true);
                    for (const auto& line : events) {
                        ImGui::TextUnformatted(line.c_str());
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    HRESULT __stdcall HookedEndScene(IDirect3DDevice9* device) {
        Core::Log::InfoOnce("debug_menu_endscene", "[TEST] HookedEndScene entered (device=0x%p)", device);
        if (device && g_Running) {
            if (!g_ImGuiReady) {
                InitImGui(device);
            }
            if (g_ImGuiReady) {
                ImGui_ImplDX9_NewFrame();
                ImGui_ImplWin32_NewFrame();
                if (g_MenuOpen && g_GameHwnd) {
                    ImGuiIO& io = ImGui::GetIO();
                    POINT pt{};
                    if (GetCursorPos(&pt) && ScreenToClient(g_GameHwnd, &pt)) {
                        io.AddMousePosEvent(static_cast<float>(pt.x), static_cast<float>(pt.y));
                    }
                    io.AddMouseButtonEvent(0, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
                    io.AddMouseButtonEvent(1, (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
                }
                ImGui::NewFrame();
                ImGui::GetIO().MouseDrawCursor = g_MenuOpen;

                RenderDebugMenu();

                ImGui::EndFrame();
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            }
        }
        return oEndScene(device);
    }

    HRESULT __stdcall HookedReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {
        if (g_ImGuiReady) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
        }
        const HRESULT hr = oReset(device, params);
        if (SUCCEEDED(hr) && g_ImGuiReady) {
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return hr;
    }

    DWORD WINAPI SetupThread(LPVOID) {
        HMODULE d3d9Mod = nullptr;
        while (g_Running) {
            d3d9Mod = GetModuleHandleA("d3d9.dll");
            if (d3d9Mod) break;
            Sleep(50);
        }
        if (!d3d9Mod || !g_Running) return 0;

        using Direct3DCreate9Fn = IDirect3D9* (WINAPI*)(UINT);
        auto d3dCreate9 = reinterpret_cast<Direct3DCreate9Fn>(GetProcAddress(d3d9Mod, "Direct3DCreate9"));
        if (!d3dCreate9) return 0;

        IDirect3D9* d3d = d3dCreate9(D3D_SDK_VERSION);
        if (!d3d) return 0;

        HWND tmpHwnd = CreateWindowExA(0, "STATIC", "gta4_rev_dummy", WS_OVERLAPPEDWINDOW,
            0, 0, 100, 100, nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);
        if (!tmpHwnd) {
            d3d->Release();
            return 0;
        }

        D3DPRESENT_PARAMETERS pp{};
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.BackBufferFormat = D3DFMT_UNKNOWN;
        pp.BackBufferCount = 1;
        pp.hDeviceWindow = tmpHwnd;

        IDirect3DDevice9* dummyDev = nullptr;
        const char* devType = "HAL";
        HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpHwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &pp, &dummyDev);
        if (FAILED(hr) || !dummyDev) {
            devType = "NULLREF";
            hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, tmpHwnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &dummyDev);
        }

        if (SUCCEEDED(hr) && dummyDev) {
            void** vtable = *reinterpret_cast<void***>(dummyDev);
            void* resetAddr = vtable[16];
            void* endSceneAddr = vtable[42];
            dummyDev->Release();

            const bool okReset = InstallInlineHook(resetAddr, reinterpret_cast<void*>(&HookedReset),
                reinterpret_cast<void**>(&oReset), g_ResetHook, "Reset");
            const bool okEnd = InstallInlineHook(endSceneAddr, reinterpret_cast<void*>(&HookedEndScene),
                reinterpret_cast<void**>(&oEndScene), g_EndSceneHook, "EndScene");
            Core::Log::Info("[DebugMenu] D3D9 %s inline hooks: EndScene=%s, Reset=%s",
                devType, okEnd ? "OK" : "FAIL", okReset ? "OK" : "FAIL");
        } else {
            Core::Log::Error("[DebugMenu] Failed to create dummy D3D9 device (hr=0x%08X)", static_cast<uint32>(hr));
        }

        DestroyWindow(tmpHwnd);
        d3d->Release();

        // Hook Win32 cursor & DirectInput8 so opening the menu blocks all game input
        if (HMODULE user32 = GetModuleHandleA("user32.dll")) {
            void* pSetCursorPos = reinterpret_cast<void*>(GetProcAddress(user32, "SetCursorPos"));
            void* pClipCursor   = reinterpret_cast<void*>(GetProcAddress(user32, "ClipCursor"));
            const bool okScp = InstallInlineHook(pSetCursorPos, reinterpret_cast<void*>(&HookedSetCursorPos),
                reinterpret_cast<void**>(&oSetCursorPos), g_SetCursorPosHook, "SetCursorPos");
            const bool okClip = InstallInlineHook(pClipCursor, reinterpret_cast<void*>(&HookedClipCursor),
                reinterpret_cast<void**>(&oClipCursor), g_ClipCursorHook, "ClipCursor");
            Core::Log::Info("[DebugMenu] Cursor hooks: SetCursorPos=%s, ClipCursor=%s",
                okScp ? "OK" : "FAIL", okClip ? "OK" : "FAIL");
        }

        if (HMODULE di8Mod = GetModuleHandleA("dinput8.dll")) {
            using DirectInput8CreateFn = HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
            auto pDi8Create = reinterpret_cast<DirectInput8CreateFn>(GetProcAddress(di8Mod, "DirectInput8Create"));
            static const GUID kIID_IDirectInput8A = { 0xBF798030, 0x483A, 0x4DA2, { 0xAA, 0x99, 0x5D, 0x64, 0xED, 0x36, 0x97, 0x00 } };
            static const GUID kGUID_SysKeyboard  = { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
            IDirectInput8A* di8 = nullptr;
            if (pDi8Create && SUCCEEDED(pDi8Create(GetModuleHandleA(nullptr), DIRECTINPUT_VERSION, kIID_IDirectInput8A, reinterpret_cast<void**>(&di8), nullptr)) && di8) {
                IDirectInputDevice8A* diDev = nullptr;
                if (SUCCEEDED(di8->CreateDevice(kGUID_SysKeyboard, &diDev, nullptr)) && diDev) {
                    void** diVtable = *reinterpret_cast<void***>(diDev);
                    void* stateAddr = diVtable[9];
                    void* dataAddr  = diVtable[10];
                    diDev->Release();

                    const bool okState = InstallInlineHook(stateAddr, reinterpret_cast<void*>(&HookedGetDeviceState),
                        reinterpret_cast<void**>(&oGetDeviceState), g_DiStateHook, "DI8::GetDeviceState");
                    const bool okData = InstallInlineHook(dataAddr, reinterpret_cast<void*>(&HookedGetDeviceData),
                        reinterpret_cast<void**>(&oGetDeviceData), g_DiDataHook, "DI8::GetDeviceData");
                    Core::Log::Info("[DebugMenu] DirectInput8 hooks: GetDeviceState=%s, GetDeviceData=%s",
                        okState ? "OK" : "FAIL", okData ? "OK" : "FAIL");
                }
                di8->Release();
            }
        }
        return 0;
    }
} // namespace

void Init() {
    g_Running = true;
    g_HookThread = CreateThread(nullptr, 0, &SetupThread, nullptr, 0, nullptr);
}

void Shutdown() {
    g_Running = false;
    if (g_HookThread) {
        WaitForSingleObject(g_HookThread, 1000);
        CloseHandle(g_HookThread);
        g_HookThread = nullptr;
    }
    RemoveInlineHook(g_ClipCursorHook);
    RemoveInlineHook(g_SetCursorPosHook);
    RemoveInlineHook(g_DiDataHook);
    RemoveInlineHook(g_DiStateHook);
    RemoveInlineHook(g_ResetHook);
    RemoveInlineHook(g_EndSceneHook);
    if (g_GameHwnd && g_OrigWndProc) {
        SetWindowLongPtrA(g_GameHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_OrigWndProc));
        g_OrigWndProc = nullptr;
    }
    if (g_ImGuiReady) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_ImGuiReady = false;
    }
}

} // namespace Tools::DebugMenu
