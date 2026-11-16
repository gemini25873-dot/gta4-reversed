#include "StdInc.h"
#include "Game/Cam.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CCam::InjectHooks()
{
    Core::Hook::Install(0x9AD000, &CCam::Ctor);
    Core::Hook::Install(0x9AD190, &CCam::CleanupUnk12C);
    Core::Hook::Install(0x9AD1C0, &CCam::CleanupUnk110);
    Core::Hook::Install(0x9AD220, &CCam::AttachChild);
    Core::Hook::Install(0x9AD380, &CCam::UpdateChildren);
    Core::Hook::Install(0x9AD3B0, &CCam::UpdateNearFarClip);

    // Skipped functions (due to unsafe / ambiguous IDA callees):
    // - 0x9AD030 (Dtor): sub_9ACF10() __thiscall argument (this) is ambiguous
    // - 0x9AD060: sub_D1112B (__usercall@<xmm0>) and sub_9F03A0/sub_9F0710 missing this argument
    // - 0x9AD1F0: sub_9AC9D0/sub_9ACA80 argument mismatch and uninitialized v2
    // - 0x9AD2D0: missing this argument in sub_9ACC20() __thiscall call
    // - 0x9AD350: missing this argument in sub_91F510() __thiscall call
    // - 0x9AD4C0: sub_874640/sub_9F03A0/sub_BE0440 missing this and 0x9AD060 (__usercall) call
    // - 0x9AD740 (ScalarDeletingDtor): sub_9AD030() and sub_C79660() missing arguments

    Core::Log::Info("CCam hooks installed (6/13 functions)");
}

CCam* __thiscall CCam::Ctor()
{
    m_vftable = reinterpret_cast<void**>(Core::Rebase(0xD80464));
    Core::Hook::CallMethod<void*>(0x9EFFE0, m_pad10);
    Core::Hook::CallMethod<void*>(0x9EFFE0, m_pad90);
    Core::Hook::CallMethod<int32>(0x9AC880, this);
    return this;
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

int32 __thiscall CCam::UpdateChildren()
{
    int32 result = 0;
    for (CCam* child = m_pChild; child; child = child->m_pNext)
    {
        if ((static_cast<uint8>(child->m_nFlags) & 4) != 0)
            result = Core::Hook::CallMethod<int32>(0x9ACF50, child);
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
