#ifndef FMOD_SOCKET_FMODSERVER_H
#define FMOD_SOCKET_FMODSERVER_H

#include "unix-server.h"

#include <string>
#include <map>
#include <list>

#include "fmod_studio.hpp"

class FmodServer : public UnixServer {

public:
    FmodServer();

    ~FmodServer();

    std::string loadBank(const std::string &bankPath);

    std::string startEvent(const std::string &eventId);

    std::string stopEvent(const std::string &eventId);

    std::string playEvent(const std::string &eventId);

    bool isPlaying(const std::string &eventId);

    FMOD::Studio::EventDescription *loadEventDescription(const std::__cxx11::basic_string<char> &eventId);

protected:
    virtual std::string process_request(std::string request);

private:
    FMOD::Studio::System *system;

    std::list<FMOD::Studio::Bank *> _banks;
    std::map<std::string, FMOD::Studio::EventDescription *> _eventDescriptionsById;
    std::map<std::string, FMOD::Studio::EventInstance *> _eventInstancesById;

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
