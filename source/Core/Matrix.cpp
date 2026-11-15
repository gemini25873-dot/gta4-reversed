#include "StdInc.h"
#include "Core/Matrix.h"
#include "Core/Hook.h"
#include "Core/Log.h"

void CMatrix::InjectHooks()
{
    Core::Hook::Install(0x42F680, &CMatrix::Multiply);

    Core::Log::Info("CMatrix hooks yuklendi (1 fonksiyon)");
}

const CMatrix* __thiscall CMatrix::Multiply(const CMatrix* a, const CMatrix* b)
{
    right.x = a->right.x * b->right.x + a->right.y * b->up.x + a->right.z * b->at.x;
    right.y = a->right.y * b->up.y + b->right.y * a->right.x + b->at.y * a->right.z;
    right.z = a->right.y * b->up.z + b->right.z * a->right.x + b->at.z * a->right.z;

    up.x = a->up.z * b->at.x + a->up.y * b->up.x + a->up.x * b->right.x;
    up.y = a->up.z * b->at.y + a->up.y * b->up.y + a->up.x * b->right.y;
    up.z = a->up.z * b->at.z + a->up.y * b->up.z + a->up.x * b->right.z;

    at.x = a->at.x * b->right.x + a->at.z * b->at.x + a->at.y * b->up.x;
    at.y = b->at.y * a->at.z + b->up.y * a->at.y + b->right.y * a->at.x;
    at.z = b->at.z * a->at.z + b->up.z * a->at.y + b->right.z * a->at.x;

    pos.x = (a->pos.z * b->at.x + a->pos.y * b->up.x + a->pos.x * b->right.x) + b->pos.x;
    pos.y = (a->pos.z * b->at.y + a->pos.y * b->up.y + a->pos.x * b->right.y) + b->pos.y;
    pos.z = (a->pos.z * b->at.z + a->pos.y * b->up.z + a->pos.x * b->right.z) + b->pos.z;

    return b;
}
