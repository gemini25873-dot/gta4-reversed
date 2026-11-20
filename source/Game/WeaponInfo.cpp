#include "StdInc.h"
#include "Game/WeaponInfo.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstring>

namespace
{
    void RecordSimpleEvent(int32 eventId, bool flushAfter = false)
    {
        uint8 buf[52];
        Core::Hook::CallMethod<void*>(0xBCAF00, buf, eventId);
        Core::Hook::Call<int32>(0x81DB10, buf, 52);
        if (flushAfter)
            Core::Hook::Call<int32>(0x81DA10);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }

    void RecordNamedEvent(int32 eventId, const char* name, bool flushAfter = false)
    {
        uint8 buf[52];
        Core::Hook::CallMethod<void*>(0xBCAF00, buf, eventId);
        const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
        Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
        Core::Hook::Call<int32>(0x81DB10, buf, 52);
        if (flushAfter)
            Core::Hook::Call<int32>(0x81DA10);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }

    void RecordSessionPacketEvent(
        int32 eventId,
        uint32 a0,
        uint32 a1,
        uint32 a2,
        uint32 a3,
        uint8 a4,
        uint8 a5,
        uint8 a6,
        uint8 a7,
        uint8 a8,
        bool flushAndSaveSession)
    {
        if (flushAndSaveSession)
        {
            Core::Hook::Call<void>(0x81DA10);
            Core::StaticRef<uint32>(0x11D51B0) = a0;
            Core::StaticRef<uint32>(0x11D51B4) = a1;
        }

        uint8 buf[80];
        Core::Hook::CallMethod<void*>(0xBCAF20, buf, 6, eventId);
        *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B27C);
        *reinterpret_cast<uint32*>(buf + 0x38) = a0;
        *reinterpret_cast<uint32*>(buf + 0x3C) = a1;
        *reinterpret_cast<uint32*>(buf + 0x40) = a2;
        *reinterpret_cast<uint32*>(buf + 0x44) = a3;
        buf[0x48] = a4;
        buf[0x49] = a6;
        buf[0x4A] = a7;
        buf[0x4B] = a8;
        buf[0x4C] = a5;
        Core::Hook::Call<int32>(0x81DB10, buf, 80);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }

    void RecordPosEvent(int32 eventId)
    {
        uint8 buf[56];
        Core::Hook::CallMethod<void*>(0xBCAF20, buf, 1, eventId);
        *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B204);
        float vec[4]{};
        const float* pos = Core::Hook::Call<const float*>(0x818050, vec);
        CWeaponInfo::PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));
        Core::Hook::Call<int32>(0x81DB10, buf, 56);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }

    void RecordWeaponHashEvent(int32 eventId, int32 index, uintptr_t knownStrAddr, uintptr_t unkStr1Addr, uintptr_t unkStr2Addr)
    {
        uint8 buf[56];
        Core::Hook::CallMethod<void*>(0xBCAF20, buf, 1, eventId);
        *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B204);
        float vec[4]{};
        const float* pos = Core::Hook::Call<const float*>(0x818050, vec);
        CWeaponInfo::PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));

        uint32 hash = 0;
        const char* str = nullptr;
        if (index >= 0)
        {
            const uint8* info = reinterpret_cast<const uint8**>(Core::Rebase(0x15F73B0))[index];
            hash = *reinterpret_cast<const uint32*>(info + 0x3C);
            str = reinterpret_cast<const char*>(Core::Rebase(knownStrAddr));
        }
        else
        {
            hash = Core::Hook::Call<uint32>(0x5B1CA0, reinterpret_cast<const char*>(Core::Rebase(unkStr2Addr)), 0u);
            str = reinterpret_cast<const char*>(Core::Rebase(unkStr1Addr));
        }

        Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, str);
        Core::Hook::Call<int32>(0x81DB10, buf, 56);
        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }
}

