#pragma once

class CWeaponInfo
{
public:
    static inline uint32& m_dwLastEventTime = Core::StaticRef<uint32>(0x11D4D5C);

    static void InjectHooks();

    static void __cdecl SortPtrArray(int32* first, int32* last, int32 pred);
    static int32 __cdecl FindAvailableSlot();
    static int32 __cdecl GetMaxPacketSize1();
    static int32 __cdecl GetMaxPacketSize2();
    static int32 __cdecl SocketSelectWrite(int32 sock, int32 timeoutMs);

    bool __thiscall IsConnected();
    uint8 __thiscall HasPendingFlag();
    void __thiscall ClearPendingFlag();
    bool __thiscall StartConnect();
    void __thiscall ResolveServers();
    void __thiscall CancelResolve();

    static int32 __cdecl SendEncryptedChunk(int32 sock, const void* data, int32 a3, int32 a4, uint8 xorKey, int32* totalSent, uint8 a7);
    static bool __cdecl IsTelemetryStatAllowed(uint32 id);
    static void __cdecl ShutdownTelemetry();
    static bool __cdecl FormatMacAddress(const char* prefix, char* outBuf);
    static uint64 __cdecl GetSessionId();
    static void __cdecl FlushEvents();

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

    static void __cdecl RecordEvent32(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8);
    static void __cdecl RecordEvent33(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8);
    static void __cdecl RecordEvent34();
    static void __cdecl RecordEvent36(const char* name);
    static void __cdecl RecordEvent37(const char* name);
    static void __cdecl RecordEvent38(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8);
    static void __cdecl RecordEvent39();
    static void __cdecl RecordEvent21();
    static void __cdecl RecordEvent22();
    static void __cdecl RecordEvent24(const char* name, int32 val);
    static void __cdecl RecordEvent25(const char* name, int32 a2, int32 a3);
    static void __cdecl RecordEvent26(const char* name, float val);
    static void __cdecl RecordEvent27(const char* name, int32 a2, float val);
    static void __cdecl RecordEvent31(const char* name);
    static void __cdecl RecordEvent44(int32 val);
    static void __cdecl SetTelemetryValue(int32 val);
    static void __cdecl PackVec2ToInt16(const float* in, int16* out);
    static void __cdecl ResetStatBuffers();

    bool __thiscall ReadBitStreamBits(uint32* outVal, uint32 bitCount);
    void* __thiscall ScalarDeletingDtor(uint8 flags);
    CWeaponInfo* __thiscall InitClient();
    bool __thiscall ReconnectClient();
    void __thiscall DisconnectAndResetClient();
    bool __thiscall ConnectToServer();

    static void __cdecl ResetAndRecordEvent21();
    static void __cdecl RecordStatChanges();
    static void __cdecl SetSessionId(uint32 low, uint32 high);
    static void __cdecl RecordEvent7(int32 index);
    static void __cdecl RecordEvent8(int32 index);
    static void __cdecl RecordEvent42(int32 index);
    static void __cdecl RecordEvent1();
    static void __cdecl RecordEvent2(int32 index, const void* obj);
    static void __cdecl RecordEvent3();
    static void __cdecl RecordEvent12();
    static void __cdecl RecordEvent16();
    static void __cdecl RecordEvent20();
    static void __cdecl RecordEvent40(const char** namePtr, const float* pos);
    static void __cdecl RecordEvent41();
    static void __cdecl RecordEvent46(int32 index);
    static void __cdecl RecordEvent43(int32 val);
};
