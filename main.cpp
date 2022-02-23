#include <iostream>
#include <csignal>

#include "server.hpp"

int main(int, char**) {
    sigset_t signal_set;
    int sig;

    rtt::central::Server server;

    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGTERM);

    std::cout << "[CENTRAL] Server starting. Graceful shutdown may take up to 10s" << std::endl;
    server.start();

    sigwait(&signal_set, &sig);
    std::cout << "[CENTRAL] Got shutdown signal" << std::endl;

    server.stop();
    server.wait();
}
