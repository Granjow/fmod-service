#include <chrono>
#include <iostream>
#include <ostream>
#include "../lib/cppzmq/zmq.hpp"

#include "common.h"

#include "ZmqApi.h"
#include "FmodException.h"
#include "ApiParameterException.h"
#include "Parameters.h"

ZmqApi::ZmqApi(const FmodController &fmodController) : fmodController(fmodController), startedAt(std::time(nullptr)) {
}

ZmqApi::ZmqApi()
    : ZmqApi(FmodController()) {
}

std::string ZmqApi::process_request(std::string raw_request) {
    verbose && std::cout << "Processing request: " << raw_request << std::endl << std::flush;

    std::string request;
    if (raw_request.back() == '\n') {
        request = raw_request.substr(0, raw_request.length() - 1);
    } else {
        request = raw_request;
    }

    std::stringstream response;
    std::string key = request;
    std::string value;

    size_t pos = request.find(':');
    if (pos != std::string::npos) {
        key = request.substr(0, pos);
        value = request.substr(pos + 1);
    }

    // TODO Add ID as metadata in requests to detect reconnects/restarts quickly and let the client load banks again
    try {
        if (key == "get" && value == "id") {
            response << "ID=" << startedAt;
        } else if (key == "load-bank") {
            auto result = fmodController.loadBank(value);
            response << result;
        } else if (key == "unload-bank") {
            auto result = fmodController.unloadBank(value);
            response << result;
        } else if (key == "play-event") {
            response << fmodController.playEvent(value);
        } else if (key == "start-event") {
            response << fmodController.startEvent(value);
        } else if (key == "stop-event") {
            response << fmodController.stopEvent(value);
        } else if (key == "stop-started-events") {
            response << fmodController.stopAllStartedEvents();
        } else if (key == "set-parameter") {
            // Value format: eventId;parameterName;parameterValue
            auto params = Parameters::parse(value, 3);

            std::string eventId = params[0];
            std::string parameterName = params[1];
            float parameterValue = std::stof(params[2]);

            verbose && std::cout << "Setting " << eventId << " param " << parameterName << " to " << parameterValue
                    << std::endl;

            if (eventId == "global") {
                response << fmodController.setGlobalParameter(parameterName, parameterValue);
            } else {
                response << fmodController.setParameter(eventId, parameterName, parameterValue);
            }
        } else if (key == "play-voice") {
            // Value format: eventId;voiceKey
            auto params = Parameters::parse(value, 2);

            std::string eventId = params[0];
            std::string voiceKey = params[1];

            verbose && std::cout << "Starting event " << eventId << " with programmer instrument key " << voiceKey <<
                    std::endl;
            response << fmodController.playVoice(eventId, voiceKey);
        } else if (key == "list-bank-paths") {
            auto bankList = fmodController.getLoadedBankPaths();
            for (auto &path: bankList) {
                response << path << ";";
            }
        } else {
            response << "Error: Unknown key";
        }
    } catch (FmodException &err) {
        response << "Error: " << err.what();
    } catch (ApiParameterException &err) {
        response << "Error: " << err.what();
    }

    if (response.tellp() == 0) {
        response << "OK";
    }


    verbose && std::cout << "Processed " << raw_request << std::endl << std::flush;

    return response.str();
}

void ZmqApi::run() {
    run("tcp://127.0.0.1:3030", "tcp://127.0.0.1:3031");
}

void ZmqApi::run(const std::string &repAddress) {
    run(repAddress, "tcp://127.0.0.1:3031");
}

void ZmqApi::run(const std::string &repAddress, const std::string &pubAddress) {
    fmodController.setMarkerCallback([this](const std::string &eventId, const std::string &markerName) {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::string message = "marker:" + eventId + ";" + markerName + ";t=" + std::to_string(ms);
        std::lock_guard<std::mutex> lock(_pendingMutex);
        _pendingPublish.push_back(std::move(message));
    });

    zmq::context_t ctx;
    zmq::socket_t rep_sock(ctx, zmq::socket_type::rep);
    rep_sock.bind(repAddress);

    zmq::socket_t pub_sock(ctx, zmq::socket_type::pub);
    pub_sock.bind(pubAddress);

    std::cout << "ZMQ REP listening on " << repAddress << std::endl
            << "ZMQ PUB listening on " << pubAddress << std::endl << std::flush;

    zmq::pollitem_t items[] = {{static_cast<void *>(rep_sock), 0, ZMQ_POLLIN, 0}};

    // TODO listen to SIGINT and close open sockets
    while (true) {
        zmq::poll(items, 1, std::chrono::milliseconds(10));

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t message;
            rep_sock.recv(message, zmq::recv_flags::none);

#ifdef DEBUG
            std::cout << "Received " << message.size() << " bytes:" << message << std::endl;
#endif

            try {
                std::string result = process_request(message.to_string());
                verbose && std::cout << result << std::endl;

                zmq::message_t replyMessage(result.c_str(), result.length());
                rep_sock.send(replyMessage, zmq::send_flags::none);
            } catch (std::exception &exception) {
                std::cerr << "UNHANDLED EXCEPTION! " << exception.what() << std::endl;

                std::stringstream ss;
                ss << "UNHANDLED EXCEPTION! " << exception.what() << std::endl;

                auto errorMessage = ss.str();
                std::cerr << errorMessage;

                zmq::message_t replyMessage(errorMessage.c_str(), errorMessage.length());
                rep_sock.send(replyMessage, zmq::send_flags::none);
            }
        }

        // Drain the marker event queue and publish
        std::deque<std::string> toPublish; {
            std::lock_guard<std::mutex> lock(_pendingMutex);
            toPublish.swap(_pendingPublish);
        }
        for (const auto &msg: toPublish) {
            verbose && std::cout << "PUB: " << msg << std::endl;
            pub_sock.send(zmq::message_t(msg.c_str(), msg.length()), zmq::send_flags::none);
        }
    }
}
