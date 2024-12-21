#pragma once

#include <cstdint>
#include "Fixed.h"

template <size_t N, size_t K>
struct FastFixedWrap: FastFixedWrap<N+1, K> {using type = typename FastFixedWrap<N+1, K>::type;};

template<size_t K> struct FastFixedWrap<8, K>:  Fixed<8, K>  {using type = Fixed<8, K>;};
template<size_t K> struct FastFixedWrap<16, K>: Fixed<16, K> {using type = Fixed<16, K>;};
template<size_t K> struct FastFixedWrap<32, K>: Fixed<32, K> {using type = Fixed<32, K>;};
template<size_t K> struct FastFixedWrap<64, K>: Fixed<64, K> {using type = Fixed<64, K>;};

template <size_t N, size_t K>
using FastFixed = typename FastFixedWrap<N, K>::type;
