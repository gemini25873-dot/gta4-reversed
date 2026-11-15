#include "StdInc.h"
#include "Log.h"
#include <cstdarg>

namespace Core::Log {
    static FILE* g_LogFile = nullptr;
    static CRITICAL_SECTION g_Lock{};
    static bool g_LockInit = false;

    static void WriteLine(const char* level, const char* fmt, va_list args) {
        if (!g_LockInit) {
            return;
        }
        EnterCriticalSection(&g_Lock);

        char msg[2048];
        std::vsnprintf(msg, sizeof(msg), fmt, args);

        SYSTEMTIME st{};
        GetLocalTime(&st);

        std::printf("[%02d:%02d:%02d.%03d][%s] %s\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, level, msg);
        if (g_LogFile) {
            std::fprintf(g_LogFile, "[%02d:%02d:%02d.%03d][%s] %s\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, level, msg);
            std::fflush(g_LogFile);
        }

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
        WriteLine("INFO", fmt, args);
        va_end(args);
    }

    void Error(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        WriteLine("ERR ", fmt, args);
        va_end(args);
    }
}
