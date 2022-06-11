
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <algorithm>

#include "common/filereads.cpp"
#include "common/stringutils.cpp"
#include "common/systemutils.cpp"




class PylError {
  public:
    PylError(const std::string &msg) : errorCodeName(msg) {}

    bool operator==(const PylError &other) const {
        return errorCodeName == other.errorCodeName;
    }

    bool operator<(const PylError &other) const {
        return errorCodeName < other.errorCodeName;
    }


    const std::string errorCodeName;
};

bool validErrorCodeName(const std::string &errorCodeName) {
    if (errorCodeName.length() == 0) {
        return false;
    }
    bool valid = false;
    for (int i = 0; i < errorCodeName.length(); i++) {
        if (errorCodeName[i] == '-') {
            valid = true;
        } else if (!isalpha(errorCodeName[i])) {
            return false;
        }
    }
    return valid;
}

std::string parseErrorCodeName(const std::string &error) {
    // std::cout << error << std::endl;
    std::vector<std::string> paraPairs;
    for (int i = 0; i < error.length(); i++) {
        if (error[i] == '(') {
            int j = i + 1;
            while (error[j] != ')') {
                j++;
            }
            std::string paraPair = error.substr(i + 1, j - i - 1);
            paraPairs.push_back(paraPair);
        }
    }

    for (auto &paraPair : paraPairs) {
        if (validErrorCodeName(paraPair)) {
            return paraPair;
        }
    }

    return "FLAG_NOT_FOUND";
}

std::set<PylError> parse(const std::vector<std::string> lines) {
    std::set<PylError> errors;
    for (auto &line : lines) {
        const std::string errorCodeName = parseErrorCodeName(line);
        if (errorCodeName != "FLAG_NOT_FOUND") {
            PylError error(errorCodeName);
            if (errors.find(error) == errors.end()) {
                errors.insert(error);
            }
        }
    }
    return errors;
}

int findIndexOfMessagesControlSection(const std::vector<std::string> &lines) {
    return findFirstIndexOfString(lines, "[MESSAGES CONTROL]");
}

int findEndOfMessagesControlSection(const std::vector<std::string> &lines) {
    int startIndex = findIndexOfMessagesControlSection(lines);
    if (startIndex == -1) {
        return -1;
    }
    return findFirstIndexOfStringAfterIndex(lines, "[", startIndex);
}


std::vector<std::string> getMessagesControlSection(const std::vector<std::string> &lines) {
    std::vector<std::string> messagesControlSection;
    // open file
    int index = findIndexOfMessagesControlSection(lines);
    if (index == -1) {
        return std::vector<std::string>();
    }
    
    messagesControlSection.push_back(lines[index]);
    messagesControlSection.push_back("\n");
    for (int i = index + 1; i < lines.size(); i++) {
        if (lines[i].find("[") != std::string::npos) {
            break;
        }
        if (lines[i].length() > 0) {
            messagesControlSection.push_back(lines[i]);
        }
    }
    return messagesControlSection;
}


std::vector<std::string> getPylintrcLines() {
    std::ifstream file(".pylintrc");
    if (!file.is_open()) {
        std::cerr << "Could not open .pylintrc" << std::endl;
        exit(1);
    }
    return readLinesOfFile(file);
}

std::vector<std::string> linesOfVectorBeforeIndex(const std::vector<std::string> &lines, int index) {
    return std::vector<std::string>(lines.begin(), lines.begin() + index);
}

std::vector<std::string> linesOfVectorAfterIndex(const std::vector<std::string> &lines, int index) {
    return std::vector<std::string>(lines.begin() + index, lines.end());
}

std::vector<std::string> beforeMsgControl(const std::vector<std::string> &lines) {
    int index = findIndexOfMessagesControlSection(lines);
    if (index == -1) {
        return lines;
    }
    return linesOfVectorBeforeIndex(lines, index);
}

std::vector<std::string> afterMsgControl(const std::vector<std::string> &lines) {
    int index = findEndOfMessagesControlSection(lines);
    if (index == -1) {
        return lines;
    }
    return linesOfVectorAfterIndex(lines, index);
}

void addErrorsToMsgControlSection(std::vector<std::string> &msgControlSection, const std::set<PylError> &errors) {
    for (auto &error : errors) {
        msgControlSection.push_back("    " + error.errorCodeName + ",");
    }
}

void writePylintrcFile(const std::vector<std::string> &lines) {
    std::ofstream file(".pylintrc");
    if (!file.is_open()) {
        std::cerr << "Could not open .pylintrc" << std::endl;
        exit(1);
    }
    for (auto &line : lines) {
        file << line << std::endl;
    }
}

void createNewPylintRcFile(const std::vector<std::string> &fileContents, std::set<PylError> &errors) {
    std::vector<std::string> beforeMessagesControlSection = beforeMsgControl(fileContents);
    std::vector<std::string> messagesControlSection = getMessagesControlSection(fileContents);
    std::vector<std::string> afterMessagesControlSection = afterMsgControl(fileContents);

    // add errors to messages control section
    addErrorsToMsgControlSection(messagesControlSection, errors);
    // write messages control section back to file
    std::vector<std::string> newFileContents = beforeMessagesControlSection;
    newFileContents.insert(newFileContents.end(), messagesControlSection.begin(), messagesControlSection.end());
    newFileContents.insert(newFileContents.end(), afterMessagesControlSection.begin(), afterMessagesControlSection.end());
    writePylintrcFile(newFileContents);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <command>" << std::endl;
        return 1;
    }
    std::string command = convertToSpaceSeperatedString(argv, 1);
    std::cout << command << std::endl;
    std::set<PylError> errors = parse(splitStringIntoLines(exec(command)));
    std::cout << "Found " << errors.size() << " errors" << std::endl;
    std::vector<std::string> fileContents = getPylintrcLines();
    
    createNewPylintRcFile(fileContents, errors);

    return 0;
}