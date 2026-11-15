#pragma once

namespace Core::SteamAuth {
    // Verifies GTA IV (AppID 12210) ownership via the running Valve-signed Steam client.
    // Returns true only when Steam is active, logged in, steamclient.dll passes
    // WinVerifyTrust (Valve Corp.), GetAppID() == 12210, and BIsSubscribedApp(12210) == true.
    bool VerifyOwnership();

    // Returns the cached result of the last VerifyOwnership() call.
    bool IsVerified();
}
