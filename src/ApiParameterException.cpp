#include "ApiParameterException.h"

ApiParameterException::ApiParameterException(const std::string &message) {
    _message = "Parameter exception: " + message;
}

const char *ApiParameterException::what() {
    return _message.c_str();
}