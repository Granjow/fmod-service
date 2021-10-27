#ifndef CAVE_SOUND_CONTINUOUSFMODEVENT_H
#define CAVE_SOUND_CONTINUOUSFMODEVENT_H

#include <string>

#include "fmod_studio.hpp"

class ContinuousFmodEvent {

public:
    ContinuousFmodEvent(std::string id, const char *path, FMOD::Studio::System *system);

    /**
     * Start the event. After that, it will be running until stopped.
     * @return false, if the event was playing already.
     */
    bool start();

    /**
     * Stops the event when it is playing.
     * @return false, if the event was stopped already
     */
    bool stop();

    bool isPlaying();

private:
    std::string id;

    FMOD::Studio::System *system;
    FMOD::Studio::EventDescription *eventDescription;
    FMOD::Studio::EventInstance *eventInstance;

};


#endif //CAVE_SOUND_CONTINUOUSFMODEVENT_H