void CWeaponInfo::InjectHooks()
{
    // Skipped in 0x81D000..0x81F500:
    // - 0x81D000: mid-function fragment of sub_81CF80
    // - 0x81D160: 5-byte import thunk (jmp WSAGetLastError)
    // - 0x81D3E0: custom LTCG calling convention (edx + stack arg)
    // - 0x81D400: custom LTCG calling convention (edi register arg)
    // - 0x81DB10: ambiguous this/ECX in sub_75D510 and unknown size of byte_11D51D0
    // - 0x81E980: custom LTCG calling convention (ecx + edx + stack args with caller cleanup)
    // - 0x81EBF0: 5-byte tail-jump thunk to 0x81E3D0
    // - 0x81F490: truncated at 0x81F500 (jump table at 0x81F508/0x81F510 outside range)

    Core::Hook::Install(0x81D040, &CWeaponInfo::SortPtrArray);
    Core::Hook::Install(0x81D090, &CWeaponInfo::FindAvailableSlot);
    Core::Hook::Install(0x81D140, &CWeaponInfo::GetMaxPacketSize1);
    Core::Hook::Install(0x81D150, &CWeaponInfo::GetMaxPacketSize2);
    Core::Hook::Install(0x81D170, &CWeaponInfo::SocketSelectWrite);
    Core::Hook::Install(0x81D1D0, &CWeaponInfo::IsConnected);
    Core::Hook::Install(0x81D1E0, &CWeaponInfo::HasPendingFlag);
    Core::Hook::Install(0x81D1F0, &CWeaponInfo::ClearPendingFlag);
    Core::Hook::Install(0x81D210, &CWeaponInfo::StartConnect);
    Core::Hook::Install(0x81D2B0, &CWeaponInfo::ResolveServers);
    Core::Hook::Install(0x81D3B0, &CWeaponInfo::CancelResolve);
    Core::Hook::Install(0x81D470, &CWeaponInfo::SendEncryptedChunk);
    Core::Hook::Install(0x81D620, &CWeaponInfo::IsTelemetryStatAllowed);
    Core::Hook::Install(0x81D8F0, &CWeaponInfo::ShutdownTelemetry);
    Core::Hook::Install(0x81D970, &CWeaponInfo::FormatMacAddress);
    Core::Hook::Install(0x81DA00, &CWeaponInfo::GetSessionId);
    Core::Hook::Install(0x81DA10, &CWeaponInfo::FlushEvents);

    Core::Hook::Install(0x81DBE0, &CWeaponInfo::RecordEventByIndex);
    Core::Hook::Install(0x81DC60, &CWeaponInfo::RecordEvent9);
    Core::Hook::Install(0x81DCA0, &CWeaponInfo::RecordEvent10);
    Core::Hook::Install(0x81DCE0, &CWeaponInfo::RecordEvent4);
    Core::Hook::Install(0x81DD40, &CWeaponInfo::RecordEvent5);
    Core::Hook::Install(0x81DDA0, &CWeaponInfo::RecordEvent6);
    Core::Hook::Install(0x81DE10, &CWeaponInfo::RecordEvent23);
    Core::Hook::Install(0x81DE70, &CWeaponInfo::RecordEvent13);
    Core::Hook::Install(0x81DEB0, &CWeaponInfo::RecordEvent14);
    Core::Hook::Install(0x81DEF0, &CWeaponInfo::RecordEvent15);
    Core::Hook::Install(0x81DF30, &CWeaponInfo::RecordEvent17);
    Core::Hook::Install(0x81DF90, &CWeaponInfo::RecordEvent18);
    Core::Hook::Install(0x81DFF0, &CWeaponInfo::RecordEvent19);

    Core::Hook::Install(0x81E050, &CWeaponInfo::RecordEvent32);
    Core::Hook::Install(0x81E100, &CWeaponInfo::RecordEvent33);
    Core::Hook::Install(0x81E1B0, &CWeaponInfo::RecordEvent34);
    Core::Hook::Install(0x81E200, &CWeaponInfo::RecordEvent36);
    Core::Hook::Install(0x81E260, &CWeaponInfo::RecordEvent37);
    Core::Hook::Install(0x81E2C0, &CWeaponInfo::RecordEvent38);
    Core::Hook::Install(0x81E350, &CWeaponInfo::RecordEvent39);
    Core::Hook::Install(0x81E390, &CWeaponInfo::RecordEvent21);
    Core::Hook::Install(0x81E3D0, &CWeaponInfo::RecordEvent22);
    Core::Hook::Install(0x81E420, &CWeaponInfo::RecordEvent24);
    Core::Hook::Install(0x81E490, &CWeaponInfo::RecordEvent25);
    Core::Hook::Install(0x81E510, &CWeaponInfo::RecordEvent26);
    Core::Hook::Install(0x81E590, &CWeaponInfo::RecordEvent27);
    Core::Hook::Install(0x81E610, &CWeaponInfo::RecordEvent31);
    Core::Hook::Install(0x81E670, &CWeaponInfo::RecordEvent44);
    Core::Hook::Install(0x81E6C0, &CWeaponInfo::SetTelemetryValue);
    Core::Hook::Install(0x81E6D0, &CWeaponInfo::PackVec2ToInt16);
    Core::Hook::Install(0x81E6F0, &CWeaponInfo::ResetStatBuffers);
    Core::Hook::Install(0x81E730, &CWeaponInfo::ReadBitStreamBits);
    Core::Hook::Install(0x81E790, &CWeaponInfo::ScalarDeletingDtor);
    Core::Hook::Install(0x81E7B0, &CWeaponInfo::InitClient);
    Core::Hook::Install(0x81E830, &CWeaponInfo::ReconnectClient);
    Core::Hook::Install(0x81E840, &CWeaponInfo::DisconnectAndResetClient);
    Core::Hook::Install(0x81E860, &CWeaponInfo::ConnectToServer);
    Core::Hook::Install(0x81EB10, &CWeaponInfo::ResetAndRecordEvent21);
    Core::Hook::Install(0x81EB30, &CWeaponInfo::RecordStatChanges);
    Core::Hook::Install(0x81EC00, &CWeaponInfo::SetSessionId);
    Core::Hook::Install(0x81EC20, &CWeaponInfo::RecordEvent7);
    Core::Hook::Install(0x81ECD0, &CWeaponInfo::RecordEvent8);
    Core::Hook::Install(0x81ED80, &CWeaponInfo::RecordEvent42);
    Core::Hook::Install(0x81EE30, &CWeaponInfo::RecordEvent1);
    Core::Hook::Install(0x81EEA0, &CWeaponInfo::RecordEvent2);
    Core::Hook::Install(0x81EFA0, &CWeaponInfo::RecordEvent3);
    Core::Hook::Install(0x81F010, &CWeaponInfo::RecordEvent12);
    Core::Hook::Install(0x81F140, &CWeaponInfo::RecordEvent16);
    Core::Hook::Install(0x81F1E0, &CWeaponInfo::RecordEvent20);
    Core::Hook::Install(0x81F250, &CWeaponInfo::RecordEvent40);
    Core::Hook::Install(0x81F310, &CWeaponInfo::RecordEvent41);
    Core::Hook::Install(0x81F380, &CWeaponInfo::RecordEvent46);
    Core::Hook::Install(0x81F420, &CWeaponInfo::RecordEvent43);

    Core::Log::Info("CWeaponInfo hooks installed (70 functions, 8 skipped)");
}

