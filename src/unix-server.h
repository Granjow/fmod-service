#pragma once

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <string>

#include "server.h"

class UnixServer : public Server {

public:
    UnixServer();

    ~UnixServer();

protected:
    void create();

    void close_socket();

    virtual std::string process_request(string request);

private:
    static void interrupt(int);

    static const char *socket_name_;
};
