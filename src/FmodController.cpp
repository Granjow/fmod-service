#include "FmodController.h"

#include <iostream>
#include <sstream>
#include <utility>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include "FmodException.h"

FmodController::FmodController() : FmodController(16000, FMOD_SPEAKERMODE_7POINT1, false) {

}

FmodController::FmodController(int sampleRate, FMOD_SPEAKERMODE speakerMode, bool enableLiveUpdate) {

    void *extraDriverData = nullptr;

    system = nullptr;
    checkFmodResult(FMOD::Studio::System::create(&system));

    // TODO Support raw speaker mode for special setups: https://www.fmod.com/resources/documentation-api?version=2.02&page=core-api-common.html#fmod_speakermode_raw
    checkFmodResult(system->getCoreSystem(&coreSystem));
    checkFmodResult(coreSystem->setSoftwareFormat(sampleRate, speakerMode, 0));

    auto result = system->initialize(1024, enableLiveUpdate ? FMOD_STUDIO_INIT_LIVEUPDATE : FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData);
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
    for (auto &bank: _banksByPath) {
        checkFmodResultNothrow(bank.second->unload());
    }

    checkFmodResultNothrow(system->release());
}

void FmodController::setEventCallback(std::function<void(const std::string &, EventType)> callback) {
    _eventCallback = std::move(callback);
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

    _banksByPath.insert({bankPath, bank});

    int eventCount;
    bank->getEventCount(&eventCount);
    std::cout << "Found " << eventCount << " events in bank " << bankPath << "." << std::endl << std::flush;

    return "OK";
}

std::string FmodController::unloadBank(const std::string &bankPath) {
    auto result = _banksByPath.find(bankPath);
    if (result == _banksByPath.end()) {
        std::stringstream msg;
        msg << "Error unloading bank " << bankPath << ", not listed as loaded in map." << std::endl;
        return msg.str();
    }

    checkFmodResult(result->second->unload());

    _banksByPath.erase(bankPath);

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

    // Add context info to the event to allow informing about its state (played/stopped)
    auto *context = new BaseContext;
    context->eventId = eventId;
    context->eventCallback = _eventCallback;
    checkFmodResult(eventInstance->setUserData(context));
    checkFmodResult(eventInstance->setCallback(
            programmerSoundCallback,
            FMOD_STUDIO_EVENT_CALLBACK_STARTED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED | FMOD_STUDIO_EVENT_CALLBACK_START_FAILED
    ));

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

        // Add context info to the event to allow informing about its state (played/stopped)
        auto *context = new BaseContext;
        context->eventId = eventId;
        context->eventCallback = _eventCallback;
        checkFmodResult(eventInstance->setUserData(context));
        checkFmodResult(eventInstance->setCallback(
                programmerSoundCallback,
                FMOD_STUDIO_EVENT_CALLBACK_STARTED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED | FMOD_STUDIO_EVENT_CALLBACK_START_FAILED
        ));

        // Start it right now (system->update() still needs to be called!)
        checkFmodResult(eventInstance->start());

        checkFmodResult(system->update());

        return "OK";
    }
}

std::string FmodController::stopEvent(const std::string &eventId) {
    auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        return "Event not running or does not exist";
    }

    std::cout << "Event: Stopping " << eventId << std::endl;
    auto result = instance->second->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT);

    if (result != FMOD_OK) {
        throw FmodException("Could not stop event", result);
    }

    checkFmodResult(system->update());

    return "OK";
}

