//
// Created by eliad on 6/20/2019.
//

#include "Simulator.h"

constexpr char const *Simulator::resultsDirName;
constexpr char const *Simulator::summaryFileName;
constexpr char const *Simulator::summaryFileHeader;
constexpr char const *Simulator::extendedOutputFileMiddle;
constexpr char const *Simulator::extendedOutputHeader;

void Simulator::createSummaryFile(){
    auto summaryFilePath = std::experimental::filesystem::path(resultsDirName);
    summaryFilePath.concat("/");
    summaryFilePath.concat(summaryFileName);
    assert(!fileExists(summaryFilePath));
    std::ofstream summaryFile(summaryFilePath,
                              std::ofstream::out | std::ofstream::app);
    summaryFile << summaryFileHeader;
    summaryFile.close();
}

void Simulator::exportSummary(System::GreedyOutput& o){
    if(!std::experimental::filesystem::is_directory(resultsDirName)){
        if(!std::experimental::filesystem::create_directory(resultsDirName)){
            cout << "Error creating results directory" << endl;
        }
    }
    auto summaryFilePath = std::experimental::filesystem::path(resultsDirName);
    summaryFilePath.concat("/");
    summaryFilePath.concat(summaryFileName);
    if(!fileExists(summaryFilePath)){
        createSummaryFile();
    }
    std::ofstream summaryFile(summaryFilePath,
    std::ofstream::out | std::ofstream::app);
    if(summaryFile.bad()){
        cout << "Error opening " << summaryFileName << endl;
        return;
    }
    for(auto& iter: o.summariesMap){
        System::GreedySummaryUnique su = iter.second;
        summaryFile << o.greedySummaryCommon.fileName << ","
        << o.greedySummaryCommon.deduplicationLevel << "," << o.greedySummaryCommon.K
        << "," << o.greedySummaryCommon.depth  << ","
        << o.greedySummaryCommon.systemStart << ","
        << o.greedySummaryCommon.systemEnd << ","
        << o.greedySummaryCommon.numFiles << ","
        << o.greedySummaryCommon.numBlocks << "," << su.MFraction << ","
        << su.M << "," << su.MFractionActual << "," << su.MActual << "," <<
        su.epsilonFraction << "," << su.epsilon << "," << su.replicationFraction
        << "," << su.replication << "," << su.totalTime << "," << su.greedyTime
        << "," << o.greedySummaryCommon.ingestTime << "," << su.numIterations
        << "," << su.solved << "\n";
    }
    summaryFile.close();
}

void Simulator::exportOutput(System::GreedyOutput& output){
    exportSummary(output);
    exportExtendedOutput(output);
}

string Simulator::extendedOutputName(System::GreedyOutput& output){
    string fileName = output.greedySummaryCommon.fileName +
                      string(extendedOutputFileMiddle) + ".csv";
    return fileName;
}

void Simulator::exportExtendedOutput(System::GreedyOutput& output){
    auto dirPath = std::experimental::filesystem::path(resultsDirName);
    dirPath.concat("/");
    dirPath.concat(output.greedySummaryCommon.fileName);
    if(!std::experimental::filesystem::is_directory(dirPath)){
        if(!std::experimental::filesystem::create_directory(dirPath)){
            cout << "Error creating " << dirPath.string() << endl;
        }
    }
    std::experimental::filesystem::path filePath = dirPath;
    filePath.concat("/");
    filePath.concat(extendedOutputName(output));
    std::ofstream file(filePath, std::ofstream::out | std::ofstream::trunc);
    if(file.bad()){
        cout << "Error opening " << extendedOutputName(output) << endl;
        return;
    }
    file << extendedOutputHeader;
    for(auto& it: output.iterationsStats){
        file << it.iteration << "," << it.sourceSize << "," <<
             it.destinationSize << "," << it.moved << "," <<
             it.copied << "," << it.iterationTime << "," << it.fileId << "\n";
    }
    file.close();
}

bool Simulator::fileExists(std::experimental::filesystem::path& path) {
    std::ifstream file(path);
    return (bool)file;
}

Simulator::Simulator(ArgumentsParser& argumentsParser) :
        filePath(argumentsParser.getFilePath()),
        deduplicationLevel(argumentsParser.getDeduplicationLevel()),
        depth(argumentsParser.getDepth()),
        systemStart(argumentsParser.getSystemStart()),
        systemEnd(argumentsParser.getSystemEnd()),
        containerSize(argumentsParser.getContainerSize()),
        K(argumentsParser.getK()){}

void Simulator::run(){
    System source(filePath, depth, systemStart, systemEnd, containerSize, K);
    System target(source.getFilesArraySize(),
                  source.getBlocksArraySize());
    System::GreedyOutput output = source.greedy(target);
    exportOutput(output);
}