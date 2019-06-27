#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

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
#include <set>
#include <algorithm>
#include <map>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

using std::cout;
using std::endl;

using std::vector;
using std::unordered_map;
using std::list;
using std::array;
using std::pair;
using std::set;
using std::map;

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

public:
    /*
     * Used for organizing greedy
     * information for a specific iteration
     */
    struct GreedyIterationStats {
        int iteration{};
        double sourceSize{};
        double destinationSize{};
        double moved{};
        double copied{};
        double iterationTime{};
        unsigned int fileId = -1;

        GreedyIterationStats() = default;
    };
    /*
     * Used for organizing summarized greedy
     * information for a specific run
     */
    struct GreedySummaryCommon {
        string fileName; // input file
        char dedupLevel{};
        string K{};
        string depth{};
        string systemStart{};
        string systemEnd{};
        unsigned int numFiles{}; // num files in source
        unsigned int numBlocks{};
        double ingestTime{};
        GreedySummaryCommon(string& fileName, string K, string depth,
                string systemStart, string systemEnd, unsigned int numFiles,
                unsigned int numBlocks, double ingestTime):
        fileName(fileName),
        dedupLevel('B'),
        depth(std::move(depth)),
        systemStart(std::move(systemStart)),
        systemEnd(std::move(systemEnd)),
        numFiles(numFiles),
        numBlocks(numBlocks),
        K(std::move(K)),
        ingestTime(ingestTime){}
    };

    struct GreedySummaryUnique{
        double MFraction{}; // % blocks to move from source to target
        double M{}; // num blocks to move from source to target
        double epsilonFraction{}; // the input epsilon
        double epsilon{}; // num blocks Epsilon represents
        double MFractionActual{};
        double MActual{};
        double replicationFraction{}; // num blocks copied
        double replication{}; // % blocks copied
        double totalTime{};
        double greedyTime{};
        unsigned int numIterations{};
        GreedySummaryUnique(double MFraction, double M, double epsilonFraction,
                double epsilon, double MFractionActual, double MActual,
                double replicationFraction, double replication, double totalTime,
                double greedyTime, unsigned int numIterations) :
                MFraction(MFraction),
                M(M),
                epsilonFraction(epsilonFraction),
                epsilon(epsilon),
                MFractionActual(MFractionActual),
                MActual(MActual),
                replicationFraction(replicationFraction),
                replication(replication),
                totalTime(totalTime),
                greedyTime(greedyTime),
                numIterations(numIterations) {}
    };

    struct GreedyOutput {
        GreedySummaryCommon greedySummaryCommon;
        map<pair<double, double>, GreedySummaryUnique> summariesMap;
        list<GreedyIterationStats> iterationsStats;

        GreedyOutput(string fileName, string K, string depth,
        string systemStart, string systemEnd, unsigned int numFiles,
        unsigned int numBlocks, double ingestTime) :
                greedySummaryCommon(fileName, std::move(K), std::move(depth),
                        std::move(systemStart), std::move(systemEnd), numFiles,
                        numBlocks, ingestTime){}
    };

private:
    /*
     * Private static methods
     */
    static string getFileName(const string& path);
    static inline bool canMigrate(double M, double epsilon,
            double originalSpace, double reclaimed, double reclaimAddition);
    static inline bool isSolution(double M, double epsilon, double moved);
    static bool isFailed(double m, double e, double moved);
    /*
     * Private methods
     */
    void initObjects(ifstream& input_csv);
    void initAllPairs();
    bool isFinalState(double moved, GreedyOutput& greedyOutput,
                      GreedySummaryUnique& greedySummaryUnique);
public:
    /*
     * Constructors
     */
    System(unsigned int filesArraySize, unsigned int blocksArraySize);
    System(string filePath, string depth,
                    string systemStart, string systemEnd,
                    string containerSize, string K);
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
    GreedyOutput greedy(System &target);
    void migrateVolume(System& target, int fileId, double& numMoved,
            double& numReplicated);
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
    string K;
    string depth;
    string systemStart;
    string systemEnd;
    string containerSize;
    /*
     * A File is a set of blocks from 0 to blocksArraySize
     * where file[i]==true indicates that file contains block i
     */
    //
    File** files = nullptr;
    // blocks[i] is the reference count of block i
    int* blocks = nullptr;
    double ingestTime = 0;
    vector<double> mVector = {10, 20, 25, 30, 33, 40, 50, 60};
    vector<double> epsilonVector = {1, 2, 5, 10, 15};
    set<pair<double, double>> unsolvedPairs;
    set<pair<double, double>> solvedPairs;
    set<pair<double, double>> failedPairs;
};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SYSTEM_H
