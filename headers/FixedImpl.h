#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>

template <typename V, size_t K1>
struct FixedImpl
{
    template <typename V2, size_t K2>
    explicit constexpr FixedImpl(FixedImpl<V2, K2> f): v((K1>=K2)?(((int64_t)f.v) << (K1-K2)):(f.v >> (K2-K1))) {}

    explicit constexpr FixedImpl(int64_t v): v(v << K1) {}
    explicit constexpr FixedImpl(float f): v(f * (1ll << K1)) {}
    explicit constexpr FixedImpl(double f): v(f * (1ll << K1)) {}
    explicit constexpr FixedImpl(): v(0) {}

    static constexpr FixedImpl from_raw(int64_t x)
    {
        FixedImpl ret;
        ret.v = x;
        return ret;
    }

    V v;

    auto operator<=>(const FixedImpl&) const = default;
    bool operator== (const FixedImpl&) const = default;

    explicit constexpr operator float() const {return (double)v / ((double) (1ll << K1));}
    explicit constexpr operator double() const {return (double)v / ((double) (1ll << K1));}

    static const size_t k = K1;
};

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator+(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>::from_raw((K1>K2) ? (a.v + (b.v << (K1-K2))) : (a.v + (b.v >> (K2-K1))));
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator-(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>::from_raw((K1>K2) ? (a.v - (b.v << (K1-K2))) : (a.v - (b.v >> (K2-K1))));
}

template<typename V1, size_t K1, typename V2, size_t K2>
FixedImpl<V1, K1> operator*(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>(double(a) * double(b));
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator/(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>(double(a) / double(b));
}

template<typename V1, size_t K1, typename V2, size_t K2>
FixedImpl<V1, K1>& operator+=(FixedImpl<V1, K1> &a, const FixedImpl<V2, K2>& b) {
    return a = a + b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator-=(FixedImpl<V1, K1> &a, const FixedImpl<V2, K2>& b) {
    return a = a - b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator*=(FixedImpl<V1, K1> &a, const FixedImpl<V2, K2>& b) {
    return a = a * b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator/=(FixedImpl<V1, K1> &a, const FixedImpl<V2, K2>& b) {
    return a = a / b;
}

template<typename V1, size_t K1>
auto operator-(FixedImpl<V1, K1> x) {
    return FixedImpl<V1, K1>::from_raw(-x.v);
}

template<typename V1, size_t K1>
std::ostream &operator<<(std::ostream &out, const FixedImpl<V1, K1>& x) {
    return out << x.v / (double) (1ll << K1);
}