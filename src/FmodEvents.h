#ifndef CAVE_SOUND_FMODEVENTS_H
#define CAVE_SOUND_FMODEVENTS_H

#include <string>
#include <map>
#include "fmod_studio.hpp"
#include "SingleFmodEvent.h"
#include "ContinuousFmodEvent.h"

class FmodEvents {

public:
    explicit FmodEvents(FMOD::Studio::System *system);

    void addSingleEvent(const std::string &id, const std::string &path);

    void addContinuousEvent(const std::string &id, const std::string &path);

    bool playSingleEvent(const std::string &what);

    bool startContinuousEvent(const std::string &what);

    bool stopContinuousEvent(const std::string &what);

private:
    FMOD::Studio::System *system;
    std::map<std::string, SingleFmodEvent> singleEvents;
    std::map<std::string, ContinuousFmodEvent> continuousEvents;

};


#endif //CAVE_SOUND_FMODEVENTS_H
