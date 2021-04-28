#include "server.hpp"
#include <roboteam_utils/Time.h>
#include <roboteam_utils/Timer.h>

namespace rtt::central {

    Server::Server()
        : modules{ &this->module_handshakes } {
    }

    void Server::handle_roboteam_ai() {
        // read incoming data from roboteam_ai
        roboteam_utils::Timer timer;
        timer.loop([&](){
                     if(!_run.load()){ timer.stop();}
                     auto ai = roboteam_ai.acquire();

                     ai->read_next<proto::ModuleState>()
                         .match(
                             [this](proto::ModuleState&& ok) { this->handle_ai_state(ok); },
                             [](std::string&& err) {
                               if (err.size()) {
                                 std::cout << "Error packet received: " << std::endl;
                                 std::cout << err << std::endl;
                               } });
        ;},
                   100
        );

    }


    void Server::handle_ai_state(proto::ModuleState ok) {
      //TODO: send state to interface and modules
      auto handshakes = module_handshakes.acquire();
      for (auto const& each : *handshakes) {
        *ok.add_handshakes() = each;
      }
        // send it to the interface
        //ok.PrintDebugString();
        roboteam_interface.acquire()->write(ok);
        // forward this state to all modules.
        //modules.broadcast(ok); //TODO: This call locks/blocks for some reason?
    }

    void Server::handle_interface(proto::UiValues data) {

      *this->current_settings.acquire() = stx::Some(std::move(data));
        //TODO: if settingschanged or every 0.5 seconds, send settings to AI
        roboteam_ai.acquire()->write(current_settings.acquire()->value());
    }

    void Server::handle_modules() {
        modules.run(std::ref(this->_run));
    }

    void Server::run() {
        std::cout << "Constructing AI thread." << std::endl;
        new (&ai_thread) Mutex{ std::thread([this]() {
            this->handle_roboteam_ai();
        }) };

        roboteam_interface.acquire()->run([this](proto::UiValues data) {
            handle_interface(std::move(data));
        });

        std::cout << "Constructing Modules thread." << std::endl;
        new (&module_thread) Mutex{ std::thread([this]() {
            this->handle_modules();
        }) };

        ai_thread.acquire()->join();
        module_thread.acquire()->join();
    }

    void Server::stop() {
        this->_run.store(false);
        // roboteam_interface.acquire()->stop();
    }
}  // namespace rtt::central