//
// Created by eliad on 6/10/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H

#include <vector>
#include <string>
#include <iostream>

using std::vector;
using std::string;

using std::stoi;
using std::stod;
using std::cout;
using std::endl;

class ArgumentsParser {

private:

    static const unsigned int numArgs = 8;

    static const int argFile = 1;
    static const int argDedupLevel = 2;
    static const int argDepth = 3;
    static const int argSystemStart = 4;
    static const int argSystemEnd = 5;
    static const int argContainerSize = 6;
    static const int argK = 7;

    vector<string> arguments;

public:

    ArgumentsParser(int argc, char *argv[]) :
            arguments() {
        if (argc != numArgs) {
            cout << "Usage:\n   ./prog file dedup depth sys_start sys_end "
                    "container_size k" << endl;
            exit(0);
        }
        for (int i = 0; i < argc; i++) {
            arguments.emplace_back(string(argv[i]));
        }
    }

    const string &getFilePath() const {
        return arguments[argFile];
    }

    const string &getDedupLevel() const {
        return arguments[argDedupLevel];
    }

    const string &getDepth() const {
        return arguments[argDepth];
    }

    const string &getSystemStart() const {
        return arguments[argSystemStart];
    }

    const string &getSystemEnd() const {
        return arguments[argSystemEnd];
    }

    const string &getContainerSize() const {
        return arguments[argContainerSize];
    }

    const string &getK() const {
        return arguments[argK];
    }
};

#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_ARGUMENTSPARSER_H
