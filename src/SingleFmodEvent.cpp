#include <iostream>
#include <utility>
#include "SingleFmodEvent.h"
#include "common.h"

SingleFmodEvent::SingleFmodEvent(std::string id, FMOD::Studio::System *system, const char *path)
        : _id(std::move(id)), system(system), _path(path), eventDescription(nullptr) {
    ERRCHECK(system->getEvent(path, &eventDescription));
    ERRCHECK(eventDescription->loadSampleData());

}

std::string SingleFmodEvent::id() {
    return _id;
}

void SingleFmodEvent::play() {
    std::cout << "Event: Playing " << _id << std::endl;

    FMOD::Studio::EventInstance *eventInstance = nullptr;
    ERRCHECK(eventDescription->createInstance(&eventInstance));

    // Start it right now (system->update() still needs to be called!)
    ERRCHECK(eventInstance->start());

    // Release will clean up the instance when it completes
    ERRCHECK(eventInstance->release());

    // Send FMOD an update so the event is played
    ERRCHECK(system->update());
}