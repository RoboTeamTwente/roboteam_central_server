#include "server.hpp"
#include <map>
#include <string>

#include <chrono>
namespace rtt::central {

    void Server::wait() {
        if (!this->thrd.joinable()) {
            return;
        }

        this->thrd.join();
    }

    void Server::start() {
        std::thread tmp_thrd{&Server::run, this};
        this->thrd = std::move(tmp_thrd);

    }

    void Server::run() {
        zmqpp::poller _poller;
        std::map<std::string, std::unique_ptr<Connection<zmqpp::socket_type::pair>>> connections;

        connections.insert({"roboteam_interface", std::make_unique<Connection<zmqpp::socket_type::pair>>(16971)});
        connections.insert({"roboteam_ai", std::make_unique<Connection<zmqpp::socket_type::pair>>(16970)});


        for (const auto& [name, conn] : connections) {
            std::cout << "[CENTRAL] Listening to " << name << std::endl;
            _poller.add(conn->socket);
        }

        while (this->_run) {
            using namespace std::chrono_literals;
            auto event_happened = _poller.poll(std::chrono::milliseconds(10s).count());
            if (!event_happened) {
                continue;
            }


            for (const auto& [name, connection] : connections) { // We should have a guarantee that ptrs in the map are valid
                if (_poller.has_input(connection->socket)) {
                    auto input = connection->read();

                    if (input.is_err()) {
                        std::cout << "[CENTRAL] Error when reading from " << name << ": " << input.err_value() << std::endl;
                        continue;
                    }

                    std::for_each(connections.begin(), connections.end(), [c = input.value(), src_name = name](auto& item){
                        if (item.first != src_name) {
                            item.second->write_str(c);
                        }
                    });
                }
            }
        }

        std::cout << "[CENTRAL] Main loop exited" << std::endl;

        for (const auto& [name, connection] : connections) {
            std::cout << "[CENTRAL] Closing " << name << std::endl;
            connection->socket.close();
        }
    }

    void Server::stop() {
        this->_run.store(false);
    }
}  // namespace rtt::central