#include "StdInc.h"
#include "Game/PedAudio.h"
#include "Core/Hook.h"
#include "Core/Vector.h"
#include <cmath>
#include <cstring>
#include <intrin.h>

namespace
{
    struct AudVec4
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;
    };

    struct AudSoundInitParams
    {
        uint8 data[0x4C]{};
    };

    struct AudSpeechParams
    {
        uint32 unk0 = 0;
        int32  unk4 = -1;
        int32  type = 0;
    };

    struct AudAnimSpeechParams
    {
        uint8 data[0x30]{};
    };

    inline bool CanPlayPedAudio()
    {
        if (Core::StaticRef<int32>(0x10D0170) == 1)
            return false;
        if (Core::StaticRef<int32>(0x10E1984) != Core::StaticRef<int32>(0xE3447C))
            return false;
        if (Core::StaticRef<int32>(0xF0E4A0) == 0x12)
            return false;
        return true;
    }

    inline bool CanPlayPedAudioAlt()
    {
        if (Core::StaticRef<int32>(0x10D0170) == 1)
            return true;
        if (Core::StaticRef<int32>(0x10E1984) == Core::StaticRef<int32>(0xE3447C)
            && Core::StaticRef<int32>(0xF0E4A0) != 0x12)
        {
            return false;
        }
        return true;
    }

    inline bool IsNetworkOrCutsceneMode4Or3()
    {
        const int32 mode = Core::StaticRef<int32>(0x10CFDEC);
        if (CanPlayPedAudioAlt() && mode == 4)
            return true;
        return mode == 3;
    }

    inline void StopAndReleaseSound(uint8* soundSlotPtr)
    {
        uint8* snd = *reinterpret_cast<uint8**>(soundSlotPtr);
        if (snd == nullptr)
            return;

        if (CanPlayPedAudio())
        {
            const uint32 handle = *reinterpret_cast<uint32*>(snd + 0xA4);
            Core::Hook::Call<void>(0x7A9C90, handle);
        }
        Core::Hook::CallMethod<void>(0xCE1830, snd, 0);
    }

    inline void StopSoundImmediate(uint8* snd)
    {
        if (snd == nullptr)
            return;
        const uint32 handle = *reinterpret_cast<uint32*>(snd + 0xA4);
        Core::Hook::Call<void>(0x7A9C90, handle);
        Core::Hook::CallMethod<void>(0xCE1830, snd, 0);
    }

    inline void InitSoundParams(AudSoundInitParams& params)
    {
        Core::Hook::CallMethod<void>(0x797520, &params);
    }

    inline void ResetSoundParamsZero(AudSoundInitParams& params)
    {
        std::memset(&params, 0, sizeof(params));
        *reinterpret_cast<int32*>(params.data + 0x38) = -1;
        *reinterpret_cast<float*>(params.data + 0x3C) = Core::StaticRef<float>(0xD90AA0);
        *reinterpret_cast<int32*>(params.data + 0x40) = -1;
        params.data[0x44] = 0xFF;
        params.data[0x45] = 0;
        params.data[0x46] = 0x20;
    }

    inline void FinalizeAndPlaySound(uint8* snd, uint32 unkA4, uint32 unkA8)
    {
        *reinterpret_cast<uint32*>(snd + 0xA4) = unkA4;
        *reinterpret_cast<uint32*>(snd + 0xA8) = unkA8;
        *reinterpret_cast<uint32*>(snd + 0xAC) = 0;
        Core::Hook::CallMethod<void>(0xCE1720, snd, 0, 0, 0);
    }

    inline int32 ComputePedSpeechFlags(uint8* ped)
    {
        const int16 modelIdx = *reinterpret_cast<int16*>(ped + 0x2E);
        uint8* modelInfo = (&Core::StaticRef<uint8*>(0x15F73B0))[modelIdx];
        const uint32 flags = *reinterpret_cast<uint32*>(modelInfo + 0x120);
        return (flags & 2) ? 0 : 0x258;
    }

    inline bool IsPedFemale(uint8* ped)
    {
        return ped[0x218] == 0 && ped[0x219] != 0;
    }

    inline void* DynamicCastSkeletonData(void* obj)
    {
        return Core::Hook::Call<void*>(0xD0F75A, obj, 0, Core::Rebase(0x10C5460), Core::Rebase(0x10C54E0), 0);
    }

    inline float ComputeLinearToMillibels(float gain)
    {
        const float l = static_cast<float>(std::log10(static_cast<double>(gain)));
        return l * Core::StaticRef<float>(0xD91D68) * Core::StaticRef<float>(0xD83530);
    }

    inline void SetEntityRef(uint8** slot, uint8* newEntity)
    {
        if (*slot != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6580, *slot, slot);
            *slot = nullptr;
        }
        *slot = newEntity;
        if (newEntity != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6550, newEntity, slot);
        }
    }

    // 0xAAD000
    void __fastcall PlaySoundByHash(uint8* self, void* /*edx*/, uint32 soundHash)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if ((*reinterpret_cast<uint32*>(ped + 0x28) & 0x800000) != 0)
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        params.data[0x46] &= 0xEF;
        *reinterpret_cast<uint32*>(params.data + 0x18) = Core::StaticRef<uint32>(0x16677AC);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8930, self, soundHash, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x34 };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    // 0xAAD0F0
    void __cdecl UpdatePlayerPedAudio()
    {
        uint8* playerPed = Core::Hook::Call<uint8*>(0x8788D0, 0);
        if (playerPed != nullptr)
            Core::Hook::CallMethod<void>(0xAACCA0, playerPed + 0x3D0);
    }

    // 0xAAD110
    void* __fastcall AudPedAudioEntity_Ctor(uint8* self, void* /*edx*/)
    {
        Core::Hook::CallMethod<void>(0xCE8270, self);
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD91D70);
        Core::Hook::CallMethod<void>(0xCECF80, self + 0x24);
        Core::Hook::CallMethod<void>(0xCECF80, self + 0x44);
        Core::Hook::CallMethod<void>(0xCECF80, self + 0x90);
        Core::Hook::CallMethod<void>(0xCECF80, self + 0x17C);

        *reinterpret_cast<void**>(self + 0x120) = nullptr;
        *reinterpret_cast<uint32*>(self + 0x74) = 0;
        *reinterpret_cast<void**>(self + 0x8) = nullptr;
        self[0x128] = 0;
        *reinterpret_cast<void**>(self + 0xC) = nullptr;
        self[0x10] = 0;
        *reinterpret_cast<void**>(self + 0x14) = nullptr;
        *reinterpret_cast<uint32*>(self + 0xAC) = 0;
        *reinterpret_cast<uint32*>(self + 0xB0) = 0;
        *reinterpret_cast<void**>(self + 0xB4) = nullptr;
        uint32* unkTable = Core::StaticRef<uint32*>(0x1685CE4);
        *reinterpret_cast<uint32*>(self + 0xB8) = unkTable[0];
        *reinterpret_cast<void**>(self + 0x1C) = nullptr;
        *reinterpret_cast<void**>(self + 0x130) = nullptr;
        *reinterpret_cast<void**>(self + 0x134) = nullptr;
        *reinterpret_cast<uint32*>(self + 0x138) = 0;
        *reinterpret_cast<uint32*>(self + 0x13C) = 0;
        Core::Hook::CallMethod<void>(0xCECFF0, self + 0x24, Core::StaticRef<float>(0xEBE4AC), Core::StaticRef<float>(0xEBE4AC), 0.0f, 1.0f);
        *reinterpret_cast<void**>(self + 0x20) = nullptr;
        *reinterpret_cast<void**>(self + 0x18) = nullptr;
        *reinterpret_cast<void**>(self + 0x124) = nullptr;
        *reinterpret_cast<uint32*>(self + 0x140) = 0;
        self[0x1A0] = 0;
        *reinterpret_cast<void**>(self + 0x198) = nullptr;
        *reinterpret_cast<void**>(self + 0x19C) = nullptr;
        *reinterpret_cast<float*>(self + 0x144) = 0.0f;
        return self;
    }

    // 0xAAD200
    void* __fastcall AudPedAudioEntity_GetPed(uint8* self, void* /*edx*/)
    {
        return *reinterpret_cast<void**>(self + 0x120);
    }

    // 0xAAD210
    void __fastcall AudPedAudioEntity_Dtor(uint8* self, void* /*edx*/)
    {
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD91D70);
        if (*reinterpret_cast<void**>(self + 0xC) != nullptr)
        {
            StopAndReleaseSound(self + 0xC);
            *reinterpret_cast<void**>(self + 0xC) = nullptr;
        }
        uint8** refSlot = reinterpret_cast<uint8**>(self + 0xB4);
        if (*refSlot != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6580, *refSlot, refSlot);
            *refSlot = nullptr;
        }
        if (*reinterpret_cast<void**>(self + 0x8) != nullptr)
        {
            Core::Hook::CallMethod<void>(0xCEE020, *reinterpret_cast<void**>(self + 0x8), 0);
        }
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD5295C);
        Core::Hook::CallMethod<void>(0xCE83F0, self);
    }

    // 0xAAD2A0
    void __fastcall PlayUnk130Sound(uint8* self, void* /*edx*/, int32 duration)
    {
        if (!CanPlayPedAudio())
            return;

        uint8** sndSlot = reinterpret_cast<uint8**>(self + 0x130);
        if (*sndSlot == nullptr)
        {
            AudSoundInitParams params;
            InitSoundParams(params);
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

            Core::Hook::CallMethod<void>(0xCE8CE0, self, Core::Rebase(0xD9177C), sndSlot, &params, -1, 0, 0);
            if (*sndSlot != nullptr)
            {
                using SetVarFn = void(__thiscall*)(void*, uintptr_t, float);
                reinterpret_cast<SetVarFn>((*reinterpret_cast<void***>(*sndSlot))[0xC / 4])(
                    *sndSlot, Core::Rebase(0xD9178C), *reinterpret_cast<float*>(self + 0x12C));
                Core::Hook::CallMethod<void>(0xAACFB0, *sndSlot, 0.0f);

                AudSpeechParams sp{ 0, -1, 0x55 };
                AudAnimSpeechParams animSp;
                Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x55);
                *reinterpret_cast<float*>(animSp.data + 0x24) = *reinterpret_cast<float*>(self + 0x12C);
                animSp.data[0x2E] = 2;

                uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91798), 0);
                void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 0, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
                Core::Hook::Call<void>(0x7A98F0, mem, &animSp, *reinterpret_cast<void**>(self + 0x120));
                uint32 obj = Core::Hook::Call<uint32>(0x799CC0, mem);
                FinalizeAndPlaySound(*sndSlot, reinterpret_cast<uint32>(mem), obj);
            }
        }

        if (duration > 0)
            *reinterpret_cast<uint32*>(self + 0x138) = Core::StaticRef<uint32>(0x11DDE74) + static_cast<uint32>(duration);
        else
            *reinterpret_cast<uint32*>(self + 0x138) = 0;
    }

    // 0xAAD430
    void __fastcall StopUnk130Sound(uint8* self, void* /*edx*/)
    {
        StopAndReleaseSound(self + 0x130);
    }

    // 0xAAD480
    void __fastcall PlayUnk134Sound(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8** sndSlot = reinterpret_cast<uint8**>(self + 0x134);
        if (*sndSlot != nullptr)
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        Core::Hook::CallMethod<void>(0xCE8CE0, self, Core::Rebase(0xD917A8), sndSlot, &params, -1, 0, 0);
        if (*sndSlot != nullptr)
        {
            AudSpeechParams sp{ 0, -1, 0x4D };
            uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD917C0), 0);
            void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
            uint32 obj = Core::Hook::Call<uint32>(0x799CC0, mem);
            FinalizeAndPlaySound(*sndSlot, reinterpret_cast<uint32>(mem), obj);
        }
    }

    // 0xAAD580
    void __fastcall StopUnk134Sound(uint8* self, void* /*edx*/)
    {
        StopAndReleaseSound(self + 0x134);
    }

    // 0xAAD5D0
    void __fastcall RandomizePedAudioParams(uint8* self, void* /*edx*/)
    {
        Core::Hook::CallMethod<void>(0xAACCA0, self);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped[0x219] != 0)
        {
            *reinterpret_cast<float*>(self + 0xBC) = 0.0f;
        }
        else
        {
            int32 val78 = *reinterpret_cast<int32*>(self + 0x78);
            if (val78 == 0)
            {
                *reinterpret_cast<int32*>(self + 0x78) = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xD79274)) ? 1 : 2;
            }
            else if (val78 > 6)
            {
                *reinterpret_cast<int32*>(self + 0x78) = 2;
            }

            *reinterpret_cast<float*>(self + 0xBC) = Core::Hook::Call<float>(0xCDBCC0, Core::StaticRef<float>(0xD886BC), 0.0f);
            if (Core::Hook::CallMethod<bool>(0xAACF30, self))
            {
                bool b1 = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xDCE03C));
                bool b2 = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xDC9290));
                bool b3 = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xDC9290));
                if (b1)
                {
                    if (b2) self[0x128] |= 1;
                    if (b3) self[0x128] |= 8;
                }
                else
                {
                    if (b2) self[0x128] |= 2;
                    if (b3) self[0x128] |= 4;
                }
            }
        }

        const int32 r = Core::Hook::Call<int32>(0xCDBC80, -600, 400);
        const double expVal = static_cast<double>(static_cast<float>(r) * Core::StaticRef<float>(0xD52954));
        *reinterpret_cast<float*>(self + 0xC0) = static_cast<float>(std::pow(Core::StaticRef<double>(0xEBE5F8), expVal));

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped[0x219] == 0 && *reinterpret_cast<int32*>(self + 0x78) == 2)
        {
            const uint32 bit0 = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xD90B0C)) ? 1u : 0u;
            *reinterpret_cast<uint32*>(self + 0x74) |= bit0;
            const uint32 bit1 = Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xD90B0C)) ? 2u : 0u;
            *reinterpret_cast<uint32*>(self + 0x74) |= bit1;
        }
        *reinterpret_cast<float*>(self + 0x12C) = Core::Hook::Call<float>(0xCDBCC0, 0.0f, 1.0f);
    }

    // 0xAAD770
    void __fastcall PlayClothOrGearSound(uint8* self, void* /*edx*/, uint32 eventId, float arg1)
    {
        if (!CanPlayPedAudio())
            return;

        uint32 soundHash = 0;
        if (eventId <= 0x13)
        {
            const uint8* jumpIdx = reinterpret_cast<const uint8*>(Core::Rebase(0xAAD8FC));
            switch (jumpIdx[eventId])
            {
            case 0:
            {
                const uint32 idx = *reinterpret_cast<uint32*>(self + 0x7C);
                uint8* item = (&Core::StaticRef<uint8*>(0x16677B0))[idx];
                if (item == nullptr)
                    return;
                soundHash = *reinterpret_cast<uint32*>(item + 0xA);
                break;
            }
            case 1:
            {
                const uint32 idx = *reinterpret_cast<uint32*>(self + 0x80);
                uint8* item = (&Core::StaticRef<uint8*>(0x16677F8))[idx];
                if (item == nullptr)
                    return;
                soundHash = *reinterpret_cast<uint32*>(item + 0xA);
                break;
            }
            case 2:
            {
                uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
                if (!Core::Hook::CallMethod<bool>(0x8A5700, ped))
                    return;
                uint32& guard = Core::StaticRef<uint32>(0x1667B90);
                if ((guard & 1) == 0)
                {
                    guard |= 1;
                    Core::Hook::CallMethod<uint32*>(0x435E10, &Core::StaticRef<uint32>(0x1667B8C), Core::Rebase(0xD91850));
                }
                soundHash = Core::StaticRef<uint32>(0x1667B8C);
                break;
            }
            default:
                goto update_time;
            }
        }
        else
        {
            goto update_time;
        }

        if (soundHash != 0)
        {
            AudSoundInitParams params;
            InitSoundParams(params);
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            uint8* mat = *reinterpret_cast<uint8**>(ped + 0x20);
            *reinterpret_cast<void**>(params.data + 0x14) = mat + 0x30;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
            const float vol = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667914)), arg1);
            *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0x794280, vol);

            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8930, self, soundHash, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, 0x4A };
                Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
        }

    update_time:
        *reinterpret_cast<uint32*>(self + 0x158) = Core::StaticRef<uint32>(0x11DDE74);
    }

    // 0xAAD910
    void __fastcall PlayUnk19CSound(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8** sndSlot = reinterpret_cast<uint8**>(self + 0x19C);
        if (*sndSlot != nullptr)
            return;

        uint32& guard = Core::StaticRef<uint32>(0x1667B98);
        if ((guard & 1) == 0)
        {
            guard |= 1;
            Core::StaticRef<uint32>(0x1667B94) = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91860), 0);
        }

        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        const uint32 soundHash = Core::StaticRef<uint32>(0x1667B94);
        if (Core::Hook::CallMethod<bool>(0xCE8A00, self, soundHash, sndSlot, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x48 };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    // 0xAADA20
    void __fastcall GetFootstepPitchAndVolume(uint8* self, void* /*edx*/, float* outPitch, float* outVol)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        float pitch = 0.0f;
        if (Core::Hook::CallMethod<bool>(0x89C780, ped))
        {
            pitch = Core::StaticRef<float>(0xD90AF0);
        }
        else
        {
            const int32 pedType = *reinterpret_cast<int32*>(ped + 0xB90);
            if (pedType == 3)
                pitch = Core::StaticRef<float>(0xD90AE0);
            else if (pedType == 4)
                pitch = 0.0f;
            else
                pitch = Core::StaticRef<float>(0xD90AD8);
        }

        *outPitch = pitch;
        *outVol = Core::StaticRef<float>(0xD50398);

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        const float step = Core::StaticRef<float>(0xD90B00);
        if (IsPedFemale(ped))
            *outPitch += Core::StaticRef<float>(0xD90AFC);
        else
            *outPitch += step;

        *outVol *= *reinterpret_cast<float*>(self + 0xC0);
        if ((self[0xAF] & 1) != 0)
            *outPitch += step;

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        if ((*reinterpret_cast<uint32*>(ped + 0x24) & 0x20000000) != 0
            && (*reinterpret_cast<uint32*>(ped + 0x118) & 0x80000) != 0)
        {
            const float f = Core::Hook::CallMethod<float>(0x89E570, ped) * Core::StaticRef<float>(0xEBDFB4);
            const float one = Core::StaticRef<float>(0xD50398);
            const float clamped = (f > 1.0f) ? one : f;
            *outPitch += Core::Hook::Call<float>(0x794280, one - clamped);
        }
    }

    // 0xAADB50
    float __fastcall ComputeAmbientAudioScale(uint8* self, void* /*edx*/)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        uint32 unkB8 = *reinterpret_cast<uint32*>(ped + 0xB8);
        float speed = std::sqrt(
            *reinterpret_cast<float*>(ped + 0xB0) * *reinterpret_cast<float*>(ped + 0xB0)
            + *reinterpret_cast<float*>(ped + 0xB4) * *reinterpret_cast<float*>(ped + 0xB4));

        float val = 0.0f;
        uint8* sub = *reinterpret_cast<uint8**>(ped + 0xDC);
        if (sub != nullptr)
        {
            if (sub[0x154] != 0)
            {
                if (*reinterpret_cast<uint32*>(sub + 0x138) == 0x2F)
                {
                    if (sub[0x154] == 1)
                        val = speed * Core::StaticRef<float>(0xDCE03C);
                }
            }
        }

        if (*reinterpret_cast<int32*>(ped + 0xB90) != 0)
        {
            if (Core::Hook::Call<bool>(0xB131C0, unkB8))
            {
                val = speed * Core::StaticRef<float>(0xDCE03C);
            }
            else
            {
                switch (unkB8)
                {
                case 0x24: case 0x25: case 0x26: case 0x27:
                case 0x2C: case 0x2D: case 0x2E: case 0x2F:
                case 0x39: case 0x3A:
                    val = speed * Core::StaticRef<float>(0xD50964);
                    break;
                case 0x33: case 0x34: case 0x3B: case 0x3C:
                    val = speed * Core::StaticRef<float>(0xDCE03C);
                    break;
                default:
                    break;
                }
            }
            const float one = Core::StaticRef<float>(0xD50398);
            if (val > one)
                val = one;
        }

        return val * Core::StaticRef<float>(0xDCE03C) * 1.0f;
    }

    // 0xAADC90
    void __fastcall GetBoneOrPedPos(uint8* self, void* /*edx*/, uint32 boneType, AudVec4* outPos)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if ((ped[0xF4] & 4) != 0)
        {
            uint32 boneId = 0x1A9;
            switch (boneType)
            {
            case 0: boneId = 0x1A4; break;
            case 2: boneId = 0x4C3; break;
            case 3: boneId = 0x4D0; break;
            case 4: boneId = 0x4B5; break;
            default: boneId = 0x1A9; break;
            }
            Core::Hook::CallMethod<void>(0x8A4C40, ped, outPos, boneId);
            return;
        }
        const AudVec4* matPos = reinterpret_cast<const AudVec4*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);
        *outPos = *matPos;
    }

    // 0xAADD50
    void __fastcall PlayCollisionSound(uint8* self, void* /*edx*/, void* arg0, uint8* colInfo)
    {
        if (!CanPlayPedAudio())
            return;

        self[0x10] = 1;
        uint32 colEntityId = *reinterpret_cast<uint32*>(colInfo);
        bool isPedTarget = false;
        uint32 unk18 = 0;

        if (colEntityId != 0)
        {
            uint8* ent = Core::Hook::Call<uint8*>(0xA72220, colEntityId);
            if (ent != nullptr && (*reinterpret_cast<uint32*>(ent + 0x28) & 0x3C0) == 0xC0)
                isPedTarget = true;
        }

        uint32 soundHash = 0;
        uint8* matSound = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg0);
        if (matSound != nullptr)
        {
            soundHash = isPedTarget ? *reinterpret_cast<uint32*>(matSound + 0x12) : *reinterpret_cast<uint32*>(matSound + 0x0E);
        }

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped != nullptr)
        {
            uint8* weapMgr = Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0);
            if (weapMgr != nullptr)
            {
                bool skipWeap = (soundHash != 0 && soundHash != Core::StaticRef<uint32>(0x16677F0))
                    && (*reinterpret_cast<uint32*>(weapMgr + 0x18) == 0 || Core::Hook::CallMethod<bool>(0x45A740, weapMgr));
                if (!skipWeap)
                {
                    uint8* weapInfo = *reinterpret_cast<uint8**>(weapMgr + 0x10);
                    if (weapInfo != nullptr)
                    {
                        unk18 = *reinterpret_cast<uint32*>(weapMgr + 0x18);
                        soundHash = isPedTarget ? *reinterpret_cast<uint32*>(weapInfo + 0x1E) : *reinterpret_cast<uint32*>(weapInfo + 0x1A);
                    }
                }
            }
        }

        if (soundHash == 0)
            return;

        AudVec4 pos;
        pos.x = *reinterpret_cast<float*>(colInfo + 0x10);
        pos.y = *reinterpret_cast<float*>(colInfo + 0x14);
        pos.z = *reinterpret_cast<float*>(colInfo + 0x18);
        float volDb = 0.0f;

        if (!isPedTarget)
        {
            const float rawVol = Core::Hook::CallMethod<float>(0xAD8300, reinterpret_cast<void*>(Core::Rebase(0x1674D30)), colInfo);
            volDb = Core::Hook::Call<float>(0x794280, rawVol);
        }

        AudSoundInitParams params;
        InitSoundParams(params);
        *reinterpret_cast<float*>(params.data + 0x00) = volDb;
        *reinterpret_cast<AudVec4**>(params.data + 0x14) = &pos;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        const int32 speechFlags = ComputePedSpeechFlags(*reinterpret_cast<uint8**>(self + 0x120));
        *reinterpret_cast<int32*>(params.data + 0x04) = speechFlags;

        Core::Hook::CallMethod<void>(0xCE8930, self, soundHash, &params, -1, 0, 0);
        Core::Hook::Call<void>(0x7AA750, *reinterpret_cast<void**>(self + 0x120), arg0, isPedTarget ? 1 : 0, &pos, volDb, unk18);

        if (isPedTarget)
        {
            uint8* targetPed = Core::Hook::Call<uint8*>(0xA72220, colEntityId);
            const uint32 boneTag = *reinterpret_cast<uint16*>(colInfo + 0x52);
            Core::Hook::CallMethod<void>(0xAAD770, targetPed + 0x3D0, boneTag, Core::StaticRef<float>(0xDCE03C));
        }
    }

    // 0xAADF70
    void __fastcall PlayMeleeHitSound(uint8* self, void* /*edx*/, void* arg0, uint8 arg1, uint32 posPtr, float volDb, void* arg3)
    {
        uint32 soundHash = 0;
        uint8* matSound = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg0);
        if (matSound != nullptr)
        {
            if (arg1 != 0)
            {
                soundHash = IsNetworkOrCutsceneMode4Or3()
                    ? *reinterpret_cast<uint32*>(matSound + 0x1A)
                    : *reinterpret_cast<uint32*>(matSound + 0x12);
            }
            else
            {
                soundHash = *reinterpret_cast<uint32*>(matSound + 0x0E);
            }
        }

        if (*reinterpret_cast<void**>(self + 0x120) == nullptr)
            return;
        if (soundHash != 0 && soundHash != Core::StaticRef<uint32>(0x16677F0) && arg3 == nullptr)
            return;

        uint8 weapTracker[0x18]{};
        Core::Hook::CallMethod<void>(0xACBA30, weapTracker);
        Core::Hook::CallMethod<void>(0xACBA50, weapTracker, arg3);
        uint8* weapInfo = *reinterpret_cast<uint8**>(weapTracker + 0x10);
        if (weapInfo != nullptr)
        {
            if (arg1 != 0)
            {
                soundHash = IsNetworkOrCutsceneMode4Or3()
                    ? *reinterpret_cast<uint32*>(weapInfo + 0x47)
                    : *reinterpret_cast<uint32*>(weapInfo + 0x1E);
            }
            else
            {
                soundHash = *reinterpret_cast<uint32*>(weapInfo + 0x1A);
            }
        }
        *reinterpret_cast<uintptr_t*>(weapTracker) = Core::Rebase(0xD5295C);
        Core::Hook::CallMethod<void>(0xCE83F0, weapTracker);

        if (soundHash == 0)
            return;

        AudSoundInitParams params;
        ResetSoundParamsZero(params);
        *reinterpret_cast<float*>(params.data + 0x00) = volDb;
        *reinterpret_cast<uint32*>(params.data + 0x14) = posPtr;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<int32*>(params.data + 0x04) = ComputePedSpeechFlags(*reinterpret_cast<uint8**>(self + 0x120));

        Core::Hook::CallMethod<void>(0xCE8930, self, soundHash, &params, -1, 0, 0);
    }

    // 0xAAE120
    void __fastcall StopUnk14Sound(uint8* self, void* /*edx*/)
    {
        StopAndReleaseSound(self + 0x14);
    }

    // 0xAAE170
    void __fastcall PlayScriptedSpeechWithAnim(uint8* self, void* /*edx*/, void* arg0)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* animData = Core::Hook::CallMethod<uint8*>(0x58ABB0, arg0, 0x82, 0);
        if (animData == nullptr)
            return;

        const uint32 count = *reinterpret_cast<uint16*>(animData + 0x4);
        const uint32 maxIdx = *reinterpret_cast<uint16*>(animData + 0xC) - 1;
        int32 idx = static_cast<int32>(0.0f / static_cast<float>(count));
        if (idx >= static_cast<int32>(maxIdx))
            idx = static_cast<int32>(maxIdx);

        uint8* entry = (*reinterpret_cast<uint8***>(animData + 0x8))[idx];
        void* obj = *reinterpret_cast<void**>(entry + 0x4);
        uint32 flags = 0;
        const float timeOffset = Core::StaticRef<float>(0xEBDED0) - static_cast<float>(count * idx);
        using VFn5 = void(__thiscall*)(void*, float, uint32*);
        reinterpret_cast<VFn5>((*reinterpret_cast<void***>(obj))[0x14 / 4])(obj, timeOffset, &flags);

        if ((flags & 0x3000000) == 0)
            return;

        uint8* animData2 = Core::Hook::CallMethod<uint8*>(0x58ABB0, arg0, 0x83, 0);
        if (animData2 == nullptr)
            return;

        Core::Hook::CallMethod<void>(0x58BEA0, animData2, 0.0f, &arg0);
        if (arg0 == nullptr)
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<uint32*>(params.data + 0x18) = Core::StaticRef<uint32>(0x16677AC);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        uint8** sndSlot = reinterpret_cast<uint8**>(self + 0x1C);
        Core::Hook::CallMethod<void>(0xCE8890, self, arg0, sndSlot, &params, -1, 0, 0);
        if (*sndSlot != nullptr)
        {
            AudSpeechParams sp{ 0, -1, 0x33 };
            void* mem = Core::Hook::Call<void*>(0x7A9810, arg0, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
            uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
            FinalizeAndPlaySound(*sndSlot, reinterpret_cast<uint32>(mem), objId);
        }
    }

    // 0xAAE310
    void __fastcall StopUnk1CSound(uint8* self, void* /*edx*/)
    {
        StopAndReleaseSound(self + 0x1C);
    }

    // 0xAAE360
    void __fastcall SetFootstepMaterial(uint8* self, void* /*edx*/, uint32 matHash, uint8* entity, uint32 boneId)
    {
        self[0x1A0] = Core::Hook::CallMethod<bool>(0x908190, reinterpret_cast<void*>(Core::Rebase(0x13D3630)), (matHash >> 8) & 0xFF) ? 1 : 0;

        if (entity != nullptr && (*reinterpret_cast<uint32*>(entity + 0x28) & 0x3C0) == 0x80
            && *reinterpret_cast<int32*>(entity + 0x1354) == 2)
        {
            *reinterpret_cast<uint32*>(self + 0xB8) = Core::StaticRef<uint32>(0x1667860);
            return;
        }

        if (*reinterpret_cast<uint32*>(self + 0xAC) == matHash
            && *reinterpret_cast<uint8**>(self + 0xB4) == entity
            && *reinterpret_cast<uint32*>(self + 0xB0) == boneId)
        {
            return;
        }

        uint32 resolved = 0;
        if (entity != nullptr && *reinterpret_cast<void**>(entity + 0x38) != nullptr)
        {
            resolved = Core::Hook::CallMethod<uint32>(0xAD5160, reinterpret_cast<void*>(Core::Rebase(0x1674D30)), *reinterpret_cast<void**>(entity + 0x38), boneId);
        }
        if (resolved == 0)
        {
            uint32* unkTable = Core::StaticRef<uint32*>(0x1685CE4);
            resolved = unkTable[matHash & 0xFF];
        }

        uint8** refSlot = reinterpret_cast<uint8**>(self + 0xB4);
        if (*refSlot != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6580, *refSlot, refSlot);
            *refSlot = nullptr;
        }

        if (resolved == 0)
        {
            *reinterpret_cast<uint32*>(self + 0xB8) = Core::StaticRef<uint32*>(0x1685CE4)[0];
            *reinterpret_cast<uint32*>(self + 0xAC) = 0;
            *reinterpret_cast<uint32*>(self + 0xB0) = 0;
            return;
        }

        *reinterpret_cast<uint32*>(self + 0xB8) = resolved;
        if (entity != nullptr && (*reinterpret_cast<uint32*>(entity + 0x28) & 0x3C0) == 0x80
            && *reinterpret_cast<int32*>(entity + 0x1354) == 1)
        {
            *reinterpret_cast<uint32*>(self + 0xB8) = Core::StaticRef<uint32>(0x1667864);
        }

        *reinterpret_cast<uint32*>(self + 0xAC) = matHash;
        *refSlot = entity;
        if (entity != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6550, entity, refSlot);
        }
        *reinterpret_cast<uint32*>(self + 0xB0) = boneId;
    }

    // 0xAAE4B0
    void __fastcall SetFootstepMaterialDirect(uint8* self, void* /*edx*/, uint32 matPtr, uint8* entity, uint32 boneId)
    {
        uint8** refSlot = reinterpret_cast<uint8**>(self + 0xB4);
        if (*refSlot != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6580, *refSlot, refSlot);
            *refSlot = nullptr;
        }

        if (matPtr == 0)
        {
            *reinterpret_cast<uint32*>(self + 0xB8) = Core::StaticRef<uint32*>(0x1685CE4)[0];
            *reinterpret_cast<uint32*>(self + 0xAC) = 0;
            *reinterpret_cast<uint32*>(self + 0xB0) = 0;
            return;
        }

        *reinterpret_cast<uint32*>(self + 0xB8) = matPtr;
        if (entity != nullptr && (*reinterpret_cast<uint32*>(entity + 0x28) & 0x3C0) == 0x80
            && *reinterpret_cast<int32*>(entity + 0x1354) == 1)
        {
            *reinterpret_cast<uint32*>(self + 0xB8) = Core::StaticRef<uint32>(0x1667864);
        }

        *reinterpret_cast<uint32*>(self + 0xAC) = 0;
        *refSlot = entity;
        if (entity != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6550, entity, refSlot);
        }
        *reinterpret_cast<uint32*>(self + 0xB0) = boneId;
    }

    // 0xAAE560
    void __fastcall UpdateLimbSpeeds(uint8* self, void* /*edx*/)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped == nullptr)
            return;

        const uint32 tlsIdx = Core::StaticRef<uint32>(0x17237B0);
        uint8** tlsSlots = reinterpret_cast<uint8**>(__readfsdword(0x2C));
        const int32 matIdx = *reinterpret_cast<int32*>(tlsSlots[tlsIdx] + 0x8E8);
        const float* refPos = reinterpret_cast<const float*>(Core::Rebase(0x1717220) + (matIdx << 6));
        const float* pedPos = reinterpret_cast<const float*>(*reinterpret_cast<uint8**>(ped + 0x20) + 0x30);

        const float dx = pedPos[0] - refPos[0];
        const float dy = pedPos[1] - refPos[1];
        const float dz = pedPos[2] - refPos[2];
        const float distSq = dx * dx + dy * dy + dz * dz;

        if ((ped[0x218] != 0 || ped[0x219] == 0) && distSq >= Core::StaticRef<float>(0xEBE3BC))
        {
            *reinterpret_cast<float*>(self + 0xC4) = 0.0f;
            *reinterpret_cast<float*>(self + 0xC8) = 0.0f;
            *reinterpret_cast<float*>(self + 0xCC) = 0.0f;
            *reinterpret_cast<float*>(self + 0xD0) = 0.0f;
            return;
        }

        uint8* p7c4 = *reinterpret_cast<uint8**>(ped + 0x7C4);
        if (p7c4 == nullptr)
            return;
        uint8* p4 = *reinterpret_cast<uint8**>(p7c4 + 0x4);
        if (p4 == nullptr)
            return;
        uint8* skel = reinterpret_cast<uint8*>(DynamicCastSkeletonData(*reinterpret_cast<void**>(p4 + 0xC)));
        if (skel == nullptr)
            return;

        uint8* boneMatrices = *reinterpret_cast<uint8**>(skel + 0x84);
        const int32 bones[4] = { 3, 6, 0x10, 0x14 };
        float* outSpeeds = reinterpret_cast<float*>(self + 0xC4);
        AudVec4* prevPos = reinterpret_cast<AudVec4*>(self + 0xE0);

        for (int i = 0; i < 4; ++i)
        {
            const float* bm = reinterpret_cast<const float*>(boneMatrices + (bones[i] << 6) + 0x30);
            const float bx = bm[0], by = bm[1], bz = bm[2], bw = bm[3];

            const float scaleA = (ped[0x219] != 0) ? Core::StaticRef<float>(0xD50398) : Core::StaticRef<float>(0xD90BB8);
            const float scaleB = (*reinterpret_cast<int32*>(ped + 0xB90) == 4 || (ped[0x26C] & 1) == 0)
                ? Core::StaticRef<float>(0xD90BBC)
                : Core::StaticRef<float>(0xD50398);

            const float ddx = bx - prevPos[i].x;
            const float ddy = by - prevPos[i].y;
            const float ddz = bz - prevPos[i].z;
            float val = std::sqrt(ddx * ddx + ddy * ddy + ddz * ddz)
                * Core::StaticRef<float>(0x11DDE7C) * scaleB * scaleA * Core::StaticRef<float>(0xD7E2BC);

            const float one = Core::StaticRef<float>(0xD50398);
            if (val > one)
                val = one;

            const float cur = outSpeeds[i];
            if (val - cur <= Core::StaticRef<float>(0xDCC5F8))
            {
                const float step = Core::StaticRef<float>(0xD90BC0);
                outSpeeds[i] = (cur - val > step) ? (cur - step) : val;
            }
            prevPos[i] = { bx, by, bz, bw };
        }
    }

    // 0xAAE7C0
    void __fastcall GetBoneWorldPos(uint8* self, void* /*edx*/, AudVec4* outPos)
    {
        outPos->x = 0.0f;
        outPos->y = 0.0f;
        outPos->z = 0.0f;

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped == nullptr)
            return;
        uint8* p7c4 = *reinterpret_cast<uint8**>(ped + 0x7C4);
        if (p7c4 == nullptr)
            return;
        uint8* p4 = *reinterpret_cast<uint8**>(p7c4 + 0x4);
        if (p4 == nullptr)
            return;
        uint8* skel = reinterpret_cast<uint8*>(DynamicCastSkeletonData(*reinterpret_cast<void**>(p4 + 0xC)));
        if (skel == nullptr)
            return;

        const int32 boneIdx = Core::Hook::Call<int32>(0xA4B940, *reinterpret_cast<void**>(self + 0x120), *reinterpret_cast<uint32*>(self + 0x70));
        uint8* srcMat = *reinterpret_cast<uint8**>(skel + 0x84) + (boneIdx << 6);
        uint8 matBuf[0x40]{};
        Core::Hook::CallMethod<void>(0x441ED0, matBuf, srcMat);
        *outPos = *reinterpret_cast<const AudVec4*>(matBuf + 0x30);
    }
}

