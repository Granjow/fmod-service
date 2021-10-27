#include "fmod-server.h"
#include <iostream>

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

FmodServer::FmodServer() {

    void *extraDriverData = NULL;

    system = NULL;
    ERRCHECK(FMOD::Studio::System::create(&system));

    // The example Studio project is authored for 5.1 sound, so set up the system output mode to match
    FMOD::System *coreSystem = NULL;
    ERRCHECK(system->getCoreSystem(&coreSystem));
    ERRCHECK(coreSystem->setSoftwareFormat(12000, FMOD_SPEAKERMODE_7POINT1, 0));

    ERRCHECK(system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData));

    masterBank = NULL;
    ERRCHECK(system->loadBankFile("media/fmod-banks/Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

    stringsBank = NULL;
    ERRCHECK(system->loadBankFile("media/fmod-banks/Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

    int eventCount;
    masterBank->getEventCount(&eventCount);

    std::cout << "Found " << eventCount << " events." << std::endl << std::flush;

    // [s] = Single Event, [c] = Continuous
    // [s] Explosion/Explosion
    // [c] Wasser/Tropfen
    // [c] Wasser/Bach
    // [s] Säule/Fahrt
    // [c] Countdown/Steine
    // [s] Verschütten/Verschütten
    // [s] Verschütten/GameOver

    dropsDescription = NULL;
    ERRCHECK(system->getEvent("event:/Wasser/Tropfen", &dropsDescription));

    explosionDescription = NULL;
    ERRCHECK(system->getEvent("event:/Explosion/Explosion", &explosionDescription));

    streamDescription = NULL;
    ERRCHECK(system->getEvent("event:/Wasser/Bach", &streamDescription));

    woodenHatchDescription = NULL;
    ERRCHECK(system->getEvent("event:/Luke/Auf", &woodenHatchDescription));

    pillarDescription = NULL;
    ERRCHECK(system->getEvent("event:/Säule/Fahrt", &pillarDescription));

    countdownDescription = NULL;
    ERRCHECK(system->getEvent("event:/Countdown/Steine", &countdownDescription));

    caveInDescription = NULL;
    ERRCHECK(system->getEvent("event:/Verschütten/Verschütten", &caveInDescription));

    gameOverDescription = NULL;
    ERRCHECK(system->getEvent("event:/Verschütten/GameOver", &gameOverDescription));

    dropsInstance = NULL;
    ERRCHECK(dropsDescription->createInstance(&dropsInstance));

    streamInstance = NULL;
    ERRCHECK(streamDescription->createInstance(&streamInstance));

    countdownInstance = NULL;
    ERRCHECK(countdownDescription->createInstance(&countdownInstance));

    // Start loading explosion sample data and keep it in memory
    ERRCHECK(explosionDescription->loadSampleData());
    ERRCHECK(dropsDescription->loadSampleData());
    ERRCHECK(caveInDescription->loadSampleData());
    ERRCHECK(gameOverDescription->loadSampleData());
    ERRCHECK(countdownDescription->loadSampleData());
    ERRCHECK(streamDescription->loadSampleData());
    ERRCHECK(pillarDescription->loadSampleData());
    ERRCHECK(woodenHatchDescription->loadSampleData());

    ERRCHECK(dropsInstance->start());
    ERRCHECK(system->update());

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

FmodServer::~FmodServer() {
    ERRCHECK(stringsBank->unload());
    ERRCHECK(masterBank->unload());

    ERRCHECK(system->release());
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
        stop_event(dropsInstance, "Drops");
    } else if (request == "start:drops") {
        start_event(dropsInstance, "Drops");
    } else if (request == "start:countdown") {
        start_event(countdownInstance, "Countdown");
    } else if (request == "stop:countdown") {
        stop_event(countdownInstance, "Countdown");
    } else if (request == "start:stream") {
        start_event(streamInstance, "Stream");
    } else if (request == "stop:stream") {
        stop_event(streamInstance, "Stream");
    } else if (request == "play:explosion") {
        play_event(explosionDescription, "Explosion");
    } else if (request == "play:wooden-hatch") {
        play_event(woodenHatchDescription, "Wooden Hinch");
    } else if (request == "play:pillar") {
        play_event(pillarDescription, "Pillar");
    } else if (request == "play:cave-in") {
        play_event(caveInDescription, "Cave-in");
    } else if (request == "play:game-over") {
        play_event(gameOverDescription, "Game Over");
    } else {
        std::cout << "Unknown request: >>" << request << "<<" << std::endl;
        response = "ERROR: Unknown request\n";
    }

    // Submit command buffer
    ERRCHECK(system->update());

    std::cout << "FMOD has processed the request." << std::endl << std::flush;

    return response;
}

void FmodServer::play_event(FMOD::Studio::EventDescription *eventDescription, std::string what) {

    std::cout << "Event: Playing " << what << std::endl;

    FMOD::Studio::EventInstance *eventInstance = NULL;
    ERRCHECK(eventDescription->createInstance(&eventInstance));

    // Start it right now (system->update() still needs to be called!)
    ERRCHECK(eventInstance->start());

    // Release will clean up the instance when it completes
    ERRCHECK(eventInstance->release());
}

void FmodServer::start_event(FMOD::Studio::EventInstance *eventInstance, std::string what) {
    bool playing = is_playing(eventInstance);
    if (playing) {
        std::cout << "Event " << what << " already playing." << std::endl << std::flush;
    } else {
        std::cout << "Event " << what << " not playing yet, starting" << std::endl << std::flush;
        ERRCHECK(eventInstance->start());
    }
}

void FmodServer::stop_event(FMOD::Studio::EventInstance *eventInstance, std::string what) {
    std::cout << "Event: Stopping " << what << std::endl;
    ERRCHECK(eventInstance->stop(FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT));
}

bool FmodServer::is_playing(FMOD::Studio::EventInstance *eventInstance) {
    FMOD_STUDIO_PLAYBACK_STATE state;
    ERRCHECK(eventInstance->getPlaybackState(&state));
    bool isPlaying = state == FMOD_STUDIO_PLAYBACK_PLAYING;
    std::cout << "Playback state: " << state << ", " << (isPlaying ? "playing" : "not playing") << std::endl
              << std::flush;
    return isPlaying;
}

int main() {
    FmodServer server;
    server.run();
}
