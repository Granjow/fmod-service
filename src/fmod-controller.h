#ifndef FMOD_SERVICE_FMODCONTROLLER_H
#define FMOD_SERVICE_FMODCONTROLLER_H

#include <string>
#include <map>
#include <list>

#include "fmod_studio.hpp"

class FmodController {
public:
    FmodController();

    ~FmodController();

    std::string loadBank(const std::string &bankPath);

    std::string startEvent(const std::string &eventId);

    std::string stopEvent(const std::string &eventId);

    std::string playEvent(const std::string &eventId);

    bool isPlaying(const std::string &eventId);


private:
    FMOD::Studio::System *system;

    FMOD::Studio::EventDescription *loadEventDescription(const std::string &eventId);

    std::list<FMOD::Studio::Bank *> _banks;
    std::map<std::string, FMOD::Studio::EventDescription *> _eventDescriptionsById;
    std::map<std::string, FMOD::Studio::EventInstance *> _eventInstancesById;
};


#endif //FMOD_SERVICE_FMODCONTROLLER_H
