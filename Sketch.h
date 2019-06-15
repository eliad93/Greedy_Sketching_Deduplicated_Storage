#include <utility>

//
// Created by eliad on 6/4/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SKETCH_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SKETCH_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

using std::string;
using std::stringstream;
using std::ifstream;
using std::cout;
using std::endl;
using std::vector;
using std::map;

class Sketch {
private:
    class SBlock{
    private:
        int serialNumber;
        string objectId;  // not relevant for now
        int refCount;  // todo: calculate this - not from sketch itself
        vector<int> files;  // todo: ignore this for calculations
        double compRatio = -1;
    public:
        // static constants //
        static const double defaultCompRatio; // todo: review this
        static const double defaultBlockSize;
        static const int defaultPhysicalCopies;
        // standard public methods //
        explicit SBlock(const string& line);
        SBlock(const SBlock&) = default;
        SBlock& operator=(SBlock const&) = default;
        bool operator==(SBlock const&) const;
        // getters //
        const vector<int> &getFiles() const;
        const int getSerialNumber() const;
        const string& getObjectId() const;
        const int getRefCount() const;
        const double getCompRatio() const;
    };
    class SFile{
    private:
        int serialNumber;
        string fileId;
        int paretDirSN; // todo: necessary?
        int numBaseObjects;
        map<int, int> baseObjects; // these are the blocks
        map<int, SBlock> blocks;
    public:
        // standard public methods //
        explicit SFile(string& line);
        void addBlock(SBlock& block);
        // algorithms //
        bool contains(SBlock& block) const;
        // getters //
        const string &getFileId() const;
        const int getSerialNumber() const;
    };
    // private Sketch members //
    string path;
    map<int, SFile> filesMap;
    vector<string> PLines;
    vector<SBlock> blocks;  // todo: should be a hash
    vector<string> DLines;
    vector<string> RLines;
    // private methods //
    void populateFlatData(ifstream& input_csv);
    void initFiles();
public:
    // standard public methods //
    explicit Sketch(const string& path);
    bool contains(const SBlock& block) const;
    // algorithms //
    int blockRefCount(SBlock& block) const;
    double calculateReclaimable(Sketch& full);
    double calculateSpaceInTargetSystem(Sketch& target);
    // helper functions //
    void print();
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SKETCH_H
