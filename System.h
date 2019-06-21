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

        GreedyIterationStats() = default;
    };
    /*
     * Used for organizing summarized greedy
     * information for a specific run
     */
    struct GreedySummary {
        string fileName; // input file
        char dedupLevel{};
        unsigned int K{};
        string depth{};// last char of second token
        string systemStart{}; // system start - 3'rd token
        string systemEnd{}; // system end - 4'th token
        unsigned int numFiles{}; // num files in source
        unsigned int numBlocks{};
        double MFraction{}; // % blocks to move from source to target
        double M{}; // num blocks to move from source to target
        double MFractionActual{};
        double MActual{};
        double epsilonFraction{}; // the input epsilon
        double epsilon{}; // num blocks Epsilon represents
        double replicationFactor{}; // num blocks copied
        double replication{}; // % blocks copied
        double totalTime{};
        double greedyTime{};
        double ingestTime{};
        unsigned int numIterations{};
        explicit GreedySummary(string& fileName, double M, double _epsilon,
                unsigned int numFiles, unsigned int numBlocks):
        fileName(fileName),
        M(M/100.0*(double)numBlocks), MFraction(M),
        epsilon(_epsilon/100.0*(double)numBlocks), epsilonFraction(_epsilon){
            vector<string> tokens = splitFileName();
            dedupLevel = tokens[0][0];
            K = 1;
            depth = *(tokens[2].begin()+5);
            systemStart = tokens[3];
            systemEnd = tokens[4];
        }

        vector<string> splitFileName(){
            vector<string> tokens;
            stringstream ss(fileName);
            string token;
            while(std::getline(ss, token, '_')){
                tokens.emplace_back(token);
            }
            return tokens;
        }
    };

    struct GreedyOutput {
        GreedySummary summary;
        list<GreedyIterationStats> iterationsStats;
        vector<int> filesToMove;
        explicit GreedyOutput(string fileName, double M, double epsilon,
                unsigned int numFiles, unsigned int numBlocks) :
        summary(fileName, M, epsilon, numFiles, numBlocks){}
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
    GreedyOutput greedy(System &target, double M, double epsilon);
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
