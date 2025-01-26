
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <string>
#include <sstream>
#include <iterator>

#include "utils.h"

// memory space: N / CHUNK_SIZE + CHUNK_SIZE

const int CHUNK_SIZE = 32768;

void sortChunkToFile(std::vector<int>& chunk, const std::string& filename) {
    std::sort(chunk.begin(), chunk.end());
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    for (int num : chunk) {
        outFile << num << ' ';
    }
}

void mergeFiles(const std::string& file1, const std::string& file2, const std::string& mergedFile) {
    std::ifstream in1(file1), in2(file2);
    std::ofstream out(mergedFile);
    if (!in1 || !in2 || !out) {
        std::cerr << "Error opening files for merging.\n";
        return;
    }

    int num1, num2;
    bool hasNum1 = (in1 >> num1) ? true : false;
    bool hasNum2 = (in2 >> num2) ? true : false;

    while (hasNum1 && hasNum2) {
        if (num1 < num2) {
            out << num1 << ' ';
            hasNum1 = (in1 >> num1) ? true : false;
        } else {
            out << num2 << ' ';
            hasNum2 = (in2 >> num2) ? true : false;
        }
    }

    while (hasNum1) {
        out << num1 << ' ';
        hasNum1 = (in1 >> num1) ? true : false;
    }

    while (hasNum2) {
        out << num2 << ' ';
        hasNum2 = (in2 >> num2) ? true : false;
    }
}

void externalMergeSort(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Error opening input file.\n";
        return;
    }

    std::vector<std::string> tempFiles;
    int chunkIndex = 0;
    while (!input.eof()) {
        std::vector<int> chunk;
        int num;
        while (chunk.size() < CHUNK_SIZE && input >> num) {
            chunk.push_back(num);
        }

        if (!chunk.empty()) {
            std::string tempFilename = "chunk_" + std::to_string(chunkIndex++) + ".txt";
            sortChunkToFile(chunk, tempFilename);
            tempFiles.push_back(tempFilename);
        }
    }
    input.close();

    int chunt_iter = 0;
    while (tempFiles.size() > 1) {
        std::vector<std::string> newTempFiles;
        for (size_t i = 0; i < tempFiles.size(); i += 2) {
            if (i + 1 < tempFiles.size()) {
                std::string mergedFile = "merged_chunk_" + std::to_string(chunt_iter++) + ".txt";
                mergeFiles(tempFiles[i], tempFiles[i + 1], mergedFile);
                newTempFiles.push_back(mergedFile);
                std::remove(tempFiles[i].c_str());
                std::remove(tempFiles[i + 1].c_str());
            } else {
                newTempFiles.push_back(tempFiles[i]);
            }
        }
        tempFiles = newTempFiles;
    }

    if (!tempFiles.empty()) {
        std::rename(tempFiles[0].c_str(), filename.c_str());
        std::remove(tempFiles[0].c_str());
    }
}

std::string splitFile(const std::string& inputFilePath) {
    std::ifstream inputFile(inputFilePath);

    std::vector<int32_t> numbers;
    int32_t number;
    
    while (inputFile >> number) {
        numbers.push_back(number);
    }
    
    inputFile.close();
    
    size_t totalNumbers = numbers.size();
    size_t half = totalNumbers / 2;
    
    std::ofstream outputFile(inputFilePath, std::ios::trunc);
    std::copy(numbers.begin(), numbers.begin() + half, std::ostream_iterator<int32_t>(outputFile, " "));
    outputFile.close();

    std::string randomFileName("greater_part.tmp");

    std::ofstream newFile(randomFileName);
    std::copy(numbers.begin() + half, numbers.end(), std::ostream_iterator<int32_t>(newFile, " "));
    newFile.close();

    return randomFileName;
}