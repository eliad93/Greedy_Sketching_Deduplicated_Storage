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
#include <cfloat>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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
                        File* file = new File();
                        assert(files[fLine.id]== nullptr);
                        files[fLine.id] = file;
                        for(int blockId: fLine.blocks){
                            (*file)[blockId] = true;
                        }
                    } else if(line[0] == 'B') {
                        BLine bLine(line);
                        blocks[bLine.id] = bLine.refCount;
                    }
                } else {
                    if(lineNumber == 3){
                        filesArraySize = std::stoi((line.begin()+13).base());
                        files = new File*[filesArraySize]();
                    } else if(lineNumber == 5){
                        blocksArraySize = std::stoi((line.begin()+14).base());
                        blocks = new int[blocksArraySize];
                    }
                }
            }
            lineNumber++;
        }
    }

public:

    System(int filesArraySize, int blocksArraySize):
        filesArraySize(filesArraySize),
        blocksArraySize(blocksArraySize){
        files = new File*[filesArraySize]();
        blocks = new int[blocksArraySize]();
    }

    explicit System(const string& path) :
        filesArraySize(0),
        blocksArraySize(0){
        ifstream input_csv(path);
        if(!input_csv.is_open()){
            cout << "ERROR: File open failed" << endl;
        }
        vector<BLine> bLines;
        vector<FLine> fLines;
        initObjects(input_csv);
    }

    ~System(){
        if(files){
            for(int i=0; i<filesArraySize; i++){
                if(files[i]){
                    delete files[i];
                }
            }
            delete[] files;
        }
        delete[] blocks;
    }

    int blockRefCount(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= blocksArraySize + 1 ?
        blocks[blockId] : 0;
    }

    double getCompRatio(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= blocksArraySize + 1 ?
        blocksCompRatio : 0;
    }

    int blockPhysicalCount(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= blocksArraySize + 1 ?
        blocksPhysicalCount : 0;
    }

    double calculateReclaimable(System& full){
        double reclaimable = 0;
        for(int blockId=0; blockId < blocksArraySize; blockId++){
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
        for(int blockId=0; blockId < blocksArraySize; blockId++){
            if(!target.containsBlock(blockId)){
                targetSpace += getCompRatio(blockId);
            }
        }
        return targetSpace;
    }

    double calculateSpaceInTargetSystem(File& file){
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

    inline bool canMigrate(double M, double epsilon, double originalSpace,
            double reclaimed, double reclaimAddition) const {
        return ((reclaimed + reclaimAddition) / originalSpace)
        <= ((M + epsilon) / 100);
    }

    void reclaimGreedy(System& target, double M, double epsilon){
        double reclaimed = 0, copied = 0, originalSpace = blocksArraySize,
        moved = 0;
        int bestReclaimId = 0, iterations = 0;
        if(!canMigrate(M, epsilon, originalSpace, reclaimed, 0)){
            return;
        }
        while(bestReclaimId != -1){
            iterations++;
            double bestSavingRatio = DBL_MAX, bestReclaim = 0,
                    bestCopied = 0;
            bestReclaimId = -1;
            for(int i=0; i<filesArraySize; i++){
                if(files[i]){
                    File file = *files[i];
                    double currentReclaim = calculateReclaimable(file),
                    currentCopied = target.calculateSpaceInTargetSystem(file),
                    savingRatio = currentCopied / MAX(1.0, currentReclaim);
                    if(savingRatio < bestSavingRatio && canMigrate(M, epsilon,
                            originalSpace, reclaimed, currentReclaim)){
                        bestReclaimId = i;
                        bestSavingRatio = savingRatio;
                        bestReclaim = currentReclaim;
                        bestCopied = currentCopied;
                    }
                }
            }
            if(bestReclaimId != -1){
                migrateVolume(target, bestReclaimId);
                reclaimed += bestReclaim;
                copied += bestCopied;
                moved = reclaimed / originalSpace;
            }
        }
        cout << "reclaimed = " << reclaimed << endl;
        cout << "moved = " << moved << endl;
    }

    void migrateVolume(System& target, int fileId){
        assert(fileId >= 0);
        if(fileId + 1 > filesArraySize){
            return;
        }
        File* file = files[fileId];
        if(!file){
            return;
        }
        for(auto& iter: *file){
            bool blockFound = iter.second;
            if(blockFound){
                int blockId = iter.first;
                assert(blockId >= 0 && blockId + 1 <= blocksArraySize &&
                blocks[blockId]>0);
                blocks[blockId]--;
            }
        }
        target.addVolume(fileId, file);
        files[fileId] = nullptr;
    }

    void addVolume(int fileId, File* file){
        assert(fileId >= 0 && fileId + 1 <= filesArraySize);
        if(!file){
            return;
        }
        files[fileId] = file;
        for(auto& iter: *file){
            bool blockFound = iter.second;
            if(blockFound){
                int blockId = iter.first;
                assert(blockId >= 0 && blockId + 1 <= blocksArraySize &&
                       blocks[blockId] >= 0);
                blocks[blockId]++;
            }
        }
    }

    bool containsBlock(int blockId) const {
        assert(blockId >= 0);
        return blocks != nullptr && blockId <= blocksArraySize + 1 ?
        blocks[blockId] > 0 : false;
    }

private:
    // pairs of {fileId, blocks}
    // blocks is a pair of {blockId, refCount}
    File** files = nullptr;
    unsigned int filesArraySize = 0;
    // pairs of {blockId, refCount}
    int* blocks = nullptr;
    unsigned int blocksArraySize = 0;
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
