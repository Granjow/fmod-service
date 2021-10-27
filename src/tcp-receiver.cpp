#include <iostream>
#include "../lib/cppzmq/zmq.hpp"

int main()
{
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::pull);
    sock.bind("tcp://127.0.0.1:3030");

    zmq::message_t message;
    sock.recv(message, zmq::recv_flags::none);
    std::cout << "Received:" << message << std::endl;
}