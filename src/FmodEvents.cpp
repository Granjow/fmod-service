#include "FmodEvents.h"
#include <iostream>

FmodEvents::FmodEvents(FMOD::Studio::System *system) :
        system(system) {
}

void FmodEvents::addSingleEvent(const std::string &id, const std::string &path) {
    singleEvents.insert(std::make_pair(id, SingleFmodEvent(id, system, path.c_str())));
}

void FmodEvents::addContinuousEvent(const std::string &id, const std::string &path) {
    continuousEvents.insert(std::make_pair(id, ContinuousFmodEvent(id, path.c_str(), system)));
}

bool FmodEvents::playSingleEvent(const std::string &what) {
    if (singleEvents.find(what) != singleEvents.end()) {
        std::cout << "Event " << what << " found in the new list!" << std::endl;
        singleEvents.at(what).play();
        return true;
    } else {
        std::cout << "Event " << what << " not found." << std::endl;
    }
    return false;
}

bool FmodEvents::startContinuousEvent(const std::string &what) {
    if (continuousEvents.find(what) != continuousEvents.end()) {
        continuousEvents.at(what).start();
        return true;
    } else {
        std::cout << "Event " << what << " not found." << std::endl;
        return false;
    }
}

bool FmodEvents::stopContinuousEvent(const std::string &what) {
    if (continuousEvents.find(what) != continuousEvents.end()) {
        continuousEvents.at(what).stop();
        return true;
    } else {
        std::cout << "Event " << what << " not found." << std::endl;
        return false;
    }
}