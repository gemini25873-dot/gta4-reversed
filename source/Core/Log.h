#pragma once

namespace Core::Log {
    void Init();
    void Shutdown();
    void Info(const char* fmt, ...);
    void Error(const char* fmt, ...);
}
