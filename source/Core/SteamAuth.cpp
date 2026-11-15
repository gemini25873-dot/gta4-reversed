#include "StdInc.h"
#include "Core/SteamAuth.h"
#include "Core/Log.h"

#include <Softpub.h>
#include <WinTrust.h>
#include <wincrypt.h>

namespace Core::SteamAuth {

namespace {
    constexpr uint32 kGtaIvAppId = 12210u;

    bool g_isVerified = false;

    template<typename Ret, typename... Args>
    Ret CallVTable(void* obj, size_t index, Args... args) {
        auto** vtable = *reinterpret_cast<void***>(obj);
        using Fn = Ret(__thiscall*)(void*, Args...);
        return reinterpret_cast<Fn>(vtable[index])(obj, args...);
    }

    bool VerifyValveAuthenticode(const wchar_t* filePath, HANDLE hFile) {
        WINTRUST_FILE_INFO fileInfo{};
        fileInfo.cbStruct = sizeof(fileInfo);
        fileInfo.pcwszFilePath = filePath;
        fileInfo.hFile = hFile;

        WINTRUST_DATA winTrustData{};
        winTrustData.cbStruct = sizeof(winTrustData);
        winTrustData.dwUIChoice = WTD_UI_NONE;
        winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
        winTrustData.dwUnionChoice = WTD_CHOICE_FILE;
        winTrustData.pFile = &fileInfo;
        winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
        winTrustData.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

        GUID policyGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        const LONG status = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &policyGuid, &winTrustData);

        bool validValveSigner = false;
        if (status == ERROR_SUCCESS) {
            CRYPT_PROVIDER_DATA* provData = WTHelperProvDataFromStateData(winTrustData.hWVTStateData);
            if (provData) {
                CRYPT_PROVIDER_SGNR* signer = WTHelperGetProvSignerFromChain(provData, 0, FALSE, 0);
                if (signer) {
                    CRYPT_PROVIDER_CERT* cert = WTHelperGetProvCertFromChain(signer, 0);
                    if (cert && cert->pCert) {
                        wchar_t signerName[128]{};
                        if (CertGetNameStringW(
                                cert->pCert,
                                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                0,
                                nullptr,
                                signerName,
                                static_cast<DWORD>(std::size(signerName))) > 1) {
                            validValveSigner = (std::wcscmp(signerName, L"Valve Corp.") == 0);
                            if (!validValveSigner) {
                                Core::Log::Error("[SteamAuth] Unexpected certificate signer: %ls", signerName);
                            }
                        }
                    }
                }
            }
        } else {
            Core::Log::Error("[SteamAuth] WinVerifyTrust failed: 0x%08X", static_cast<uint32>(status));
        }

        winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &policyGuid, &winTrustData);
        return validValveSigner;
    }

    bool ReadSteamActiveProcess(DWORD& outPid, DWORD& outActiveUser, std::wstring& outClientDllPath) {
        const wchar_t* subKey = L"Software\\Valve\\Steam\\ActiveProcess";

        DWORD size = sizeof(outPid);
        if (RegGetValueW(HKEY_CURRENT_USER, subKey, L"pid", RRF_RT_REG_DWORD, nullptr, &outPid, &size) != ERROR_SUCCESS) {
            return false;
        }

        size = sizeof(outActiveUser);
        if (RegGetValueW(HKEY_CURRENT_USER, subKey, L"ActiveUser", RRF_RT_REG_DWORD, nullptr, &outActiveUser, &size) != ERROR_SUCCESS) {
            outActiveUser = 0;
        }

        wchar_t pathBuf[MAX_PATH]{};
        DWORD pathBytes = sizeof(pathBuf);
        if (RegGetValueW(HKEY_CURRENT_USER, subKey, L"SteamClientDll", RRF_RT_REG_SZ, nullptr, pathBuf, &pathBytes) != ERROR_SUCCESS) {
            return false;
        }

        outClientDllPath = pathBuf;
        return !outClientDllPath.empty();
    }
} // namespace

