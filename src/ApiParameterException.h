#ifndef FMOD_SERVICE_APIPARAMETEREXCEPTION_H
#define FMOD_SERVICE_APIPARAMETEREXCEPTION_H

#include <string>

class ApiParameterException : public std::exception {
public:
    explicit ApiParameterException(const std::string &message);

    const char *what();

private:
    std::string _message;

};

#endif //FMOD_SERVICE_APIPARAMETEREXCEPTION_H
