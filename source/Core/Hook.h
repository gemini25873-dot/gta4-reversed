#pragma once

namespace Core::Hook {
    bool InstallJmp(uintptr_t idaAddr, void* detour);
    void UninstallAll();

    template<typename T>
    inline bool Install(uintptr_t idaAddr, T* detour) {
        return InstallJmp(idaAddr, reinterpret_cast<void*>(detour));
    }

    template<typename Ret, typename Class, typename... Args>
    inline bool Install(uintptr_t idaAddr, Ret(__thiscall Class::* detour)(Args...)) {
        static_assert(sizeof(detour) == sizeof(void*), "Only single-inheritance member pointers supported");
        union {
            Ret(__thiscall Class::* m)(Args...);
            void* p;
        } u{ detour };
        return InstallJmp(idaAddr, u.p);
    }

    template<typename Ret, typename Class, typename... Args>
    inline bool Install(uintptr_t idaAddr, Ret(__thiscall Class::* detour)(Args...) const) {
        static_assert(sizeof(detour) == sizeof(void*), "Only single-inheritance member pointers supported");
        union {
            Ret(__thiscall Class::* m)(Args...) const;
            void* p;
        } u{ detour };
        return InstallJmp(idaAddr, u.p);
    }

    template<typename Ret, typename... Args>
    inline Ret Call(uintptr_t idaAddr, Args... args) {
        using Fn = Ret(__cdecl*)(Args...);
        return reinterpret_cast<Fn>(Rebase(idaAddr))(args...);
    }

    template<typename Ret, typename... Args>
    inline Ret CallStdcall(uintptr_t idaAddr, Args... args) {
        using Fn = Ret(__stdcall*)(Args...);
        return reinterpret_cast<Fn>(Rebase(idaAddr))(args...);
    }

    template<typename Ret, typename This, typename... Args>
    inline Ret CallMethod(uintptr_t idaAddr, This* self, Args... args) {
        using Fn = Ret(__thiscall*)(This*, Args...);
        return reinterpret_cast<Fn>(Rebase(idaAddr))(self, args...);
    }
}