namespace
{
    // 0xAAE870
    void __fastcall PlayUnk16CSound(uint8* self, void* /*edx*/)
    {
        if (Core::StaticRef<uint32>(0x11DDE74) <= *reinterpret_cast<uint32*>(self + 0x16C) + 0x3E8)
            return;

        AudSoundInitParams params;
        ResetSoundParamsZero(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        const uintptr_t soundNameAddr = IsNetworkOrCutsceneMode4Or3() ? 0xD91968 : 0xD91984;
        Core::Hook::CallMethod<void>(0xCE8E70, self, Core::Rebase(soundNameAddr), &params, -1, 0, 0);

        if (CanPlayPedAudio())
            Core::Hook::Call<void>(0x7AA6C0, *reinterpret_cast<void**>(self + 0x120));

        *reinterpret_cast<uint32*>(self + 0x16C) = Core::StaticRef<uint32>(0x11DDE74);
    }

    // 0xAAE9A0
    void __fastcall PlayBodyCollisionSounds(uint8* self, void* /*edx*/, uint8* otherPed, int32 level)
    {
        if (!CanPlayPedAudio() || otherPed == nullptr || level >= 5)
            return;

        uint8* itemSelf = (&Core::StaticRef<uint8*>(0x16677F8))[*reinterpret_cast<uint32*>(self + 0x80)];
        uint8* itemOther = (&Core::StaticRef<uint8*>(0x16677F8))[*reinterpret_cast<uint32*>(otherPed + 0x450)];
        if (itemSelf == nullptr || itemOther == nullptr)
            return;

        const uint32 hashOther = *reinterpret_cast<uint32*>(itemOther + 0xA);
        const uint32 hashSelf = *reinterpret_cast<uint32*>(itemSelf + 0xA);

        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        const float minVal = Core::StaticRef<float>(0xF3803C);
        const float maxVal = Core::StaticRef<float>(0xF38040);
        const float* levelOffsets = reinterpret_cast<const float*>(Core::Rebase(0xF38044));

        float vol1 = Core::Hook::Call<float>(0xCDBCC0, minVal, maxVal) + levelOffsets[level];
        if (hashSelf == hashOther)
            vol1 += Core::StaticRef<float>(0xDCD0EC);
        *reinterpret_cast<float*>(params.data + 0x00) = vol1;

        auto playOne = [&](uint32 sndHash, int32 speechType) {
            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8930, self, sndHash, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, speechType };
                Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
        };

        if (hashSelf == hashOther)
        {
            playOne(hashSelf, 0x59);
        }
        else
        {
            playOne(hashSelf, 0x5A);
            *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0xCDBCC0, minVal, maxVal) + levelOffsets[level];
            playOne(hashOther, 0x5B);
        }

