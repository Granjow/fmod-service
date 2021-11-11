#ifndef FMOD_SERVICE_FMODEXCEPTION_H
#define FMOD_SERVICE_FMODEXCEPTION_H

#include <string>
#include <exception>
#include "fmod_studio.hpp"

class FmodException : public std::exception {
    const char* file;
    int line;
    const char* func;
    const char* info;

public:
    FmodException(const std::string& message, FMOD_RESULT result);

    const char *what();

private:
    std::string _message;
    FMOD_RESULT _result;
};

#endif //FMOD_SERVICE_FMODEXCEPTION_H