void __cdecl CWeaponInfo::SortPtrArray(int32* first, int32* last, int32 pred)
{
    if (first == last)
        return;

    int32 count = static_cast<int32>(last - first);
    int32 depth = 0;
    while (count != 1)
    {
        count >>= 1;
        ++depth;
    }

    Core::Hook::Call<void>(0x81CF80, first, last, 0, depth * 2, pred);
    Core::Hook::Call<void>(0x81CE90, first, last, pred);
}

int32 __cdecl CWeaponInfo::FindAvailableSlot()
{
    uint32 ptrs[32];
    const uintptr_t base = Core::Rebase(0x11A7088);
    for (int32 i = 0; i < 32; ++i)
        ptrs[i] = static_cast<uint32>(base + i * 4);

    SortPtrArray(reinterpret_cast<int32*>(ptrs), reinterpret_cast<int32*>(ptrs + 32), 0);

    const uint32* slotArray = reinterpret_cast<const uint32*>(Core::Rebase(0x11A7008));
    const uint8* extBase = reinterpret_cast<const uint8*>(Core::Rebase(0x192243C));
    for (int32 i = 0; i < 32; ++i)
    {
        const int32 slotIdx = static_cast<int32>(ptrs[i] - base) >> 2;
        const uint8* obj = reinterpret_cast<const uint8*>(slotArray[slotIdx]);
        if (!obj || *reinterpret_cast<const int32*>(obj + 0x4DC) == 6)
        {
            if (Core::Hook::Call<bool>(0x462F90) && *reinterpret_cast<const int32*>(extBase + i * 0x1C) != 0)
                continue;
            return slotIdx;
        }
    }
    return -1;
}

int32 __cdecl CWeaponInfo::GetMaxPacketSize1()
{
    return 0x3C;
}

int32 __cdecl CWeaponInfo::GetMaxPacketSize2()
{
    return 0x58;
}

int32 __cdecl CWeaponInfo::SocketSelectWrite(int32 sock, int32 timeoutMs)
{
    struct TimeVal
    {
        int32 tv_sec;
        int32 tv_usec;
    } tv{ static_cast<int32>(static_cast<uint32>(timeoutMs) / 1000u), 0 };

    struct FdSet
    {
        uint32 fd_count;
        int32 fd_array[64];
    } writeFds{};
    writeFds.fd_count = 1;
    writeFds.fd_array[0] = sock;

    return Core::Hook::CallStdcall<int32>(0xD30378, sock + 1, nullptr, &writeFds, nullptr, &tv);
}

bool __thiscall CWeaponInfo::IsConnected()
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    return *reinterpret_cast<const int32*>(self + 0x7E0) == 1;
}

uint8 __thiscall CWeaponInfo::HasPendingFlag()
{
    const uint8* self = reinterpret_cast<const uint8*>(this);
    return *reinterpret_cast<const uint8*>(self + 0x7E8) & 1;
}

void __thiscall CWeaponInfo::ClearPendingFlag()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    *reinterpret_cast<uint8*>(self + 0x7E8) &= 0xFE;
    *reinterpret_cast<uint32*>(self + 0x120) = 0;
}

bool __thiscall CWeaponInfo::StartConnect()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void* resolver = self + 0x124;
    if (Core::Hook::CallMethod<bool>(0x7663B0, resolver))
        return false;

    uint32 localVal = 0;
    void** handlePtr = reinterpret_cast<void**>(self + 0x148);
    if (Core::Hook::CallStdcall<int32>(0xD30384, 0, 8, &localVal, handlePtr) != 0)
        return false;

    Core::Hook::CallMethod<void>(0x7662C0, resolver, reinterpret_cast<const char*>(Core::Rebase(0xD5B128)), self + 0x7E0);
    const void* host = Core::Hook::CallMethod<const void*>(0x404B90, resolver);
    const int32 err = Core::Hook::CallStdcall<int32>(0xD3037E, *handlePtr, self + 0x150, 0x680, 0, host);
    return err == 0 || err == 0x3E5;
}

void __thiscall CWeaponInfo::ResolveServers()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    void* resolver = self + 0x124;
    if (!Core::Hook::CallMethod<bool>(0x7663B0, resolver))
        return;

    Core::Hook::CallMethod<void>(0x766350, resolver);
    if (Core::Hook::CallMethod<bool>(0x7663B0, resolver))
        return;

    uint32 count = 0;
    const void* host = Core::Hook::CallMethod<const void*>(0x404B90, resolver);
    if (Core::Hook::CallStdcall<int32>(0xD30396, host, &count, 0) == 0 && static_cast<int32>(count) > 0)
    {
        uint8* dst = self + 2;
        const uint8* entry = self + 0x150;
        for (int32 i = 0; i < static_cast<int32>(count) && i < 8; ++i)
        {
            struct {
                uint32 port = 0;
                uint32 ip = 0;
                uint32 extra = 0;
            } resolved;
            Core::Hook::CallMethod<void>(0x766670, &resolved);
            const uint32 addrVal = Core::Hook::CallStdcall<uint32>(
                0xD30390,
                *reinterpret_cast<const char* const*>(entry),
                entry + 8);
            Core::Hook::CallMethod<void>(0x766680, &resolved, addrVal);
            if (resolved.extra == 0)
                resolved.extra = 0x7D0;
            if (resolved.ip == 0)
                resolved.ip = 0x5454080A;

            *reinterpret_cast<uint32*>(dst + 2) = *reinterpret_cast<const uint32*>(entry);
            *reinterpret_cast<uint16*>(dst) = Core::Hook::CallStdcall<uint16>(0xD3038A, static_cast<uint16>(resolved.extra));
            *reinterpret_cast<uint32*>(dst + 0x1E) = resolved.port;
            ++(*reinterpret_cast<int32*>(self + 0x120));

            entry += 0xD0;
            dst += 0x24;
        }
    }

    Core::Hook::CallStdcall<void>(0xD3034E, *reinterpret_cast<void**>(self + 0x148));
    *reinterpret_cast<void**>(self + 0x148) = nullptr;
}