        const float* levelVols = reinterpret_cast<const float*>(Core::Rebase(0xF38058));
        *reinterpret_cast<float*>(params.data + 0x00) = levelVols[level];
        playOne(Core::StaticRef<uint32>(0x1667A3C), 0x50);
    }

    // 0xAAEC80
    void __fastcall PlayWaterSplashSounds(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* item = (&Core::StaticRef<uint8*>(0x16677F8))[*reinterpret_cast<uint32*>(self + 0x80)];
        if (item == nullptr)
            return;
        const uint32 hashSelf = *reinterpret_cast<uint32*>(item + 0xA);

        AudSoundInitParams params;
        ResetSoundParamsZero(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0xCDBCC0, Core::StaticRef<float>(0xF3803C), Core::StaticRef<float>(0xF38040));

        auto playSplash = [&](uint32 sndHash, int32 speechType) {
            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8930, self, sndHash, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, speechType };
                Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
        };

        playSplash(hashSelf, 0x5C);
        *reinterpret_cast<float*>(params.data + 0x00) = 0.0f;
        playSplash(Core::StaticRef<uint32>(0x1667968), 0x57);
    }

    // 0xAAEE40
    void __fastcall PlayWeaponOrMeleeSwingSounds(uint8* self, void* /*edx*/, uint32 arg0)
    {
        if (!CanPlayPedAudio())
            return;

        const int32 val178 = *reinterpret_cast<int32*>(self + 0x178);
        float extraVol = Core::StaticRef<float>(0xF38094);
        if (val178 == 3)
            extraVol = Core::StaticRef<float>(0xF38090);
        else if (val178 == 4)
            extraVol = 0.0f;

        const int32 minPitch = Core::StaticRef<int32>(0x1667B9C);
        const int32 maxPitch = Core::StaticRef<int32>(0xF3808C);
        const int32 randPitch1 = Core::Hook::Call<int32>(0xCDBC80, minPitch, maxPitch);
        const int32 randPitch2 = Core::Hook::Call<int32>(0xCDBC80, minPitch, maxPitch);

        uint8* item = (&Core::StaticRef<uint8*>(0x16677F8))[*reinterpret_cast<uint32*>(self + 0x80)];
        const uint32 hashSelf = (item != nullptr) ? *reinterpret_cast<uint32*>(item + 0xA) : 0;

        AudSoundInitParams params;
        ResetSoundParamsZero(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<int32*>(params.data + 0x28) = randPitch2;

        const float rndVol = Core::Hook::Call<float>(0xCDBCC0, Core::StaticRef<float>(0xEBDED4), 1.0f);
        *reinterpret_cast<float*>(params.data + 0x00) = rndVol + (extraVol - Core::StaticRef<float>(0xDCB2A0));

        auto playSwing = [&](uint32 sndHash, int32 speechType) {
            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8930, self, sndHash, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, speechType };
                Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
        };

        playSwing(hashSelf, 0x3A);

        const float rndVol2 = Core::Hook::Call<float>(0xCDBCC0, Core::StaticRef<float>(0xEBDED4), 1.0f);
        *reinterpret_cast<float*>(params.data + 0x00) = rndVol2 + extraVol;
        *reinterpret_cast<int32*>(params.data + 0x28) = 0;
        playSwing(Core::StaticRef<uint32>(0x166793C), 0x39);

        if (arg0 == Core::StaticRef<uint32>(0x1667A0C))
            return;

        const float rndVol3 = Core::Hook::Call<float>(0xCDBCC0, Core::StaticRef<float>(0xEBDED4), 1.0f);
        *reinterpret_cast<float*>(params.data + 0x00) = rndVol3 + extraVol;
        *reinterpret_cast<int32*>(params.data + 0x28) = randPitch1;
        ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped == nullptr)
            return;
        uint8* obj = *reinterpret_cast<uint8**>(ped + 0x2DC);
        if (obj == nullptr)
            return;
        uint8* weapSlot = *reinterpret_cast<uint8**>(obj + 0x25C);
        if (weapSlot == nullptr)
            return;
        uint8* weapInfo = *reinterpret_cast<uint8**>(weapSlot + 0x10);
        if (weapInfo == nullptr)
            return;

        playSwing(*reinterpret_cast<uint32*>(weapInfo + 0x26), 0x3B);
        playSwing(*reinterpret_cast<uint32*>(weapInfo + 0x2A), 0x3C);
    }

    // 0xAAF1D0
    bool __fastcall TryPlayScriptedPainSound(uint8* self, void* /*edx*/, uint32 bitIndex)
    {
        if (!CanPlayPedAudio())
            return false;

        if (Core::StaticRef<uint32>(0x11DDE74) < *reinterpret_cast<uint32*>(self + 0x158) + 0x1388)
            return false;

        const uint32 bitMask = 1u << (bitIndex & 0x1F);
        bool played = false;
        if ((*reinterpret_cast<uint32*>(self + 0x148) & bitMask) == 0)
        {
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            if (Core::Hook::CallMethod<bool>(0x444410, ped) || !Core::Hook::CallMethod<bool>(0x454450, ped))
                ;
            else
                return false;

            AudSoundInitParams params;
            InitSoundParams(params);
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

            const uint32 soundHash = (&Core::StaticRef<uint32>(0x1667B38))[bitIndex];
            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8930, self, soundHash, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, 0x36 };
                Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
            played = true;
        }

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        const int32 valA84 = *reinterpret_cast<int32*>(ped + 0xA84);
        if ((ped[0x210] != 0 && (valA84 == 1 || valA84 == 2)) || *reinterpret_cast<void**>(ped + 0x224) == nullptr)
        {
            *reinterpret_cast<uint32*>(self + 0x148) |= bitMask;
        }
        return played;
    }

    // 0xAAF330
    void __fastcall AccumulateUnk40And144(uint8* self, void* /*edx*/, float val)
    {
        float v = val * Core::StaticRef<float>(0xDC9290);
        const float one = Core::StaticRef<float>(0xD50398);
        if (v > one)
            v = one;
        if (v < *reinterpret_cast<float*>(self + 0x40))
            v = *reinterpret_cast<float*>(self + 0x40);
        *reinterpret_cast<float*>(self + 0x40) = v;

        float v144 = *reinterpret_cast<float*>(self + 0x144) + Core::StaticRef<float>(0xD79274);
        *reinterpret_cast<float*>(self + 0x144) = (v144 > one) ? one : v144;
    }

    // 0xAAF390
    void __fastcall PlayUnk140Sound(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        if (*reinterpret_cast<uint32*>(self + 0x140) + 0x3E8 >= Core::StaticRef<uint32>(0x11DDE74))
            return;

        *reinterpret_cast<float*>(self + 0x144) = Core::StaticRef<float>(0xD50398);
        *reinterpret_cast<uint32*>(self + 0x140) = Core::StaticRef<uint32>(0x11DDE74);

        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        uint8* mat = *reinterpret_cast<uint8**>(ped + 0x20);
        *reinterpret_cast<void**>(params.data + 0x14) = mat + 0x30;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8E70, self, Core::Rebase(0xD91D28), &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x35 };
            uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91D38), 0);
            Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    // 0xAAF4A0
    void __cdecl AnimEvent_PlaySoundByHash(uint32 soundHash, uint8* ped)
    {
        Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, soundHash);
    }

    // 0xAAF4C0
    void __cdecl AnimEvent_PlaySoundAndVehicle(uint32 soundHash, uint8* ped)
    {
        if ((ped[0x26C] & 4) != 0)
        {
            uint8* veh = *reinterpret_cast<uint8**>(ped + 0xB40);
            if (veh != nullptr && *reinterpret_cast<int32*>(veh + 0x1354) == 0)
            {
                Core::Hook::CallMethod<void>(0xAF6270, veh + 0x260);
            }
        }
        Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, soundHash);
    }

    // 0xAAF500
    void __cdecl AnimEvent_PlayParachuteOrWeaponSound(uint32 /*unused*/, uint8* ped)
    {
        uint32& guard = Core::StaticRef<uint32>(0x1667BA8);
        if ((guard & 1) == 0)
        {
            guard |= 1;
            Core::StaticRef<uint32>(0x1667BA4) = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91420), 0);
        }
        if ((guard & 2) == 0)
        {
            guard |= 2;
            Core::StaticRef<uint32>(0x1667BA0) = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91434), 0);
        }

        if (*reinterpret_cast<void**>(ped + 0x2DC) == nullptr)
            return;

        uint8* weapMgr = Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0);
        if (weapMgr == nullptr)
            return;

        weapMgr = Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0);
        if (*reinterpret_cast<int32*>(weapMgr + 0x18) != 0x2E)
            return;

        uint8* obj = *reinterpret_cast<uint8**>(ped + 0x2DC);
        const int32 modelIdx = *reinterpret_cast<int16*>(obj + 0x2E);
        uint32 soundHash = 0;
        if (modelIdx == Core::StaticRef<int32>(0x16678E4))
            soundHash = Core::StaticRef<uint32>(0x1667BA0);
        else if (modelIdx == Core::StaticRef<int32>(0x1667AAC))
            soundHash = Core::StaticRef<uint32>(0x1667BA4);

        Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, soundHash);
    }

    // 0xAAF5D0
    void __cdecl AnimEvent_PlayWeapon22Sound(uint32 /*unused*/, uint8* ped)
    {
        uint8* obj = *reinterpret_cast<uint8**>(ped + 0x2DC);
        if (obj == nullptr)
            return;
        uint8* slot = *reinterpret_cast<uint8**>(obj + 0x25C);
        if (slot == nullptr)
            return;
        uint8* info = *reinterpret_cast<uint8**>(slot + 0x10);
        if (info != nullptr)
            Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, *reinterpret_cast<uint32*>(info + 0x22));
    }

    // 0xAAF600
    void __cdecl AnimEvent_PlayWeapon26And2ASounds(uint32 /*unused*/, uint8* ped)
    {
        uint8* obj = *reinterpret_cast<uint8**>(ped + 0x2DC);
        if (obj == nullptr)
            return;
        uint8* slot = *reinterpret_cast<uint8**>(obj + 0x25C);
        if (slot == nullptr)
            return;
        uint8* info = *reinterpret_cast<uint8**>(slot + 0x10);
        if (info != nullptr)
        {
            Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, *reinterpret_cast<uint32*>(info + 0x26));
            Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, *reinterpret_cast<uint32*>(info + 0x2A));
        }
    }

    // 0xAAF640
    void __cdecl AnimEvent_PlaySoundAndReportCrime(uint32 soundHash, uint8* ped)
    {
        Core::Hook::CallMethod<void>(0xAAD000, ped + 0x3D0, soundHash);
        Core::Hook::CallMethod<void>(0xA5AA70, ped + 0x580, Core::Rebase(0xD914E0), 0, 0, 0, 0x2BC, 0, 0, 1.0f, 0, 0);
    }

    // 0xAAF680
    void __cdecl AnimEvent_PlaySoundIfFemale(uint32 soundHash, uint8* ped)
    {
        if (ped != nullptr && IsPedFemale(ped))
            AnimEvent_PlaySoundByHash(soundHash, ped);
    }

    // 0xAAF6B0
    void __cdecl AnimEvent_PlayMeleeSwing(uint32 arg0, uint8* ped)
    {
        if (ped != nullptr)
            Core::Hook::CallMethod<void>(0xAAEE40, ped + 0x3D0, arg0);
    }

    // 0xAAF6D0
    void* __fastcall AudPedAudioEntity_ScalarDeletingDtor(uint8* self, void* /*edx*/, uint8 flags)
    {
        AudPedAudioEntity_Dtor(self, nullptr);
        if ((flags & 1) != 0)
            Core::Hook::Call<void>(0x5B1C10, self);
        return self;
    }

    // 0xAAF6F0
    void __fastcall UpdatePedAudio(uint8* self, void* /*edx*/)
    {
        auto updateSpeechFlags = [&](uint8* speechCtrl) {
            if (speechCtrl == nullptr)
                return;
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            if (ped != nullptr && (ped[0x26C] & 4) != 0 && *reinterpret_cast<void**>(ped + 0xB40) != nullptr)
            {
                uint8* veh = *reinterpret_cast<uint8**>(ped + 0xB40);
                Core::Hook::CallMethod<void>(0xAC7AC0, speechCtrl, (ped[0x219] != 0) ? 0xC8 : 0x3E8, 0xFA0, Core::StaticRef<float>(0xDCE03C));
                Core::Hook::CallMethod<void>(0xAC7B40, speechCtrl, *reinterpret_cast<uint32*>(veh + 0xC80));
            }
            else
            {
                Core::Hook::CallMethod<void>(0xAC7AC0, speechCtrl, (ped != nullptr && ped[0x219] != 0) ? 0xC8 : 0x3E8, 0xFA0, Core::StaticRef<float>(0xDCE03C));
                Core::Hook::CallMethod<void>(0xAC7B40, speechCtrl, 0);
            }
            uint8* ped2 = *reinterpret_cast<uint8**>(self + 0x120);
            speechCtrl[0xAA] = IsPedFemale(ped2) ? 1 : 0;
        };

        if (CanPlayPedAudioAlt())
        {
            if (self[0x129] != 0)
            {
                RandomizePedAudioParams(self, nullptr);
                self[0x129] = 0;
            }
            UpdateLimbSpeeds(self, nullptr);
            updateSpeechFlags(*reinterpret_cast<uint8**>(self + 0x8));
            return;
        }

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        const uint32 now = Core::StaticRef<uint32>(0x11DDE74);
        if (!Core::Hook::CallMethod<bool>(0x9C3090, *reinterpret_cast<void**>(ped + 0x224))
            && self[0x174] != 0
            && *reinterpret_cast<uint32*>(self + 0x170) + 0x3E8 < now)
        {
            AudSoundInitParams params;
            InitSoundParams(params);
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8E70, self, Core::Rebase(0xD917D8), &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, 0x37 };
                uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD917E8), 0);
                Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
            *reinterpret_cast<uint32*>(self + 0x170) = now;
        }

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        self[0x174] = Core::Hook::CallMethod<bool>(0x9C3090, *reinterpret_cast<void**>(ped + 0x224)) ? 1 : 0;
        if (self[0x129] != 0)
        {
            RandomizePedAudioParams(self, nullptr);
            self[0x129] = 0;
        }
        UpdateLimbSpeeds(self, nullptr);

        uint8* clothItem = (&Core::StaticRef<uint8*>(0x16677B0))[*reinterpret_cast<uint32*>(self + 0x7C)];
        if (clothItem != nullptr)
        {
            const float s1 = Core::Hook::CallMethod<float>(0xAACF20, self, 1);
            const float s0 = Core::Hook::CallMethod<float>(0xAACF20, self, 0);
            const float maxLimb = (s1 > s0) ? s1 : s0;

            float f1 = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667940)), maxLimb);
            if (f1 > 1.0f)
                f1 = Core::StaticRef<float>(0xD50398);

            const float f2 = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667AB4)), maxLimb);
            const int32 pitchVal = static_cast<int32>(f2 * Core::StaticRef<float>(0xD8F340));

            uint8** sndSlot18 = reinterpret_cast<uint8**>(self + 0x18);
            if (f1 > Core::StaticRef<float>(0xD90AA8))
            {
                if (*sndSlot18 == nullptr)
                {
                    AudSoundInitParams params;
                    InitSoundParams(params);
                    ped = *reinterpret_cast<uint8**>(self + 0x120);
                    *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
                    *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
                    const uint32 sndHash = *reinterpret_cast<uint32*>(clothItem + 0x12);
                    Core::Hook::CallMethod<void>(0xCE8890, self, sndHash, sndSlot18, &params, -1, 0, 0);
                    if (*sndSlot18 != nullptr)
                    {
                        AudSpeechParams sp{ 0, -1, 0x58 };
                        void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
                        uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
                        FinalizeAndPlaySound(*sndSlot18, reinterpret_cast<uint32>(mem), objId);
                    }
                }
                if (*sndSlot18 != nullptr)
                {
                    const float volDb = Core::Hook::Call<float>(0x794280, f1);
                    Core::Hook::CallMethod<void>(0x7B95C0, *sndSlot18, volDb);
                    Core::Hook::CallMethod<void>(0x8B5750, *sndSlot18, pitchVal);

                    AudAnimSpeechParams animSp;
                    Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x58);
                    *reinterpret_cast<float*>(animSp.data + 0x0C) = volDb;
                    *reinterpret_cast<int32*>(animSp.data + 0x1C) = pitchVal;
                    animSp.data[0x2D] |= 0x0A;
                    Core::Hook::Call<void>(0x7A9990, *reinterpret_cast<uint32*>(*sndSlot18 + 0xA4), &animSp);
                }
            }
            else if (*sndSlot18 != nullptr)
            {
                StopSoundImmediate(*sndSlot18);
            }
        }

        // Weapon loop sound (+0x14)
        ped = *reinterpret_cast<uint8**>(self + 0x120);
        uint8* weapMgr = (*reinterpret_cast<void**>(ped + 0x2DC) != nullptr)
            ? Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0)
            : nullptr;
        if (weapMgr != nullptr && *reinterpret_cast<int32*>(weapMgr + 0x18) == 5)
        {
            uint8** sndSlot14 = reinterpret_cast<uint8**>(self + 0x14);
            if (*sndSlot14 == nullptr)
            {
                Core::Hook::CallMethod<void>(0xCE9110, self, Core::Rebase(0xD917F8), sndSlot14, 1, 0, *reinterpret_cast<void**>(self + 0x8), -1, 0, 0);
                if (*sndSlot14 != nullptr)
                {
                    AudVec4 bonePos;
                    GetBoneOrPedPos(self, nullptr, 3, &bonePos);
                    Core::Hook::CallMethod<void>(0x7B9610, *sndSlot14, &bonePos);
                    Core::Hook::CallMethod<void>(0x8B6310, *sndSlot14, 3);
                    Core::Hook::CallMethod<void>(0x8B57D0, *sndSlot14, 1);

                    AudSoundInitParams params;
                    InitSoundParams(params);
                    *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

                    AudAnimSpeechParams animSp;
                    Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x4F);
                    animSp.data[0x2C] |= 4;
                    animSp.data[0x2D] |= 0x24;
                    *reinterpret_cast<float*>(animSp.data + 0x10) = bonePos.x;
                    *reinterpret_cast<float*>(animSp.data + 0x14) = bonePos.y;
                    *reinterpret_cast<float*>(animSp.data + 0x18) = bonePos.z;
                    *reinterpret_cast<int32*>(animSp.data + 0x24) = 3;
                    animSp.data[0x2E] = 1;

                    AudSpeechParams sp{ 0, -1, 0x4F };
                    uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91818), 0);
                    void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 0, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
                    Core::Hook::Call<void>(0x7A98F0, mem, &animSp, *reinterpret_cast<void**>(self + 0x120));
                    uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
                    FinalizeAndPlaySound(*sndSlot14, reinterpret_cast<uint32>(mem), objId);
                }
            }
            if (*sndSlot14 != nullptr)
            {
                ped = *reinterpret_cast<uint8**>(self + 0x120);
                using GetVelFn = float*(__thiscall*)(void*, AudVec4*);
                AudVec4 velTmp;
                float* vel = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(ped))[0xEC / 4])(ped, &velTmp);
                float spd = std::sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2])
                    * Core::StaticRef<float>(0x11DDE7C) * Core::StaticRef<float>(0xD7E2BC);
                const float one = Core::StaticRef<float>(0xD50398);
                if (spd > one)
                    spd = one;

                const float armLimb = Core::Hook::CallMethod<float>(0xAACF20, self, 2) * spd;
                const float smoothed = Core::Hook::CallMethod<float>(0xCED4B0, self + 0x90, armLimb, now);
                const float rawVol = Core::Hook::CallMethod<float>(0xAE5390, reinterpret_cast<void*>(Core::Rebase(0x168CFF0)), 7);
                const float mixVol = rawVol * Core::StaticRef<float>(0xDCB2A0) + smoothed * Core::StaticRef<float>(0xD8523C);
                const int32 mixPitch = static_cast<int32>(rawVol * Core::StaticRef<float>(0xD582A0) + smoothed * Core::StaticRef<float>(0xD84CA0));

                Core::Hook::CallMethod<void>(0x7B95C0, *sndSlot14, mixVol);
                Core::Hook::CallMethod<void>(0x8B5750, *sndSlot14, mixPitch);

                AudAnimSpeechParams animSp;
                Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x4F);
                animSp.data[0x2D] |= 0x0A;
                *reinterpret_cast<float*>(animSp.data + 0x0C) = mixVol;
                *reinterpret_cast<int32*>(animSp.data + 0x1C) = mixPitch;
                Core::Hook::Call<void>(0x7A9990, *reinterpret_cast<uint32*>(*sndSlot14 + 0xA4), &animSp);
            }
        }
        else if (*reinterpret_cast<void**>(self + 0x14) != nullptr)
        {
            StopSoundImmediate(*reinterpret_cast<uint8**>(self + 0x14));
        }

        if (*reinterpret_cast<void**>(self + 0x130) != nullptr)
        {
            const uint32 endTime = *reinterpret_cast<uint32*>(self + 0x138);
            if (endTime != 0 && endTime < Core::StaticRef<uint32>(0x11DDE74))
                StopSoundImmediate(*reinterpret_cast<uint8**>(self + 0x130));
        }

        updateSpeechFlags(*reinterpret_cast<uint8**>(self + 0x8));

        const float smooth40 = Core::Hook::CallMethod<float>(0xCED4B0, self + 0x24, *reinterpret_cast<float*>(self + 0x40), now);
        *reinterpret_cast<float*>(self + 0x40) *= Core::StaticRef<float>(0xD90BB4);
        uint8** sndSlot20 = reinterpret_cast<uint8**>(self + 0x20);
        if (*sndSlot20 == nullptr && smooth40 > Core::StaticRef<float>(0xD90AA8))
        {
            uint32& guard = Core::StaticRef<uint32>(0x1667BB4);
            if ((guard & 1) == 0)
            {
                guard |= 1;
                Core::StaticRef<uint32>(0x1667BB0) = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91838), 0);
            }
            AudSoundInitParams params;
            InitSoundParams(params);
            ped = *reinterpret_cast<uint8**>(self + 0x120);
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
            const uint32 sndHash = Core::StaticRef<uint32>(0x1667BB0);
            Core::Hook::CallMethod<void>(0xCE8890, self, sndHash, sndSlot20, &params, -1, 0, 0);
            if (*sndSlot20 != nullptr)
            {
                AudSpeechParams sp{ 0, -1, 0x5F };
                void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
                uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
                FinalizeAndPlaySound(*sndSlot20, reinterpret_cast<uint32>(mem), objId);
            }
        }
        if (*sndSlot20 != nullptr)
        {
            const float volDb = Core::Hook::Call<float>(0x794280, smooth40);
            Core::Hook::CallMethod<void>(0x7B95C0, *sndSlot20, volDb);
            if (smooth40 <= Core::StaticRef<float>(0xD90AA8))
                StopSoundImmediate(*sndSlot20);
        }

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        if ((ped[0x26C] & 4) != 0 && *reinterpret_cast<void**>(ped + 0xB40) != nullptr)
            *reinterpret_cast<float*>(self + 0x144) = 0.0f;

        if (!IsPedFemale(ped))
            return;

        uint8* veh = ((ped[0x26C] & 4) != 0) ? *reinterpret_cast<uint8**>(ped + 0xB40) : nullptr;
        if (veh != nullptr)
        {
            if (*reinterpret_cast<int32*>(veh + 0x1354) != 1)
            {
                StopSoundImmediate(*reinterpret_cast<uint8**>(self + 0x198));
                return;
            }
        }
        else
        {
            veh = ped;
        }

        AudVec4 velVec;
        using GetVelFn = float*(__thiscall*)(void*, AudVec4*);
        float* v = reinterpret_cast<GetVelFn>((*reinterpret_cast<void***>(veh))[0xEC / 4])(veh, &velVec);
        const float speedVal = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        const uintptr_t curveAddr = (veh != ped) ? 0x1667994 : 0x1667A10;
        const float rawWindVol = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(curveAddr)), speedVal);
        const float windVol = Core::Hook::CallMethod<float>(0xCED4B0, self + 0x17C, rawWindVol, now);

        uint8** sndSlot198 = reinterpret_cast<uint8**>(self + 0x198);
        if (windVol <= Core::StaticRef<float>(0xD90AA8))
        {
            StopSoundImmediate(*sndSlot198);
            return;
        }

        if (*sndSlot198 == nullptr)
        {
            uint32& guard = Core::StaticRef<uint32>(0x1667BB4);
            if ((guard & 2) == 0)
            {
                guard |= 2;
                Core::Hook::CallMethod<uint32*>(0x435E10, &Core::StaticRef<uint32>(0x1667BAC), Core::Rebase(0xD91844));
            }
            AudSoundInitParams params;
            InitSoundParams(params);
            ped = *reinterpret_cast<uint8**>(self + 0x120);
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

            const uint32 sndHash = Core::StaticRef<uint32>(0x1667BAC);
            void* mem = Core::Hook::Call<void*>(0x799A90);
            void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
            if (Core::Hook::CallMethod<bool>(0xCE8A00, self, sndHash, sndSlot198, &params, mem, speechObj, 0))
            {
                AudSpeechParams sp{ 0, -1, 0x38 };
                Core::Hook::Call<void>(0x7A9810, sndHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
            }
            else
            {
                Core::Hook::Call<void>(0x799B20, mem);
            }
        }

        if (*sndSlot198 != nullptr)
        {
            const float volDb = Core::Hook::Call<float>(0x794280, windVol);
            Core::Hook::CallMethod<void>(0x7B95C0, *sndSlot198, volDb);

            AudAnimSpeechParams animSp;
            Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x38);
            animSp.data[0x2D] |= 2;
            *reinterpret_cast<float*>(animSp.data + 0x0C) = volDb;
            Core::Hook::Call<void>(0x7A9990, *reinterpret_cast<uint32*>(*sndSlot198 + 0xA4), &animSp);
        }
    }
}

