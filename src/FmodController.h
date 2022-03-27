#ifndef FMOD_SERVICE_FMODCONTROLLER_H
#define FMOD_SERVICE_FMODCONTROLLER_H

#include <string>
#include <map>
#include <list>
#include <functional>

#include "fmod_studio.hpp"

enum EventType {
    EventType_Started,
    EventType_Stopped,
};

struct BaseContext {
    std::string eventId;
    std::function<void(const std::string &eventId, EventType eventType)> eventCallback;
};

struct ProgrammerSoundContext : BaseContext {
    FMOD::System *coreSystem;
    FMOD::Studio::System *system;
    std::string dialogueString;
};

class FmodController {
public:
    FmodController();

    FmodController(int sampleRate, FMOD_SPEAKERMODE speakerMode, bool enableLiveUpdate);

    ~FmodController();

    void setEventCallback(std::function<void(const std::string &eventId, EventType eventType)> callback);

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

    std::string playEvent(const std::string &eventId);

    std::string playVoice(const std::string &eventId, const std::string &voiceKey);

    std::string setParameter(const std::string &eventId, const std::string &parameterName, float value);

    bool isPlaying(const std::string &eventId);


private:
    static FMOD_RESULT F_CALLBACK programmerSoundCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters);

    static void checkFmodResult(FMOD_RESULT result);

    static void checkFmodResultNothrow(FMOD_RESULT result);

private:
    FMOD::Studio::System *system;
    FMOD::System *coreSystem = nullptr;

    FMOD::Studio::EventDescription *loadEventDescription(const std::string &eventId);

    std::map<std::string, FMOD::Studio::Bank *> _banksByPath;
    std::map<std::string, FMOD::Studio::EventDescription *> _eventDescriptionsById;
    std::map<std::string, FMOD::Studio::EventInstance *> _eventInstancesById;

    std::function<void(const std::string &eventId, EventType eventType)> _eventCallback;
};


#endif //FMOD_SERVICE_FMODCONTROLLER_H
