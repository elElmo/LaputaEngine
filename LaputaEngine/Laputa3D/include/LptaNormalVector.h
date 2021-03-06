#ifndef _LPTANORMALVECTOR_H_
#define _LPTANORMALVECTOR_H_

#include "LptaVector.h"

namespace lpta_3d
{

class LptaNormalVector : public LptaVector
{
public:
    LptaNormalVector(void);
    LptaNormalVector(float x, float y, float z);

    inline static LptaNormalVector MakeFrom(const LptaVector &vector);
    inline static LptaNormalVector MakeFrom(float x, float y, float z);

    inline static LptaNormalVector MakeXAlignedVector(VECTOR_DIRECTION direction);
    inline static LptaNormalVector MakeYAlignedVector(VECTOR_DIRECTION direction);
    inline static LptaNormalVector MakeZAlignedVector(VECTOR_DIRECTION direction);

    // todo: this is awkward, might need to rethink the design of this class and the
    // vector class.
    virtual void SetX(float x);
    virtual void SetY(float y);
    virtual void SetZ(float z);
};

LptaNormalVector LptaNormalVector::MakeFrom(const LptaVector &vector)
{
    return MakeFrom(vector.GetX(), vector.GetY(), vector.GetZ());
}
LptaNormalVector LptaNormalVector::MakeFrom(float x, float y, float z)
{
    LptaVector vector(x, y, z);
    vector.Normalize();
    return LptaNormalVector(vector.GetX(), vector.GetY(), vector.GetZ());
}

LptaNormalVector LptaNormalVector::MakeXAlignedVector(VECTOR_DIRECTION direction)
{
    return LptaNormalVector(direction == POSITIVE ? 1.0f : -1.0f, 0.0f, 0.0f);
}
LptaNormalVector LptaNormalVector::MakeYAlignedVector(VECTOR_DIRECTION direction)
{
    return LptaNormalVector(0.0f, direction == POSITIVE ? 1.0f : -1.0f, 0.0f);
}
LptaNormalVector LptaNormalVector::MakeZAlignedVector(VECTOR_DIRECTION direction)
{
    return LptaNormalVector(0.0f, 0.0f, direction == POSITIVE ? 1.0f : -1.0f);
}

}

#endif