namespace
{
    // 0xAB0300
    void __fastcall PlayMeleeHitSound2(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        uint8* obj = *reinterpret_cast<uint8**>(ped + 0x2DC);
        if (obj == nullptr)
            return;
        uint8* slot = *reinterpret_cast<uint8**>(obj + 0x25C);
        if (slot == nullptr)
            return;
        uint8* weapInfo = *reinterpret_cast<uint8**>(slot + 0x10);
        if (weapInfo == nullptr || *reinterpret_cast<uint32*>(self + 0xB8) == 0)
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        AudVec4 bonePos;
        GetBoneOrPedPos(self, nullptr, 1, &bonePos);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<AudVec4**>(params.data + 0x14) = &bonePos;

        uint32 soundHash = Core::StaticRef<uint32>(0x17192D0);
        const uint8 flag32 = weapInfo[0x32];
        if (flag32 == 1)
        {
            uint8* mat = *reinterpret_cast<uint8**>(self + 0xB8);
            soundHash = *reinterpret_cast<uint32*>(mat + 0x4B);
        }
        else if (flag32 == 0)
        {
            if (Core::StaticRef<float>(0xD50398) > Core::StaticRef<float>(0xF1D9D8))
            {
                uint32& guard = Core::StaticRef<uint32>(0x1667BBC);
                if ((guard & 1) == 0)
                {
                    guard |= 1;
                    Core::Hook::CallMethod<uint32*>(0x435E10, &Core::StaticRef<uint32>(0x1667BB8), Core::Rebase(0xD91870));
                }
                soundHash = Core::StaticRef<uint32>(0x1667BB8);
            }
            else
            {
                uint8* mat = *reinterpret_cast<uint8**>(self + 0xB8);
                soundHash = *reinterpret_cast<uint32*>(mat + 0x47);
            }
        }

        uint8* snd = nullptr;
        *reinterpret_cast<uint32*>(params.data + 0x28) = 0x1F4;
        Core::Hook::CallMethod<void>(0xCE86D0, self, soundHash, &snd, &params, -1, 0, 0);
        if (snd == nullptr)
            return;

        uint8* mat = *reinterpret_cast<uint8**>(self + 0xB8);
        AudSpeechParams sp{ 0, -1, 0x56 };
        if (mat != nullptr)
        {
            const float volDb = Core::Hook::Call<float>(0x794280, static_cast<float>(mat[0x1A]) * Core::StaticRef<float>(0xDAFA18));
            Core::Hook::CallMethod<void>(0x7B95C0, snd, volDb);

            AudAnimSpeechParams animSp;
            Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x56);
            *reinterpret_cast<float*>(animSp.data + 0x0C) = volDb;
            animSp.data[0x2D] |= 2;

            void* mem = Core::Hook::Call<void*>(0x7A9810, soundHash, 0, 0, 0, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
            Core::Hook::Call<void>(0x7A98F0, mem, &animSp, *reinterpret_cast<void**>(self + 0x120));
            uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
            FinalizeAndPlaySound(snd, reinterpret_cast<uint32>(mem), objId);
        }
        else
        {
            void* mem = Core::Hook::Call<void*>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
            uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
            FinalizeAndPlaySound(snd, reinterpret_cast<uint32>(mem), objId);
        }
    }

