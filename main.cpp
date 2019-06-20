#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "ArgumentsParser.h"
#include "System.h"
#include "Simulator.h"

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
    Simulator simulator(argumentsParser);
    simulator.run();
    return 0;
}