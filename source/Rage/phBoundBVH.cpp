#include "StdInc.h"
#include "Rage/phBoundBVH.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cstring>
#include <intrin.h>

namespace
{
    void RageFree(void* ptr)
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        void* allocator = tlsBlock[2];
        void** vtable = *reinterpret_cast<void***>(allocator);
        using FreeFn = void(__thiscall*)(void*, void*);
        reinterpret_cast<FreeFn>(vtable[3])(allocator, ptr);
    }

    void* GetRageFixupTable()
    {
        auto* tlsArray = reinterpret_cast<uintptr_t*>(__readfsdword(0x2C));
        auto* tlsBlock = reinterpret_cast<void**>(tlsArray[0]);
        return tlsBlock[1];
    }

    void FixupPtr(void* fixupTable, void*& ptr)
    {
        if (ptr)
        {
            const int32 delta = Core::Hook::CallMethod<int32>(0x43B540, fixupTable, reinterpret_cast<int32>(ptr));
            ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + delta);
        }
    }

    // 0x6C20B0: int __usercall@<eax>(int a1@<eax>)
    __declspec(noinline) void CallBoundPolyhedronInit(void* self)
    {
        const uintptr_t fn = Core::Rebase(0x6C20B0);
        __asm {
            mov eax, self
            call fn
        }
    }

    // 0x6C2110: phBoundPolyhedron::Fixup
    void FixupPolyhedron(rage::phBoundGeometry* self)
    {
        self->m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC3A4));
        self->m_nUnkBC = 0xFFFF;
        self->m_nUnkBE = 0xFFFF;
        self->m_nUnkC0 = 0;
        self->m_nUnkC4 = 0;

        void* fixupTable = GetRageFixupTable();
        FixupPtr(fixupTable, self->m_pUnk88);
        FixupPtr(fixupTable, self->m_pUnk8C);
        FixupPtr(fixupTable, self->m_pUnkB4);
        FixupPtr(fixupTable, self->m_pUnk84);
        FixupPtr(fixupTable, self->m_pUnkB0);

        if (self->m_nType == static_cast<uint8>(rage::eBoundType::BOUND_GEOMETRY))
        {
            const float margin = self->m_vecUnk70.x;
            if (margin < 0.025f)
            {
                const float delta = 0.025f - margin;
                const uint32 refCount = Core::StaticRef<uint8>(0x18A87F6) ? self->m_nRefCount : 1;

                self->m_vecUnk70 = { 0.025f, 0.025f, 0.025f };
                self->m_vecBoundingBoxMax.x += delta;
                self->m_vecBoundingBoxMax.y += delta;
                self->m_vecBoundingBoxMax.z += delta;
                self->m_vecBoundingBoxMin.x -= delta;
                self->m_vecBoundingBoxMin.y -= delta;
                self->m_vecBoundingBoxMin.z -= delta;
                self->m_nRefCount = refCount;
                self->m_fRadiusAroundCentroid += delta;
            }
        }
    }

    // 0x6B8C70: phBoundGeometry::Init
    void InitBoundGeometry(rage::phBoundGeometry* self)
    {
        CallBoundPolyhedronInit(self);
        self->m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC09C));
        self->m_nType = static_cast<uint8>(rage::eBoundType::BOUND_GEOMETRY);
        self->m_nUnkD8 = 0;
        self->m_nUnkC8 = 0;
        self->m_nUnkCC = 0;
        self->m_pUnk8C = nullptr;
        self->m_pUnkB0 = nullptr;
        self->m_pUnkD0 = nullptr;
        self->m_nUnkD4 = 0;
        self->m_pUnkB4 = nullptr;
        self->m_pUnk88 = nullptr;
        std::memset(self->m_padD9, 0, sizeof(self->m_padD9));
    }

    // 0x6C1720: datOwner<phBVH> fixup
    void FixupBVHData(void** bvhSlot)
    {
        void* fixupTable = GetRageFixupTable();
        if (fixupTable && *bvhSlot != nullptr &&
            Core::Hook::CallMethod<int32>(0x43B4C0, *reinterpret_cast<void**>(fixupTable), bvhSlot) != -1)
        {
            const int32 delta = Core::Hook::CallMethod<int32>(0x43B540, fixupTable, reinterpret_cast<int32>(*bvhSlot));
            auto* bvh = reinterpret_cast<int32*>(reinterpret_cast<uintptr_t>(*bvhSlot) + delta);
            *bvhSlot = bvh;
            if (bvh)
            {
                void* fixupTable2 = GetRageFixupTable();
                if (bvh[0])
                {
                    bvh[0] += Core::Hook::CallMethod<int32>(0x43B540, fixupTable2, bvh[0]);
                }
                if (bvh[20])
                {
                    bvh[20] += Core::Hook::CallMethod<int32>(0x43B540, fixupTable2, bvh[20]);
                }
            }
        }
        else
        {
            *bvhSlot = nullptr;
        }
    }

    // 0x748EE0: phBVH::Dtor
    void DestroyBVHData(int32* bvh)
    {
        if (bvh[20])
        {
            RageFree(reinterpret_cast<void*>(bvh[20]));
            bvh[20] = 0;
            reinterpret_cast<uint16*>(bvh)[42] = 0;
        }
        if (bvh[0])
        {
            RageFree(reinterpret_cast<void*>(bvh[0]));
        }
    }
}