    // 0xAB0540
    void __fastcall PlayWaterDiveOrSplashSound(uint8* self, void* /*edx*/, float arg0)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (ped[0x219] == 0)
            return;
        if (Core::Hook::CallMethod<bool>(0xA58530, ped + 0x580))
            return;
        if (Core::Hook::CallMethod<bool>(0xA58B70, ped + 0x580, 0, 0))
            return;
        if (Core::Hook::CallMethod<bool>(0xA58510, ped + 0x580))
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        AudVec4 bonePos;
        GetBoneOrPedPos(self, nullptr, 4, &bonePos);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        const uintptr_t sndName = Core::Hook::CallMethod<bool>(0x42FFC0, *reinterpret_cast<void**>(self + 0x120))
            ? 0xD9188C
            : 0xD9189C;

        uint8* snd = nullptr;
        Core::Hook::CallMethod<void>(0xCE8B80, self, Core::Rebase(sndName), &snd, &params, -1, 0, 0);
        if (snd == nullptr)
            return;

        using VFn0 = float*(__thiscall*)(void*, uint32);
        float* volPtr = reinterpret_cast<VFn0>((*reinterpret_cast<void***>(snd))[0])(snd, Core::StaticRef<uint32>(0x1667A38));
        if (volPtr != nullptr)
        {
            const float one = Core::StaticRef<float>(0xD50398);
            *volPtr = one - ((arg0 - one) * Core::StaticRef<float>(0xEBE088));
        }
        Core::Hook::CallMethod<void>(0x8B6310, snd, 4);
        Core::Hook::CallMethod<void>(0x7B9610, snd, &bonePos);
        Core::Hook::CallMethod<void>(0x8B57D0, snd, 1);

