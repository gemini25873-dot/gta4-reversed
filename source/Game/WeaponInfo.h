#pragma once

class CWeaponInfo
{
public:
    static inline uint32& m_dwLastEventTime = Core::StaticRef<uint32>(0x11D4D5C);

    static void InjectHooks();

    static void __cdecl RecordEventByIndex(int32 index);
    static void __cdecl RecordEvent9();
    static void __cdecl RecordEvent10();
    static void __cdecl RecordEvent4(const char* name);
    static void __cdecl RecordEvent5(const char* name);
    static void __cdecl RecordEvent6(const char* name);
    static void __cdecl RecordEvent23(const char* name);
    static void __cdecl RecordEvent13();
    static void __cdecl RecordEvent14();
    static void __cdecl RecordEvent15();
    static void __cdecl RecordEvent17(const char* name);
    static void __cdecl RecordEvent18(const char* name);
    static void __cdecl RecordEvent19(const char* name);
};
