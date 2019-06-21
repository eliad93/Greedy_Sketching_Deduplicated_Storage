//
// Created by eliad on 6/20/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H

//#include <direct.h>
#include <windows.h>
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
                         "Iterations\n";
    static constexpr char const
    *extendedOutputFileMiddle = "_optimization_solution_";
    static constexpr char const
            *extendedOutputHeader = "Iteration,Soutrce size,Destination size,"
                                  "M factor,Replication "
                                  "factor,Iteration time\n";
    vector<string> filesPaths;
    double M;
    double epsilon;
    /*
     * static private functions
     */
    static void createSummaryFile();
    static void exportSummary(System::GreedySummary& s);
    static void exportOutput(System::GreedyOutput& output);
    static string extendedOutputName(System::GreedyOutput& output);
    static std::experimental::filesystem::path extendedOutputPath(System::GreedyOutput& output);
    static void exportExtendedOutput(System::GreedyOutput& output);
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
