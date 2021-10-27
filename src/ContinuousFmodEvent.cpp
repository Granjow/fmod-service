#include <common.h>
#include <iostream>
#include <utility>
#include "ContinuousFmodEvent.h"

ContinuousFmodEvent::ContinuousFmodEvent(std::string id, const char *path, FMOD::Studio::System *system) :
        id(std::move(id)), system(system), eventDescription(nullptr), eventInstance(nullptr) {

    // Load the event from the studiobank
    ERRCHECK(system->getEvent(path, &eventDescription));

    // Load the sample data into memory
    ERRCHECK(eventDescription->loadSampleData());

    // Event has a single instance which is playing or stopped
    ERRCHECK(eventDescription->createInstance(&eventInstance));
}

bool ContinuousFmodEvent::isPlaying() {
    FMOD_STUDIO_PLAYBACK_STATE state;
    ERRCHECK(eventInstance->getPlaybackState(&state));
    bool isPlaying = state == FMOD_STUDIO_PLAYBACK_PLAYING;
    std::cout << "Playback state: " << state << ", " << (isPlaying ? "playing" : "not playing") << std::endl
              << std::flush;
    return isPlaying;
}

bool ContinuousFmodEvent::start() {

    bool playing = isPlaying();

    if (playing) {

        std::cout << "Event " << id << " already playing." << std::endl << std::flush;

    } else {

        // Start the event
        ERRCHECK(eventInstance->start());

        // Send FMOD an update so the event is played
        ERRCHECK(system->update());

        std::cout << "Event " << id << " started." << std::endl << std::flush;

    }

    return !playing;
}

bool ContinuousFmodEvent::stop() {

    bool playing = isPlaying();

    if (playing) {

        // Stop the event with fade-out
        ERRCHECK(eventInstance->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT));

        // Send FMOD an update so the event is played
        ERRCHECK(system->update());

        std::cout << "Event " << id << "stopped." << std::endl;

    } else {
        std::cout << "Event " << id << " already stopped." << std::endl;
    }

    return playing;
}
