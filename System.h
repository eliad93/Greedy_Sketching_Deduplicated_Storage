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

using std::string;
using std::ifstream;
using std::stringstream;

class System {

    static const double blocksCompRatio;
    static const int blocksPhysicalCount;

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

    void countProcessObjects(ifstream& input_csv, int& blocksNum,
            int& filesNum, vector<BLine>& bLines, vector<FLine>& flines){
        string line;
        int blocksCount=0, filesCount=0;
        while(input_csv.good()){
            getline(input_csv, line, '\n');
            if(!line.empty()){
                switch(line[0]){
                    case 'F':
                        flines.emplace_back(line);
                        filesCount++;
                        break;
                    case 'B':
                        bLines.emplace_back(line);
                        blocksCount++;
                        break;
                    default:
                        break;
                }
            }
        }
        blocksNum = blocksCount;
        filesNum = filesCount;
    }

    void initBlocks(vector<BLine>& bLines){
        blocks = unordered_map<int, int>(bLines.size());
        for(auto& bLine: bLines){
            blocks[bLine.id] = bLine.refCount;
        }
    }

    void initFiles(vector<FLine>& fLines){
        files = unordered_map<int, unordered_map<int, int>>(fLines.size());
        for(auto& fLine: fLines){
            unordered_map<int, int>& file = files[fLine.id];
            for(int blockId: fLine.blocks){
                file[blockId]++;
            }
        }
    }

public:

    System() = default;

    explicit System(const string& path){
        ifstream input_csv(path);
        if(!input_csv.is_open()){
            cout << "ERROR: File open failed" << endl;
        }
        int blocksNum, filesNum;
        vector<BLine> bLines;
        vector<FLine> fLines;
        countProcessObjects(input_csv, blocksNum, filesNum, bLines, fLines);
        initBlocks(bLines);
        initFiles(fLines);
    }

    int blockRefCount(int blockId) const {
        assert(blockId >= 0);
        const auto iter = blocks.find(blockId);
        int refCount = iter != blocks.end() ? iter->second : 0;
        return refCount;
    }

    double getCompRatio(int blockId) const {
        assert(blockId >= 0);
        const auto iter = blocks.find(blockId);
        return iter != blocks.end() ? blocksCompRatio : 0;
    }

    int blockPhysicalCount(int blockId) const {
        assert(blockId >= 0);
        const auto iter = blocks.find(blockId);
        return iter != blocks.end() ? blocksPhysicalCount : 0;
    }

    double calculateReclaimable(System& full){
        double reclaimable = 0;
        for(int blockId=0; blockId < blocks.size(); blockId++){
            if(blockRefCount(blockId) == full.blockRefCount(blockId)){
                reclaimable += getCompRatio(blockId) *
                        blockPhysicalCount(blockId);
            }
        }
        return reclaimable;
    }

    double calculateReclaimable(unordered_map<int, int>& file){
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
        for(int blockId=0; blockId < blocks.size(); blockId++){
            if(!target.containsBlock(blockId)){
                targetSpace += getCompRatio(blockId);
            }
        }
        return targetSpace;
    }

    double calculateSpace(unordered_map<int, int>& file){
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
        double reclaimed = 0, originalSpace = blocks.size(),
        savePercentage = 0;
        while(savePercentage < (M / 100)){
            double bestSavingRatio = 0, currentReclaim = 0;
            int bestReclaimId = -1;
            for(auto& iter: files){
                int fileId = iter.first;
                auto& file = iter.second;
                double reclaim = calculateReclaimable(file);
                double targetSpace = target.calculateSpace(file);
                double savingRatio = reclaim / targetSpace;
                if(savingRatio > bestSavingRatio){
                    bestReclaimId = fileId;
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
        return blocks.find(blockId) != blocks.end();
    }

private:
    // pairs of {fileId, blocks}
    // blocks is a pair of {blockId, refCount}
    unordered_map<int, unordered_map<int, int>> files;
    // pairs of {blockId, refCount}
    unordered_map<int, int> blocks;
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
