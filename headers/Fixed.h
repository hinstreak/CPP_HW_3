#pragma once

#include <cstdint>
#include "FixedImpl.h"

template <size_t N>
struct nSizeType;

template<> struct nSizeType<8>  {using type = int8_t; };
template<> struct nSizeType<16> {using type = int16_t;};
template<> struct nSizeType<32> {using type = int32_t;};
template<> struct nSizeType<64> {using type = int64_t;};

template <size_t N, size_t K> requires (N >= K)
using Fixed = FixedImpl<typename nSizeType<N>::type, K>;
