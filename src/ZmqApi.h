#ifndef FMOD_SERVICE_ZMQAPI_H
#define FMOD_SERVICE_ZMQAPI_H

#include "fmod-controller.h"

class ZmqApi {

public:
    ZmqApi();

public:
    std::string process_request(std::string request);

private:
    std::time_t startedAt;
    FmodController fmodController;
};


#endif //FMOD_SERVICE_ZMQAPI_H
