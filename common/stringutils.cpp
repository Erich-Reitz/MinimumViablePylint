#include <string>
#include <vector>

std::string convertToSpaceSeperatedString(char **array, int startingIndex) {
    std::string str = "";
    for (int i = startingIndex; array[i] != nullptr; i++) {
        str += array[i];
        str += " ";
    }
    return str.substr(0, str.size() - 1);
}

std::vector<std::string> splitStringIntoLines(const std::string &str) {
    std::vector<std::string> lines;
    std::string line;
    for (char c : str) {
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        } else {
            line += c;
        }
    }
    return lines;
}