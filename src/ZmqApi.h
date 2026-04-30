#ifndef FMOD_SERVICE_ZMQAPI_H
#define FMOD_SERVICE_ZMQAPI_H

#include "FmodController.h"
#include <ctime>
#include <deque>
#include <mutex>
#include <string>

class ZmqApi {

public:
    ZmqApi();
    explicit ZmqApi(const FmodController& fmodController);

    void run();

    void run(const std::string &repAddress);

    void run(const std::string &repAddress, const std::string &pubAddress);

public:
    std::string process_request(std::string request);

    bool verbose = false;

private:
    std::time_t startedAt;
    FmodController fmodController;

    std::deque<std::string> _pendingPublish;
    std::mutex _pendingMutex;
};


#endif //FMOD_SERVICE_ZMQAPI_H
