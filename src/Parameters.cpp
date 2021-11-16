#include "Parameters.h"
#include "ApiParameterException.h"

std::vector<std::string> Parameters::parse(const std::string &value, int expectedParameterCount) {
    std::vector<std::string> items;
    size_t lastPos = 0;
    size_t newPos;
    while ((newPos = value.find(';', lastPos)) != std::string::npos) {
        items.push_back(value.substr(lastPos, newPos - lastPos));
        lastPos = newPos + 1;
    }
    if (lastPos <= value.size()) {
        items.push_back(value.substr(lastPos));
    }

    if (items.size() != expectedParameterCount) {
        throw ApiParameterException("Expected other number of arguments");
    }

    return items;
}