#pragma once

#include <exception>
#include <iostream>
#include <vector>

template <typename T, size_t Nv, size_t Mv>
struct CusMatrix
{
    T v[Nv][Mv]{};
    void init(size_t N, size_t M);
    T* operator[](size_t index);
    CusMatrix& operator=(const CusMatrix& b);
};

template <typename T>
struct CusMatrix<T, 0, 0>
{
    std::vector<std::vector<T>> v;

    void init(size_t N, size_t M);
    std::vector<T>& operator[](size_t index);
    CusMatrix& operator=(const CusMatrix& b);
};

template <typename T, size_t Nv, size_t Mv>
void CusMatrix<T, Nv, Mv>::init(size_t N, size_t M) {
    if (N != Nv || M != Mv) {std::cout << "Wrong field size\n"; throw std::exception();}
}

template <typename T>
void CusMatrix<T, 0, 0>::init(size_t N, size_t M) {
    v.resize(N, std::vector<T>(M));
}

template <typename T, size_t Nv, size_t Mv>
T* CusMatrix<T, Nv, Mv>::operator[](size_t index) {
    return v[index];
}

template <typename T>
std::vector<T>& CusMatrix<T, 0, 0>::operator[](size_t index) {
    return v[index];
}

template <typename T, size_t Nv, size_t Mv>
CusMatrix<T, Nv, Mv>& CusMatrix<T, Nv, Mv>::operator=(const CusMatrix& other)
{
    if(this == &other) {return *this;}
    memcpy(v, other.v, sizeof(v));
    return *this;
}
