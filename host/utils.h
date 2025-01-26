
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <string>

#define coutv(v) for(auto it:v)std::cout<<it<<' ';std::cout<<std::endl;

void sortChunkToFile(std::vector<int>& chunk, const std::string& filename);
void mergeFiles(const std::string& file1, const std::string& file2, const std::string& mergedFile);
void externalMergeSort(const std::string& filename);
std::string splitFile(const std::string& inputFilePath);
