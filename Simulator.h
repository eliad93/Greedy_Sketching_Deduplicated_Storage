//
// Created by eliad on 6/20/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H

#include <experimental/filesystem>
#include "System.h"
#include "ArgumentsParser.h"

class Simulator {

private:
    static constexpr char const *resultsDirName = "./results";
    static constexpr char const *summaryFileName = "summary_sketch.csv";
    static constexpr char const
    *summaryFileHeader = "File name,Deduplication level,K,Depth,System start,"
                         "System end,Number of files,Number of blocks,"
                         "M fraction,M,M fraction actual,M actual,"
                         "Epsilon fraction,Epsilon,Replication fraction,"
                         "Replication,Total time,Solver time,Ingest time,"
                         "Iterations,Solved\n";
    static constexpr char const
    *extendedOutputFileMiddle = "_optimization_solution_";
    static constexpr char const
            *extendedOutputHeader = "Iteration,Source size,Destination size,"
                                  "M fraction,Replication "
                                  "fraction,Iteration time,File\n";
    string filePath;
    string dedupLevel;
    string depth;
    string systemStart;
    string systemEnd;
    string containerSize;
    string K;
    /*
     * static private functions
     */
    static void createSummaryFile();
    static void exportSummary(System::GreedyOutput& o);
    static void exportOutput(System::GreedyOutput& o);
    static string extendedOutputName(System::GreedyOutput& o);
    static std::experimental::filesystem::path extendedOutputPath(System::GreedyOutput& o);
    static void exportExtendedOutput(System::GreedyOutput& o);
    static bool fileExists(std::experimental::filesystem::path& path);

public:
    /*
     * public constructors
     */
    explicit Simulator(ArgumentsParser& argumentsParser);
    /*
     * public methods
     */
    void run();

};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H