        const float volVal = volPtr ? *volPtr : 0.0f;
        void* mem = Core::Hook::Call<void*>(0x7A9D90, volVal, *reinterpret_cast<void**>(self + 0x120));
        uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
        FinalizeAndPlaySound(snd, reinterpret_cast<uint32>(mem), objId);
    }

    // 0xAB06E0
    void __fastcall PlayFootstepSound(uint8* self, void* /*edx*/, uint32 stepType, bool forceFlag)
    {
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        if ((*reinterpret_cast<uint32*>(ped + 0x28) & 0x800000) != 0)
            return;

        const uint32 boneType = Core::Hook::CallStdcall<uint32>(0xAACDA0, stepType);
        AudVec4 bonePos;
        GetBoneOrPedPos(self, nullptr, boneType, &bonePos);

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        float v144 = *reinterpret_cast<float*>(self + 0x144);
        if (((*reinterpret_cast<uint32*>(ped + 0x24) >> 27) & 1) == 0
            && (ped[0x218] != 0 || ped[0x219] == 0
                || !Core::Hook::CallMethod<bool>(0xAD9AE0, reinterpret_cast<void*>(Core::Rebase(0x10CC370)))))
        {
            const float glob = Core::StaticRef<float>(0x163952C);
            if (glob > Core::StaticRef<float>(0xD79274))
                v144 += glob * Core::StaticRef<float>(0xDCB2A8);
            else
                v144 -= Core::StaticRef<float>(0xEBDEE0);
        }
        else
        {
            v144 -= Core::StaticRef<float>(0xEBDEE0);
        }

        if (v144 < 0.0f)
            v144 = 0.0f;
        else if (v144 > Core::StaticRef<float>(0xD50398))
            v144 = Core::StaticRef<float>(0xD50398);
        *reinterpret_cast<float*>(self + 0x144) = v144;

        if (stepType <= 5 && !forceFlag && (ped[0x26C] & 1) == 0)
            return;

        auto playStepStr = [&](uintptr_t nameAddr) {
            AudSoundInitParams params;
            InitSoundParams(params);
            *reinterpret_cast<float*>(params.data + 0x00) = *reinterpret_cast<float*>(self + 0xBC);
            *reinterpret_cast<void**>(params.data + 0x0C) = *reinterpret_cast<uint8**>(self + 0x120) + 0x790;
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
            Core::Hook::CallMethod<void>(0xCE8E70, self, Core::Rebase(nameAddr), &params, -1, 0, 0);
        };

        if (stepType == 0 || stepType == 0xA || stepType == 0xE || stepType == 0x10)
        {
            const uint8 flags128 = self[0x128];
            if ((flags128 & 1) != 0)
            {
                playStepStr(0xD918B4);
            }
            else if ((flags128 & 4) != 0)
            {
                if (Core::Hook::CallMethod<int32>(0x447370, *reinterpret_cast<uint8**>(self + 0x120) + 0xB98) >= 10)
                    playStepStr(0xD918C4);
            }
            if (*reinterpret_cast<int32*>(self + 0x88) == 1)
                playStepStr(0xD918D4);
        }
        else if (stepType == 1 || stepType == 0xB || stepType == 0xF || stepType == 0x11)
        {
            const uint8 flags128 = self[0x128];
            if ((flags128 & 2) != 0)
            {
                playStepStr(0xD918DC);
            }
            else if ((flags128 & 8) != 0)
            {
                if (Core::Hook::CallMethod<int32>(0x447370, *reinterpret_cast<uint8**>(self + 0x120) + 0xB98) >= 10)
                    playStepStr(0xD918EC);
            }
            if (*reinterpret_cast<int32*>(self + 0x88) == 1)
                playStepStr(0xD918FC);
        }

        if (stepType >= 8 && stepType <= 0xD && Core::StaticRef<uint32>(0x1667840) != 0)
        {
            const uint32 sndHash = Core::Hook::CallMethod<uint32>(0xAACE00, self, Core::StaticRef<uint32>(0x1667840), stepType - 8, *reinterpret_cast<uint32*>(self + 0x78));
            AudSoundInitParams params;
            InitSoundParams(params);
            params.data[0x46] |= 4;
            *reinterpret_cast<AudVec4**>(params.data + 0x14) = &bonePos;
            *reinterpret_cast<int32*>(params.data + 0x04) = static_cast<int32>(ComputeLinearToMillibels(*reinterpret_cast<float*>(self + 0xC0)));
            *reinterpret_cast<float*>(params.data + 0x00) = *reinterpret_cast<float*>(self + 0xBC);
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
            *reinterpret_cast<uint32*>(params.data + 0x34) = boneType;
            Core::Hook::CallMethod<void>(0xCE8930, self, sndHash, &params, -1, 0, 0);
        }

        uint8* mat = *reinterpret_cast<uint8**>(self + 0xB8);
        if (mat != nullptr)
        {
            uint8* matSnd = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), *reinterpret_cast<uint32*>(mat + 0x21));
            if (self[0x1A0] != 0)
                matSnd = Core::StaticRef<uint8*>(0x166783C);

            ped = *reinterpret_cast<uint8**>(self + 0x120);
            if ((ped[0x26C] & 4) != 0 && *reinterpret_cast<void**>(ped + 0xB40) != nullptr)
            {
                uint8* veh = *reinterpret_cast<uint8**>(ped + 0xB40);
                uint8* vehMat = *reinterpret_cast<uint8**>(veh + 0xB24);
                if (vehMat != nullptr)
                    matSnd = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), *reinterpret_cast<uint32*>(vehMat + 0x21));
            }

            if (matSnd != nullptr)
            {
                float pitch = 0.0f;
                float vol = 0.0f;
                if (stepType == 0x10 || stepType == 0x11)
                {
                    ped = *reinterpret_cast<uint8**>(self + 0x120);
                    pitch = IsPedFemale(ped) ? Core::StaticRef<float>(0xD91D8C) : Core::StaticRef<float>(0xD90B00);
                    vol = Core::StaticRef<float>(0xD50398);
                }
                else
                {
                    GetFootstepPitchAndVolume(self, nullptr, &pitch, &vol);
                }

                if (stepType == 2 || stepType == 3)
                    pitch -= Core::StaticRef<float>(0xDCB2A4);

                AudSoundInitParams footParams;
                InitSoundParams(footParams);

                const int32 stepGroup = *reinterpret_cast<int32*>(self + 0x78);
                if (stepGroup > 0 && stepGroup <= 6 && mat[0x25] > 0
                    && (stepType <= 5 || (stepType >= 0xE && stepType <= 0x11)))
                {
                    const uint32 sndHash = Core::Hook::CallMethod<uint32>(0xAACE00, self, matSnd, stepType, stepGroup);
                    footParams.data[0x46] |= 4;
                    *reinterpret_cast<uint32*>(footParams.data + 0x1C) = (&Core::StaticRef<uint32>(0x1667844))[stepGroup];
                    *reinterpret_cast<void**>(footParams.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
                    *reinterpret_cast<AudVec4**>(footParams.data + 0x14) = &bonePos;
                    const float baseDb = Core::Hook::Call<float>(0x794280, static_cast<float>(mat[0x25]) * Core::StaticRef<float>(0xDAFA18));
                    *reinterpret_cast<float*>(footParams.data + 0x00) = baseDb + *reinterpret_cast<float*>(self + 0xBC) + pitch;
                    *reinterpret_cast<int32*>(footParams.data + 0x04) = static_cast<int32>(ComputeLinearToMillibels(vol));
                    *reinterpret_cast<uint32*>(footParams.data + 0x34) = boneType;
                    Core::Hook::CallMethod<void>(0xCE8930, self, sndHash, &footParams, -1, 0, 0);

                    if ((stepType == 0 || stepType == 1 || stepType == 0xE || stepType == 0xF || stepType == 0x10 || stepType == 0x11)
                        && stepGroup == 4)
                    {
                        float f62 = 0.0f;
                        float f63 = 0.0f;
                        uint8* pedObj = *reinterpret_cast<uint8**>(self + 0x120);
                        Core::Hook::CallMethod<void>(0xA598A0, pedObj + 0x580, 1, &f62, &f63, 0);
                        if (f62 > Core::StaticRef<float>(0xD908F8))
                        {
                            *reinterpret_cast<float*>(footParams.data + 0x28) = f63;
                            *reinterpret_cast<float*>(footParams.data + 0x00) += f62 - Core::StaticRef<float>(0xDCB2A0);
                            uint8* snd = nullptr;
                            Core::Hook::CallMethod<void>(0xCE86D0, self, sndHash, &snd, &footParams, -1, 0, 0);
                            if (snd != nullptr)
                            {
                                const float* camPos = Core::Hook::CallMethod<const float*>(0x8B4CA0, reinterpret_cast<void*>(Core::Rebase(0x17171F0)), 0);
                                AudVec4 dir{ bonePos.x - camPos[0], bonePos.y - camPos[1], bonePos.z - camPos[2], 0.0f };
                                Core::Hook::CallMethod<void>(0x5B1DC0, &dir, Core::StaticRef<float>(0xEBE478), 0x7A);
                                AudVec4 finalPos{ camPos[0] + dir.x, camPos[1] + dir.y, camPos[2] + dir.z, 0.0f };
                                Core::Hook::CallMethod<void>(0x7B9610, snd, &finalPos);
                                Core::Hook::CallMethod<void>(0xCE1720, snd, 0, 0, 0);
                            }
                        }
                    }
                }

                if (*reinterpret_cast<float*>(self + 0x144) > Core::StaticRef<float>(0xDAFA18))
                {
                    uint32& guard = Core::StaticRef<uint32>(0x1667BC4);
                    if ((guard & 1) == 0)
                    {
                        guard |= 1;
                        Core::Hook::CallMethod<uint32*>(0x435E10, &Core::StaticRef<uint32>(0x1667BC0), Core::Rebase(0xD91904));
                    }
                    footParams.data[0x44] = 0xFF;
                    const float baseDb = Core::Hook::Call<float>(0x794280,
                        static_cast<float>(mat[0x25]) * Core::StaticRef<float>(0xDAFA18) * *reinterpret_cast<float*>(self + 0x144));
                    *reinterpret_cast<float*>(footParams.data + 0x00) = baseDb + *reinterpret_cast<float*>(self + 0xBC) + pitch;
                    Core::Hook::CallMethod<void>(0xCE8930, self, Core::StaticRef<uint32>(0x1667BC0), &footParams, -1, 0, 0);
                }

                ped = *reinterpret_cast<uint8**>(self + 0x120);
                if (stepType == 0xF || stepType == 0xE || *reinterpret_cast<int32*>(ped + 0xB90) == 4)
                {
                    if (Core::Hook::CallMethod<bool>(0x9AB6D0, ped + 0x2B0, 1))
                    {
                        uint8* weapMgr = Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0);
                        if (weapMgr != nullptr && *reinterpret_cast<void**>(weapMgr + 0x10) != nullptr)
                        {
                            uint8* weapInfo = *reinterpret_cast<uint8**>(weapMgr + 0x10);
                            footParams.data[0x44] = 0xFF;
                            *reinterpret_cast<float*>(footParams.data + 0x00) = *reinterpret_cast<float*>(self + 0xBC) + pitch;
                            *reinterpret_cast<uint32*>(footParams.data + 0x04) = 0;
                            Core::Hook::CallMethod<void>(0xCE8930, self, *reinterpret_cast<uint32*>(weapInfo + 0x2A), &footParams, -1, 0, 0);
                        }
                    }
                }

                if ((stepType == 6 || stepType == 7) && *reinterpret_cast<void**>(self + 0xB4) != nullptr)
                {
                    uint8* ent = *reinterpret_cast<uint8**>(self + 0xB4);
                    if (*reinterpret_cast<void**>(ent + 0x38) != nullptr)
                    {
                        uint8 impactBuf[0x60]{};
                        Core::Hook::CallMethod<void>(0x425360, impactBuf);
                        *reinterpret_cast<void**>(impactBuf + 0x00) = *reinterpret_cast<void**>(ent + 0x38);
                        *reinterpret_cast<uint32*>(impactBuf + 0x48) = *reinterpret_cast<uint32*>(self + 0xAC);
                        *reinterpret_cast<uint16*>(impactBuf + 0x52) = *reinterpret_cast<uint16*>(self + 0xB0);
                        *reinterpret_cast<float*>(impactBuf + 0x10) = bonePos.x;
                        *reinterpret_cast<float*>(impactBuf + 0x14) = bonePos.y;
                        *reinterpret_cast<float*>(impactBuf + 0x18) = bonePos.z;
                        Core::Hook::CallMethod<void>(0xAD8FB0, reinterpret_cast<void*>(Core::Rebase(0x1674D30)), impactBuf, Core::StaticRef<float>(0xDC9290), 0, 0);
                    }
                }

                if (mat[0x26] > 0 && (stepType == 0 || stepType == 1 || stepType == 0xE || stepType == 0xF || stepType == 0x10 || stepType == 0x11))
                {
                    uint8* ent = *reinterpret_cast<uint8**>(self + 0xB4);
                    if (ent != nullptr && *reinterpret_cast<void**>(ent + 0x38) != nullptr)
                    {
                        uint8 impactBuf[0x60]{};
                        Core::Hook::CallMethod<void>(0x425360, impactBuf);
                        *reinterpret_cast<void**>(impactBuf + 0x00) = *reinterpret_cast<void**>(ent + 0x38);
                        *reinterpret_cast<uint32*>(impactBuf + 0x48) = *reinterpret_cast<uint32*>(self + 0xAC);
                        *reinterpret_cast<uint16*>(impactBuf + 0x52) = *reinterpret_cast<uint16*>(self + 0xB0);
                        *reinterpret_cast<float*>(impactBuf + 0x10) = bonePos.x;
                        *reinterpret_cast<float*>(impactBuf + 0x14) = bonePos.y;
                        *reinterpret_cast<float*>(impactBuf + 0x18) = bonePos.z;
                        const float f = static_cast<float>(mat[0x26]) * Core::StaticRef<float>(0xDAFA18) * Core::StaticRef<float>(0xD90B00);
                        Core::Hook::CallMethod<void>(0xAD8FB0, reinterpret_cast<void*>(Core::Rebase(0x1674D30)), impactBuf, f, 0, 0);
                    }
                }

                if ((*reinterpret_cast<uint32*>(self + 0x74) & (1u << (stepType & 0x1F))) != 0)
                {
                    AudSoundInitParams params;
                    InitSoundParams(params);
                    params.data[0x46] |= 4;
                    *reinterpret_cast<float*>(params.data + 0x00) = *reinterpret_cast<float*>(self + 0xBC) + pitch;
                    *reinterpret_cast<uint32*>(params.data + 0x1C) = (&Core::StaticRef<uint32>(0x1667844))[*reinterpret_cast<int32*>(self + 0x78)];
                    *reinterpret_cast<AudVec4**>(params.data + 0x14) = &bonePos;
                    *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
                    *reinterpret_cast<uint32*>(params.data + 0x34) = boneType;
                    Core::Hook::CallMethod<void>(0xCE8E70, self, Core::Rebase(0xD91914), &params, -1, 0, 0);
                }

                if (stepType == 0 || stepType == 1)
                {
                    const float ambScale = Core::Hook::CallMethod<float>(0xAADB50, self);
                    if (ambScale > Core::StaticRef<float>(0xD90AA8)
                        && Core::Hook::Call<bool>(0xCDBD10, Core::StaticRef<float>(0xD90B08)))
                    {
                        const int32 stepGroup2 = *reinterpret_cast<int32*>(self + 0x78);
                        const uint32 sndHash = Core::Hook::CallMethod<uint32>(0xAACE00, self, matSnd, 4, stepGroup2);
                        AudSoundInitParams params;
                        InitSoundParams(params);
                        params.data[0x46] |= 4;
                        *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0x794280, ambScale);
                        *reinterpret_cast<uint32*>(params.data + 0x1C) = (&Core::StaticRef<uint32>(0x1667844))[stepGroup2];
                        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
                        *reinterpret_cast<AudVec4**>(params.data + 0x14) = &bonePos;
                        *reinterpret_cast<uint32*>(params.data + 0x34) = boneType;
                        Core::Hook::CallMethod<void>(0xCE8930, self, sndHash, &params, -1, 0, 0);
                    }
                }
            }
        }

        if (CanPlayPedAudio())
        {
            Core::Hook::Call<void>(0x7AA500, *reinterpret_cast<void**>(self + 0x120), stepType, forceFlag ? 1 : 0);
        }

        ped = *reinterpret_cast<uint8**>(self + 0x120);
        if (stepType == 0 || stepType == 4 || stepType == 0xE || stepType == 0x10)
            Core::Hook::CallMethod<void>(0x8A3100, ped, 1);
        else if (stepType == 1 || stepType == 5 || stepType == 0xF || stepType == 0x11)
            Core::Hook::CallMethod<void>(0x8A3100, ped, 0);
    }

    // 0xAB11E0
    void __fastcall PlayScriptedSpeech(uint8* self, void* /*edx*/, uint32 unused, uint32 arg1)
    {
        if (!CanPlayPedAudio())
            return;
        if (*reinterpret_cast<void**>(self + 0xC) != nullptr)
            return;

        *reinterpret_cast<uint32*>(self + 0x70) = arg1;
        const bool isSpecial = (arg1 == 0x1A2 || arg1 == 0x1A3 || arg1 == 0x1A4 || arg1 == 0x1A5
            || arg1 == 0x38B0 || arg1 == 0x1A7 || arg1 == 0x1A8 || arg1 == 0x1A9
            || arg1 == 0x4B0 || arg1 == 0x39B0);
        self[0x12A] = isSpecial ? 1 : 0;

        AudVec4 bonePos;
        GetBoneWorldPos(self, nullptr, &bonePos);
        *reinterpret_cast<AudVec4*>(self + 0x60) = bonePos;

        uint32 soundHash = Core::StaticRef<uint32>(0x16677F0);
        AudSoundInitParams params;
        InitSoundParams(params);
        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        if (ped != nullptr)
        {
            *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            uint8* matSnd = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg1);
            if (matSnd == nullptr)
                return;
            const uint32 h = *reinterpret_cast<uint32*>(matSnd + 0xA);
            if (h > 0)
                soundHash = h;
            if (soundHash == Core::StaticRef<uint32>(0x16677F0))
            {
                uint8* weapMgr = Core::Hook::CallMethod<uint8*>(0x9A9CF0, ped + 0x2B0);
                if (weapMgr != nullptr && *reinterpret_cast<void**>(weapMgr + 0x10) != nullptr)
                {
                    uint8* weapInfo = *reinterpret_cast<uint8**>(weapMgr + 0x10);
                    const uint32 wh = *reinterpret_cast<uint32*>(weapInfo + 0x16);
                    if (wh > 0)
                        soundHash = wh;
                }
            }
            if (soundHash == Core::StaticRef<uint32>(0x16677F0))
                soundHash = Core::StaticRef<uint32>(0x16677A8);
        }
        else
        {
            soundHash = Core::StaticRef<uint32>(0x16677A8);
        }

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8A00, self, soundHash, self + 0xC, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x4C };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    // 0xAB1390
    void __fastcall PlayMeleeHitSound3(uint8* self, void* /*edx*/, uint32 arg0, uint8* colInfo, uint32 /*unused*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* matSnd = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg0);
        if (matSnd == nullptr)
            return;

        const uint32 h = *reinterpret_cast<uint32*>(matSnd + 0x16);
        if (h == 0)
            return;
        if (h == Core::StaticRef<uint32>(0x1667968))
        {
            PlayWaterSplashSounds(self, nullptr);
            return;
        }

        AudVec4 pos{
            *reinterpret_cast<float*>(colInfo + 0x10),
            *reinterpret_cast<float*>(colInfo + 0x14),
            *reinterpret_cast<float*>(colInfo + 0x18),
            0.0f
        };

        AudSoundInitParams params;
        InitSoundParams(params);
        *reinterpret_cast<AudVec4**>(params.data + 0x14) = &pos;
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8930, self, h, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x4B };
            Core::Hook::Call<void>(0x7A9810, h, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    // 0xAB14C0
    void __fastcall UpdateSoundBonePos(uint8* self, void* /*edx*/, uint8* snd, uint32 /*unused*/)
    {
        const uint32 slotIdx = snd[0x4];
        uint8* boneInfo = nullptr;
        if (slotIdx != 0xFF)
        {
            const uint32 stride = Core::StaticRef<uint32>(0x1719040);
            const uint32 base = Core::StaticRef<uint32>(0x1719060);
            const uint32 bankOff = static_cast<uint32>(snd[0x40]) * 0x6F40;
            boneInfo = *reinterpret_cast<uint8**>(bankOff + base + 0x6F14) + stride * slotIdx;
        }

        if (boneInfo == nullptr)
            return;

        AudVec4 bonePos;
        GetBoneOrPedPos(self, nullptr, *reinterpret_cast<uint32*>(boneInfo + 0xCC), &bonePos);
        Core::Hook::CallMethod<void>(0x7B9610, snd, &bonePos);
    }

    // 0xAB1530
    void __fastcall UpdateActiveSpeechSound(uint8* self, void* /*edx*/)
    {
        if (!CanPlayPedAudio())
            return;

        uint8* snd = *reinterpret_cast<uint8**>(self + 0xC);
        if (snd == nullptr)
            return;

        AudVec4 bonePos;
        GetBoneWorldPos(self, nullptr, &bonePos);
        const float dx = bonePos.x - *reinterpret_cast<float*>(self + 0x60);
        const float dy = bonePos.y - *reinterpret_cast<float*>(self + 0x64);
        const float dz = bonePos.z - *reinterpret_cast<float*>(self + 0x68);
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz) * Core::StaticRef<float>(0x11DDE80);
        if (self[0x12A] != 0)
            dist *= Core::StaticRef<float>(0xDCE03C);
        dist /= Core::StaticRef<float>(0xF38010);

        const float one = Core::StaticRef<float>(0xD50398);
        const float clamped = (dist < one) ? dist : one;

        const uint32 divisor = Core::StaticRef<uint32>(0xF38014);
        const float invDiv = 1.0f / static_cast<float>(divisor);
        Core::Hook::CallMethod<void>(0xCED030, self + 0x44, invDiv, invDiv);
        const float smoothed = Core::Hook::CallMethod<float>(0xCED4B0, self + 0x44, clamped, Core::StaticRef<uint32>(0x11DDE74));

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        float pitch = 0.0f;
        float gain = 0.0f;
        if (ped[0x219] != 0)
        {
            pitch = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x166796C)), smoothed);
            gain = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x16678EC)), smoothed);
        }
        else
        {
            pitch = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x16678B8)), smoothed);
            gain = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667890)), smoothed);
        }

        const float volDb = Core::Hook::Call<float>(0x794280, gain);
        Core::Hook::CallMethod<void>(0x7B95C0, *reinterpret_cast<void**>(self + 0xC), volDb);
        const int32 mb = static_cast<int32>(ComputeLinearToMillibels(pitch));

        snd = *reinterpret_cast<uint8**>(self + 0xC);
        const uint32 slotIdx = snd[0x4];
        uint32* waveObj = nullptr;
        if (slotIdx != 0xFF)
        {
            const uint32 stride = Core::StaticRef<uint32>(0x1719040);
            const uint32 base = Core::StaticRef<uint32>(0x1719060);
            const uint32 bankOff = static_cast<uint32>(snd[0x40]) * 0x6F40;
            waveObj = reinterpret_cast<uint32*>(*reinterpret_cast<uint8**>(bankOff + base + 0x6F14) + stride * slotIdx);
        }
        if (waveObj != nullptr)
            Core::Hook::CallMethod<void>(0xCEF500, waveObj, mb);

        AudAnimSpeechParams animSp;
        Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x4C);
        animSp.data[0x2D] |= 0x0A;
        *reinterpret_cast<float*>(animSp.data + 0x0C) = volDb;
        *reinterpret_cast<int32*>(animSp.data + 0x1C) = mb;
        Core::Hook::Call<void>(0x7A9990, *reinterpret_cast<uint32*>(*reinterpret_cast<uint8**>(self + 0xC) + 0xA4), &animSp);

        *reinterpret_cast<AudVec4*>(self + 0x60) = bonePos;
    }
}

