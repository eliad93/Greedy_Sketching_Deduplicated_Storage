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
    vector<string> filesPaths;
    double M;
    double epsilon;

public:

    ArgumentsParser(int argc, char* argv[]) :
        arguments(),
        filesPaths(){
        for(int i=1; i < argc; i++){
            arguments.emplace_back(string(argv[i]));
        }
        int i=0;
        while(i<arguments.size()-2){
            filesPaths.emplace_back(arguments[i]);
            i++;
        }
        M = stod(arguments[i++]);
        epsilon = stod(arguments[i]);
    }

    const string& getFilePath(int i) const {
        return filesPaths[i];
    }

    const vector<string>& getFilesPaths() const {
        return filesPaths;
    }

    const double getM() const {
        return M;
    }

    const double getEpsilon() const {
        return epsilon;
    }
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H
