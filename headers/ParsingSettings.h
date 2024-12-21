#pragma once

#include <cstdint>
#include <string>

#define FIXED(n, k) (100*n+k)
#define FAST_FIXED(n, k) (10000*n+k)
#define FLOAT 1000000
#define DOUBLE 2000000
#define DYNAMIC pair{0, 0}
#define S(a, b) pair<int, int>(a, b)

struct SimSetts
{
    int p_type = 0, v_type = 0, vf_type = 0;
    std::string input_filename, output_filename;
    int64_t n_ticks;
};

SimSetts parseArgs(int argc, char* argv[]);