namespace
{
    // 0xAB1780
    void __fastcall PlayVehicleHitSound(uint8* self, void* /*edx*/, uint32 arg0, uint8* targetEntity, float arg2, uint32 arg3, uint32 arg4)
    {
        if (!CanPlayPedAudio())
            return;

        const float prob = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667A70)), arg2);
        if (Core::Hook::Call<bool>(0xCDBD10, prob))
            TryPlayScriptedPainSound(self, nullptr, arg3);

        uint32& guard = Core::StaticRef<uint32>(0x1667BCC);
        if ((guard & 1) == 0)
        {
            guard |= 1;
            Core::StaticRef<uint32>(0x1667BC8) = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91D0C), 0);
        }

        const float vol = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667A44)), arg2);
        if (vol <= Core::StaticRef<float>(0xD90AA8))
            return;

        AudSoundInitParams params;
        InitSoundParams(params);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<uint32*>(params.data + 0x14) = arg0;
        *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0x794280, vol);

        const uint32 soundHash = (*reinterpret_cast<int32*>(targetEntity + 0x1354) == 1)
            ? Core::StaticRef<uint32>(0x1667BC8)
            : (&Core::StaticRef<uint32>(0x1667AE0))[arg3];

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8930, self, soundHash, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x5D };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }
    }

    inline void AnimEvent_StepHelper(uint8* ped, uint32 stepType, bool forceFlag)
    {
        Core::Hook::CallMethod<void>(0xAB06E0, ped + 0x3D0, stepType, forceFlag);
    }

    // 0xAB18F0 .. 0xAB1B10 (18 wrappers)
    void __cdecl AnimEvent_Step_0_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x00, true); }
    void __cdecl AnimEvent_Step_2_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x02, true); }
    void __cdecl AnimEvent_Step_4_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x04, true); }
    void __cdecl AnimEvent_Step_10_1(uint32, uint8* ped)  { AnimEvent_StepHelper(ped, 0x10, true); }
    void __cdecl AnimEvent_Step_1_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x01, true); }
    void __cdecl AnimEvent_Step_3_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x03, true); }
    void __cdecl AnimEvent_Step_5_1(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x05, true); }
    void __cdecl AnimEvent_Step_11_1(uint32, uint8* ped)  { AnimEvent_StepHelper(ped, 0x11, true); }
    void __cdecl AnimEvent_Step_7_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x07, false); }
    void __cdecl AnimEvent_Step_6_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x06, false); }
    void __cdecl AnimEvent_Step_8_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x08, false); }
    void __cdecl AnimEvent_Step_A_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0A, false); }
    void __cdecl AnimEvent_Step_9_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x09, false); }
    void __cdecl AnimEvent_Step_B_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0B, false); }
    void __cdecl AnimEvent_Step_C_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0C, false); }
    void __cdecl AnimEvent_Step_D_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0D, false); }
    void __cdecl AnimEvent_Step_E_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0E, false); }
    void __cdecl AnimEvent_Step_F_0(uint32, uint8* ped)   { AnimEvent_StepHelper(ped, 0x0F, false); }

    // 0xAB1B30
    void __cdecl AnimEvent_LadderOrClimbStep(uint32 eventHash, uint8* ped)
    {
        if (!CanPlayPedAudio())
            return;

        uint32& guard = Core::StaticRef<uint32>(0x1667C34);
        if ((guard & 1) == 0)
        {
            guard |= 1;
            const uintptr_t keys[5] = { 0xD9156C, 0xD9154C, 0xD9152C, 0xD9150C, 0xD914EC };
            uint32* dst = &Core::StaticRef<uint32>(0x1667C20);
            for (int i = 0; i < 5; ++i)
                dst[i] = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(keys[i]), 0);
        }
        if ((guard & 2) == 0)
        {
            guard |= 2;
            const uintptr_t sndKeys[20] = {
                0xD91730, 0xD91714, 0xD916F8, 0xD916DC, 0xD916C0,
                0xD916AC, 0xD9169C, 0xD9168C, 0xD91678, 0xD91668,
                0xD91654, 0xD91640, 0xD9162C, 0xD91618, 0xD91604,
                0xD915EC, 0xD915D4, 0xD915BC, 0xD915A4, 0xD9158C
            };
            uint32* dst = &Core::StaticRef<uint32>(0x1667BD0);
            for (int i = 0; i < 20; ++i)
                dst[i] = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(sndKeys[i]), 0);
        }

        const uint32* eventHashes = &Core::StaticRef<uint32>(0x1667C20);
        uint32 idx = 0;
        while (idx < 5 && eventHashes[idx] != eventHash)
            ++idx;
        if (idx >= 5)
            return;

        uint8* intel = *reinterpret_cast<uint8**>(ped + 0x224);
        const uint32 boneFlag = (idx != 4) ? 0u : 2u;
        void* taskEntity = nullptr;
        uint32 matHash = 0;

        uint8* taskFE = Core::Hook::CallMethod<uint8*>(0x426850, intel + 0x44, 0xFE);
        if (taskFE != nullptr && *reinterpret_cast<void**>(taskFE + 0x64) != nullptr)
        {
            taskEntity = *reinterpret_cast<void**>(taskFE + 0x64);
            matHash = *reinterpret_cast<uint32*>(taskFE + 0x68);
        }
        else
        {
            uint8* taskD2 = Core::Hook::CallMethod<uint8*>(0x426850, intel + 0x44, 0xD2);
            if (taskD2 == nullptr || *reinterpret_cast<void**>(taskD2 + 0x34) == nullptr)
                return;
            taskEntity = *reinterpret_cast<void**>(taskD2 + 0x34);
            matHash = *reinterpret_cast<uint32*>(taskD2 + 0x38);
        }

        if (matHash == 0)
            return;

        uint8* matSnd = Core::Hook::CallMethod<uint8*>(0xCDB630, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), matHash);
        if (matSnd == nullptr)
            return;

        uint32 subIdx = 0;
        switch (matSnd[0x63])
        {
        case 1: subIdx = 3; break;
        case 2: subIdx = 2; break;
        case 3: subIdx = 1; break;
        default: subIdx = 0; break;
        }

        if (idx == 1)
        {
            Core::Hook::CallMethod<void>(0xAB06E0, ped + 0x3D0, 0, 1);
            return;
        }

        uint8* pedAudio = ped + 0x3D0;
        AudSoundInitParams params;
        InitSoundParams(params);
        const uint32 boneType = Core::Hook::CallMethod<uint32>(0xBE61F0, pedAudio);
        *reinterpret_cast<uint32*>(params.data + 0x20) = boneType;

        AudVec4 bonePos;
        GetBoneOrPedPos(pedAudio, nullptr, boneFlag, &bonePos);
        *reinterpret_cast<AudVec4**>(params.data + 0x14) = &bonePos;

        const uint32 soundHash = (&Core::StaticRef<uint32>(0x1667BD0))[idx + subIdx * 5];
        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8930, pedAudio, soundHash, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x5E };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, ped, mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }

        SetFootstepMaterialDirect(pedAudio, nullptr, reinterpret_cast<uint32>(matSnd), static_cast<uint8*>(taskEntity), 0);
    }

    // 0xAB1F00
    void __fastcall UpdatePedAudioFrame(uint8* self, void* /*edx*/, uint32 /*unused*/)
    {
        if (*reinterpret_cast<void**>(self + 0x8) != nullptr && *reinterpret_cast<void**>(self + 0x120) != nullptr)
        {
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            Core::Hook::CallMethod<void>(0xAC7AF0, *reinterpret_cast<void**>(self + 0x8), *reinterpret_cast<void**>(ped + 0x20));
        }

        if (!CanPlayPedAudio())
            return;

        if (self[0x10] != 0)
        {
            if (*reinterpret_cast<void**>(self + 0xC) != nullptr)
            {
                StopAndReleaseSound(self + 0xC);
                *reinterpret_cast<void**>(self + 0xC) = nullptr;
            }
            self[0x10] = 0;
        }

        if (*reinterpret_cast<void**>(self + 0xC) != nullptr)
            UpdateActiveSpeechSound(self, nullptr);

        uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
        const uint32 stateD40 = ped[0xD40];
        if (stateD40 != 0xFF && *reinterpret_cast<void**>(self + 0x124) == nullptr)
        {
            const uintptr_t fmtAddr = (stateD40 > 0xA && (Core::StaticRef<uint8>(0x1667839) != 0 || !Core::Hook::CallMethod<bool>(0x454450, ped)))
                ? 0xD91940
                : 0xD91928;
            char nameBuf[0x40]{};
            Core::Hook::Call<void>(0x4387F0, nameBuf, 0x40, Core::Rebase(fmtAddr), stateD40);

            AudSoundInitParams params;
            InitSoundParams(params);
            *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
            const uint32 unkVar = Core::Hook::Call<uint32>(0xCDF870, Core::Rebase(0xD91958));
            *reinterpret_cast<uint32*>(params.data + 0x18) = unkVar;

            ped = *reinterpret_cast<uint8**>(self + 0x120);
            if (ped[0x218] != 0 || ped[0x219] == 0)
            {
                *reinterpret_cast<void**>(params.data + 0x0C) = ped + 0x790;
            }
            else
            {
                *reinterpret_cast<uint32*>(params.data + 0x40) = 0;
                params.data[0x45] = 2;
            }

            uint8** sndSlot124 = reinterpret_cast<uint8**>(self + 0x124);
            Core::Hook::CallMethod<void>(0xCE8CE0, self, nameBuf, sndSlot124, &params, -1, 0, 0);
            if (*sndSlot124 != nullptr)
            {
                Core::Hook::CallMethod<void>(0xAACFB0, *sndSlot124, 0.0f);
                AudSpeechParams sp{ 0, -1, 0x4E };
                uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, nameBuf, 0);
                void* mem = Core::Hook::Call<void*>(0x7A9810, sndHash, 0, 1, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), -1);
                uint32 objId = Core::Hook::Call<uint32>(0x799CC0, mem);
                *reinterpret_cast<uint32*>(*sndSlot124 + 0xA4) = reinterpret_cast<uint32>(mem);
                *reinterpret_cast<uint32*>(*sndSlot124 + 0xA8) = objId;
                *reinterpret_cast<uint32*>(*sndSlot124 + 0xAC) = 0;
                Core::Hook::CallMethod<void>(0xCE1720, *sndSlot124, unkVar, 1, 0x9C4);
            }
        }

        uint8* snd124 = *reinterpret_cast<uint8**>(self + 0x124);
        if (snd124 != nullptr)
        {
            Core::Hook::CallMethod<void>(0x7B95C0, snd124, 0.0f);
            if (stateD40 == 0xFF)
            {
                StopAndReleaseSound(self + 0x124);
            }
            else
            {
                AudAnimSpeechParams animSp;
                Core::Hook::CallMethod<void>(0xB22A90, &animSp, 0x4E);
                animSp.data[0x2D] |= 2;
                *reinterpret_cast<float*>(animSp.data + 0x0C) = 0.0f;
                Core::Hook::Call<void>(0x7A9990, *reinterpret_cast<uint32*>(snd124 + 0xA4), &animSp);
            }
        }
    }

    // 0xAB21A0
    void __fastcall PlayPedImpactSound(uint8* self, void* /*edx*/, uint32 posPtr, uint32 /*unusedA3*/, uint8* targetEntity, float vol, uint32 boneIdx, uint32 arg6, uint32 flagA8)
    {
        if (!CanPlayPedAudio())
            return;

        const uint32 now = Core::StaticRef<uint32>(0x11DDE74);
        if (targetEntity != nullptr && (*reinterpret_cast<uint32*>(targetEntity + 0x28) & 0x3C0) == 0x80)
        {
            PlayVehicleHitSound(self, nullptr, posPtr, targetEntity, vol, boneIdx, arg6);
            *reinterpret_cast<uint32*>(self + 0x14C) = now;
        }
        else
        {
            const uint32 dt = now - *reinterpret_cast<uint32*>(self + 0x14C);
            vol *= Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x16679BC)), static_cast<float>(dt));
        }

        if ((targetEntity == nullptr || (*reinterpret_cast<uint32*>(targetEntity + 0x28) & 0x3C0) != 0x80)
            && now > *reinterpret_cast<uint32*>(self + 0x150) + 0x7D0)
        {
            const float prob = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x16679E4)), vol);
            if (Core::Hook::Call<bool>(0xCDBD10, prob))
            {
                if ((flagA8 & 0xFF) == 0 || (&Core::StaticRef<uint32>(0x1667B38))[boneIdx] == Core::StaticRef<uint32>(0x1667B3C))
                    TryPlayScriptedPainSound(self, nullptr, boneIdx);
            }
        }

        uint8* item = (boneIdx >= 7)
            ? (&Core::StaticRef<uint8*>(0x16677F8))[*reinterpret_cast<uint32*>(self + 0x80)]
            : (&Core::StaticRef<uint8*>(0x16677B0))[*reinterpret_cast<uint32*>(self + 0x7C)];
        if (item == nullptr && boneIdx != 0xC)
            return;

        const float finalVol = Core::Hook::CallMethod<float>(0xCE4D10, reinterpret_cast<void*>(Core::Rebase(0x1667868)), vol);
        if (finalVol <= Core::StaticRef<float>(0xD90AA8))
            return;

        uint32 soundHash = Core::StaticRef<uint32>(0x17192D0);
        if (boneIdx != 0xC)
        {
            soundHash = *reinterpret_cast<uint32*>(item + 0xA);
        }
        else
        {
            uint8* ped = *reinterpret_cast<uint8**>(self + 0x120);
            if (Core::Hook::CallMethod<bool>(0x8A5700, ped))
            {
                uint32& guard = Core::StaticRef<uint32>(0x1667C3C);
                if ((guard & 1) == 0)
                {
                    guard |= 1;
                    Core::Hook::CallMethod<uint32*>(0x435E10, &Core::StaticRef<uint32>(0x1667C38), Core::Rebase(0xD91CDC));
                }
                soundHash = Core::StaticRef<uint32>(0x1667C38);
            }
        }

        AudSoundInitParams params;
        InitSoundParams(params);
        *reinterpret_cast<void**>(params.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<uint32*>(params.data + 0x14) = posPtr;
        *reinterpret_cast<float*>(params.data + 0x00) = Core::Hook::Call<float>(0x794280, finalVol);

        void* mem = Core::Hook::Call<void*>(0x799A90);
        void* speechObj = Core::Hook::Call<void*>(0x799CC0, mem);
        if (Core::Hook::CallMethod<bool>(0xCE8930, self, soundHash, &params, mem, speechObj, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x45 };
            Core::Hook::Call<void>(0x7A9810, soundHash, 0, 0, 1, &params, &sp, *reinterpret_cast<void**>(self + 0x120), mem);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem);
        }

        const float extraVol = *reinterpret_cast<float*>(self + 0x144) * vol;
        if (extraVol <= Core::StaticRef<float>(0xD90AA8))
            return;

        AudSoundInitParams p2;
        InitSoundParams(p2);
        *reinterpret_cast<void**>(p2.data + 0x20) = *reinterpret_cast<void**>(self + 0x8);
        *reinterpret_cast<uint32*>(p2.data + 0x14) = posPtr;
        *reinterpret_cast<float*>(p2.data + 0x00) = Core::Hook::Call<float>(0x794280, extraVol);

        void* mem2 = Core::Hook::Call<void*>(0x799A90);
        void* speechObj2 = Core::Hook::Call<void*>(0x799CC0, mem2);
        if (Core::Hook::CallMethod<bool>(0xCE8E70, self, Core::Rebase(0xD91CEC), &p2, mem2, speechObj2, 0))
        {
            AudSpeechParams sp{ 0, -1, 0x46 };
            uint32 sndHash = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(0xD91CFC), 0);
            Core::Hook::Call<void>(0x7A9810, sndHash, 0, 0, 1, &p2, &sp, *reinterpret_cast<void**>(self + 0x120), mem2);
        }
        else
        {
            Core::Hook::Call<void>(0x799B20, mem2);
        }
    }

    // 0xAB24C0
    void __cdecl InitPedAudioClass()
    {
        const uintptr_t footKeys[7] = { 0xD90C38, 0xD90C48, 0xD90C5C, 0xD90C70, 0xD90C80, 0xD90C90, 0xD90CA0 };
        uint32* footDst = &Core::StaticRef<uint32>(0x1667844);
        for (int i = 0; i < 7; ++i)
        {
            uint32 h = Core::Hook::Call<uint32>(0x5B1C30, Core::Rebase(footKeys[i]), 0);
            footDst[i] = Core::Hook::CallMethod<uint32>(0xCDC1B0, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), h);
        }

        void* animMgr = reinterpret_cast<void*>(Core::Rebase(0x1667A98));
        Core::Hook::CallMethod<void>(0x842290, animMgr, Core::Rebase(0xAAF4A0));

        struct AnimCbEntry { uintptr_t nameAddr; uintptr_t fnAddr; };
        const AnimCbEntry callbacks[] = {
            { 0xD90CB4, 0xAB18F0 }, { 0xD90CC0, 0xAB1910 }, { 0xD90CCC, 0xAB1930 },
            { 0xD90CD8, 0xAB1970 }, { 0xD90CE4, 0xAB1990 }, { 0xD90CF0, 0xAB19B0 },
            { 0xD90CFC, 0xAB1950 }, { 0xD90D0C, 0xAB19D0 }, { 0xD90D1C, 0xAB19F0 },
            { 0xD90D28, 0xAB1A10 }, { 0xD90D34, 0xAB1A30 }, { 0xD90D50, 0xAB1A50 },
            { 0xD90D70, 0xAB1A70 }, { 0xD90D8C, 0xAB1A90 }, { 0xD90DAC, 0xAB1AB0 },
            { 0xD90DC8, 0xAB1AD0 }, { 0xD90DE4, 0xAB1AF0 }, { 0xD90E04, 0xAB1B10 },
            { 0xD90E24, 0xAB1B30 }, { 0xD90E44, 0xAB1B30 }, { 0xD90E64, 0xAB1B30 },
            { 0xD90E84, 0xAB1B30 }, { 0xD90EA4, 0xAB1B30 }, { 0xD90EC4, 0xAAF6B0 },
            { 0xD90ED8, 0xAAF6B0 }, { 0xD90EF4, 0xAAF500 }, { 0xD90F0C, 0xAAF5D0 },
            { 0xD90F24, 0xAAF600 }, { 0xD90F40, 0xAAC790 }, { 0xD90F50, 0xAAC7C0 },
            { 0xD90F6C, 0xAAC7F0 }, { 0xD90F80, 0xAAC820 }, { 0xD90F90, 0xAAC8A0 },
            { 0xD90F9C, 0xAAC8D0 }, { 0xD90FB4, 0xAAC920 }, { 0xD90FCC, 0xAAC970 },
            { 0xD90FE4, 0xAAC9C0 }, { 0xD90FF0, 0xAAF640 }, { 0xD9100C, 0xAACA10 },
            { 0xD91018, 0xAACA60 }, { 0xD91024, 0xAAF680 }, { 0xD91038, 0xAAF4C0 },
            { 0xD91058, 0xAAC760 },
        };
        for (const auto& cb : callbacks)
            Core::Hook::CallMethod<void>(0xCCF340, animMgr, Core::Rebase(cb.nameAddr), Core::Rebase(cb.fnAddr));

        Core::StaticRef<uint32>(0x16677A8) = Core::Hook::Call<uint32>(0x5B1C30, Core::StaticRef<uintptr_t>(0xF3801C), 0);
        Core::StaticRef<uint32>(0x16677F0) = Core::Hook::Call<uint32>(0x5B1C30, Core::StaticRef<uintptr_t>(0xF38020), 0);

        struct CurveInit { uintptr_t curveAddr; uintptr_t nameAddr; };
        const CurveInit curves1[] = {
            { 0x16678B8, 0xD9106C }, { 0x1667890, 0xD9108C },
            { 0x166796C, 0xD910AC }, { 0x16678EC, 0xD910D4 },
        };
        for (const auto& c : curves1)
            Core::Hook::CallMethod<void>(0xCE4CF0, reinterpret_cast<void*>(Core::Rebase(c.curveAddr)), Core::Rebase(c.nameAddr));

        Core::StaticRef<uint32>(0x16677AC) = Core::Hook::Call<uint32>(0xCDF870, Core::Rebase(0xD910FC));

        const uint32* table1 = reinterpret_cast<const uint32*>(Core::Rebase(0xF380CC));
        uint32* dst1 = &Core::StaticRef<uint32>(0x16677F8);
        for (uint32 i = 0; i < 0x10; ++i)
        {
            const uint32 arg = (i <= 6) ? table1[i] : Core::Rebase(0xD91194);
            dst1[i] = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg);
        }

        const uint32* table2 = reinterpret_cast<const uint32*>(Core::Rebase(0xF38098));
        uint32* dst2 = &Core::StaticRef<uint32>(0x16677B0);
        for (uint32 i = 0; i < 0x10; ++i)
        {
            const uint32 arg = (i <= 0xC) ? table2[i] : Core::Rebase(0xD91280);
            dst2[i] = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), arg);
        }

        Core::Hook::CallMethod<void>(0xCE4CF0, reinterpret_cast<void*>(Core::Rebase(0x1667940)), Core::Rebase(0xD91290));
        Core::Hook::CallMethod<void>(0xCE4CF0, reinterpret_cast<void*>(Core::Rebase(0x1667AB4)), Core::Rebase(0xD912A8));
        Core::StaticRef<uint32>(0x166783C) = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), Core::Rebase(0xD912C0));
        Core::StaticRef<uint32>(0x1667840) = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), Core::Rebase(0xD912D0));

        const CurveInit curves2[] = {
            { 0x16679E4, 0xD912E8 }, { 0x1667A70, 0xD91308 }, { 0x1667A44, 0xD9132C },
            { 0x1667868, 0xD91348 }, { 0x16679BC, 0xD91364 }, { 0x1667A10, 0xD91380 },
            { 0x1667994, 0xD9139C }, { 0x1667914, 0xD913B8 },
        };
        for (const auto& c : curves2)
            Core::Hook::CallMethod<void>(0xCE4CF0, reinterpret_cast<void*>(Core::Rebase(c.curveAddr)), Core::Rebase(c.nameAddr));

        Core::StaticRef<uint32>(0x1667860) = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), Core::Rebase(0xD913DC));
        Core::StaticRef<uint32>(0x1667864) = Core::Hook::CallMethod<uint32>(0xCDB620, reinterpret_cast<void*>(Core::Rebase(0x1716CA0)), Core::Rebase(0xD913EC));
    }

    // 0xAB2AD0
    void __fastcall AudSpeechDtorBase(uint8* self, void* /*edx*/)
    {
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD91FAC);
        uint8** refSlot = reinterpret_cast<uint8**>(self + 0x14);
        if (*refSlot != nullptr)
            Core::Hook::CallMethod<void>(0x9E6580, *refSlot, refSlot);
        Core::Hook::CallMethod<void>(0xA28B00, self);
    }

    // 0xAB2B00
    int32 __cdecl GetUnk39D()
    {
        return 0x39D;
    }

    // 0xAB2B10
    void __fastcall UpdatePedSpeechPitchAndForward(uint8* self, void* /*edx*/, uint8* ped)
    {
        if (*reinterpret_cast<float*>(self + 0x50) < 0.0f)
        {
            const int32 pedType = *reinterpret_cast<int32*>(ped + 0xB90);
            float val = Core::StaticRef<float>(0xDCD0EC);
            if (pedType == 1)
                val = 0.0f;
            else if (pedType == 2)
                val = Core::StaticRef<float>(0xD50398);
            else if (pedType == 3)
                val = Core::StaticRef<float>(0xDCB2A4);
            *reinterpret_cast<float*>(self + 0x50) = val;
        }
        using VFn18 = void(__thiscall*)(void*, void*);
        reinterpret_cast<VFn18>((*reinterpret_cast<void***>(self))[0x48 / 4])(self, ped);
    }

    // 0xAB2B60
    void __fastcall UpdateSpeechIntensity(uint8* self, void* /*edx*/, uint32 /*unused*/)
    {
        uint32& initFlags = Core::StaticRef<uint32>(0x1667C5C);
        if ((initFlags & 0x01) == 0) { initFlags |= 0x01; Core::StaticRef<float>(0x1667C58) = Core::StaticRef<float>(0xD675BC); }
        if ((initFlags & 0x02) == 0) { initFlags |= 0x02; Core::StaticRef<float>(0x1667C54) = Core::StaticRef<float>(0xD50398); }
        if ((initFlags & 0x04) == 0) { initFlags |= 0x04; Core::StaticRef<float>(0x1667C50) = Core::StaticRef<float>(0xD5A154); }
        if ((initFlags & 0x08) == 0) { initFlags |= 0x08; Core::StaticRef<float>(0x1667C4C) = Core::StaticRef<float>(0xD50964); }
        if ((initFlags & 0x10) == 0) { initFlags |= 0x10; Core::StaticRef<float>(0x1667C48) = Core::StaticRef<float>(0xEBE2C0); }

        const float minVal = Core::StaticRef<float>(0x1667C58);
        const float one = Core::StaticRef<float>(0xD50398);
        const float step = Core::StaticRef<float>(0xDCB2A8);
        const float cur70 = *reinterpret_cast<float*>(self + 0x70);

        float val54 = 0.0f;
        if (minVal > cur70)
        {
            val54 = 0.0f;
        }
        else if (cur70 > Core::StaticRef<float>(0x1667C50))
        {
            val54 = one;
        }
        else if (Core::StaticRef<float>(0x1667C54) > cur70)
        {
            val54 = (std::sqrt(cur70) - step) * Core::StaticRef<float>(0x1667C48) * Core::StaticRef<float>(0xDCE03C);
        }
        else
        {
            const float half = Core::StaticRef<float>(0xDCE03C);
            val54 = (std::sqrt(cur70) - one) * Core::StaticRef<float>(0x1667C4C) * half + half;
        }

        val54 = std::sqrt(val54) * Core::StaticRef<float>(0xDCD0EC);
        if (self[0x30] == 0 && val54 > Core::StaticRef<float>(0xDCB2A4))
            val54 = Core::StaticRef<float>(0xDCB2A4);
        *reinterpret_cast<float*>(self + 0x54) = val54;

        const float cur50 = *reinterpret_cast<float*>(self + 0x50);
        *reinterpret_cast<float*>(self + 0x50) = (val54 - cur50 > step) ? (cur50 + step) : val54;
    }

    // 0xAB2CD0
    void* __fastcall AudSpeechObj2_Ctor(uint8* self, void* /*edx*/, void* entity)
    {
        Core::Hook::CallMethod<void>(0xA28AF0, self);
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD92004);
        *reinterpret_cast<void**>(self + 0x14) = entity;
        *reinterpret_cast<float*>(self + 0x18) = 0.0f;
        *reinterpret_cast<uint32*>(self + 0x1C) = 0;
        *reinterpret_cast<float*>(self + 0x20) = 0.0f;
        *reinterpret_cast<float*>(self + 0x24) = 0.0f;
        *reinterpret_cast<float*>(self + 0x28) = 0.0f;
        self[0x30] = 0;
        Core::StaticRef<int32>(0x1667C40) += 1;
        void** refSlot = reinterpret_cast<void**>(self + 0x14);
        if (*refSlot != nullptr)
            Core::Hook::CallMethod<void>(0x9E6550, *refSlot, refSlot);
        return self;
    }

    // 0xAB2D20
    int32 __cdecl GetUnk4BC()
    {
        return 0x4BC;
    }

    // 0xAB2D30
    void __fastcall AudSpeechObj2_Dtor(uint8* self, void* /*edx*/)
    {
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD92004);
        Core::StaticRef<int32>(0x1667C40) -= 1;
        void** refSlot = reinterpret_cast<void**>(self + 0x14);
        if (*refSlot != nullptr)
        {
            Core::Hook::CallMethod<void>(0x9E6580, *refSlot, refSlot);
            *refSlot = nullptr;
        }
        Core::Hook::CallMethod<void>(0xA28B00, self);
    }

    // 0xAB2D60
    float __fastcall GetUnkFloatEBDED4(uint8* /*self*/, void* /*edx*/, uint32 /*unused*/)
    {
        return Core::StaticRef<float>(0xEBDED4);
    }

    // 0xAB2D70
    void* __fastcall AudSpeechObj1_Ctor(uint8* self, void* /*edx*/, void* entity, const AudVec4* vec, uint8 flag30, uint32 val34, float val38, float val3C, uint8 flag40)
    {
        Core::Hook::CallMethod<void>(0xA28AF0, self);
        *reinterpret_cast<uintptr_t*>(self) = Core::Rebase(0xD91FAC);
        *reinterpret_cast<uint32*>(self + 0x44) = 0;
        *reinterpret_cast<uint32*>(self + 0x48) = 0;
        self[0x4C] = 0;
        self[0x4D] = 0;
        *reinterpret_cast<void**>(self + 0x14) = entity;
        *reinterpret_cast<AudVec4*>(self + 0x20) = *vec;
        self[0x30] = flag30;
        *reinterpret_cast<uint32*>(self + 0x34) = val34;
        *reinterpret_cast<float*>(self + 0x38) = val38;
        *reinterpret_cast<float*>(self + 0x3C) = val3C;
        self[0x40] = flag40;
        void** refSlot = reinterpret_cast<void**>(self + 0x14);
        Core::Hook::CallMethod<void>(0x9E6550, *refSlot, refSlot);
        *reinterpret_cast<float*>(self + 0x50) = Core::StaticRef<float>(0xEBDED4);
        return self;
    }
}