void __thiscall CWeaponInfo::CancelResolve()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (IsConnected())
    {
        Core::Hook::CallMethod<void>(0x7663D0, self + 0x124);
        Core::Hook::CallStdcall<void>(0xD3034E, *reinterpret_cast<void**>(self + 0x148));
        *reinterpret_cast<void**>(self + 0x148) = nullptr;
    }
}

int32 __cdecl CWeaponInfo::SendEncryptedChunk(int32 sock, const void* data, int32 a3, int32 a4, uint8 xorKey, int32* totalSent, uint8 a7)
{
    struct ChunkHeader
    {
        int32 field0;
        int32 field4;
        int32 field8;
        const void* dataPtr;
        uint32 remaining;
    };

    uint8 buf[1028];
    (void)a3;
    const uint8* dataBytes = static_cast<const uint8*>(data);
    ChunkHeader* hdr = *reinterpret_cast<ChunkHeader* const*>(dataBytes + 0x400);
    bool ok = true;

    hdr->field4 = a4;
    hdr->field0 = a3;
    const int32 flags = a7 ? 4 : 0;

    while (true)
    {
        if (hdr->remaining == 0)
        {
            hdr->remaining = 0x400;
            hdr->dataPtr = dataBytes;
        }

        if (Core::Hook::Call<int32>(0x5BFC70, hdr, flags) < 0)
            return -1;

        if (hdr->remaining > 0 && a7 == 0)
            return ok ? a4 : -1;

        uint32 rem = 0x400u - hdr->remaining;
        uint8* dst = buf;
        if (*totalSent == 0)
        {
            buf[0] = 0;
            buf[1] = 1;
            buf[2] = xorKey;
            dst = buf + 3;
        }

        for (uint32 i = 0; i < rem; ++i)
            dst[i] = dataBytes[i] ^ xorKey;

        const uint8* sendPtr = buf;
        while (rem > 0 && ok)
        {
            const int32 sel = SocketSelectWrite(sock, 0x1B58);
            if (sel == 0)
            {
                ok = false;
            }
            else if (sel < 0)
            {
                ok = false;
                Core::Hook::CallStdcall<int32>(0xD30372);
            }
            else
            {
                const int32 sent = Core::Hook::CallStdcall<int32>(0xD303B4, sock, sendPtr, rem, 0);
                if (sent <= 0)
                {
                    if (Core::Hook::CallStdcall<int32>(0xD30372) != 0x2733)
                        ok = false;
                }
                else
                {
                    rem -= static_cast<uint32>(sent);
                    sendPtr += sent;
                    *totalSent += sent;
                }
            }
        }

        if (hdr->remaining != 0 || !ok)
            break;
    }

    return ok ? a4 : -1;
}

bool __cdecl CWeaponInfo::IsTelemetryStatAllowed(uint32 id)
{
    if (Core::StaticRef<int32>(0x10F8044) != 2)
        return false;

    if (id <= 0x15E)
    {
        if (id >= 0x14D)
            return true;

        const uint32 idx = id - 1;
        if (idx <= 0xFB && reinterpret_cast<const uint8*>(Core::Rebase(0x81D6C4))[idx] == 0)
            return true;
    }
    else if (id <= 0x1CE)
    {
        if (id == 0x1CE)
            return true;

        const uint32 idx = id - 0x160;
        if (idx <= 0x4C && reinterpret_cast<const uint8*>(Core::Rebase(0x81D7C8))[idx] == 0)
            return true;
    }
    else
    {
        const uint32 idx = id - 0x1D6;
        if (idx <= 0xCD && reinterpret_cast<const uint8*>(Core::Rebase(0x81D820))[idx] == 0)
            return true;
    }

    Core::Hook::Call<bool>(0x4626B0);
    return false;
}

void __cdecl CWeaponInfo::ShutdownTelemetry()
{
    const uint32 current = Core::StaticRef<uint32>(0xE3447C);
    uint32& lastSynced = Core::StaticRef<uint32>(0x11D51CC);
    void*& objA = Core::StaticRef<void*>(0x11D368C);
    void*& objB = Core::StaticRef<void*>(0x11D4DB0);

    if (current != lastSynced)
    {
        Core::StaticRef<uint8>(0x11D36A6) = 1;
        Core::Hook::Call<void>(0x5A86D0, objA);
        Core::Hook::Call<void>(0x5A8690, objB);
        Core::StaticRef<uint8>(0x11D36A6) = 0;
        lastSynced = Core::StaticRef<uint32>(0xE3447C);
    }

    if (objB)
    {
        Core::Hook::Call<void>(0x5A86F0, objB);
        objB = nullptr;
    }
    if (objA)
    {
        Core::Hook::Call<void>(0x5A86F0, objA);
        objA = nullptr;
    }
    Core::StaticRef<uint32>(0x11D4D64) = 0;
}

