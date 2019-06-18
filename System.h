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
#include <ctime>
#include <chrono>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

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
    /*
     * Used for block lines parsing
     */
    struct BlockLine {
        int id;
        int refCount;
        explicit BlockLine(string& line);
    };
    /*
     * Used for file line parsing
     */
    struct FileLine {
        static const int ARG_ID_IDX = 1;
        static const int ARG_BLOCKS_IDX = 5;
        static const int ARG_BLOCKS_STEP = 2;
        int id;
        list<int> blocks;
        explicit FileLine(string& line);
    };
    /*
     * Used for organizing reclaimGreedy
     * information for a specific iteration
     */
    struct IterationStats{
        int iteration=0;
        double sourceSize=0;
        double destinationSize=0;
        double moved=0;
        double copied=0;
        double iterationTime=0;
    };

private:
    /*
     * Private static methods
     */
    static string getFileName(const string& path);
    static inline bool canMigrate(double M, double epsilon,
            double originalSpace, double reclaimed, double reclaimAddition);
    static inline bool isSolution(double M, double epsilon, double moved);
    /*
     * Private methods
     */
    void initObjects(ifstream& input_csv);

public:
    /*
     * Constructors
     */
    System(unsigned int filesArraySize, unsigned int blocksArraySize);
    explicit System(const string& path);
    /*
     * Destructor
     */
    ~System();
    /*
     * Algorithms
     */
    double calculateReclaimable(System& full);
    double calculateReclaimable(File& file);
    double calculateSpaceInTargetSystem(System& target);
    double calculateSpaceInTargetSystem(File& file);
    void reclaimGreedy(System& target, double M, double epsilon);
    void migrateVolume(System& target, int fileId);
    void addVolume(int fileId, File* file);
    /*
     * Simple getters
     */
    unsigned int getFilesArraySize() const;
    unsigned int getBlocksArraySize() const;
    /*
     * Complex getters
     */
    double getCompRatio(int blockId) const;
    int getBlockRefCount(int blockId) const;
    int getBlockPhysicalCount(int blockId) const;
    /*
     * System queries
     */
    bool containsBlock(int blockId) const;

private:
    string path;
    unsigned int blocksArraySize = 0;
    unsigned int filesArraySize = 0;
    /*
     * A File is a set of blocks from 0 to blocksArraySize
     * where file[i]==true indicates that file contains block i
     */
    //
    File** files = nullptr;
    // blocks[i] is the reference count of block i
    int* blocks = nullptr;
    double ingestTime = 0;
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
