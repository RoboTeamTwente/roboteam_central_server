#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <proto/Setting.pb.h>
#include <proto/State.pb.h>
#include <optional>

#include <thread>
#include <vector>

#include "connection.hpp"

namespace rtt::central {

    class Server {
    private:
        /**
         * Boolean that describes whether the server should keep running
         */
        std::atomic<bool> _run = true;
        /**
         * The current thread associated with the given Server
         */
        std::thread thrd;

        /**
         * The main loop of the server.
         *
         * The loop will receive data from all registered zmqpp connections and forward it to every other connection but the source.
         * It will block until _run is set to false
         *
         * @sa Server::stop()
         */
        void run();
    public:
        /**
         * Start the main loop on its own thread.
         */
        void start();

        /**
         * Wait for the thread of the main loop to exit.
         *
         * The method will join() the running thread if it is joinable(), otherwise it will not block.
         */
        void wait();

        /**
         * Stop the main server loop.
         *
         * Sets an internal atomic boolean to false, terminating the while loop the next time the condition is evaluated. It can take up to 10s with the given configuration.
         *
         * @sa Server::wait()
         */
        void stop();
    };

}  // namespace rtt::central
#endif