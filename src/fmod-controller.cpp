#include "fmod-controller.h"

#include <iostream>
#include <sstream>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

FmodController::FmodController() {

    void *extraDriverData = nullptr;

    system = nullptr;
    ERRCHECK(FMOD::Studio::System::create(&system));

    FMOD::System *coreSystem = nullptr;
    ERRCHECK(system->getCoreSystem(&coreSystem));
    ERRCHECK(coreSystem->setSoftwareFormat(12000, FMOD_SPEAKERMODE_7POINT1, 0));

    auto result = system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData);
    if (result != FMOD_RESULT::FMOD_OK) {
        std::cerr << "system->initialize() returned " << result << " in " << __FILE__ << " on line " << __LINE__
                  << std::endl;
        std::cerr << "Exiting because ALSA failed." << std::endl;
        exit(1);
    }

    ERRCHECK(system->update());

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

FmodController::~FmodController() {
    for (auto &bank: _banks) {
        ERRCHECK(bank->unload());
    }

    ERRCHECK(system->release());
}

std::string FmodController::loadBank(const std::string &bankPath) {
    FMOD::Studio::Bank *bank;
    auto result = system->loadBankFile(bankPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
    ERRCHECK(result);

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
        throw std::runtime_error("Cannot create event instance.");
    }

    // Start it right now (system->update() still needs to be called!)
    ERRCHECK(eventInstance->start());

    ERRCHECK(system->update());

    // Release will clean up the instance when it completes
    ERRCHECK(eventInstance->release());

    return "OK";
}

std::string FmodController::startEvent(const std::string &eventId) {
    FMOD::Studio::EventInstance *eventInstance = nullptr;

    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        auto eventDescription = loadEventDescription(eventId);


        ERRCHECK(eventDescription->createInstance(&eventInstance));
    } else {
        eventInstance = instance->second;
    }

    if (isPlaying(eventId)) {
        return "Already playing";
    } else {
        // Start it right now (system->update() still needs to be called!)
        ERRCHECK(eventInstance->start());
        return "OK";
    }
}

std::string FmodController::stopEvent(const std::string &eventId) {
    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        std::cout << "Event: Stopping " << eventId << std::endl;
        auto result = instance->second->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT);

        if (result != FMOD_OK) {
            throw std::runtime_error("Could not stop event");
        }

        return "OK";
    } else {
        return "Event not running or does not exist";
    }
}

FMOD::Studio::EventDescription *FmodController::loadEventDescription(const std::string &eventId) {
    auto description = _eventDescriptionsById.find(eventId);
    if (description == _eventDescriptionsById.end()) {

        FMOD::Studio::EventDescription *eventDescription;
        auto result = system->getEvent(eventId.c_str(), &eventDescription);

        if (result != FMOD_OK) {
            throw std::runtime_error("Could not load event");
        } else {
            _eventDescriptionsById.insert({eventId, eventDescription});

            // Start loading explosion sample data and keep it in memory
            result = eventDescription->loadSampleData();

            if (result != FMOD_OK) {
                throw std::runtime_error("Could not load sample data");
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
        ERRCHECK(instance->second->getPlaybackState(&state));
        bool isPlaying = state == FMOD_STUDIO_PLAYBACK_PLAYING;
        std::cout << "Playback state of " << eventId << ": " << state << ", " << (isPlaying ? "playing" : "not playing")
                  << std::endl
                  << std::flush;
        return isPlaying;
    }
    return false;
}


