#include <utility>

#include <utility>

#include <utility>

#include <utility>

//
// Created by eliad on 6/15/2019.
//

#include <experimental/filesystem>

#include "System.h"

////////////////////////////////////////////////
////////////        System      ////////////////
////////////////////////////////////////////////

System::System(unsigned int filesArraySize,
        unsigned int blocksArraySize):
        filesArraySize(filesArraySize),
        blocksArraySize(blocksArraySize){
    files = new File*[filesArraySize]();
    blocks = new int[blocksArraySize]();
}

System::System(string filePath, string depth,
        string systemStart, string systemEnd, string containerSize, string K) :
        path(std::move(filePath)),
        filesArraySize(0),
        blocksArraySize(0),
        K(std::move(K)),
        depth(std::move(depth)),
        systemStart(std::move(systemStart)),
        systemEnd(std::move(systemEnd)),
        containerSize(std::move(containerSize)),
        unsolvedPairs(),
        solvedPairs(),
        failedPairs(){
    clock_t ingestBegin = clock();
    ifstream input_csv(path);
    if(!input_csv.is_open()){
        cout << "ERROR: File open failed" << endl;
    }
    vector<BlockLine> bLines;
    vector<FileLine> fLines;
    initObjects(input_csv);
    clock_t ingestEnd = clock();
    ingestTime = double(ingestEnd - ingestBegin) / CLOCKS_PER_SEC;
    initAllPairs();
}

void System::initAllPairs(){
    if(mVector.empty() || epsilonVector.empty()){
        return;
    }
    for(double m: mVector){
        for(double e: epsilonVector){
            if(m > e) {
                unsolvedPairs.insert(pair<double, double>(m, e));
            }
        }
    }
}

System::~System(){
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

int System::getBlockRefCount(int blockId) const {
    assert(blockId >= 0);
    return blocks != nullptr && blockId <= blocksArraySize + 1 ?
           blocks[blockId] : 0;
}

double System::getCompRatio(int blockId) const {
    assert(blockId >= 0);
    return blocks != nullptr && blockId <= blocksArraySize + 1 ?
           blocksCompRatio : 0;
}

double System::calculateReclaimable(System& full){
    double reclaimable = 0;
    for(int blockId=0; blockId < blocksArraySize; blockId++){
        if(getBlockRefCount(blockId) == full.getBlockRefCount(blockId)){
            reclaimable += getCompRatio(blockId) *
                    getBlockPhysicalCount(blockId);
        }
    }
    return reclaimable;
}

double System::calculateReclaimable(File& file){
    double reclaimable = 0;
    for(auto& block: file){ // block is a pair {blockId, refCount}
        int blockId = block.first;
        int blockRefCountInfile = block.second;
        if(blockRefCountInfile == getBlockRefCount(blockId)){
            reclaimable += getCompRatio(blockId) *
                    getBlockPhysicalCount(blockId);
        }
    }
    return reclaimable;
}

double System::calculateSpaceInTargetSystem(System& target){
    double targetSpace = 0;
    for(int blockId=0; blockId < blocksArraySize; blockId++){
        if(!target.containsBlock(blockId)){
            targetSpace += getCompRatio(blockId);
        }
    }
    return targetSpace;
}

double System::calculateSpaceInTargetSystem(File& file){
    double targetSpace = 0;
    for(auto& block: file){ // block is a pair {blockId, refCount}
        int blockId = block.first;
        if(!containsBlock(blockId)){
            targetSpace += getCompRatio(blockId);
        }
    }
    return targetSpace;
}

inline bool System::canMigrate(double M, double epsilon, double originalSpace,
                       double reclaimed, double reclaimAddition){
    return ((reclaimed + reclaimAddition) / originalSpace)
           <= ((M + epsilon) / 100);
}

inline bool System::isSolution(double M, double epsilon, double moved){
    return moved >= (M - epsilon) && moved <= (M + epsilon);
}

bool System::isFailed(double m, double e, double moved){
    return moved > (m + e);
}

bool System::isFinalState(double moved, GreedyOutput& greedyOutput,
                          GreedySummaryUnique& greedySummaryUnique){
    if(unsolvedPairs.empty()){
        return true;
    }
    for(auto& p: unsolvedPairs){
        double m = p.first;
        double e = p.second;
        if(isSolution(m, e, moved)){
            greedySummaryUnique.MFraction = m;
            greedySummaryUnique.M = m / 100.0 * blocksArraySize;
            greedySummaryUnique.epsilonFraction = e;
            greedySummaryUnique.epsilon = e / 100.0 * blocksArraySize;
            greedySummaryUnique.solved = "yes";
            greedyOutput.summariesMap.insert(pair<pair<double, double>,
                    GreedySummaryUnique>(pair<double, double>(m, e),
                    greedySummaryUnique));
            solvedPairs.insert(pair<double, double>(m, e));
        } else if(isFailed(m, e, moved)){
            greedySummaryUnique.solved = "no";
            greedyOutput.summariesMap.insert(pair<pair<double, double>,
                    GreedySummaryUnique>(pair<double, double>(m, e),
                                         greedySummaryUnique));
            failedPairs.insert(p);
        }
    }
    for(auto& p: solvedPairs){
        unsolvedPairs.erase(p);
    }
    for(auto& p: failedPairs){
        unsolvedPairs.erase(p);
    }
    return unsolvedPairs.empty();
}

System::GreedyOutput System::greedy(System &target){
    clock_t greedyBegin = clock();
    double reclaimed = 0, replicatedSize = 0, replicated = 0, moved = 0;
    int bestReclaimId = 0, iterationNum = 0;
    string fileName = getFileName(path);
    GreedyOutput output(fileName,
            K, depth, systemStart, systemEnd, filesArraySize, blocksArraySize,
            ingestTime);
    GreedySummaryUnique greedySummaryUnique(-1, -1, -1, -1, 0, 0, 0, 0,
            ingestTime, 0, 0);
    while(bestReclaimId != -1 && !isFinalState(moved, output,
            greedySummaryUnique)){
        iterationNum++;
        clock_t iterationBegin = clock();
        GreedyIterationStats iteration;
        iteration.iteration = iterationNum;
        double bestSavingRatio = DBL_MAX, bestReclaim = 0;
        bestReclaimId = -1;
        for(int i=0; i<filesArraySize; i++){
            if(files[i]){
                File file = *files[i];
                double currentReclaim = calculateReclaimable(file),
                        currentReplicatedEstimationSize = target.calculateSpaceInTargetSystem(file),
                        savingRatio = currentReplicatedEstimationSize / MAX(1.0, currentReclaim);
                if(savingRatio < bestSavingRatio){
                    bestReclaimId = i;
                    bestSavingRatio = savingRatio;
                    bestReclaim = currentReclaim;
                }
            }
        }
        if(bestReclaimId != -1){
            migrateVolume(target, bestReclaimId,
                    reclaimed, replicatedSize);
            double prevMoved = moved;
            double prevCopied = replicated;
            moved = 100 * reclaimed / blocksArraySize;
            replicated = 100 * replicatedSize / blocksArraySize;
            iteration.moved = moved - prevMoved;
            iteration.copied = replicated - prevCopied;
            iteration.fileId = bestReclaimId;
            greedySummaryUnique.replicationFraction = replicatedSize;
            greedySummaryUnique.replication = replicated;
            greedySummaryUnique.MFractionActual = moved;
            greedySummaryUnique.MActual = reclaimed;
        }
        clock_t iterationEnd = clock();
        iteration.sourceSize = 100 * (blocksArraySize - reclaimed) / blocksArraySize;
        iteration.destinationSize = moved;
        iteration.iterationTime =
                double(iterationEnd - iterationBegin) / CLOCKS_PER_SEC;
        output.iterationsStats.emplace_back(iteration);
        greedySummaryUnique.greedyTime = double(iterationEnd - greedyBegin) / CLOCKS_PER_SEC;
        greedySummaryUnique.totalTime = greedySummaryUnique.greedyTime +
                ingestTime;
        greedySummaryUnique.numIterations = iterationNum;
    }
    return output;
}

string System::getFileName(const string& path) {
    char sep = '/';
//#ifdef _WIN32
//    sep = '\\';
//#endif
    size_t i = path.rfind(sep, path.length());
    if (i != string::npos) {
        string fullname = path.substr(i+1, path.length() - i);
        size_t lastindex = fullname.find_last_of('.');
        string rawname = fullname.substr(0, lastindex);
        return rawname;
    } else {
        i = path.rfind('\\', path.length());
        if (i != string::npos) {
            string fullname = path.substr(i + 1, path.length() - i);
            size_t lastindex = fullname.find_last_of('.');
            string rawname = fullname.substr(0, lastindex);
            return rawname;
        }
    }
    return("");
}

void System::migrateVolume(System& target, int fileId,
        double& numMoved, double& numReplicated){
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
            if(blocks[blockId] == 0){
                numMoved++;
            } else {
                numReplicated++;
            }
        }
    }
    target.addVolume(fileId, file);
    files[fileId] = nullptr;
}

