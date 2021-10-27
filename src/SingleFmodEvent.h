#ifndef CAVE_SOUND_SINGLEFMODEVENT_H
#define CAVE_SOUND_SINGLEFMODEVENT_H

#include <string>
#include "fmod_studio.hpp"

class SingleFmodEvent {

public:
    SingleFmodEvent(std::string id, FMOD::Studio::System *system, const char *path);

    std::string id();

    void play();

private:
    std::string _id;
    std::string _path;

    FMOD::Studio::System *system;
    FMOD::Studio::EventDescription *eventDescription;
};


#endif //CAVE_SOUND_SINGLEFMODEVENT_H
