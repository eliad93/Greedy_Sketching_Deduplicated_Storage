//
// Created by eliad on 6/15/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <cassert>

using std::cout;
using std::endl;

using std::vector;
using std::unordered_map;
using std::list;
using std::array;

using std::string;
using std::ifstream;
using std::stringstream;

class System {

    using File = unordered_map<int, bool>;

    static constexpr double blocksCompRatio = 1;
    static const int blocksPhysicalCount = 1;

    struct BLine {
        int id;
        int refCount;
        explicit BLine(string& line){
            vector<string> tokens;
            stringstream ss(line);
            string token;
            while(std::getline(ss, token, ',')) {
                tokens.emplace_back(token);
            }
            id = std::stoi(tokens[1]);
            refCount = std::stoi(tokens[3]);
        }
    };

    struct FLine {
        static const int ARG_ID_IDX = 1;
        static const int ARG_BLOCKS_IDX = 5;
        static const int ARG_BLOCKS_STEP = 2;
        int id;
        list<int> blocks;
        explicit FLine(string& line){
            vector<string> tokens;
            stringstream ss(line);
            string token;
            while(std::getline(ss, token, ',')) {
                tokens.emplace_back(token);
            }
            id = std::stoi(tokens[ARG_ID_IDX]);
            for(auto iter = tokens.begin() + ARG_BLOCKS_IDX;
            iter!=tokens.end(); iter+=ARG_BLOCKS_STEP){
                blocks.emplace_back(std::stoi(*iter));
            }
        }
    };

private:

    void initObjects(ifstream& input_csv){
        string line;
        int lineNumber = 0;
        while(input_csv.good()){
            getline(input_csv, line, '\n');
            if(!line.empty()){
                if(lineNumber > 5){
                    if(line[0] == 'F'){
                        FLine fLine(line);
                        File& file = files[fLine.id];
                        for(int blockId: fLine.blocks){
                            file[blockId] = true;
                        }
                    } else if(line[0] == 'B') {
                        BLine bLine(line);
                        blocks[bLine.id] = bLine.refCount;
                    }
                } else {
                    if(lineNumber == 3){
                        numFiles = std::stoi((line.begin()+13).base());
                        files = new File[numFiles];
                    } else if(lineNumber == 5){
                        numBlocks = std::stoi((line.begin()+14).base());
                        blocks = new int[numBlocks];
                    }
                }
            }
            lineNumber++;
        }
    }

    void initBlocks(vector<BLine>& bLines){
        for(auto& bLine: bLines){
            blocks[bLine.id] = bLine.refCount;
        }
    }

    void initFiles(vector<FLine>& fLines){
        for(auto& fLine: fLines){
            File& file = files[fLine.id];
            for(int blockId: fLine.blocks){
                file[blockId] = true;
            }
        }
    }

public:

    System() = default;

    explicit System(const string& path) :
        numFiles(0),
        numBlocks(0){
        ifstream input_csv(path);
        if(!input_csv.is_open()){
            cout << "ERROR: File open failed" << endl;
        }
        vector<BLine> bLines;
        vector<FLine> fLines;
        initObjects(input_csv);
    }

    ~System(){
        delete[] files;
        delete[] blocks;
    }

    int blockRefCount(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= numBlocks + 1 ?
        blocks[blockId] : 0;
    }

    double getCompRatio(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= numBlocks + 1 ?
        blocksCompRatio : 0;
    }

    int blockPhysicalCount(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= numBlocks + 1 ?
        blocksPhysicalCount : 0;
    }

    double calculateReclaimable(System& full){
        double reclaimable = 0;
        for(int blockId=0; blockId < numBlocks; blockId++){
            if(blockRefCount(blockId) == full.blockRefCount(blockId)){
                reclaimable += getCompRatio(blockId) *
                        blockPhysicalCount(blockId);
            }
        }
        return reclaimable;
    }

    double calculateReclaimable(File& file){
        double reclaimable = 0;
        for(auto& block: file){ // block is a pair {blockId, refCount}
            int blockId = block.first;
            int blockRefCountInfile = block.second;
            if(blockRefCountInfile == blockRefCount(blockId)){
                reclaimable += getCompRatio(blockId) *
                               blockPhysicalCount(blockId);
            }
        }
        return reclaimable;
    }

    double calculateSpaceInTargetSystem(System& target){
        double targetSpace = 0;
        for(int blockId=0; blockId < numBlocks; blockId++){
            if(!target.containsBlock(blockId)){
                targetSpace += getCompRatio(blockId);
            }
        }
        return targetSpace;
    }

    double calculateSpace(File& file){
        double targetSpace = 0;
        for(auto& block: file){ // block is a pair {blockId, refCount}
            int blockId = block.first;
            int blockRefCountInfile = block.second;
            if(!containsBlock(blockId)){
                targetSpace += getCompRatio(blockId);
            }
        }
        return targetSpace;
    }

    void reclaimGreedy(System& target, double M, double epsilon){
        double reclaimed = 0, originalSpace = numBlocks,
        savePercentage = 0;
        while(savePercentage < (M / 100)){
            double bestSavingRatio = 0, currentReclaim = 0;
            int bestReclaimId = -1;
            for(int i=0; i<numFiles; i++){
                File& file = files[i];
                double reclaim = calculateReclaimable(file);
                double targetSpace = target.calculateSpace(file);
                double savingRatio = reclaim / targetSpace;
                if(savingRatio > bestSavingRatio){
                    bestReclaimId = i;
                    bestSavingRatio = savingRatio;
                    currentReclaim = reclaim;
                }
            }
            reclaimed += currentReclaim;
            savePercentage = reclaimed / originalSpace;
        }
        cout << "reclaimed = " << reclaimed << endl;
        cout << "savePercentage = " << savePercentage << endl;
    }

    bool containsBlock(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= numBlocks + 1 ?
        blocks[blockId] > 0 : false;
    }

private:
    // pairs of {fileId, blocks}
    // blocks is a pair of {blockId, refCount}
    File* files = nullptr;
    unsigned int numFiles = 0;
    // pairs of {blockId, refCount}
    int* blocks = nullptr;
    unsigned int numBlocks = 0;
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
