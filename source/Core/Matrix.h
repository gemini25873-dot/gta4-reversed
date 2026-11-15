#pragma once
#include "Core/Vector.h"

class CMatrix
{
public:
    CVector right{1.0f, 0.0f, 0.0f};
    float pad0 = 0.0f;
    CVector up{0.0f, 1.0f, 0.0f};
    float pad1 = 0.0f;
    CVector at{0.0f, 0.0f, 1.0f};
    float pad2 = 0.0f;
    CVector pos{0.0f, 0.0f, 0.0f};
    float pad3 = 0.0f;

    static void InjectHooks();

    const CMatrix* __thiscall Multiply(const CMatrix* a, const CMatrix* b);
};

VALIDATE_SIZE(CMatrix, 0x40);
VALIDATE_OFFSET(CMatrix, right, 0x00);
VALIDATE_OFFSET(CMatrix, up, 0x10);
VALIDATE_OFFSET(CMatrix, at, 0x20);
VALIDATE_OFFSET(CMatrix, pos, 0x30);
