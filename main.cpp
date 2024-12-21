#include <memory>

#include "headers/Simulator.h"
#include "headers/ParsingSettings.h"
#include "headers/TypeGen.h"

constexpr auto simulators = generateSimulators();
constexpr auto types = generateTypes();

int main(int argc, char* argv[])
{
    SimSetts sets = parseArgs(argc, argv);
    InfoF info(sets.input_filename);

    tuple need = {sets.p_type, sets.v_type, sets.vf_type, info.height, info.width};
    auto index = std::find(types.begin(), types.end(), need) - types.begin();
    if (index == types.size())
    {
        need = {sets.p_type, sets.v_type, sets.vf_type, 0, 0};
        index = std::find(types.begin(), types.end(), need) - types.begin();
    }

    if (index == types.size()) {
        std::cout << "Simulator does not exist\n"; exit(EXIT_FAILURE);
    }

    auto sim = simulators[index]();
    sim->init(info, sets);

    for (size_t i = 0; i < 1000000; ++i) {
        sim->nextTick();
    }
}