bool __cdecl CWeaponInfo::FormatMacAddress(const char* prefix, char* outBuf)
{
    uint8 guid[16]{};
    const uint32 res = Core::Hook::CallStdcall<uint32>(0xD303BA, guid);
    if (res & 0x1C)
    {
        Core::Hook::Call<int32>(
            0xD0C8E4,
            outBuf,
            reinterpret_cast<const char*>(Core::Rebase(0xD5B17C)),
            prefix,
            guid[0],
            guid[1],
            guid[2],
            guid[3]);
    }
    else
    {
        Core::Hook::Call<int32>(
            0xD0C8E4,
            outBuf,
            reinterpret_cast<const char*>(Core::Rebase(0xD5B194)),
            prefix);
    }
    return true;
}

uint64 __cdecl CWeaponInfo::GetSessionId()
{
    const uint32 lo = Core::StaticRef<uint32>(0x11D51B8);
    const uint32 hi = Core::StaticRef<uint32>(0x11D51BC);
    return (static_cast<uint64>(hi) << 32) | lo;
}

void __cdecl CWeaponInfo::FlushEvents()
{
    const uint32 now = Core::Hook::Call<uint32>(0x5AFB10) | 1u;
    Core::StaticRef<uint32>(0x11D4DAC) = now;
    uint32& pendingBytes = Core::StaticRef<uint32>(0x11D4DA8);

    if (pendingBytes > 0 && Core::Hook::CallMethod<bool>(0x5BC270, reinterpret_cast<void*>(Core::Rebase(0x11D5298))))
    {
        uint32& bufIndex = Core::StaticRef<uint32>(0x11D51AC);
        const uint32 nextIndex = (bufIndex == 0) ? 1u : 0u;
        std::memcpy(reinterpret_cast<void*>(Core::Rebase(0x11D5238)), reinterpret_cast<const void*>(Core::Rebase(0x11D51D0)), 0x60);

        const uint32 b4 = Core::StaticRef<uint32>(0x11D51B4);
        bufIndex = nextIndex;
        Core::StaticRef<uintptr_t>(0x11D5230) = Core::Rebase(0x11D3CD8) + (nextIndex << 11);
        Core::StaticRef<uint32>(0x11D4D64) = pendingBytes;

        const uint32 b0 = Core::StaticRef<uint32>(0x11D51B0);
        if ((b0 | b4) != 0)
        {
            Core::StaticRef<uint32>(0x11D3698) = 0;
            Core::StaticRef<uint32>(0x11D51C0) = b0;
            Core::StaticRef<uint32>(0x11D51C4) = b4;
        }
        else
        {
            Core::StaticRef<uint32>(0x11D3698) = 1;
            Core::StaticRef<uint32>(0x11D51C0) = Core::StaticRef<uint32>(0x11D51B8);
            Core::StaticRef<uint32>(0x11D51C4) = Core::StaticRef<uint32>(0x11D51BC);
        }

        uint32& lastSendTime = Core::StaticRef<uint32>(0x11D3690);
        if (lastSendTime == 0 || (now - lastSendTime) >= Core::StaticRef<uint32>(0xF1CC98))
        {
            Core::Hook::Call<void>(0x467E60, reinterpret_cast<void*>(Core::Rebase(0x11D369C)), 0x50);
            Core::StaticRef<uint8>(0x11D51C8) = 1;
            lastSendTime = now;
        }

        Core::Hook::CallMethod<void>(0x5BC290, reinterpret_cast<void*>(Core::Rebase(0x11D5298)));
        Core::Hook::Call<void>(0x5A86D0, Core::StaticRef<void*>(0x11D368C));
    }

    pendingBytes = 0;
}