bool VerifyOwnership() {
    g_isVerified = false;

    DWORD steamPid = 0;
    DWORD activeUser = 0;
    std::wstring clientDllPath;
    if (!ReadSteamActiveProcess(steamPid, activeUser, clientDllPath)) {
        Core::Log::Info("[SteamAuth] Failed to read Steam registry state.");
        return false;
    }

    if (steamPid == 0 || activeUser == 0) {
        Core::Log::Info("[SteamAuth] Steam is not running or user is not logged in (pid=%lu, user=%lu).",
            static_cast<unsigned long>(steamPid),
            static_cast<unsigned long>(activeUser));
        return false;
    }

    // Lock the DLL file with FILE_SHARE_READ while verifying signature and loading (TOCTOU protection)
    HANDLE hFile = CreateFileW(
        clientDllPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        Core::Log::Error("[SteamAuth] Failed to open steamclient.dll (err=%lu).", GetLastError());
        return false;
    }

    if (!VerifyValveAuthenticode(clientDllPath.c_str(), hFile)) {
        CloseHandle(hFile);
        Core::Log::Error("[SteamAuth] steamclient.dll Valve Corp. signature verification failed!");
        return false;
    }

    HMODULE hSteamClient = LoadLibraryExW(clientDllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    CloseHandle(hFile);

    if (!hSteamClient) {
        Core::Log::Error("[SteamAuth] Failed to load steamclient.dll (err=%lu).", GetLastError());
        return false;
    }

    using CreateInterfaceFn = void* (__cdecl*)(const char*, int*);
    auto createInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hSteamClient, "CreateInterface"));
    if (!createInterface) {
        Core::Log::Error("[SteamAuth] CreateInterface export not found.");
        return false;
    }

    SetEnvironmentVariableA("SteamAppId", "12210");
    SetEnvironmentVariableA("SteamGameId", "12210");

    void* pClient = createInterface("SteamClient017", nullptr);
    if (!pClient) {
        pClient = createInterface("SteamClient020", nullptr);
    }
    if (!pClient) {
        Core::Log::Error("[SteamAuth] Failed to acquire ISteamClient interface.");
        return false;
    }

    const int32 pipe = CallVTable<int32>(pClient, 0); // CreateSteamPipe
    if (pipe == 0) {
        Core::Log::Error("[SteamAuth] CreateSteamPipe failed.");
        return false;
    }

    const int32 user = CallVTable<int32>(pClient, 2, pipe); // ConnectToGlobalUser
    if (user == 0) {
        CallVTable<bool>(pClient, 1, pipe); // BReleaseSteamPipe
        Core::Log::Info("[SteamAuth] ConnectToGlobalUser failed (no active Steam session).");
        return false;
    }

    void* pSteamUser  = CallVTable<void*>(pClient, 5, user, pipe, "SteamUser019");
    void* pSteamUtils = CallVTable<void*>(pClient, 9, pipe, "SteamUtils009");
    void* pSteamApps  = CallVTable<void*>(pClient, 15, user, pipe, "STEAMAPPS_INTERFACE_VERSION008");

    bool loggedOn = false;
    uint32 reportedAppId = 0;
    bool subscribedGtaIv = false;

    if (pSteamUser && pSteamUtils && pSteamApps) {
        loggedOn = CallVTable<bool>(pSteamUser, 1);                      // ISteamUser::BLoggedOn
        reportedAppId = CallVTable<uint32>(pSteamUtils, 9);              // ISteamUtils::GetAppID
        subscribedGtaIv = CallVTable<bool>(pSteamApps, 6, kGtaIvAppId);  // ISteamApps::BIsSubscribedApp(12210)
    } else {
        Core::Log::Error("[SteamAuth] Failed to acquire Steam sub-interfaces (User=%p, Utils=%p, Apps=%p).",
            pSteamUser, pSteamUtils, pSteamApps);
    }

    CallVTable<void>(pClient, 4, pipe, user); // ReleaseUser
    CallVTable<bool>(pClient, 1, pipe);       // BReleaseSteamPipe

    Core::Log::Info("[SteamAuth] loggedOn=%d | appId=%u | BIsSubscribedApp(12210)=%d",
        loggedOn ? 1 : 0,
        reportedAppId,
        subscribedGtaIv ? 1 : 0);

    g_isVerified = loggedOn && subscribedGtaIv;
    return g_isVerified;
}

bool IsVerified() {
    return g_isVerified;
}

} // namespace Core::SteamAuth
