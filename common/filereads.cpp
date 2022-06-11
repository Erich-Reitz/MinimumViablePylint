#include <vector>
#include <string>
#include <iostream>
#include <fstream>

std::vector<std::string> readLinesOfFile(std::ifstream &input) {
    std::vector<std::string> data;
    std::string line;
    while (std::getline(input, line)) {
        data.push_back(line);
    }
    return data;
}

int findFirstIndexOfString(const std::vector<std::string> &lines, const std::string &str) {
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].find(str) != std::string::npos) {
            return i;
        }
    }
    return -1;
}

int findFirstIndexOfStringAfterIndex(const std::vector<std::string> &lines, const std::string &str, int index) {
    for (int i = index + 1; i < lines.size(); i++) {
        if (lines[i].find(str) != std::string::npos) {
            return i;
        }
    }
    return -1;
}