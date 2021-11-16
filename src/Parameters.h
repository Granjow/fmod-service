#ifndef FMOD_SERVICE_PARAMETERS_H
#define FMOD_SERVICE_PARAMETERS_H

#include <string>
#include <vector>

class Parameters {
public:
    static std::vector<std::string> parse(const std::string& input, int expectedParameterCount);
};


#endif //FMOD_SERVICE_PARAMETERS_H
