#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "Sketch.h"
#include "ArgumentsParser.h"
#include "System.h"

using std::vector;
using std::string;
using std::istream;
using std::stringstream;
using std::ifstream;
using std::cout;
using std::endl;
using std::getline;

int main(int argc, char* argv[]) {
    ArgumentsParser argumentsParser(argc, argv);
    System system1(argumentsParser.getFilePath());

    cout << system1.calculateReclaimable(system1) << endl;
    cout << system1.calculateSpaceInTargetSystem(system1) << endl;
    System system2;
    system1.reclaimGreedy(system2, argumentsParser.getM(),
            argumentsParser.getEpsilon());
    return 0;
}