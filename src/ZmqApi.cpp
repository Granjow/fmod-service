#include <iostream>
#include <ostream>
#include "../lib/cppzmq/zmq.hpp"

#include "common.h"

#include "ZmqApi.h"
#include "FmodException.h"
#include "ApiParameterException.h"
#include "Parameters.h"

ZmqApi::ZmqApi(const FmodController &fmodController) : fmodController(fmodController), startedAt(std::time(nullptr)) {}

ZmqApi::ZmqApi()
        : ZmqApi(FmodController()) {
}

std::string ZmqApi::process_request(std::string raw_request) {
    std::cout << "Processing request: " << raw_request << std::endl << std::flush;

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
        } else if (key == "set-parameter") {
            // Value format: eventId;parameterName;parameterValue
            auto params = Parameters::parse(value, 3);

            std::string eventId = params[0];
            std::string parameterName = params[1];
            float parameterValue = std::stof(params[2]);

            std::cout << "Setting " << eventId << " param " << parameterName << " to " << parameterValue
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

            std::cout << "Starting event " << eventId << " with programmer instrument key " << voiceKey << std::endl;
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


    std::cout << "Processed " << raw_request << std::endl << std::flush;

    return response.str();
}

void ZmqApi::run() {
    run("tcp://127.0.0.1:3030");
}

void ZmqApi::run(const std::string &socketAddress) {
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::rep);
    sock.bind(socketAddress);

    // TODO listen to SIGINT and close open sockets
    while (true) {
        zmq::message_t message;
        sock.recv(message, zmq::recv_flags::none);

#ifdef DEBUG
        std::cout << "Received:" << message << std::endl;
#endif

        try {
            std::string result = process_request(message.to_string());
            std::cout << result << std::endl;

            const char *cstr = result.c_str();
            zmq::message_t replyMessage = zmq::message_t(cstr, result.length());
            sock.send(replyMessage, zmq::send_flags::none);
        } catch (std::exception &exception) {
            std::cerr << "UNHANDLED EXCEPTION! " << exception.what() << std::endl;


            std::stringstream ss;
            ss << "UNHANDLED EXCEPTION! " << exception.what() << std::endl;

            auto errorMessage = ss.str();
            std::cerr << errorMessage;

            zmq::message_t replyMessage = zmq::message_t(errorMessage.c_str(), errorMessage.length());
            sock.send(replyMessage, zmq::send_flags::none);
        }

    }
}
