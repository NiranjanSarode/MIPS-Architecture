#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "BranchPredictor.hpp"

int main() {
    SaturatingBranchPredictor saturating(0);
    BHRBranchPredictor bhr(0);
    SaturatingBHRBranchPredictor combined(0, 1000);


    std::ifstream input("input.txt");
    std::string line;
    uint32_t pc;
    int taken;
    while (std::getline(input, line)) {
        std::stringstream ss(line);
        ss >> std::hex >> pc >> taken;
        bool saturatingResult = saturating.predict(pc);
        std::cout << saturatingResult << " ";
        saturating.update(pc, taken);
        bool bhrResult = bhr.predict(pc);
        std::cout << bhrResult << " ";
        bhr.update(pc, taken);
        bool combinedResult = combined.predict(pc);
        std::cout << combinedResult << std::endl;
        combined.update(pc, taken);
    }

    std::cout << std::endl;
    std::cout << "Saturating only accuracy: " << saturating.acc << std::endl;
    std::cout << "BHR only accuracy: " <<  bhr.acc << std::endl;
    std::cout << "Combined accuracy: " << combined.acc << std::endl;

    return 0;
}