namespace rage
{
    void phBoundGeometry::InjectHooks()
    {
        Core::Hook::Install(0x6B8D00, &phBoundGeometry::Dtor);
        Core::Hook::Install(0x6B8CD0, &phBoundGeometry::DeletingDtor);
        Core::Hook::Install(0x6B8FB0, &phBoundGeometry::Fixup);
        Core::Hook::Install(0x6B8C50, &phBoundGeometry::GetUnkD8);
        Core::Hook::Install(0x6BA400, &phBoundGeometry::ShiftCentroidOffset);

        Core::Log::Info("rage::phBoundGeometry hooks yuklendi (5 fonksiyon)");
    }

    // 0x6B8D00 (__thiscall, retn)
    void __thiscall phBoundGeometry::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC09C));

        if (m_pUnk8C)
            RageFree(m_pUnk8C);
        if (m_pUnkB0)
            RageFree(m_pUnkB0);
        if (m_pUnkD0)
            RageFree(m_pUnkD0);
        if (m_pUnk88)
            RageFree(m_pUnk88);
        if (m_pUnkB4)
            RageFree(m_pUnkB4);
        if (m_pUnk84)
            RageFree(m_pUnk84);

        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBB998));
    }

    // 0x6B8CD0 (__thiscall, retn 4)
    phBoundGeometry* __thiscall phBoundGeometry::DeletingDtor(uint8 flags)
    {
        Dtor();
        if ((flags & 1) != 0 && this)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x6B8FB0 (__thiscall, retn)
    phBoundGeometry* __thiscall phBoundGeometry::Fixup()
    {
        FixupPolyhedron(this);
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC09C));
        FixupPtr(GetRageFixupTable(), m_pUnkD0);
        return this;
    }

    // 0x6B8C50 (__thiscall, retn)
    int32 __thiscall phBoundGeometry::GetUnkD8() const
    {
        return m_nUnkD8;
    }

    // 0x6BA400 (__thiscall, retn 4)
    int32 __thiscall phBoundGeometry::ShiftCentroidOffset(const CVector* offset)
    {
        m_vecUnkA0.x += offset->x;
        m_vecUnkA0.y += offset->y;
        m_vecUnkA0.z += offset->z;

        m_vecBoundingBoxMin.x += offset->x;
        m_vecBoundingBoxMin.y += offset->y;
        m_vecBoundingBoxMin.z += offset->z;

        m_vecBoundingBoxMin.x += offset->x;
        m_vecBoundingBoxMin.y += offset->y;
        m_vecBoundingBoxMin.z += offset->z;

        m_vecCentroidOffset.x += offset->x;
        m_vecCentroidOffset.y += offset->y;
        m_vecCentroidOffset.z += offset->z;

        if (m_vecCentroidOffset.x == 0.0f && m_vecCentroidOffset.y == 0.0f && m_vecCentroidOffset.z == 0.0f)
            m_nFlags = 0;
        else
            m_nFlags = 1;

        using VFunc51Fn = int32(__thiscall*)(phBoundGeometry*);
        return reinterpret_cast<VFunc51Fn>(m_vftable[51])(this);
    }

    void phBoundBVH::InjectHooks()
    {
        Core::Hook::Install(0x6C04F0, &phBoundBVH::Ctor);
        Core::Hook::Install(0x6C05B0, &phBoundBVH::Dtor);
        Core::Hook::Install(0x6C0580, &phBoundBVH::DeletingDtor);

        Core::Log::Info("rage::phBoundBVH hooks yuklendi (3 fonksiyon)");
    }

    // 0x6C04F0 (__thiscall, retn)
    phBoundBVH* __thiscall phBoundBVH::Ctor()
    {
        InitBoundGeometry(this);
        m_nUnkB8 = 1;
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC2A4));
        FixupBVHData(&m_pBVH);
        m_nType = 10;
        m_pBVH = nullptr;
        m_nRefCount = Core::StaticRef<uint8>(0x18A87F6) ? m_nRefCount : 1;
        m_vecUnk70 = { 0.005f, 0.005f, 0.005f };
        m_nUnkE4[0] = 0;
        m_nUnkE4[1] = 0;
        m_nUnkEC = 0;
        return this;
    }

    // 0x6C05B0 (__thiscall, jmp sub_6B8D00)
    void __thiscall phBoundBVH::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC2A4));
        if (m_pBVH)
        {
            DestroyBVHData(reinterpret_cast<int32*>(m_pBVH));
            RageFree(m_pBVH);
            m_pBVH = nullptr;
        }
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBD054));
        phBoundGeometry::Dtor();
    }

    // 0x6C0580 (__thiscall, retn 4)
    phBoundBVH* __thiscall phBoundBVH::DeletingDtor(uint8 flags)
    {
        Dtor();
        if ((flags & 1) != 0 && this)
        {
            RageFree(this);
        }
        return this;
    }
}
