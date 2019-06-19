//
// Created by eliad on 6/15/2019.
//

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

System::System(const string& path) :
        path(path),
        filesArraySize(0),
        blocksArraySize(0){
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

void System::reclaimGreedy(System& target, double M, double epsilon){
    clock_t greedyBegin = clock();
    double reclaimed = 0, copiedSize = 0, copied = 0,
            originalSpace = blocksArraySize, moved = 0;
    int bestReclaimId = 0, iterationNum = 0;
    list<int> filesToMove;
    list<IterationStats> iterationsStats;
    cout << "filesArraySize= " << filesArraySize << endl;
    cout << "blocksArraySize= " << blocksArraySize << endl;
    if(!canMigrate(M, epsilon, originalSpace, reclaimed, 0)){
        return;
    }
    while(bestReclaimId != -1 && !isSolution(M , epsilon, moved)){
        clock_t iterationBegin = clock();
        IterationStats iteration;
        iteration.iteration = iterationNum;
        iterationNum++;
        double bestSavingRatio = DBL_MAX, bestReclaim = 0,
                bestCopiedSize = 0;
        bestReclaimId = -1;
        for(int i=0; i<filesArraySize; i++){
            if(files[i]){
                File file = *files[i];
                double currentReclaim = calculateReclaimable(file),
                        currentCopiedSize = target.calculateSpaceInTargetSystem(file),
                        savingRatio = currentCopiedSize / MAX(1.0, currentReclaim);
                if(savingRatio < bestSavingRatio && canMigrate(M, epsilon,
                        originalSpace, reclaimed, currentReclaim)){
                    bestReclaimId = i;
                    bestSavingRatio = savingRatio;
                    bestReclaim = currentReclaim;
                    bestCopiedSize = currentCopiedSize;
                }
            }
        }
        if(bestReclaimId != -1){
            filesToMove.emplace_back(bestReclaimId);
            migrateVolume(target, bestReclaimId);
            reclaimed += bestReclaim;
            copiedSize += bestCopiedSize;
            double prevMoved = moved;
            double prevCopied = copied;
            moved = 100 * reclaimed / originalSpace;
            copied = 100 * copiedSize / originalSpace;
            iteration.moved = moved - prevMoved;
            iteration.copied = copied - prevCopied;
        }
        clock_t iterationEnd = clock();
        iteration.sourceSize = 100 * (originalSpace - reclaimed) / originalSpace;
        iteration.destinationSize = copied;
        iteration.iterationTime =
                double(iterationEnd - iterationBegin) / CLOCKS_PER_SEC;
        iterationsStats.emplace_back(iteration);
    }
    clock_t greedyEnd = clock();
    double greedyTime = double(greedyEnd - greedyBegin) / CLOCKS_PER_SEC;
    // standard output
    if(!isSolution(M, epsilon, moved)){
        cout << "Failed migration" << endl;
    } else{
        cout << "[";
        for(auto file: filesToMove) {
            cout << file << " ";
        }
        cout << "]" << endl;
        cout << "moved = " << moved << endl;
        cout << "copied = " << copied << endl;
    }
    // CSV output per run
    cout << "inputFile = " << getFileName(path) << endl;
    cout << "numFiles = " << filesArraySize << endl;
    cout << "numBlocks = " << blocksArraySize << endl;
    cout << "M = " << M << endl;
    cout << "epsilon = " << epsilon << endl;
    cout << "greedyTime = " << greedyTime << endl;
    cout << "ingestTime = " << ingestTime << endl;
    cout << "iterations = " << iterationNum << endl;

    // per iteration output
    for(auto& it: iterationsStats){
        cout << "[iteration=" << it.iteration
             << ", sourceSize=" << it.sourceSize
             << ", destinationSize=" << it.destinationSize
             << ", moved=" << it.moved
             << ", copied=" << it.copied
             << ", iterationTime=" << it.iterationTime
             << "]" << endl;
    }
}

string System::getFileName(const string& path) {
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    size_t i = path.rfind(sep, path.length());
    if (i != string::npos) {
        return(path.substr(i+1, path.length() - i));
    }
    return("");
}

void System::migrateVolume(System& target, int fileId){
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