std::string FmodController::playVoice(const std::string &eventId, const std::string &voiceKey) {
    std::cout << "Event: Will play voice " << eventId << " with key " << voiceKey << std::endl;

    auto eventDescription = loadEventDescription(eventId);
    std::cerr << "Event description is valid: " << eventDescription->isValid() << std::endl;

    FMOD::Studio::EventInstance *eventInstance = nullptr;

    std::cout << "Event: Playing voice " << eventId << " with key " << voiceKey << std::endl;

    FMOD_RESULT result;
    result = eventDescription->createInstance(&eventInstance);
    if (result != FMOD_OK) {
        auto isValid = eventInstance->isValid();
        std::cerr << "Event instance is valid: " << isValid << std::endl;
        throw FmodException("Cannot create event instance.", result);
    }

    // Add context info to the event to allow informing about its state (played/stopped)
    // Also,
    auto *context = new ProgrammerSoundContext();
    context->eventId = eventId;
    context->eventCallback = _eventCallback;
    context->system = system;
    context->coreSystem = coreSystem;
    context->dialogueString = voiceKey;
    checkFmodResult(eventInstance->setUserData(context));
    checkFmodResult(eventInstance->setCallback(programmerSoundCallback,
                                               FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND | FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND | FMOD_STUDIO_EVENT_CALLBACK_STARTED |
                                               FMOD_STUDIO_EVENT_CALLBACK_STOPPED));

    std::cout << "Event instance configured for voice." << std::endl;

    checkFmodResult(eventInstance->start());
    checkFmodResult(system->update());

    return "OK";
}

FMOD_RESULT FmodController::programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters) {

    auto *eventInstance = (FMOD::Studio::EventInstance *) event;

    BaseContext *context = nullptr;
    checkFmodResult(eventInstance->getUserData((void **) &context));

    if (type == FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND) {
        auto *props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES *) parameters;

        // Get our context from the event instance user data
        ProgrammerSoundContext *programmerSoundContext = nullptr;
        checkFmodResult(eventInstance->getUserData((void **) &programmerSoundContext));

        if (programmerSoundContext == nullptr) {
            // No context was defined for this event; cannot play sound for it
            std::cerr << "Event does not have user data!" << std::endl;
        } else {

            // Find the audio file in the audio table with the key
            FMOD_STUDIO_SOUND_INFO info;
            checkFmodResult(programmerSoundContext->system->getSoundInfo(programmerSoundContext->dialogueString.c_str(), &info));

            FMOD::Sound *sound = nullptr;
            checkFmodResult(programmerSoundContext->coreSystem->createSound(info.name_or_data, FMOD_LOOP_NORMAL | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_NONBLOCKING | info.mode, &info.exinfo, &sound));

            // Pass the sound to FMOD
            props->sound = (FMOD_SOUND *) sound;
            props->subsoundIndex = info.subsoundindex;
        }
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) {
        auto *props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES *) parameters;

        // Obtain the sound
        auto *sound = (FMOD::Sound *) props->sound;

        // Release the sound
        checkFmodResult(sound->release());


    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_STARTED) {
        std::cout << "Event " << (context == nullptr ? "(unknown)" : context->eventId) << " STARTED" << std::endl;
        if (context != nullptr && context->eventCallback != nullptr) {
            context->eventCallback(context->eventId, EventType_Started);
        }

    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED) {
        std::cout << "Event " << (context == nullptr ? "(unknown)" : context->eventId) << " STOPPED" << std::endl;
        if (context != nullptr && context->eventCallback != nullptr) {
            context->eventCallback(context->eventId, EventType_Stopped);
        }
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_START_FAILED) {
        std::cout << "Event " << (context == nullptr ? "(unknown)" : context->eventId) << " FAILED to start" << std::endl;
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
    }

    return FMOD_OK;
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

std::vector<std::string> FmodController::getLoadedBankPaths() const {
    int maxBanks = 256;
    int loadedBanks;
    FMOD::Studio::Bank *banks[maxBanks];
    checkFmodResult(system->getBankList(banks, maxBanks, &loadedBanks));

    std::vector<std::string> bankPaths;
    for (int i = 0; i < loadedBanks; i++) {
        // Bank path is relative to project and starts with bank:/
        // Should not be very long
        int maxPathLength = 1024;
        int pathLength;
        char *path = new char[maxPathLength];
        banks[i]->getPath(path, maxPathLength, &pathLength);

        std::string str = path;
        bankPaths.push_back(str);
    }

    return bankPaths;
}
