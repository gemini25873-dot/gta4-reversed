#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

using int8   = std::int8_t;
using int16  = std::int16_t;
using int32  = std::int32_t;
using int64  = std::int64_t;
using uint8  = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

#define VALIDATE_SIZE(struc, size) \
    static_assert(sizeof(struc) == (size), "Invalid structure size of " #struc)

#define VALIDATE_OFFSET(struc, member, offset) \
    static_assert(offsetof(struc, member) == (offset), "Invalid offset of " #struc "::" #member)

namespace Core {
    inline uintptr_t GetBaseAddr() {
        static const uintptr_t s_Base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
        return s_Base;
    }

    inline uintptr_t Rebase(uintptr_t idaAddr) {
        return GetBaseAddr() + (idaAddr - 0x400000u);
    }

    template<typename T>
    inline T& StaticRef(uintptr_t idaAddr) {
        return *reinterpret_cast<T*>(Rebase(idaAddr));
    }
}
