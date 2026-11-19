#include "StdInc.h"
#include "Game/Cam.h"
#include "Core/Hook.h"
#include "Core/Log.h"
#include "Core/Pool.h"
#include <xmmintrin.h>

void CCam::InjectHooks()
{
    Core::Hook::Install(0x9AC880, &CCam::InitFields);
    Core::Hook::Install(0x9ACC20, &CCam::FindCamOfType);
    Core::Hook::Install(0x9ACF10, &CCam::ClearUnk128);
    Core::Hook::Install(0x9ACF50, &CCam::SyncBlend);
    Core::Hook::Install(0x9AD000, &CCam::Ctor);
    Core::Hook::Install(0x9AD030, &CCam::Dtor);
    Core::Hook::Install(0x9AD190, &CCam::CleanupUnk12C);
    Core::Hook::Install(0x9AD1C0, &CCam::CleanupUnk110);
    Core::Hook::Install(0x9AD220, &CCam::AttachChild);
    Core::Hook::Install(0x9AD2D0, &CCam::GetCamOfType);
    Core::Hook::Install(0x9AD380, &CCam::UpdateChildren);
    Core::Hook::Install(0x9AD3B0, &CCam::UpdateNearFarClip);
    Core::Hook::Install(0x9AD480, &CCam::InitPool);
    Core::Hook::Install(0x9AD740, &CCam::ScalarDeletingDtor);

    // Remaining deferred CCam functions:
    // - 0x9AD060: sub_D1112B (__usercall@<xmm0>) and sub_9F03A0/sub_9F0710
    // - 0x9AD1F0: sub_9AC9D0/sub_9ACA80
    // - 0x9AD350: sub_91F510()
    // - 0x9AD4C0: sub_874640/sub_9F03A0/sub_BE0440 and 0x9AD060

    Core::Log::Info("CCam hooks installed (14 functions)");
}

// 0x9AD480
void* __cdecl CCam::InitPool()
{
    auto* pool = Core::Hook::Call<CPoolBase*>(0x4011D0, 28);
    if (pool)
    {
        pool->Init(60, reinterpret_cast<const char*>(Core::Rebase(0xD80410)), 1280);
        Core::StaticRef<CPoolBase*>(0x1618020) = pool;
    }
    else
    {
        Core::StaticRef<CPoolBase*>(0x1618020) = nullptr;
    }
    return Core::StaticRef<CPoolBase*>(0x1618020);
}

CCam* __thiscall CCam::Ctor()
{
    Core::Log::InfoOnce("cam_ctor", "CCam::Ctor executed");
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD80464));
    Core::Hook::CallMethod<void*>(0x9EFFE0, m_pad10);
    Core::Hook::CallMethod<void*>(0x9EFFE0, m_pad90);
    InitFields();
    return this;
}

// 0x9AC880
int32 __thiscall CCam::InitFields()
{
    uint8* flagsBytes = reinterpret_cast<uint8*>(&m_nFlags);
    const uint8 lowFlag = flagsBytes[0];
    flagsBytes[1] |= 3u;

    m_pChild = nullptr;
    m_pPrev = nullptr;
    m_pNext = nullptr;
    m_pParent = nullptr;
    m_pUnk128 = nullptr;
    m_pUnk110 = nullptr;
    m_pUnk12C = nullptr;
    m_nUnk130 = 0;
    m_nUnk13A = 0;
    m_nUnk114 = 0;
    m_nUnk13B = 0;
    m_nUnk138 = 250;
    m_nUnk134 = -1;
    flagsBytes[0] = (lowFlag & 0x40u) | 0x94u;
    return 1;
}

// 0x9AD030
int32 __thiscall CCam::Dtor()
{
    Core::Log::InfoOnce("cam_dtor", "CCam::Dtor executed");
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD80464));
    if (Core::StaticRef<uint8>(0xF2F054))
    {
        ClearUnk128();
    }
    return 0;
}

// 0x9AD740
CCam* __thiscall CCam::ScalarDeletingDtor(uint8 flags)
{
    Dtor();
    if ((flags & 1) != 0)
    {
        Core::StaticRef<CPoolBase*>(0x1618020)->Delete(this);
    }
    return this;
}

// 0x9ACF10
void __thiscall CCam::ClearUnk128()
{
    if (!m_pUnk128)
        return;

    void** vtable = *reinterpret_cast<void***>(m_pUnk128);
    using GetTypeFn = int32(__thiscall*)(void*);
    if (reinterpret_cast<GetTypeFn>(vtable[10])(m_pUnk128) == 5)
    {
        Core::Hook::CallMethod<void>(0x9AC7E0, m_pUnk128, 0);
        reinterpret_cast<uint8*>(&m_nFlags)[0] &= static_cast<uint8>(~0x20u);
        m_pUnk128 = nullptr;
    }
}

