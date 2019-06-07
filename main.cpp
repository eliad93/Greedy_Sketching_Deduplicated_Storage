#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "Sketch.h"

using std::vector;
using std::string;
using std::istream;
using std::stringstream;
using std::ifstream;
using std::cout;
using std::endl;
using std::getline;

int main() {

    Sketch sketch(R"(C:\Users\eliad\Downloads\B_heuristic_depth10_002_002.csv)");
    sketch.print();
    cout << sketch.calculateReclaimable() << endl;
    return 0;
}