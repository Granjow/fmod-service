#ifndef FMOD_SERVICE_ZMQAPI_H
#define FMOD_SERVICE_ZMQAPI_H

#include "FmodController.h"
#include <ctime>

class ZmqApi {

public:
    ZmqApi();
    explicit ZmqApi(const FmodController& fmodController);

    void run();

    void run(const std::string &socketAddress);

public:
    std::string process_request(std::string request);

    bool verbose = false;

private:
    std::time_t startedAt;
    FmodController fmodController;
};


#endif //FMOD_SERVICE_ZMQAPI_H
