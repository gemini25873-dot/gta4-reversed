#include "StdInc.h"
#include "Rage/phArchetype.h"
#include "Core/Hook.h"
#include "Core/Log.h"

namespace
{
    inline void ReleaseBoundRef(rage::phBound* bound)
    {
        if (!bound)
            return;

        auto& refCountEnabled = Core::StaticRef<uint8>(0x18A87F6);
        if (!refCountEnabled)
            return;

        auto* pRefCount = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(bound) + 0x7C);
        --(*pRefCount);
        if (refCountEnabled && *pRefCount == 0)
        {
            void** vftable = *reinterpret_cast<void***>(bound);
            using DeletingDtorFn = void* (__thiscall*)(rage::phBound*, uint8);
            reinterpret_cast<DeletingDtorFn>(vftable[0])(bound, 1);
        }
    }

    inline void AddBoundRef(rage::phBound* bound)
    {
        if (!bound)
            return;

        auto& refCountEnabled = Core::StaticRef<uint8>(0x18A87F6);
        if (refCountEnabled)
        {
            auto* pRefCount = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(bound) + 0x7C);
            ++(*pRefCount);
        }
    }

    inline void InitPhysDefaults(rage::phArchetypePhys* self)
    {
        self->m_fMass          = 1.0f;
        self->m_fInvMass       = 1.0f;
        self->m_fGravityFactor = 1.0f;
        self->m_fMaxSpeed      = 80.0f;
        self->m_fMaxAngSpeed   = 6.2831854820251465f;
        self->m_vAngInertia    = CVector(1.0f, 1.0f, 1.0f);
        self->m_vInvAngInertia = CVector(1.0f, 1.0f, 1.0f);
    }
}

namespace rage
{
    void phArchetype::InjectHooks()
    {
        Core::Hook::Install(0x6BF150, &phArchetypeBase::DeletingDtor);

        Core::Hook::Install(0x6BF430, &phArchetype::Dtor);
        Core::Hook::Install(0x6BFE70, &phArchetype::DeletingDtor);
        Core::Hook::Install(0x6BF3E0, &phArchetype::Shutdown);
        Core::Hook::Install(0x6BF7C0, &phArchetype::SetBound);

        // 0x6BF810 (phArchetypePhys::Ctor) uses LTCG __usercall (this in ESI, not ECX)
        Core::Hook::Install(0x6BF250, &phArchetypePhys::GetMass);
        Core::Hook::Install(0x6BF260, &phArchetypePhys::GetInvMass);
        Core::Hook::Install(0x6BF290, &phArchetypePhys::GetGravityFactor);
        Core::Hook::Install(0x6BFBE0, &phArchetypePhys::SetGravityFactor);
        Core::Hook::Install(0x6BF2A0, &phArchetypePhys::GetMaxSpeed);
        Core::Hook::Install(0x6BF2B0, &phArchetypePhys::GetMaxAngSpeed);

        Core::Hook::Install(0x6BFDC0, &phArchetypeDamp::Ctor);

        Core::Hook::Install(0xCA7BE0, &phArchetypeGta::Ctor);
        Core::Hook::Install(0xCA7BD0, &phArchetypeGta::Dtor);
        Core::Hook::Install(0xCA7C00, &phArchetypeGta::DeletingDtor);

        Core::Log::Info("phArchetype hooks yuklendi (15 fonksiyon)");
    }

    // 0x6BF150 (retn 4)
    phArchetypeBase* __thiscall phArchetypeBase::DeletingDtor(uint8 flags)
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBC190));
        if (flags & 1)
        {
            Core::Hook::Call<void>(0x5B1C10, this);
        }
        return this;
    }

    // 0x6BF430 (retn)
    void __thiscall phArchetype::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBC19C));
        ReleaseBoundRef(m_pBound);
        m_pBound = nullptr;
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBC190));
    }

    // 0x6BFE70 (retn 4)
    phArchetype* __thiscall phArchetype::DeletingDtor(uint8 flags)
    {
        Dtor();
        if ((flags & 1) && this)
        {
            Core::Hook::Call<void>(0x5B1C10, this);
        }
        return this;
    }

    // 0x6BF3E0 (retn)
    void __thiscall phArchetype::Shutdown()
    {
        ReleaseBoundRef(m_pBound);
        m_pBound         = nullptr;
        m_nTypeFlags     = 1;
        m_nIncludeFlags  = 0xFFFFFFFFu;
        m_nPropertyFlags = Core::StaticRef<uint16>(0x18A87F8);
    }

    // 0x6BF7C0 (retn 4)
    void __thiscall phArchetype::SetBound(phBound* bound)
    {
        AddBoundRef(bound);
        ReleaseBoundRef(m_pBound);
        m_pBound = bound;
    }

    // 0x6BF810 (retn)
    phArchetypePhys* __thiscall phArchetypePhys::Ctor()
    {
        Core::Hook::CallMethod<phArchetype*>(0x6BF360, this);

        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBC1F4));
        m_nType   = ARCHETYPE_PHYS;
        InitPhysDefaults(this);

        return this;
    }

    // 0x6BF250 (retn)
    float __thiscall phArchetypePhys::GetMass() const
    {
        return m_fMass;
    }

    // 0x6BF260 (retn)
    float __thiscall phArchetypePhys::GetInvMass() const
    {
        return m_fInvMass;
    }

    // 0x6BF290 (retn)
    float __thiscall phArchetypePhys::GetGravityFactor() const
    {
        return m_fGravityFactor;
    }

    // 0x6BFBE0 (retn 4)
    void __thiscall phArchetypePhys::SetGravityFactor(float factor)
    {
        m_fGravityFactor = factor;
    }

    // 0x6BF2A0 (retn)
    float __thiscall phArchetypePhys::GetMaxSpeed() const
    {
        return m_fMaxSpeed;
    }

    // 0x6BF2B0 (retn)
    float __thiscall phArchetypePhys::GetMaxAngSpeed() const
    {
        return m_fMaxAngSpeed;
    }

    // 0x6BFDC0 (retn)
    phArchetypeDamp* __thiscall phArchetypeDamp::Ctor()
    {
        Core::Hook::CallMethod<phArchetype*>(0x6BF360, this);

        m_vftable = reinterpret_cast<void**>(Core::Rebase(0xEBC24C));
        m_nType   = ARCHETYPE_DAMP;
        InitPhysDefaults(this);

        std::memset(m_avDamping, 0, sizeof(m_avDamping));
        std::memset(m_abDampingActive, 0, sizeof(m_abDampingActive));
        std::memset(m_nDampPadB6, 0, sizeof(m_nDampPadB6));

        return this;
    }
}

// 0xCA7BE0 (retn)
phArchetypeGta* __thiscall phArchetypeGta::Ctor()
{
    Core::Hook::CallMethod<rage::phArchetypeDamp*>(0x6BFDC0, this);
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDC9EBC));
    m_nType   = ARCHETYPE_GTA;
    return this;
}

// 0xCA7BD0 (retn)
void __thiscall phArchetypeGta::Dtor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xDC9EBC));
    rage::phArchetype::Dtor();
}

// 0xCA7C00 (retn 4)
phArchetypeGta* __thiscall phArchetypeGta::DeletingDtor(uint8 flags)
{
    phArchetypeGta::Dtor();
    if (flags & 1)
    {
        Core::Hook::Call<void>(0x5B1C10, this);
    }
    return this;
}
