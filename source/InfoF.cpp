#include "../headers/InfoF.h"

#include <fstream>
#include <iostream>

InfoF::InfoF(const std::string &filename) {
    readFromFile(filename);
}

void InfoF::readFromFile(const std::string& filename)
{
    std::ifstream to_read(filename);
    if (!to_read) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    size_t k; uint8_t symbol; double d; std::string s;
    to_read >> height >> width >> g >> k; to_read.get();

    for (size_t i = 0; i < k; i++)
    {
        symbol = to_read.get();
        to_read >> d;
        densities[symbol] = d;
        to_read.get();
    }

    for (size_t i = 0; i < height; i++)
    {
        getline(to_read, s);
        field.emplace_back(s.begin(), s.end());
    }
    to_read.close();
}