void __cdecl CWeaponInfo::RecordEventByIndex(int32 index)
{
    const char* name = reinterpret_cast<const char**>(Core::Rebase(0xF27A88))[index];
    if (!name)
        name = "UNKNOWN";

    uint8 buf[52];
    Core::Hook::CallMethod<void*>(0xBCAF00, buf, 11);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    Core::Hook::Call<int32>(0x81DB10, buf, 52);
    m_dwLastEventTime = Core::Hook::Call<uint32>(0x5AFB10) | 1u;
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent9()
{
    RecordSimpleEvent(9);
}

void __cdecl CWeaponInfo::RecordEvent10()
{
    RecordSimpleEvent(10);
}

void __cdecl CWeaponInfo::RecordEvent4(const char* name)
{
    RecordNamedEvent(4, name);
}

void __cdecl CWeaponInfo::RecordEvent5(const char* name)
{
    RecordNamedEvent(5, name);
}

void __cdecl CWeaponInfo::RecordEvent6(const char* name)
{
    RecordNamedEvent(6, name, true);
}

void __cdecl CWeaponInfo::RecordEvent23(const char* name)
{
    RecordNamedEvent(23, name);
}

void __cdecl CWeaponInfo::RecordEvent13()
{
    RecordSimpleEvent(13);
}

void __cdecl CWeaponInfo::RecordEvent14()
{
    RecordSimpleEvent(14);
}

void __cdecl CWeaponInfo::RecordEvent15()
{
    RecordSimpleEvent(15);
}

void __cdecl CWeaponInfo::RecordEvent17(const char* name)
{
    RecordNamedEvent(17, name);
}

void __cdecl CWeaponInfo::RecordEvent18(const char* name)
{
    RecordNamedEvent(18, name);
}

void __cdecl CWeaponInfo::RecordEvent19(const char* name)
{
    RecordNamedEvent(19, name);
}

void __cdecl CWeaponInfo::RecordEvent32(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8)
{
    RecordSessionPacketEvent(32, a0, a1, a2, a3, a4, a5, a6, a7, a8, true);
}

void __cdecl CWeaponInfo::RecordEvent33(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8)
{
    RecordSessionPacketEvent(33, a0, a1, a2, a3, a4, a5, a6, a7, a8, true);
}

void __cdecl CWeaponInfo::RecordEvent34()
{
    RecordSimpleEvent(34, true);
    Core::StaticRef<uint32>(0x11D51B0) = 0;
    Core::StaticRef<uint32>(0x11D51B4) = 0;
}

void __cdecl CWeaponInfo::RecordEvent36(const char* name)
{
    RecordNamedEvent(36, name);
}

void __cdecl CWeaponInfo::RecordEvent37(const char* name)
{
    RecordNamedEvent(37, name);
}

void __cdecl CWeaponInfo::RecordEvent38(uint32 a0, uint32 a1, uint32 a2, uint32 a3, uint8 a4, uint8 a5, uint8 a6, uint8 a7, uint8 a8)
{
    RecordSessionPacketEvent(38, a0, a1, a2, a3, a4, a5, a6, a7, a8, false);
}

void __cdecl CWeaponInfo::RecordEvent39()
{
    RecordSimpleEvent(39);
}

void __cdecl CWeaponInfo::RecordEvent21()
{
    RecordSimpleEvent(21);
}

void __cdecl CWeaponInfo::RecordEvent22()
{
    RecordSimpleEvent(22, true);
}

void __cdecl CWeaponInfo::RecordEvent24(const char* name, int32 val)
{
    uint8 buf[56];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 1, 24);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B204);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    *reinterpret_cast<int32*>(buf + 0x34) = val;
    Core::Hook::Call<int32>(0x81DB10, buf, 56);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent25(const char* name, int32 a2, int32 a3)
{
    uint8 buf[60];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 3, 25);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B234);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    *reinterpret_cast<int32*>(buf + 0x34) = a2;
    *reinterpret_cast<int32*>(buf + 0x38) = a3;
    Core::Hook::Call<int32>(0x81DB10, buf, 60);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent26(const char* name, float val)
{
    uint8 buf[56];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 2, 26);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B21C);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    *reinterpret_cast<float*>(buf + 0x34) = val;
    Core::Hook::Call<int32>(0x81DB10, buf, 56);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent27(const char* name, int32 a2, float val)
{
    uint8 buf[60];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 4, 27);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B24C);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    *reinterpret_cast<int32*>(buf + 0x34) = a2;
    *reinterpret_cast<float*>(buf + 0x38) = val;
    Core::Hook::Call<int32>(0x81DB10, buf, 60);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent31(const char* name)
{
    RecordNamedEvent(31, name);
}

void __cdecl CWeaponInfo::RecordEvent44(int32 val)
{
    uint8 buf[56];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 1, 44);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B204);
    *reinterpret_cast<int32*>(buf + 0x34) = val;
    Core::Hook::Call<int32>(0x81DB10, buf, 56);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::SetTelemetryValue(int32 val)
{
    Core::StaticRef<int32>(0xF1CC84) = val;
}

void __cdecl CWeaponInfo::PackVec2ToInt16(const float* in, int16* out)
{
    out[0] = static_cast<int16>(static_cast<int32>(in[0]));
    out[1] = static_cast<int16>(static_cast<int32>(in[1]));
}

void __cdecl CWeaponInfo::ResetStatBuffers()
{
    std::memset(reinterpret_cast<void*>(Core::Rebase(0x11D4DB8)), 0, 0x3F4);
    std::memset(reinterpret_cast<void*>(Core::Rebase(0x11D36A8)), 0, 0x630);
    std::memset(reinterpret_cast<void*>(Core::Rebase(0x11D4CD8)), 0, 0x84);
}

bool __thiscall CWeaponInfo::ReadBitStreamBits(uint32* outVal, uint32 bitCount)
{
    uint8* self = reinterpret_cast<uint8*>(this);
    if (!Core::Hook::CallMethod<bool>(0x5B8FA0, this, bitCount))
    {
        *outVal = 0;
        return false;
    }

    uint32 val = 0;
    Core::Hook::Call<void>(
        0x5B8CE0,
        *reinterpret_cast<void**>(self),
        &val,
        bitCount,
        *reinterpret_cast<int32*>(self + 0xC) + *reinterpret_cast<int32*>(self + 0x4));
    *outVal = val;
    Core::Hook::CallMethod<void>(0x5B9580, this, bitCount);
    return true;
}

void* __thiscall CWeaponInfo::ScalarDeletingDtor(uint8 flags)
{
    Core::Hook::CallMethod<void>(0xBCAF50, this);
    if (flags & 1)
        Core::Hook::Call<void>(0x5B1C10, this);
    return this;
}

CWeaponInfo* __thiscall CWeaponInfo::InitClient()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    for (int32 i = 0; i < 8; ++i)
    {
        uint8* entry = self + i * 0x24;
        std::memset(entry, 0, 0x20);
        *reinterpret_cast<uint16*>(entry + 0x00) = 2;
        *reinterpret_cast<uint16*>(entry + 0x10) = 2;
        *reinterpret_cast<uint32*>(entry + 0x20) = 0x5454080A;
    }

    *reinterpret_cast<int32*>(self + 0x120) = 0;
    Core::Hook::CallMethod<void>(0x7662A0, self + 0x124);
    *reinterpret_cast<int32*>(self + 0x7E0) = 0;
    *reinterpret_cast<int32*>(self + 0x7E4) = 0;
    *reinterpret_cast<uint8*>(self + 0x7E8) &= 0xFE;
    *reinterpret_cast<void**>(self + 0x148) = nullptr;
    return this;
}

