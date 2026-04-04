#include "FmodController.h"

#include <iostream>
#include <sstream>
#include <utility>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"
#include "FmodException.h"

FmodController::FmodController() : FmodController(16000, FMOD_SPEAKERMODE_7POINT1, false, 0) {

}

FmodController::FmodController(const int sampleRate, const FMOD_SPEAKERMODE speakerMode, const bool enableLiveUpdate, const int rawSpeakerCount) {

    void *extraDriverData = nullptr;

    // Enable more debug output for FMOD
    FMOD_Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_TTY, nullptr, nullptr);

    system = nullptr;
    checkFmodResult(FMOD::Studio::System::create(&system));

    std::cout << "Common speaker mode IDs" << std::endl
            << "  " << FMOD_SPEAKERMODE_RAW << ": RAW" << std::endl
            << "  " << FMOD_SPEAKERMODE_STEREO << ": Stereo" << std::endl
            << "  " << FMOD_SPEAKERMODE_QUAD << ": Quad" << std::endl
            << "  " << FMOD_SPEAKERMODE_5POINT1 << ": 5.1" << std::endl
            << "  " << FMOD_SPEAKERMODE_7POINT1 << ": 7.1" << std::endl;

    std::cout << "Target setup for core system:" << std::endl
            << "· Speaker mode ID " << speakerMode << std::endl
            << "· Sample rate " << sampleRate << std::endl
            << "· Raw speakers " << rawSpeakerCount << std::endl
            << "· Live Update " << (enableLiveUpdate ? "enabled" : "disabled") << std::endl;

    // TODO Support raw speaker mode for special setups: https://www.fmod.com/resources/documentation-api?version=2.02&page=core-api-common.html#fmod_speakermode_raw
    checkFmodResult(system->getCoreSystem(&coreSystem));

    std::cout << "SETTING software format: Sample rate " << sampleRate
            << ", speaker mode " << speakerMode
            << ", raw speakers " << rawSpeakerCount << std::endl;
    checkFmodResult(coreSystem->setSoftwareFormat(sampleRate, speakerMode, rawSpeakerCount));

    // Output type can be set to ALSA, PulseAudio, etc. with
    // coreSystem->setOutput(FMOD_OUTPUTTYPE_PULSEAUDIO)

    std::cout << "Listing drivers, active driver marked with x" << std::endl;
    int activeDriver;
    coreSystem->getDriver(&activeDriver);
    int driverCount;
    coreSystem->getNumDrivers(&driverCount);
    for (int i = 0; i < driverCount; ++i) {
        char name[256];
        FMOD_GUID guid;
        int systemRate;
        FMOD_SPEAKERMODE driverSpeakerMode;
        int speakerModeChannels;
        coreSystem->getDriverInfo(i, name, sizeof(name), &guid, &systemRate, &driverSpeakerMode, &speakerModeChannels);

        if (i == activeDriver) {
            std::cout << "x ";
        } else {
            std::cout << "  ";
        }

        std::cout << "  Mode " << driverSpeakerMode << ", " << systemRate << " Hz, "
                << speakerModeChannels << " Channels: "
                << i << ": " << name
                << std::endl;
    }

    // Driver can be specified explicitly with
    // coreSystem->setDriver(X)

    int currentSampleRate;
    int currentRawSpeakers;
    FMOD_SPEAKERMODE currentSpeakerMode;
    coreSystem->getSoftwareFormat(&currentSampleRate, &currentSpeakerMode, &currentRawSpeakers);

    std::cout << "Current software format: "
            << "· Speaker mode " << currentSpeakerMode
            << "· Sample rate " << currentSampleRate
            << "· Raw speakers " << currentRawSpeakers
            << std::endl;

    std::cout << "INITIALISING system" << std::endl;

    const auto result = system->initialize(
        1024,
        enableLiveUpdate ? FMOD_STUDIO_INIT_LIVEUPDATE : FMOD_STUDIO_INIT_NORMAL,
        FMOD_INIT_NORMAL | (enableLiveUpdate ? (FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_PROFILE_METER_ALL) : 0),
        extraDriverData
    );
    if (result != FMOD_RESULT::FMOD_OK) {
        std::cerr << "system->initialize() returned " << result << " in " << __FILE__ << " on line " << __LINE__
                << std::endl;
        if (result == FMOD_ERR_OUTPUT_INIT) {
            std::cerr << "Error code indicates output init issue.";
        }
        std::cerr << "Exiting because ALSA failed." << std::endl;
        std::cerr << (FmodException("", result)).what() << std::endl;
        exit(1);
    } else {
        std::cout << "System init successful." << std::endl;
    }

    checkFmodResult(system->update());

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

