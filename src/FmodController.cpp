#include "FmodController.h"

#include <iostream>
#include <sstream>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include "FmodException.h"

FmodController::FmodController() {

    void *extraDriverData = nullptr;

    system = nullptr;
    checkFmodResult(FMOD::Studio::System::create(&system));

    FMOD::System *coreSystem = nullptr;
    checkFmodResult(system->getCoreSystem(&coreSystem));
    checkFmodResult(coreSystem->setSoftwareFormat(12000, FMOD_SPEAKERMODE_7POINT1, 0));

    auto result = system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData);
    if (result != FMOD_RESULT::FMOD_OK) {
        std::cerr << "system->initialize() returned " << result << " in " << __FILE__ << " on line " << __LINE__
                  << std::endl;
        std::cerr << "Exiting because ALSA failed." << std::endl;
        exit(1);
    }

    checkFmodResult(system->update());

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

FmodController::~FmodController() {
    for (auto &bank: _banks) {
        checkFmodResultNothrow(bank->unload());
    }

    checkFmodResultNothrow(system->release());
}

void FmodController::checkFmodResult(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        throw FmodException("Result not FMOD_OK", result);
    }
}

void FmodController::checkFmodResultNothrow(FMOD_RESULT result) {
    try {
        checkFmodResult(result);
    } catch (FmodException &ex) {
        std::cerr << ex.what();
    }
}

std::string FmodController::loadBank(const std::string &bankPath) {
    FMOD::Studio::Bank *bank;
    auto result = system->loadBankFile(bankPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
    checkFmodResult(result);

    if (result != FMOD_OK) {
        std::stringstream msg;
        msg << "Error loading bank " << bankPath << ". FMOD return code: " << result << std::endl;
        return msg.str();
    }

    _banks.push_back(bank);

    int eventCount;
    bank->getEventCount(&eventCount);
    std::cout << "Found " << eventCount << " events in bank " << bankPath << "." << std::endl << std::flush;

    return "OK";
}

std::string FmodController::playEvent(const std::string &eventId) {
    auto eventDescription = loadEventDescription(eventId);
    FMOD::Studio::EventInstance *eventInstance = nullptr;

    std::cout << "Event: Playing " << eventId << std::endl;

    FMOD_RESULT result;
    result = eventDescription->createInstance(&eventInstance);
    if (result != FMOD_OK) {
        throw FmodException("Cannot create event instance.", result);
    }

    // Start it right now (system->update() still needs to be called!)
    checkFmodResult(eventInstance->start());

    checkFmodResult(system->update());

    // Release will clean up the instance when it completes
    checkFmodResult(eventInstance->release());

    return "OK";
}

std::string FmodController::startEvent(const std::string &eventId) {
    FMOD::Studio::EventInstance *eventInstance = nullptr;

    auto instanceResult = _eventInstancesById.find(eventId);
    if (instanceResult == _eventInstancesById.end()) {
        auto eventDescription = loadEventDescription(eventId);

        checkFmodResult(eventDescription->createInstance(&eventInstance));
        _eventInstancesById.insert(std::make_pair(eventId, eventInstance));
    } else {
        eventInstance = instanceResult->second;
    }

    if (isPlaying(eventId)) {
        return "Already playing";
    } else {
        // Start it right now (system->update() still needs to be called!)
        checkFmodResult(eventInstance->start());

        checkFmodResult(system->update());

        return "OK";
    }
}

std::string FmodController::stopEvent(const std::string &eventId) {
    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        std::cout << "Event: Stopping " << eventId << std::endl;
        auto result = instance->second->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT);

        if (result != FMOD_OK) {
            throw FmodException("Could not stop event", result);
        }

        checkFmodResult(system->update());

        return "OK";
    } else {
        return "Event not running or does not exist";
    }
}

std::string FmodController::setParameter(const std::string &eventId, const std::string &parameterName, float value) {
    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        return "Event not running or not existing, cannot set parameter";
    }

    auto result = instance->second->setParameterByName(parameterName.c_str(), value);
    if (result != FMOD_OK) {
        throw FmodException("Could not set parameter", result);
    }

    checkFmodResult(system->update());

    return "OK";
}

FMOD::Studio::EventDescription *FmodController::loadEventDescription(const std::string &eventId) {
    auto description = _eventDescriptionsById.find(eventId);
    if (description == _eventDescriptionsById.end()) {

        FMOD::Studio::EventDescription *eventDescription;
        auto result = system->getEvent(eventId.c_str(), &eventDescription);

        if (result != FMOD_OK) {
            throw FmodException("Could not load event", result);
        } else {
            _eventDescriptionsById.insert({eventId, eventDescription});

            // Start loading explosion sample data and keep it in memory
            result = eventDescription->loadSampleData();

            if (result != FMOD_OK) {
                throw FmodException("Could not load sample data", result);
            }

            return eventDescription;
        }
    }

    return description->second;
}

bool FmodController::isPlaying(const std::string &eventId) {
    FMOD_STUDIO_PLAYBACK_STATE state;
    auto instance = _eventInstancesById.find(eventId);
    if (instance != _eventInstancesById.end()) {
        checkFmodResult(instance->second->getPlaybackState(&state));
        bool isPlaying = state == FMOD_STUDIO_PLAYBACK_PLAYING;
        std::cout << "Playback state of " << eventId << ": " << state << ", " << (isPlaying ? "playing" : "not playing")
                  << std::endl
                  << std::flush;
        return isPlaying;
    }
    return false;
}


