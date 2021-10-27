#include "fmod-server.h"
#include <iostream>
#include <sstream>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

FmodServer::FmodServer() {

    void *extraDriverData = nullptr;

    system = nullptr;
    ERRCHECK(FMOD::Studio::System::create(&system));

    // The example Studio project is authored for 5.1 sound, so set up the system output mode to match
    FMOD::System *coreSystem = nullptr;
    ERRCHECK(system->getCoreSystem(&coreSystem));
    ERRCHECK(coreSystem->setSoftwareFormat(12000, FMOD_SPEAKERMODE_7POINT1, 0));

    ERRCHECK(system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData));

    ERRCHECK(system->update());

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

FmodServer::~FmodServer() {
    ERRCHECK(system->release());

    for (auto &bank: _banks) {
        ERRCHECK(bank->unload());
    }
}

std::string FmodServer::loadBank(const std::string &bankPath) {
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

std::string FmodServer::playEvent(const std::string &eventId) {
    auto eventDescription = loadEventDescription(eventId);
    FMOD::Studio::EventInstance *eventInstance = nullptr;

    std::cout << "Event: Playing " << eventId << std::endl;

    ERRCHECK(eventDescription->createInstance(&eventInstance));

    // Start it right now (system->update() still needs to be called!)
    ERRCHECK(eventInstance->start());

    // Release will clean up the instance when it completes
    ERRCHECK(eventInstance->release());

    return "OK";
}

std::string FmodServer::startEvent(const std::string &eventId) {
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

std::string FmodServer::stopEvent(const std::string &eventId) {
    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        std::cout << "Event: Stopping " << eventId << std::endl;
        auto result = instance->second->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT);
        ERRCHECK(result);

        if (result != FMOD_OK) {
            return "Could not stop event";
        } else {
            return "OK";
        }
    } else {
        return "Event not running or does not exist";
    }
}

FMOD::Studio::EventDescription *FmodServer::loadEventDescription(const string &eventId) {
    auto description = _eventDescriptionsById.find(eventId);
    if (description == _eventDescriptionsById.end()) {

        FMOD::Studio::EventDescription *eventDescription;
        auto result = system->getEvent(eventId.c_str(), &eventDescription);
        ERRCHECK(result);

        if (result != FMOD_OK) {
            throw runtime_error("Could not load event");
        } else {
            _eventDescriptionsById.insert({eventId, eventDescription});

            // Start loading explosion sample data and keep it in memory
            ERRCHECK(eventDescription->loadSampleData());

            return eventDescription;
        }
    }

    return description->second;
}

std::string FmodServer::process_request(std::string raw_request) {
    std::cout << "FMOD is processing the request" << std::endl << std::flush;

    string request;
    if (raw_request.back() == '\n') {
        request = raw_request.substr(0, raw_request.length() - 1);
    } else {
        request = raw_request;
    }

    string response = "OK\n";
    if (request == "stop:drops") {
        // TODO do stuff
    } else {
        std::cout << "Unknown request: >>" << request << "<<" << std::endl;
        response = "ERROR: Unknown request\n";
    }

    // Submit command buffer
    ERRCHECK(system->update());

    std::cout << "FMOD has processed the request." << std::endl << std::flush;

    return response;
}

bool FmodServer::isPlaying(const std::string &eventId) {
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

int main() {
    FmodServer server;
    server.run();
}
