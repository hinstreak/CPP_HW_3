#pragma once

#include <cassert>
#include <array>
#include <algorithm>
#include "Const.h"
#include "CusMatrix.h"

template <typename Type, int Nv, int Mv>
struct VectorField
{
    size_t N = Nv, M = Mv;
    CusMatrix<std::array<Type, deltas.size()>, Nv, Mv> v;

    Type& add(int x, int y, int dx, int dy, Type dv) {
        return get(x, y, dx, dy) += dv;
    }

    Type& get(int x, int y, int dx, int dy)
    {
        return v[x][y][((dy&1)<<1) | (((dx&1)&((dx&2)>>1)) | ((dy&1)&((dy&2)>>1)))];
    }

    void clear();
    void init(size_t Nvalue, size_t Mvalue);
};

template <typename Type, int Nv, int Mv>
void VectorField<Type, Nv, Mv>::clear()
{
    for (size_t x = 0; x < N; x++) {
        for (size_t y = 0; y < M; y++) {
            for (size_t z = 0; z < deltas.size(); z++)
            {
                v[x][y][z] = Type();
            }
        }
    }
}

template <typename Type, int Nv, int Mv>
void VectorField<Type, Nv, Mv>::init(size_t Nvalue, size_t Mvalue)
{
    N = Nvalue; M = Mvalue; v.init(Nvalue, Mvalue);
}