void System::addVolume(int fileId, File* file){
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

bool System::containsBlock(int blockId) const {
    assert(blockId >= 0);
    return blocks != nullptr && blockId <= blocksArraySize + 1 ?
           blocks[blockId] > 0 : false;
}

unsigned int System::getFilesArraySize() const {
    return filesArraySize;
}

unsigned int System::getBlocksArraySize() const {
    return blocksArraySize;
}

void System::initObjects(ifstream& input_csv){
    string line;
    int lineNumber = 0;
    while(input_csv.good()){
        getline(input_csv, line, '\n');
        if(!line.empty()){
            if(line[0] != '#'){
                if(line[0] == 'F'){
                    FileLine fLine(line);
                    File* file = new File();
                    assert(files[fLine.id] == nullptr);
                    files[fLine.id] = file;
                    for(int blockId: fLine.blocks){
                        (*file)[blockId] = true;
                    }
                } else if(line[0] == 'B' || line[0] == 'C') {
                    BlockLine bLine(line);
                    blocks[bLine.id] = bLine.refCount;
                }
            } else {
                if(line.compare(0, MIN(line.size(), 12),
                                "# Num files:", 12) == 0){
                    filesArraySize = std::stoi((line.begin()+13).base());
                    files = new File*[filesArraySize]();
                } else if(line.compare(0, MIN(line.size(), 13),
                                       "# Num Blocks:", 13) == 0){
                    blocksArraySize = std::stoi((line.begin()+14).base());
                    blocks = new int[blocksArraySize];
                }
            }
        }
        lineNumber++;
    }
}

////////////////////////////////////////////////
////////////        BlockLine      /////////////
////////////////////////////////////////////////

System::BlockLine::BlockLine(string& line){
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while(std::getline(ss, token, ',')) {
        tokens.emplace_back(token);
    }
    id = std::stoi(tokens[1]);
    refCount = std::stoi(tokens[3]);
}

int System::getBlockPhysicalCount(int blockId) const {
    assert(blockId >= 0);
    return blocks != nullptr && blockId <= blocksArraySize + 1 ?
           blocksPhysicalCount : 0;
}

////////////////////////////////////////////////
////////////        FileLine      //////////////
////////////////////////////////////////////////

System::FileLine::FileLine(string& line){
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