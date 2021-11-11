#include "FmodException.h"

#include <sstream>

FmodException::FmodException(const std::string &message, FMOD_RESULT result) : _result(result) {
    std::stringstream ss;
    ss << "FMOD Exception " << result << ": " << message << ". ";
    switch (result) {
        case FMOD_ERR_EVENT_ALREADY_LOADED:
            ss << "Event is loaded already.";
            break;
        case FMOD_ERR_EVENT_NOTFOUND:
            ss << "Event does not exist.";
            break;
        case FMOD_ERR_FILE_NOTFOUND:
            ss << "File does not exist.";
            break;
        default:
            break;
    }
    _message = ss.str();
}

const char *FmodException::what() {
    return _message.c_str();
}
