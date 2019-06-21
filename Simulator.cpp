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
    auto summaryFilePath = std::experimental::filesystem::path(summaryFileName);
    assert(!fileExists(summaryFilePath));
    std::ofstream summaryFile(summaryFilePath,
                              std::ofstream::out | std::ofstream::app);
    summaryFile << summaryFileHeader;
    summaryFile.close();
}

void Simulator::exportSummary(System::GreedySummary& s){
    if(!std::experimental::filesystem::is_directory(resultsDirName)){
        if(!std::experimental::filesystem::create_directory(resultsDirName)){
            cout << "Error creating results directory" << endl;
        }
    }
    auto summaryFilePath = std::experimental::filesystem::path(summaryFileName);
    if(!fileExists(summaryFilePath)){
        createSummaryFile();
    }
    std::ofstream summaryFile(summaryFilePath,
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

void Simulator::exportOutput(System::GreedyOutput& output){
    System::GreedySummary& s = output.summary;
    exportSummary(s);
    exportExtendedOutput(output);
}

string Simulator::extendedOutputName(System::GreedyOutput& output){
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

std::experimental::filesystem::path
Simulator::extendedOutputPath(System::GreedyOutput& output){
    std::experimental::filesystem::path path =
            std::experimental::filesystem::current_path();
    path.concat("/");
    path.concat(resultsDirName);
    path.concat("/");
    path.concat(extendedOutputName(output));
    return path;
}

void Simulator::exportExtendedOutput(System::GreedyOutput& output){
    auto dirPath = std::experimental::filesystem::path(resultsDirName);
    dirPath.concat("/");
    dirPath.concat(output.summary.fileName);
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
    for(auto fileId: output.filesToMove) {
        file << fileId << " ";
    }
    file << "\n";
    file << extendedOutputHeader;
    for(auto& it: output.iterationsStats){
        file << it.iteration << "," << it.sourceSize << "," <<
             it.destinationSize << "," << it.moved << "," <<
             it.copied << "," << it.iterationTime << "\n";
    }
    file.close();
}

bool Simulator::fileExists(std::experimental::filesystem::path& path) {
    std::ifstream file(path);
    return (bool)file;
}

Simulator::Simulator(ArgumentsParser& argumentsParser):
filesPaths(argumentsParser.getFilesPaths()),
M(argumentsParser.getM()),
epsilon(argumentsParser.getEpsilon()){}

void Simulator::run(){
    for(auto& path: filesPaths){
        System source(path);
        System target(source.getFilesArraySize(),
                      source.getBlocksArraySize());
        System::GreedyOutput output = source.greedy(target, M, epsilon);
        exportOutput(output);
    }
}