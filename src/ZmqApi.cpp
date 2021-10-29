#include <iostream>
#include <ostream>
#include "../lib/cppzmq/zmq.hpp"

#include "common.h"

#include "ZmqApi.h"

ZmqApi::ZmqApi()
        : startedAt(std::time(nullptr)) {
}

std::string ZmqApi::process_request(std::string raw_request) {
    std::cout << "FMOD is processing the request: " << raw_request << std::endl << std::flush;

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

        try {
            if (key == "get" && value == "id") {
                std::ostringstream stream;
                stream << "ID=" << startedAt;
                response = stream.str();
            } else if (key == "load-bank") {
                auto result = fmodController.loadBank(value);
                response = result;
            } else if (key == "play-event") {
                response = fmodController.playEvent(value);
            } else if (key == "start-event") {
                response = fmodController.startEvent(value);
            } else if (key == "stop-event") {
                response = fmodController.stopEvent(value);
            } else {
                response = "Error: Unknown key";
            }
        } catch (std::runtime_error &err){
            response = err.what();
        }

    } else {
        std::cout << "Invalid request: >>" << request << "<<" << std::endl;
        response = "Error: Unknown request";
    }

    std::cout << "FMOD has processed the request." << std::endl << std::flush;

    return response;
}

int main() {
    ZmqApi zmqApi;

    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::rep);
    sock.bind("tcp://127.0.0.1:3030");

    while (true) {
        zmq::message_t message;
        sock.recv(message, zmq::recv_flags::none);
        std::cout << "Received:" << message << std::endl;
        std::string result = zmqApi.process_request(message.to_string());
        std::cout << result << std::endl;

        const char *cstr = result.c_str();
        zmq::message_t replyMessage = zmq::message_t(cstr, result.length());
        sock.send(replyMessage, zmq::send_flags::none);
    }
}
