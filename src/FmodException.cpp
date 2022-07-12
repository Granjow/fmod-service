#include "FmodException.h"

#include <sstream>

FmodException::FmodException(const std::string &message, FMOD_RESULT result) : _result(result) {
    std::stringstream ss;
    ss << "FMOD Exception " << result << ": " << message << ". ";
    switch (result) {
        case FMOD_ERR_EVENT_ALREADY_LOADED:
            ss << "Event/Bank is loaded already.";
            break;
        case FMOD_ERR_EVENT_NOTFOUND:
            ss << "Event does not exist.";
            break;
        case FMOD_ERR_FILE_NOTFOUND:
            ss << "File does not exist.";
            break;
        case FMOD_ERR_INVALID_HANDLE:
            ss << "Invalid handle, e.g. using event description from unloaded bank.";
            break;
        case FMOD_ERR_INVALID_PARAM:
            ss << "Invalid parameter, does not exist. Maybe you need to check “Exposed recursively via event instruments” on the event if it is local.";
            break;
        case FMOD_ERR_OUTPUT_INIT:
            ss << "Could not initialise sound output.";
            break;
        default:
            break;
    }
    _message = ss.str();
}

const char *FmodException::what() {
    return _message.c_str();
}