bool __thiscall CWeaponInfo::ReconnectClient()
{
    ClearPendingFlag();
    return StartConnect();
}

void __thiscall CWeaponInfo::DisconnectAndResetClient()
{
    if (IsConnected())
    {
        CancelResolve();
        ClearPendingFlag();
    }
}

bool __thiscall CWeaponInfo::ConnectToServer()
{
    uint8* self = reinterpret_cast<uint8*>(this);
    std::memset(self + 0x7D0, 0, 16);

    const int32 count = *reinterpret_cast<int32*>(self + 0x120);
    if (count == 0)
        return false;

    uint8 addrBytes[8]{};
    Core::Hook::Call<void>(0x7667E0, addrBytes);
    const uint32 randWord =
        (static_cast<uint32>(addrBytes[0]) << 24) |
        (static_cast<uint32>(addrBytes[1]) << 16) |
        (static_cast<uint32>(addrBytes[2]) << 8) |
        static_cast<uint32>(addrBytes[3]);

    const uint32 ticks = Core::Hook::Call<uint32>(0x5AFB10) ^ randWord;
    const uint32 adj = (ticks == 0) ? 1u : 0u;
    const uint32 rotated = (ticks << 16) | (ticks >> 16);
    const int32 hash = static_cast<int32>(((rotated ^ ticks) - ((ticks + adj) * 0x23230559u)) & 0x7FFFFFFFu);
    const int32 idx = hash % count;

    const uint8* entry = self + idx * 0x24;
    std::memcpy(self + 0x7D0, entry, 16);

    uint8 sockAddr[16]{};
    std::memcpy(sockAddr, entry, 16);
    const uint32 sock = *reinterpret_cast<const uint32*>(entry + 0x20);
    const int32 res = Core::Hook::CallStdcall<int32>(0xD303C0, *reinterpret_cast<const int32*>(sockAddr + 4), sock, self + 0x7D4);
    return res == 0;
}

void __cdecl CWeaponInfo::ResetAndRecordEvent21()
{
    Core::StaticRef<uint32>(0x11D3694) = 0;
    Core::StaticRef<uint32>(0x11D4DB4) = 0;
    RecordEvent21();
}

void __cdecl CWeaponInfo::RecordStatChanges()
{
    if (Core::StaticRef<int32>(0x10F8044) != 2)
        return;

    const int32* stats = Core::Hook::Call<const int32*>(0x963370);
    const int32 numStats = Core::Hook::Call<int32>(0x9633B0);
    int32* cached = reinterpret_cast<int32*>(Core::Rebase(0x11D36A8));

    for (int32 i = 0; i < numStats; ++i)
    {
        const int32 statId = 0xFD + i;
        if (statId >= 0x26F && statId <= 0x283 && stats[i] != cached[i])
        {
            uint8 buf[60];
            Core::Hook::CallMethod<void*>(0xBCAF20, buf, 3, 28);
            *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B234);
            cached[i] = stats[i];
            *reinterpret_cast<int32*>(buf + 0x34) = statId;
            *reinterpret_cast<int32*>(buf + 0x38) = stats[i];
            Core::Hook::Call<int32>(0x81DB10, buf, 60);
            cached[i] = stats[i];
            Core::Hook::CallMethod<void>(0xBCAF50, buf);
        }
    }

    FlushEvents();
}

void __cdecl CWeaponInfo::SetSessionId(uint32 low, uint32 high)
{
    FlushEvents();
    Core::StaticRef<uint32>(0x11D51B8) = low;
    Core::StaticRef<uint32>(0x11D51BC) = high;
}

void __cdecl CWeaponInfo::RecordEvent7(int32 index)
{
    RecordWeaponHashEvent(7, index, 0xD5B139, 0xD5B198, 0xD5B1A0);
}

void __cdecl CWeaponInfo::RecordEvent8(int32 index)
{
    RecordWeaponHashEvent(8, index, 0xD5B13A, 0xD5B1A8, 0xD5B1B0);
}

void __cdecl CWeaponInfo::RecordEvent42(int32 index)
{
    RecordWeaponHashEvent(42, index, 0xD5B13B, 0xD5B1B8, 0xD5B1C0);
}

void __cdecl CWeaponInfo::RecordEvent1()
{
    RecordPosEvent(1);
}

void __cdecl CWeaponInfo::RecordEvent2(int32 index, const void* obj)
{
    uint8 buf[88]{};
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 5, 2);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B264);

    float vec[4]{};
    const float* pos = Core::Hook::Call<const float*>(0x818050, vec);
    PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));

    const char* name = reinterpret_cast<const char**>(Core::Rebase(0xF27A88))[index];
    if (!name)
        name = reinterpret_cast<const char*>(Core::Rebase(0xD5B1D0));
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);

    if (obj)
    {
        const uint8* objBytes = static_cast<const uint8*>(obj);
        const bool flag = objBytes[0x219] != 0;
        const char* extra = nullptr;
        if (flag)
        {
            extra = Core::Hook::CallMethod<const char*>(0x75D2E0, *reinterpret_cast<void* const*>(objBytes + 0x228));
        }
        else
        {
            const uint8* sub = *reinterpret_cast<const uint8* const*>(objBytes + 0x21C);
            extra = Core::Hook::Call<const char*>(0xBCB950, *reinterpret_cast<const uint32*>(sub + 0x12C));
        }

        if (extra)
        {
            char* dst = reinterpret_cast<char*>(buf + 0x38);
            if (!flag)
            {
                dst[0] = '*';
                dst[1] = '\0';
            }
            Core::Hook::Call<void*>(0x5B2B20, dst, extra, 0x20);
        }
    }

    Core::Hook::Call<int32>(0x81DB10, buf, 88);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent3()
{
    RecordPosEvent(3);
}

