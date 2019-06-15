//
// Created by eliad on 6/10/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H

#include <vector>
#include <string>

using std::vector;
using std::string;

using std::stoi;
using std::stod;

class ArgumentsParser {

private:

    static const int argFile = 0;
    static const int argM = 1;
    static const int argEpsilon = 2;

    vector<string> arguments;
    string filePath;
    double M;
    double epsilon;

public:

    ArgumentsParser(int argc, char* argv[]){
        for(int i=1; i < argc; i++){
            arguments.emplace_back(string(argv[i]));
        }
        filePath = arguments[argFile];
        M = stod(arguments[argM]);
        epsilon = stod(arguments[argEpsilon]);
    }

    const string& getFilePath() const {
        return filePath;
    }

    const double getM() const {
        return M;
    }

    const double getEpsilon() const {
        return epsilon;
    }
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H
