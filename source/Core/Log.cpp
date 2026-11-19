#include "StdInc.h"
#include "Log.h"
#include <cstdarg>
#include <deque>
#include <unordered_set>

namespace Core::Log {
    static FILE* g_LogFile = nullptr;
    static CRITICAL_SECTION g_Lock{};
    static bool g_LockInit = false;
    static std::unordered_set<std::string> g_SeenKeys;
    static std::deque<std::string> g_RecentEvents;

    static void WriteLineUnlocked(const char* level, const char* fmt, va_list args, bool recordEvent) {
        char msg[2048];
        std::vsnprintf(msg, sizeof(msg), fmt, args);

        SYSTEMTIME st{};
        GetLocalTime(&st);

        char fullLine[2176];
        std::snprintf(fullLine, sizeof(fullLine), "[%02d:%02d:%02d.%03d][%s] %s",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, level, msg);

        std::printf("%s\n", fullLine);
        if (g_LogFile) {
            std::fprintf(g_LogFile, "%s\n", fullLine);
            std::fflush(g_LogFile);
        }

        if (recordEvent) {
            g_RecentEvents.emplace_back(fullLine);
            if (g_RecentEvents.size() > 128) {
                g_RecentEvents.pop_front();
            }
        }
    }

    static void WriteLine(const char* level, const char* fmt, va_list args, bool recordEvent = false) {
        if (!g_LockInit) {
            return;
        }
        EnterCriticalSection(&g_Lock);
        WriteLineUnlocked(level, fmt, args, recordEvent);
        LeaveCriticalSection(&g_Lock);
    }

    void Init() {
        InitializeCriticalSection(&g_Lock);
        g_LockInit = true;

        if (AllocConsole()) {
            FILE* fOut = nullptr;
            FILE* fErr = nullptr;
            freopen_s(&fOut, "CONOUT$", "w", stdout);
            freopen_s(&fErr, "CONOUT$", "w", stderr);
            SetConsoleTitleA("gta4-reversed");
        }

        g_LogFile = std::fopen("gta4_reversed.log", "w");
    }

    void Shutdown() {
        if (g_LogFile) {
            std::fclose(g_LogFile);
            g_LogFile = nullptr;
        }
        if (g_LockInit) {
            DeleteCriticalSection(&g_Lock);
            g_LockInit = false;
        }
    }

    void Info(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        WriteLine("INFO", fmt, args, false);
        va_end(args);
    }

    void Error(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        WriteLine("ERR ", fmt, args, true);
        va_end(args);
    }

    void InfoOnce(const char* key, const char* fmt, ...) {
        if (!g_LockInit || !key) {
            return;
        }
        EnterCriticalSection(&g_Lock);
        if (g_SeenKeys.emplace(key).second) {
            va_list args;
            va_start(args, fmt);
            WriteLineUnlocked("TEST", fmt, args, true);
            va_end(args);
        }
        LeaveCriticalSection(&g_Lock);
    }

    std::vector<std::string> GetRecentEvents() {
        std::vector<std::string> out;
        if (!g_LockInit) {
            return out;
        }
        EnterCriticalSection(&g_Lock);
        out.assign(g_RecentEvents.begin(), g_RecentEvents.end());
        LeaveCriticalSection(&g_Lock);
        return out;
    }
}
