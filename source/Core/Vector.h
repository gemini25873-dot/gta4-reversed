#pragma once

class CVector
{
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr CVector() = default;
    constexpr CVector(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    static void InjectHooks();

    double __thiscall InvMagnitude();
    void __thiscall Normalise();

    float MagnitudeSquared() const { return x * x + y * y + z * z; }

    CVector operator+(const CVector& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    CVector operator-(const CVector& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    CVector operator*(float s) const { return {x * s, y * s, z * s}; }
};

VALIDATE_SIZE(CVector, 0xC);
