#pragma once
#include <string>
#include <vector>

namespace Core::Log {
    void Init();
    void Shutdown();
    void Info(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void InfoOnce(const char* key, const char* fmt, ...);
    std::vector<std::string> GetRecentEvents();
}