// 0x9ACF50
void __thiscall CCam::SyncBlend()
{
    if (m_nUnk130 && (reinterpret_cast<uint8*>(&m_nFlags)[0] & 1u) != 0 && m_pUnk110)
    {
        const uint32 val = *reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(m_pUnk110) + 0x274);
        Core::Hook::CallMethod<void>(0x9F03A0, m_pad10, val);
    }
}

int32 __thiscall CCam::CleanupUnk12C()
{
    int32 result = 0;
    if (m_pUnk12C)
    {
        Core::Hook::CallMethod<int32>(0xC4D1E0, m_pUnk12C);
        result = Core::Hook::Call<int32>(0x5B1C10, m_pUnk12C);
    }
    m_pUnk12C = nullptr;
    return result;
}

int32 __thiscall CCam::CleanupUnk110()
{
    int32 result = 0;
    if (m_pUnk110)
    {
        Core::Hook::CallMethod<int32>(0xBDFD50, m_pUnk110);
        result = Core::Hook::Call<int32>(0x5B1C10, m_pUnk110);
    }
    m_pUnk110 = nullptr;
    return result;
}

CCam* __thiscall CCam::AttachChild(CCam* child)
{
    CCam* currentChild = m_pChild;
    if (currentChild)
    {
        CCam* prev = currentChild->m_pPrev;
        CCam* parent = currentChild->m_pParent;
        if (prev)
            prev->m_pNext = child;
        currentChild->m_pPrev = child;
        child->m_pPrev = prev;
        child->m_pNext = currentChild;
        child->m_pParent = parent;
        if (parent->m_pChild == currentChild)
            parent->m_pChild = child;
        return child;
    }

    child->m_pParent = this;
    m_pChild = child;
    return this;
}

// 0x9ACC20
CCam* __thiscall CCam::FindCamOfType(int32 type, int32* indexPtr)
{
    using GetTypeFn = int32(__thiscall*)(CCam*);
    if (reinterpret_cast<GetTypeFn>(m_vftable[10])(this) == type && (*indexPtr)-- <= 0)
        return this;

    CCam* cur = m_pChild;
    if (!cur)
        return nullptr;

    const uint8 allowFlag2 = Core::StaticRef<uint8>(0x1618024);
    while (cur)
    {
        CCam* next = cur->m_pNext;
        _mm_prefetch(reinterpret_cast<const char*>(next), _MM_HINT_T0);
        CCam* found = cur->FindCamOfType(type, indexPtr);
        if (found && ((found->m_nFlags & 2u) == 0 || allowFlag2))
            return found;
        cur = next;
    }
    return nullptr;
}

// 0x9AD2D0
CCam* __thiscall CCam::GetCamOfType(int32 type, int32 index)
{
    using GetTypeFn = int32(__thiscall*)(CCam*);
    if (reinterpret_cast<GetTypeFn>(m_vftable[10])(this) == type && index-- <= 0)
        return this;

    CCam* cur = m_pChild;
    if (!cur)
        return nullptr;

    const uint8 allowFlag2 = Core::StaticRef<uint8>(0x1618024);
    while (cur)
    {
        CCam* next = cur->m_pNext;
        _mm_prefetch(reinterpret_cast<const char*>(next), _MM_HINT_T0);
        CCam* found = cur->FindCamOfType(type, &index);
        if (found && ((found->m_nFlags & 2u) == 0 || allowFlag2))
            return found;
        cur = next;
    }
    return nullptr;
}

int32 __thiscall CCam::UpdateChildren()
{
    int32 result = 0;
    for (CCam* child = m_pChild; child; child = child->m_pNext)
    {
        if ((static_cast<uint8>(child->m_nFlags) & 4) != 0)
        {
            child->SyncBlend();
            result = 1;
        }
    }
    return result;
}

void __thiscall CCam::UpdateNearFarClip()
{
    const float fovDelta = m_fFov - 45.0f;
    float nearClip = 0.1f;
    float farClip = 150.0f;

    if (m_nUnk80)
    {
        m_fNearClip = nearClip;
        m_fFarClip = farClip;
        return;
    }

    if (fovDelta <= 0.0f)
    {
        float t = fovDelta * 0.018518519f;
        if (t <= -1.0f)
            t = -1.0f;
        nearClip = 0.1f - (t * 20.0f);
        farClip = 150.0f - (t * 150.0f);
        m_fNearClip = nearClip;
        m_fFarClip = farClip;
        return;
    }

    float t = fovDelta * 0.014285714f;
    if (t >= 1.0f)
        t = 1.0f;
    m_fNearClip = 0.1f;
    m_fFarClip = 150.0f - (t * 75.0f);
}
