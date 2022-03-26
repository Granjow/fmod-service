#ifndef FMOD_SERVICE_FMODCONTROLLER_H
#define FMOD_SERVICE_FMODCONTROLLER_H

#include <string>
#include <map>
#include <list>

#include "fmod_studio.hpp"


struct ProgrammerSoundContext {
    FMOD::System *coreSystem;
    FMOD::Studio::System *system;
    std::string dialogueString;
};

class FmodController {
public:
    FmodController();
    FmodController(int sampleRate, FMOD_SPEAKERMODE speakerMode, bool enableLiveUpdate);

    ~FmodController();

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

private:
    static void checkFmodResult(FMOD_RESULT result);

    static void checkFmodResultNothrow(FMOD_RESULT result);

    FMOD::Studio::System *system;
    FMOD::System *coreSystem = nullptr;

    ProgrammerSoundContext programmerSoundContext = {};

    FMOD::Studio::EventDescription *loadEventDescription(const std::string &eventId);

    std::map<std::string, FMOD::Studio::Bank *> _banksByPath;
    std::map<std::string, FMOD::Studio::EventDescription *> _eventDescriptionsById;
    std::map<std::string, FMOD::Studio::EventInstance *> _eventInstancesById;
};


#endif //FMOD_SERVICE_FMODCONTROLLER_H
