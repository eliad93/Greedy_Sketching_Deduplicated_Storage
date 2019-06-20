//
// Created by eliad on 6/20/2019.
//

#ifndef GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H
#define GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H

#include <direct.h>

#include "System.h"
#include "ArgumentsParser.h"

class Simulator {
    static constexpr char const *summaryFileName = "summary_sketch.csv";
    static constexpr char const
    *summaryFileHeader = "File name,Deduplication level,K,Depth,System start,"
                         "System end,Number of files,Number of blocks,"
                         "M fraction,M,M fraction actual,M actual,"
                         "Epsilon fraction,Epsilon,Replication fraction,"
                         "Replication,Total time,Solver time,Ingest time,"
                         "Iterations";
    static constexpr char const
    *extendedOutputFileMiddle = "_optimization_solution_";

    vector<string> filesPaths;
    double M;
    double epsilon;

    static void addSummaryHeader(){
        assert(!fileExists(summaryFileName));
        std::ofstream summaryFile("summary_sketch.csv",
                                  std::ofstream::out | std::ofstream::app);
        summaryFile << summaryFileHeader << "\n";
    }

    static void exportSummary(System::GreedySummary& s){
        if(!fileExists(summaryFileName)){
            addSummaryHeader();
        }
        std::ofstream summaryFile(summaryFileName,
                                  std::ofstream::out | std::ofstream::app);
        if(summaryFile.bad()){
            cout << "Error opening " << summaryFileName << endl;
            return;
        }
        summaryFile << s.fileName << "," << s.dedupLevel << "," << s.K
                    << "," << s.depth  << "," << s.systemStart << ","
                    << s.systemEnd << "," << s.numFiles << "," << s.numBlocks
                    << "," << s.MFraction << "," << s.M << ","
                    << s.MFractionActual << "," << s.MActual << "," <<
                    s.epsilonFraction << "," << s.epsilon << ","
                    << s.replicationFactor << "," << s.replication << ","
                    << s.totalTime << "," << s.greedyTime << ","
                    << s.ingestTime << "," << s.numIterations << "\n";
        summaryFile.close();
    }

    static void exportOutput(System::GreedyOutput& output){
        System::GreedySummary& s = output.summary;
        exportSummary(s);
        exportExtendedOutput(output);
    }

    static string extendedOutputName(System::GreedyOutput& output){
        std::ostringstream MStrs;
        MStrs << output.summary.MFraction;
        string MStr = MStrs.str();
        std::ostringstream EStrs;
        EStrs << output.summary.epsilonFraction;
        string epsilonStr = EStrs.str();

        string fileName = output.summary.fileName +
                                        string(extendedOutputFileMiddle)
                                        + MStr + "_" + epsilonStr + ".csv";
        return fileName;
    }

    static void exportExtendedOutput(System::GreedyOutput& output){
        string fileName = extendedOutputName(output);
        std::ofstream file(fileName, std::ofstream::out | std::ofstream::trunc);
        if(file.bad()){
            cout << fileName << endl;
            return;
        }
        for(auto fileId: output.filesToMove) {
            file << fileId << " ";
        }
        file << "\n";
        for(auto& it: output.iterationsStats){
            file << it.iteration << "," << it.sourceSize << "," <<
            it.destinationSize << "," << it.moved << "," <<
            it.copied << "," << it.iterationTime << "\n";
        }
        file.close();
    }

    static bool fileExists(char const* filename) {
        std::ifstream ifile(filename);
        return (bool)ifile;
    }

    static void make_directory(char const* name){
    #ifdef __linux__
        mkdir(name, 777);
    #else
        _mkdir(name);
    #endif
    }

public:

    explicit Simulator(ArgumentsParser& argumentsParser):
    filesPaths(argumentsParser.getFilesPaths()),
    M(argumentsParser.getM()),
    epsilon(argumentsParser.getEpsilon()){}

    void run(){
        for(auto& path: filesPaths){
            System source(path);
            System target(source.getFilesArraySize(),
                    source.getBlocksArraySize());
            System::GreedyOutput output = source.greedy(target, M, epsilon);
            exportOutput(output);
        }
    }

};


#endif //GREEDY_SKETCHING_DEDUPLICATED_STORAGE_SIMULATOR_H