void CPedAudio::InjectHooks()
{
    Core::Hook::Install(0xAAD000, &PlaySoundByHash);
    Core::Hook::Install(0xAAD0F0, &UpdatePlayerPedAudio);
    Core::Hook::Install(0xAAD110, &AudPedAudioEntity_Ctor);
    Core::Hook::Install(0xAAD200, &AudPedAudioEntity_GetPed);
    Core::Hook::Install(0xAAD210, &AudPedAudioEntity_Dtor);
    Core::Hook::Install(0xAAD2A0, &PlayUnk130Sound);
    Core::Hook::Install(0xAAD430, &StopUnk130Sound);
    Core::Hook::Install(0xAAD480, &PlayUnk134Sound);
    Core::Hook::Install(0xAAD580, &StopUnk134Sound);
    Core::Hook::Install(0xAAD5D0, &RandomizePedAudioParams);
    Core::Hook::Install(0xAAD770, &PlayClothOrGearSound);
    Core::Hook::Install(0xAAD910, &PlayUnk19CSound);
    Core::Hook::Install(0xAADA20, &GetFootstepPitchAndVolume);
    // Skipped 0xAADB50 (ComputeAmbientAudioScale): mismatched body in current decomp
    Core::Hook::Install(0xAADC90, &GetBoneOrPedPos);
    Core::Hook::Install(0xAADD50, &PlayCollisionSound);
    Core::Hook::Install(0xAADF70, &PlayMeleeHitSound);
    Core::Hook::Install(0xAAE120, &StopUnk14Sound);
    Core::Hook::Install(0xAAE170, &PlayScriptedSpeechWithAnim);
    Core::Hook::Install(0xAAE310, &StopUnk1CSound);
    Core::Hook::Install(0xAAE360, &SetFootstepMaterial);
    Core::Hook::Install(0xAAE4B0, &SetFootstepMaterialDirect);
    Core::Hook::Install(0xAAE560, &UpdateLimbSpeeds);
    Core::Hook::Install(0xAAE7C0, &GetBoneWorldPos);
    Core::Hook::Install(0xAAE870, &PlayUnk16CSound);
    Core::Hook::Install(0xAAE9A0, &PlayBodyCollisionSounds);
    Core::Hook::Install(0xAAEC80, &PlayWaterSplashSounds);
    Core::Hook::Install(0xAAEE40, &PlayWeaponOrMeleeSwingSounds);
    Core::Hook::Install(0xAAF1D0, &TryPlayScriptedPainSound);
    Core::Hook::Install(0xAAF330, &AccumulateUnk40And144);
    Core::Hook::Install(0xAAF390, &PlayUnk140Sound);
    Core::Hook::Install(0xAAF4A0, &AnimEvent_PlaySoundByHash);
    Core::Hook::Install(0xAAF4C0, &AnimEvent_PlaySoundAndVehicle);
    Core::Hook::Install(0xAAF500, &AnimEvent_PlayParachuteOrWeaponSound);
    Core::Hook::Install(0xAAF5D0, &AnimEvent_PlayWeapon22Sound);
    Core::Hook::Install(0xAAF600, &AnimEvent_PlayWeapon26And2ASounds);
    Core::Hook::Install(0xAAF640, &AnimEvent_PlaySoundAndReportCrime);
    Core::Hook::Install(0xAAF680, &AnimEvent_PlaySoundIfFemale);
    Core::Hook::Install(0xAAF6B0, &AnimEvent_PlayMeleeSwing);
    Core::Hook::Install(0xAAF6D0, &AudPedAudioEntity_ScalarDeletingDtor);
    Core::Hook::Install(0xAAF6F0, &UpdatePedAudio);
    Core::Hook::Install(0xAB0300, &PlayMeleeHitSound2);
    Core::Hook::Install(0xAB0540, &PlayWaterDiveOrSplashSound);
    Core::Hook::Install(0xAB06E0, &PlayFootstepSound);
    Core::Hook::Install(0xAB11E0, &PlayScriptedSpeech);
    Core::Hook::Install(0xAB1390, &PlayMeleeHitSound3);
    Core::Hook::Install(0xAB14C0, &UpdateSoundBonePos);
    Core::Hook::Install(0xAB1530, &UpdateActiveSpeechSound);
    Core::Hook::Install(0xAB1780, &PlayVehicleHitSound);
    Core::Hook::Install(0xAB18F0, &AnimEvent_Step_0_1);
    Core::Hook::Install(0xAB1910, &AnimEvent_Step_2_1);
    Core::Hook::Install(0xAB1930, &AnimEvent_Step_4_1);
    Core::Hook::Install(0xAB1950, &AnimEvent_Step_10_1);
    Core::Hook::Install(0xAB1970, &AnimEvent_Step_1_1);
    Core::Hook::Install(0xAB1990, &AnimEvent_Step_3_1);
    Core::Hook::Install(0xAB19B0, &AnimEvent_Step_5_1);
    Core::Hook::Install(0xAB19D0, &AnimEvent_Step_11_1);
    Core::Hook::Install(0xAB19F0, &AnimEvent_Step_7_0);
    Core::Hook::Install(0xAB1A10, &AnimEvent_Step_6_0);
    Core::Hook::Install(0xAB1A30, &AnimEvent_Step_8_0);
    Core::Hook::Install(0xAB1A50, &AnimEvent_Step_A_0);
    Core::Hook::Install(0xAB1A70, &AnimEvent_Step_9_0);
    Core::Hook::Install(0xAB1A90, &AnimEvent_Step_B_0);
    Core::Hook::Install(0xAB1AB0, &AnimEvent_Step_C_0);
    Core::Hook::Install(0xAB1AD0, &AnimEvent_Step_D_0);
    Core::Hook::Install(0xAB1AF0, &AnimEvent_Step_E_0);
    Core::Hook::Install(0xAB1B10, &AnimEvent_Step_F_0);
    Core::Hook::Install(0xAB1B30, &AnimEvent_LadderOrClimbStep);
    Core::Hook::Install(0xAB1F00, &UpdatePedAudioFrame);
    Core::Hook::Install(0xAB21A0, &PlayPedImpactSound);
    Core::Hook::Install(0xAB24C0, &InitPedAudioClass);
    Core::Hook::Install(0xAB2AD0, &AudSpeechDtorBase);
    Core::Hook::Install(0xAB2B00, &GetUnk39D);
    Core::Hook::Install(0xAB2B10, &UpdatePedSpeechPitchAndForward);
    Core::Hook::Install(0xAB2B60, &UpdateSpeechIntensity);
    Core::Hook::Install(0xAB2CD0, &AudSpeechObj2_Ctor);
    Core::Hook::Install(0xAB2D20, &GetUnk4BC);
    Core::Hook::Install(0xAB2D30, &AudSpeechObj2_Dtor);
    Core::Hook::Install(0xAB2D60, &GetUnkFloatEBDED4);
    Core::Hook::Install(0xAB2D70, &AudSpeechObj1_Ctor);
}
