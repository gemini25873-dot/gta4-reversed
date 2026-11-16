#include "StdInc.h"
#include "Rage/phConstraint.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include <cmath>
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
}

namespace DiscreteCollisionDetectorInterface
{
    void Result::InjectHooks()
    {
        Core::Hook::Install(0x6E1CC0, &Result::DeletingDtor);
    }

    // 0x6E1CC0 (retn 4)
    Result* __thiscall Result::DeletingDtor(uint8 /*flags*/)
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC65C));
        return this;
    }
}

namespace rage
{
    void phManifoldResult::InjectHooks()
    {
        DiscreteCollisionDetectorInterface::Result::InjectHooks();
    }

    void phConstraint::InjectHooks()
    {
        Core::Hook::Install(0x6F8640, &phConstraint::DeletingDtor);
        Core::Hook::Install(0x6F9EB0, &phConstraint::ComputeNormalAndError);

        phManifoldResult::InjectHooks();
        phRopeConstraint::InjectHooks();
        phRope::InjectHooks();

        Core::Log::Info("rage::phConstraint hooks installed (5 functions)");
    }

    // 0x6F8640 (retn 4)
    phConstraint* __thiscall phConstraint::DeletingDtor(uint8 flags)
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC778));
        if (flags & 1)
        {
            RageFree(this);
        }
        return this;
    }

    // 0x6F9EB0 (retn 10h)
    double __thiscall phConstraint::ComputeNormalAndError(const CVector* pointA, const CVector* pointB, CVector* outNormalAB, CVector* outNormalBA)
    {
        const float dx = pointB->x - pointA->x;
        const float dy = pointB->y - pointA->y;
        const float dz = pointB->z - pointA->z;

        outNormalAB->x = dx;
        outNormalAB->y = dy;
        outNormalAB->z = dz;

        const float distSq = (dx * dx) + (dy * dy) + (dz * dz);
        float dist = 0.0f;
        if (distSq <= 1.0e-12f)
        {
            dist = 0.0f;
            const auto& fallback = Core::StaticRef<CVector>(0x108A320);
            outNormalAB->x = fallback.x;
            outNormalAB->y = fallback.y;
            outNormalAB->z = fallback.z;
        }
        else
        {
            dist = std::sqrt(distSq);
            const float invDist = 1.0f / dist;
            outNormalAB->x = dx * invDist;
            outNormalAB->y = dy * invDist;
            outNormalAB->z = dz * invDist;
        }

        outNormalBA->x = -outNormalAB->x;
        outNormalBA->y = -outNormalAB->y;
        outNormalBA->z = -outNormalAB->z;

        return static_cast<double>(dist - m_fLength);
    }

    void phRopeConstraint::InjectHooks()
    {
        Core::Hook::Install(0x70DF90, &phRopeConstraint::DeletingDtor);
    }

    // 0x70DF90 (retn 4)
    phRopeConstraint* __thiscall phRopeConstraint::DeletingDtor(uint8 flags)
    {
        m_pRopeContactVftable = reinterpret_cast<void**>(Core::Rebase(0x00EBCA64));
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC778));
        if (flags & 1)
        {
            RageFree(this);
        }
        return this;
    }

    void phRope::InjectHooks()
    {
        Core::Hook::Install(0x70DFE0, &phRope::Dtor);
    }

    // 0x70DFE0 (retn)
    void __thiscall phRope::Dtor()
    {
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBCA38));
        if (m_pData0C)
        {
            RageFree(m_pData0C);
        }
        if (m_pData08)
        {
            RageFree(m_pData08);
        }
        m_constraint.m_pRopeContactVftable = reinterpret_cast<void**>(Core::Rebase(0x00EBCA64));
        m_constraint.m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00EBC778));
        m_vftable = reinterpret_cast<void**>(Core::Rebase(0x00D796BC));
    }
}
