#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "Sketch.h"
#include "ArgumentsParser.h"

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

    Sketch sketch(argumentsParser.getFilePath());
    sketch.print();
    cout << sketch.calculateReclaimable() << endl;
    cout << sketch.calculateSpaceInTargetSystem(sketch) << endl;
    return 0;
}