void __cdecl CWeaponInfo::RecordEvent12()
{
    Core::StaticRef<uint32>(0x11D52D0) |= 1u;

    float vec[4]{};
    Core::Hook::Call<const float*>(0x818050, vec);
    const float dx = vec[0] - Core::StaticRef<float>(0x11D52C0);
    const float dy = vec[1] - Core::StaticRef<float>(0x11D52C4);
    const float dz = vec[2] - Core::StaticRef<float>(0x11D52C8);
    const float distSq = dx * dx + dy * dy + dz * dz;
    const float threshold = Core::StaticRef<float>(0xF1CCA0);

    if (Core::StaticRef<uint8>(0x11D52B8) != 0 || distSq >= threshold * threshold)
    {
        uint8 buf[56];
        Core::Hook::CallMethod<void*>(0xBCAF20, buf, 1, 12);
        *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B204);
        PackVec2ToInt16(vec, reinterpret_cast<int16*>(buf + 0x34));
        Core::Hook::Call<int32>(0x81DB10, buf, 56);

        Core::StaticRef<uint32>(0x11D4D60) = Core::Hook::Call<uint32>(0x5AFB10) | 1u;
        Core::StaticRef<float>(0x11D52C0) = vec[0];
        Core::StaticRef<float>(0x11D52C4) = vec[1];
        Core::StaticRef<float>(0x11D52C8) = vec[2];
        Core::StaticRef<float>(0x11D52CC) = vec[3];
        Core::StaticRef<uint8>(0x11D52B8) = 0;

        Core::Hook::CallMethod<void>(0xBCAF50, buf);
    }
}

void __cdecl CWeaponInfo::RecordEvent16()
{
    uint8 macRaw[8]{};
    Core::Hook::Call<void>(0x7667E0, macRaw);
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CE0, macRaw, 6, 0);
    const uint64 baseVal = Core::Hook::Call<uint64>(0x5AFB70);
    const uint64 mulVal = Core::Hook::CallStdcall<uint64>(0xD12ED0, hash, 0, 0, 1);
    const uint64 sessionId = baseVal + mulVal;

    SetSessionId(static_cast<uint32>(sessionId), static_cast<uint32>(sessionId >> 32));
    FlushEvents();
    ResetStatBuffers();
    RecordSimpleEvent(16);
}

void __cdecl CWeaponInfo::RecordEvent20()
{
    RecordPosEvent(20);
}

void __cdecl CWeaponInfo::RecordEvent40(const char** namePtr, const float* pos)
{
    uint8 buf[60];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 3, 40);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B234);
    const char* name = *namePtr;
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);
    PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));

    uint32 extra = 0;
    const int32 slotIdx = Core::StaticRef<int32>(0xF1CC68);
    if (slotIdx != -1)
    {
        const uint8* obj = reinterpret_cast<const uint8**>(Core::Rebase(0x11A7008))[slotIdx];
        if (obj)
        {
            const uint8* sub = *reinterpret_cast<const uint8* const*>(obj + 0x58C);
            uint8* target = *reinterpret_cast<uint8* const*>(sub + 0x228);
            extra = Core::Hook::CallMethod<uint32>(0x920FF0, target ? target + 0x60 : nullptr);
        }
    }
    *reinterpret_cast<uint32*>(buf + 0x38) = extra;

    Core::Hook::Call<int32>(0x81DB10, buf, 60);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent41()
{
    RecordPosEvent(41);
}

void __cdecl CWeaponInfo::RecordEvent46(int32 index)
{
    uint8 buf[88]{};
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 5, 46);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B264);

    float vec[4]{};
    const float* pos = Core::Hook::Call<const float*>(0x818050, vec);
    PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));

    const char* name = reinterpret_cast<const char**>(Core::Rebase(0xF27A88))[index];
    if (!name)
        name = reinterpret_cast<const char*>(Core::Rebase(0xD5B1D8));
    const uint32 hash = Core::Hook::Call<uint32>(0x5B1CA0, name, 0u);
    Core::Hook::CallMethod<void*>(0xBCB090, buf, hash, name);

    Core::Hook::Call<int32>(0x81DB10, buf, 88);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}

void __cdecl CWeaponInfo::RecordEvent43(int32 val)
{
    uint8 buf[60];
    Core::Hook::CallMethod<void*>(0xBCAF20, buf, 3, 43);
    *reinterpret_cast<uintptr_t*>(buf + 0x00) = Core::Rebase(0xD5B234);
    *reinterpret_cast<int32*>(buf + 0x38) = val;

    float vec[4]{};
    const float* pos = Core::Hook::Call<const float*>(0x818050, vec);
    PackVec2ToInt16(pos, reinterpret_cast<int16*>(buf + 0x34));

    Core::Hook::Call<int32>(0x81DB10, buf, 60);
    Core::Hook::CallMethod<void>(0xBCAF50, buf);
}