FmodController::~FmodController() {
    for (const auto &[fst, snd]: _banksByPath) {
        checkFmodResultNothrow(snd->unload());
    }

    checkFmodResultNothrow(system->release());
}

void FmodController::setEventCallback(std::function<void(const std::string &, EventType)> callback) {
    _eventCallback = std::move(callback);
}

void FmodController::checkFmodResult(const FMOD_RESULT result) {
    if (result != FMOD_OK) {
        throw FmodException("Result not FMOD_OK", result);
    }
}

void FmodController::checkFmodResultNothrow(const FMOD_RESULT result) {
    try {
        checkFmodResult(result);
    } catch (FmodException &ex) {
        std::cerr << ex.what();
    }
}

std::string FmodController::loadBank(const std::string &bankPath) {
    FMOD::Studio::Bank *bank;
    const auto result = system->loadBankFile(bankPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
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
    const auto result = _banksByPath.find(bankPath);
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
    const auto eventDescription = loadEventDescription(eventId);
    FMOD::Studio::EventInstance *eventInstance = nullptr;

    std::cout << "Event: Playing " << eventId << std::endl;

    FMOD_RESULT result = eventDescription->createInstance(&eventInstance);
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
        FMOD_STUDIO_EVENT_CALLBACK_STARTED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED |
        FMOD_STUDIO_EVENT_CALLBACK_START_FAILED
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

    // Re-use an existing event instance, or create a new one
    if (const auto instanceResult = _eventInstancesById.find(eventId); instanceResult == _eventInstancesById.end()) {
        const auto eventDescription = loadEventDescription(eventId);

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
            FMOD_STUDIO_EVENT_CALLBACK_STARTED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED |
            FMOD_STUDIO_EVENT_CALLBACK_START_FAILED
        ));

        // Start it right now (system->update() still needs to be called!)
        checkFmodResult(eventInstance->start());

        checkFmodResult(system->update());

        return "OK";
    }
}

std::string FmodController::stopEvent(const std::string &eventId) {
    const auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        return "Event not running or does not exist";
    }

    std::cout << "Event: Stopping " << eventId << std::endl;
    const auto result = instance->second->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT);

    if (result != FMOD_OK) {
        throw FmodException("Could not stop event", result);
    }

    checkFmodResult(system->update());

    return "OK";
}

std::string FmodController::stopAllStartedEvents() const {
    for (const auto &[fst, snd]: _eventInstancesById) {
        if (const auto result = snd->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT); result != FMOD_OK) {
            std::cerr << "Could not stop event " << fst << ": " << result << std::endl;
        }
    }

    checkFmodResult(system->update());

    return "OK";
}

std::string FmodController::playVoice(const std::string &eventId, const std::string &voiceKey) {
    std::cout << "Event: Will play voice " << eventId << " with key " << voiceKey << std::endl;

    const auto eventDescription = loadEventDescription(eventId);
    std::cerr << "Event description is valid: " << eventDescription->isValid() << std::endl;

    FMOD::Studio::EventInstance *eventInstance = nullptr;

    std::cout << "Event: Playing voice " << eventId << " with key " << voiceKey << std::endl;

    FMOD_RESULT result = eventDescription->createInstance(&eventInstance);
    if (result != FMOD_OK) {
        const auto isValid = eventInstance->isValid();
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
    checkFmodResult(eventInstance->setCallback(runCheckedProgrammerSoundCallback,
                                               FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND |
                                               FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND |
                                               FMOD_STUDIO_EVENT_CALLBACK_STARTED |
                                               FMOD_STUDIO_EVENT_CALLBACK_STOPPED));

    std::cout << "Event instance configured for voice." << std::endl;

    checkFmodResult(eventInstance->start());
    checkFmodResult(system->update());

    return "OK";
}

FMOD_RESULT FmodController::programmerSoundCallback(const FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
                                                    FMOD_STUDIO_EVENTINSTANCE *event, void *parameters) {

    const auto *eventInstance = (FMOD::Studio::EventInstance *) event;

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
            checkFmodResult(
                programmerSoundContext->system->getSoundInfo(programmerSoundContext->dialogueString.c_str(), &info));

            FMOD::Sound *sound = nullptr;
            checkFmodResult(programmerSoundContext->coreSystem->createSound(
                info.name_or_data, FMOD_LOOP_NORMAL | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_NONBLOCKING | info.mode,
                &info.exinfo, &sound));

            // Pass the sound to FMOD
            props->sound = (FMOD_SOUND *) sound;
            props->subsoundIndex = info.subsoundindex;
        }
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) {
        const auto *props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES *) parameters;

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
        std::cout << "Event " << (context == nullptr ? "(unknown)" : context->eventId) << " FAILED to start" <<
                std::endl;
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
    }

    return FMOD_OK;
}

