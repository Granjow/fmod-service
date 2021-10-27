#include "unix-server.h"
#include <iostream>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>

const char *UnixServer::socket_name_ = "/tmp/fmod-crystal-cave.sock";

UnixServer::UnixServer() {
    // setup handler for Control-C so we can properly unlink the UNIX
    // socket when that occurs
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interrupt;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}

UnixServer::~UnixServer() {
}

std::string UnixServer::process_request(string request) {
    std::cout << "UnixServer is processing the request now: " << request << std::endl << std::flush;
    return request;
}

void
UnixServer::create() {
    struct sockaddr_un server_addr;

    std::cout << "Using socket " << socket_name_ << std::endl << std::flush;
    std::remove(socket_name_);

    // setup socket address structure
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_name_, sizeof(server_addr.sun_path) - 1);

    // create socket
    server_ = socket(PF_UNIX, SOCK_STREAM, 0);

    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // Ensure the socket is readable and writable by everybody
    mode_t previous_umask = umask(0);

    // call bind to associate the socket with the UNIX file system
    if (bind(server_, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // Restore umask value
    umask(previous_umask);

    // convert the socket to listen for incoming connections
    if (listen(server_, SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
UnixServer::close_socket() {
    unlink(socket_name_);
}

void
UnixServer::interrupt(int) {
    unlink(socket_name_);
}
