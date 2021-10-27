#include <iostream>
#include <ostream>
#include "../lib/cppzmq/zmq.hpp"

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "common.h"

#include "CaveSoundZMQ.h"

CaveSoundZMQ::CaveSoundZMQ()
        : startedAt(std::time(nullptr)) {

    void *extraDriverData = nullptr;

    system = nullptr;
    ERRCHECK(FMOD::Studio::System::create(&system));

    FMOD::System *coreSystem = nullptr;
    ERRCHECK(system->getCoreSystem(&coreSystem));
    ERRCHECK(coreSystem->setSoftwareFormat(12000, FMOD_SPEAKERMODE_7POINT1, 0));

    FMOD_RESULT result = system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, extraDriverData);
    if (result != FMOD_RESULT::FMOD_OK) {
        std::cerr << "system->initialize() returned " << result << " in " << __FILE__ << " on line " << __LINE__
                  << std::endl;
        std::cerr << "Exiting because ALSA failed." << std::endl;
        exit(1);
    }

    masterBank = nullptr;
    ERRCHECK(system->loadBankFile("media/fmod-banks/Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

    stringsBank = nullptr;
    ERRCHECK(system->loadBankFile("media/fmod-banks/Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

    int eventCount;
    masterBank->getEventCount(&eventCount);

    std::cout << "Found " << eventCount << " events." << std::endl << std::flush;

    fmodEvents = new FmodEvents(system);
    fmodEvents->addSingleEvent("explosion", "event:/Explosion/Explosion");
    fmodEvents->addSingleEvent("wooden-hatch", "event:/Luke/Auf");
    fmodEvents->addSingleEvent("pillar", "event:/Säule/Fahrt");
    fmodEvents->addSingleEvent("game-over", "event:/Verschütten/GameOver");
    fmodEvents->addSingleEvent("cave-in", "event:/Verschütten/Verschütten");

    fmodEvents->addContinuousEvent("drops", "event:/Wasser/Tropfen");
    fmodEvents->addContinuousEvent("stream", "event:/Wasser/Bach");
    fmodEvents->addContinuousEvent("countdown", "event:/Countdown/Steine");

    fmodEvents->startContinuousEvent("drops");

    std::cout << "FMOD should now be initialised." << std::endl << std::flush;
}

CaveSoundZMQ::~CaveSoundZMQ() {
    delete fmodEvents;

    ERRCHECK(stringsBank->unload());
    ERRCHECK(masterBank->unload());

    ERRCHECK(system->release());
}


std::string CaveSoundZMQ::process_request(std::string raw_request) {
    std::cout << "FMOD is processing the request" << std::endl << std::flush;

    std::string request;
    if (raw_request.back() == '\n') {
        request = raw_request.substr(0, raw_request.length() - 1);
    } else {
        request = raw_request;
    }

    std::string response = "OK\n";

    size_t pos = request.find(':');
    if (pos != std::string::npos) {
        std::string key = request.substr(0, pos);
        std::string value = request.substr(pos + 1);
        std::cout << "Request: key=" << key << ", value=" << value << std::endl;

        if (key == "get" && value == "id") {
            std::ostringstream stream;
            stream << "ID=" << startedAt;
            response = stream.str();
        } else if (key == "play") {
            bool found = fmodEvents->playSingleEvent(value);
            if (!found) response = "NOTFOUND";
        } else if (key == "start") {
            bool started = fmodEvents->startContinuousEvent(value);
            if (!started) response = "NOTFOUND";
        } else if (key == "stop") {
            bool stopped = fmodEvents->stopContinuousEvent(value);
            if (!stopped) response = "NOTFOUND";
        } else {
            response = "Error: Unknown key";
        }

    } else {
        std::cout << "Invalid request: >>" << request << "<<" << std::endl;
        response = "Error: Unknown request";
    }

    std::cout << "FMOD has processed the request." << std::endl << std::flush;

    return response;
}

int main() {
    CaveSoundZMQ caveSound;

    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::rep);
    sock.bind("tcp://127.0.0.1:3030");

    while (true) {
        zmq::message_t message;
        sock.recv(message, zmq::recv_flags::none);
        std::cout << "Received:" << message << std::endl;
        std::string result = caveSound.process_request(message.to_string());
        std::cout << result << std::endl;

        const char *cstr = result.c_str();
        zmq::message_t replyMessage = zmq::message_t(cstr, result.length());
        sock.send(replyMessage, zmq::send_flags::none);
    }
}
