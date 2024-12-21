#pragma once

#include <vector>
#include <cstdint>
#include <string>

struct InfoF
{
    size_t height{};
    size_t width{};
    double densities[256]{}, g{};
    std::vector<std::vector<uint8_t>> field;

    InfoF() = default;
    explicit InfoF(const std::string& filename);
    void readFromFile(const std::string& filename);
};