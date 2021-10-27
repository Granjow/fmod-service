#ifndef FMOD_SOCKET_FMODSERVER_H
#define FMOD_SOCKET_FMODSERVER_H

#include "unix-server.h"

#include <string>

#include "fmod_studio.hpp"

class FmodServer : public UnixServer {

public:
    FmodServer();

    ~FmodServer();

protected:
    virtual std::string process_request(std::string request);

private:
    FMOD::Studio::System *system;
    FMOD::Studio::Bank *masterBank;
    FMOD::Studio::Bank *stringsBank;

    FMOD::Studio::EventDescription *dropsDescription;
    FMOD::Studio::EventDescription *countdownDescription;
    FMOD::Studio::EventDescription *streamDescription;

    FMOD::Studio::EventDescription *woodenHatchDescription;
    FMOD::Studio::EventDescription *pillarDescription;
    FMOD::Studio::EventDescription *explosionDescription;
    FMOD::Studio::EventDescription *caveInDescription;
    FMOD::Studio::EventDescription *gameOverDescription;

    FMOD::Studio::EventInstance *dropsInstance;
    FMOD::Studio::EventInstance *streamInstance;
    FMOD::Studio::EventInstance *countdownInstance;

    /**
     * Play a one-shot event
     * @param eventDescription Event description to play
     */
    void play_event(FMOD::Studio::EventDescription *eventDescription, std::string what);

    /**
     * Start playing an event which will be stopped later on
     * @param eventInstance
     */
    void start_event(FMOD::Studio::EventInstance *eventInstance, std::string what);

    /**
     * Stop an event which is playing
     * @param eventInstance
     */
    void stop_event(FMOD::Studio::EventInstance *eventInstance, std::string what);

    bool is_playing(FMOD::Studio::EventInstance *eventInstance);

};


#endif //FMOD_SOCKET_FMODSERVER_H
