#ifndef CAVE_SOUND_CAVESOUNDZMQ_H
#define CAVE_SOUND_CAVESOUNDZMQ_H

#include <string>
#include <map>
#include <ctime>
#include "fmod_studio.hpp"
#include "SingleFmodEvent.h"
#include "FmodEvents.h"

class CaveSoundZMQ {

public:
    CaveSoundZMQ();

    ~CaveSoundZMQ();


public:
    std::string process_request(std::string request);

private:
    std::time_t startedAt;

    FMOD::Studio::System *system;
    FMOD::Studio::Bank *masterBank;
    FMOD::Studio::Bank *stringsBank;

    FmodEvents *fmodEvents;

};


#endif //CAVE_SOUND_CAVESOUNDZMQ_H
