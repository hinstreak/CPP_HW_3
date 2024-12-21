#pragma once

#include "Fixed.h"
#include <array>
#include "TypeGen.h"
#include "FastFixed.h"

using std::array;

#ifndef SIZES
#define SIZES
#endif

constexpr array t{TYPES};
constexpr array s{DYNAMIC, SIZES};

template <int num>
struct NToT
{
    static auto get()
    {
        if constexpr (num == FLOAT) {
            return std::type_identity<float>{};
        } else if constexpr (num == DOUBLE) {
            return std::type_identity<double>{};
        } else if constexpr (num < 10000) {
            return std::type_identity<Fixed<num/100,num%100>>{};
        } else {
            return std::type_identity<FastFixed<num/10000,num%10000>>{};
        }
    }
};

template <int num>
using numType = typename decltype(NToT<num>::get())::type;

template <typename P, typename V, typename VF, size_t N, size_t M>
constexpr std::unique_ptr<Simulator> generateSim() {
    return std::make_unique<SimulatorImpl<P, V, VF, N, M>>();
}

template <int index>
constexpr auto simGen()
{
    auto res = simGen<index + 1>();
    res[index] = generateSim<numType<t[index/(t.size()*t.size()*s.size())]>,
            numType<t[index%(t.size()*t.size()*s.size())/(t.size()*s.size())]>,
                    numType<t[index%(t.size()*s.size())/s.size()]>, s[index%s.size()].first, s[index%s.size()].second>;
    return res;
}
using genfunc = std::unique_ptr<Simulator>(*)();

template <>
constexpr auto simGen<t.size() * t.size() * t.size() * s.size()>() {
    return array<genfunc, t.size()*t.size()*t.size()*s.size()>();
}

template <int index>
constexpr auto typeGen()
{
    auto res = typeGen<index + 1>();
    res[index] = {t[index/(t.size()*t.size()*s.size())],
                  t[index%(t.size()*t.size()*s.size())/(t.size()*s.size())],
                  t[index%(t.size()*s.size())/s.size()], s[index%s.size()].first, s[index%s.size()].second};
    return res;
}

template <>
constexpr auto typeGen<t.size() * t.size() * t.size() * s.size()>() {
    return array<tuple<int, int, int, size_t, size_t>, t.size()*t.size()*t.size()*s.size()>();
}

constexpr auto generateTypes = typeGen<0>;
constexpr auto generateSimulators = simGen<0>;