FMOD_RESULT FmodController::runCheckedProgrammerSoundCallback(const FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
                                                              FMOD_STUDIO_EVENTINSTANCE *event, void *parameters) {
    try {
        return programmerSoundCallback(type, event, parameters);
    } catch (FmodException &ex) {
        std::cerr << "Error in programmer sound callback: " << ex.what() << std::endl;
        return FMOD_ERR_BADCOMMAND;
    }
}


std::string FmodController::setParameter(const std::string &eventId, const std::string &parameterName, const float value) {
    const auto instance = _eventInstancesById.find(eventId);
    if (instance == _eventInstancesById.end()) {
        std::stringstream ss;
        ss << "Event not running or not existing, cannot set parameter " << parameterName << ".";
        return ss.str();
    }

    const auto result = instance->second->setParameterByName(parameterName.c_str(), value);
    if (result != FMOD_OK) {
        std::stringstream ss;
        ss << "Could not set parameter " << parameterName << ".";
        throw FmodException(ss.str(), result);
    }

    checkFmodResult(system->update());

    return "OK";
}

std::string FmodController::setGlobalParameter(const std::string &parameterName, const float value) {
    constexpr bool ignoreSeekSpeed = false;
    auto result = system->setParameterByName(parameterName.c_str(), value, ignoreSeekSpeed);
    if (result != FMOD_OK) {

        constexpr size_t maxParameters = 20;
        int parameterCount;
        FMOD_STUDIO_PARAMETER_DESCRIPTION params[maxParameters];
        result = system->getParameterDescriptionList(params, maxParameters, &parameterCount);

        std::stringstream ss;
        ss << "Could not set global parameter " << parameterName << ".";
        if (result == FMOD_OK) {
            ss << " Available global parameters: [";
            for (auto i = 0; i < parameterCount; i++) {
                ss << params[i].name << ", ";
            }
            ss << "].";
        }
        throw FmodException(ss.str(), result);
    }

    checkFmodResult(system->update());

    return "OK";
}

FMOD::Studio::EventDescription *FmodController::loadEventDescription(const std::string &eventId) {
    const auto description = _eventDescriptionsById.find(eventId);
    if (description == _eventDescriptionsById.end()) {

        FMOD::Studio::EventDescription *eventDescription;
        auto result = system->getEvent(eventId.c_str(), &eventDescription);

        if (result != FMOD_OK) {
            std::stringstream ss;
            ss << "Could not load event " << eventId << ".";
            throw FmodException(ss.str(), result);
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
    const auto instance = _eventInstancesById.find(eventId);
    if (instance != _eventInstancesById.end()) {
        checkFmodResult(instance->second->getPlaybackState(&state));
        const bool isPlaying = state == FMOD_STUDIO_PLAYBACK_PLAYING;
        std::cout << "Playback state of " << eventId << ": " << state << ", " << (isPlaying ? "playing" : "not playing")
                << std::endl
                << std::flush;
        return isPlaying;
    }
    return false;
}

std::vector<std::string> FmodController::getLoadedBankPaths() const {
    constexpr int maxBanks = 256;
    int loadedBanks;
    FMOD::Studio::Bank *banks[maxBanks];
    checkFmodResult(system->getBankList(banks, maxBanks, &loadedBanks));

    std::vector<std::string> bankPaths;
    for (int i = 0; i < loadedBanks; i++) {
        // Bank path is relative to project and starts with bank:/
        // Should not be very long
        constexpr int maxPathLength = 1024;
        int pathLength;
        char *path = new char[maxPathLength];
        banks[i]->getPath(path, maxPathLength, &pathLength);

        std::string str = path;
        bankPaths.push_back(str);
    }

    return bankPaths;
}
