#include "Sketch.h"
//
// Created by eliad on 6/4/2019.
//

#include "Sketch.h"

///////////////////////////////////////
/////////        Sketch      //////////
///////////////////////////////////////

// private methods //
void Sketch::populateFlatData(ifstream& input_csv){
    string line;
    while(input_csv.good()){
        getline(input_csv, line, '\n');
        if(!line.empty()){
            switch(line[0]){
                case 'F':
                {
                    SFile file(line);
                    filesMap.insert(std::make_pair(file.getSerialNumber(),
                                                   file));
                    break;
                }
                case 'P':
                    PLines.emplace_back(line);
                    break;
                case 'B':
                    blocks.emplace_back(SBlock(line));
                    break;
                case 'D':
                    DLines.emplace_back(line);
                    break;
                case 'R':
                    RLines.emplace_back(line);
                    break;
            }
        }
    }
}

void Sketch::initFiles(){
    for(SBlock& block: blocks){
        for(int fileId: block.getFiles()){
            filesMap.at(fileId).addBlock(block);
        }
    }
}

// standard public methods //
Sketch::Sketch(const string& path) :
        path(path),
        filesMap(),
        PLines(),
        DLines(),
        RLines(){
    ifstream input_csv(path);
    if(!input_csv.is_open()){
        cout << "ERROR: File open failed" << endl;
    }
    populateFlatData(input_csv);
    initFiles();
}


bool Sketch::contains(const SBlock& block) const{
    return std::find(blocks.begin(), blocks.end(), block) != blocks.end();
}

// algorithms //
int Sketch::blockRefCount(SBlock& block) const {
//    int refCount = 0;
//    for(const auto& iter: filesMap){
//        refCount += iter.second.blockRefCount(block);
//    }
//    return refCount;
}

double Sketch::calculateReclaimable(Sketch& full){
    double reclaimable = 0;
    for(SBlock& block: blocks){
        if(blockRefCount(block) == full.blockRefCount(block)){
            reclaimable += block.getCompRatio() *
                           SBlock::defaultPhysicalCopies;
        }
    }
    return reclaimable;
}

double Sketch::calculateSpaceInTargetSystem(Sketch& target){
    double targetSpace = 0;
    for(SBlock& block: blocks){
        if(!target.contains(block)){
            targetSpace += block.getCompRatio();
        }
    }
    return targetSpace;
}

// helper functions //
void Sketch::print(){
    cout << "filesMap.size() = " << filesMap.size() << endl
         << "PLines.size() = " << PLines.size() << endl
         << "blocks.size() = " << blocks.size() << endl
         << "DLines.size() = " << DLines.size() << endl
         << "RLines.size() = " << RLines.size() << endl;
}

///////////////////////////////////////
/////////        SBlock      //////////
///////////////////////////////////////

// static constants //
const double Sketch::SBlock::defaultCompRatio = 1;
const double Sketch::SBlock::defaultBlockSize = 1;
const int Sketch::SBlock::defaultPhysicalCopies = 1;

// standard public methods //
Sketch::SBlock::SBlock(const string& line){
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while(std::getline(ss, token, ',')) {
        tokens.emplace_back(token);
    }
    serialNumber = std::stoi(tokens[1]);
    objectId = tokens[2];
    refCount = std::stoi(tokens[3]);
    files = vector<int>();
    for(auto iter = tokens.begin() + 4; iter!=tokens.end(); iter++){
        files.emplace_back(std::stoi(iter.operator*()));
    }
}

bool Sketch::SBlock::operator==(SBlock const& sBlock) const {
    return objectId == sBlock.getObjectId();
}

// getters //
const vector<int>& Sketch::SBlock::getFiles() const {
    return files;
}

const int Sketch::SBlock::getSerialNumber() const {
    return serialNumber;
}

const string& Sketch::SBlock::getObjectId() const{
    return objectId;
}

const int Sketch::SBlock::getRefCount() const {
    return refCount;
}

const double Sketch::SBlock::getCompRatio() const{
   if(compRatio == -1){
       return defaultCompRatio;
   }
    return compRatio;
}

///////////////////////////////////////
/////////        SFile      ///////////
///////////////////////////////////////

// standard public methods //
Sketch::SFile::SFile(string& line):
blocks(){
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while(std::getline(ss, token, ',')) {
        tokens.emplace_back(token);
    }
    serialNumber = std::stoi(tokens[1]);
    fileId = tokens[2];
    paretDirSN = std::stoi(tokens[3]);
    numBaseObjects = std::stoi(tokens[4]);
    baseObjects = map<int, int>();
    for(auto iter = tokens.begin() + 5; iter!=tokens.end(); iter++){
        int baseObjSN = std::stoi(*iter++);
        int baseObjSize = std::stoi(iter.operator*());
        baseObjects[baseObjSN] = baseObjSize;
    }
}

void Sketch::SFile::addBlock(SBlock& block){
    blocks.insert(std::make_pair(block.getSerialNumber(), block));
}

// algorithms //
bool Sketch::SFile::contains(SBlock& block) const {
//    return baseObjects.find(block.getObjectId()) != baseObjects.end();
}

// getters //
const string& Sketch::SFile::getFileId() const {
    return fileId;
}

const int Sketch::SFile::getSerialNumber() const {
    return serialNumber;
}