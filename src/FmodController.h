#ifndef FMOD_SERVICE_FMODCONTROLLER_H
#define FMOD_SERVICE_FMODCONTROLLER_H

#include <string>
#include <map>
#include <functional>

#include "fmod_studio.hpp"

enum EventType {
    EventType_Started,
    EventType_Stopped,
};

struct BaseContext {
    std::string eventId;
    std::function<void(const std::string &eventId, EventType eventType)> eventCallback;
    std::function<void(const std::string &eventId, const std::string &markerName)> markerCallback;
};

struct ProgrammerSoundContext : BaseContext {
    FMOD::System *coreSystem;
    FMOD::Studio::System *system;
    std::string dialogueString;
};

enum EventInstanceType {
    EventInstanceType_Continuous,
    EventInstanceType_SingleShot,
    EventInstanceType_Voice,
};

struct EventInstanceData {
    FMOD::Studio::EventInstance *eventInstance;
    EventInstanceType eventInstanceType;

    EventInstanceData(FMOD::Studio::EventInstance *eventInstance, const EventInstanceType eventInstanceType)
        : eventInstance(eventInstance), eventInstanceType(eventInstanceType) {
    }
};

class FmodController {
public:
    FmodController();

    FmodController(int sampleRate, FMOD_SPEAKERMODE speakerMode, bool enableLiveUpdate, int rawSpeakerCount);

    ~FmodController();

    void setEventCallback(std::function<void(const std::string &eventId, EventType eventType)> callback);

    void setMarkerCallback(std::function<void(const std::string &eventId, const std::string &markerName)> callback);

    /*
     * To check for RAW mode:
     * - https://qa.fmod.com/t/macos-raw-speakermode-device/14908
     * - https://qa.fmod.com/t/linux-surround-sound-speaker-position-issue/13624/5
     * - is setMixMatrix required?
     * setSpeakerPosition: Use raw numbers instead of speaker enum.
     */
    /*
    void setSpeakerMode(FMOD_SPEAKERMODE speakerMode);

    void setSampleRate(int sampleRate);

    void setLiveUpdate(bool enabled, int port);
     */

public:
    std::string loadBank(const std::string &bankPath);

    std::string unloadBank(const std::string &bankPath);

    std::string startEvent(const std::string &eventId);

    std::string stopEvent(const std::string &eventId);

    [[nodiscard]] std::string stopAllStartedEvents();

    std::string playEvent(const std::string &eventId);

    std::string playVoice(const std::string &eventId, const std::string &voiceKey);

    std::string setParameter(const std::string &eventId, const std::string &parameterName, float value);

    std::string setGlobalParameter(const std::string &parameterName, float value);

    [[nodiscard]] std::vector<std::string> getLoadedBankPaths() const;

    bool isPlaying(const std::string &eventId);

private:
    static FMOD_RESULT programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event,
                                               void *parameters);

    static FMOD_RESULT runCheckedProgrammerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
                                                         FMOD_STUDIO_EVENTINSTANCE *event, void *parameters);

    static void checkFmodResult(FMOD_RESULT result);

    static void checkFmodResultNothrow(FMOD_RESULT result);

    [[nodiscard]]
    static std::string generateUniqueEventId(const std::string &eventId);

private:
    FMOD::Studio::System *system;
    FMOD::System *coreSystem = nullptr;

    FMOD::Studio::EventDescription *loadEventDescription(const std::string &eventId);

    std::map<std::string, FMOD::Studio::Bank *> _banksByPath;
    std::map<std::string, FMOD::Studio::EventDescription *> _eventDescriptionsById;
    std::map<std::string, EventInstanceData> _eventInstancesById;

    std::function<void(const std::string &eventId, EventType eventType)> _eventCallback;
    std::function<void(const std::string &eventId, const std::string &markerName)> _markerCallback;

private:
    void cleanUpEventInstances();
};


#endif //FMOD_SERVICE_FMODCONTROLLER_H
