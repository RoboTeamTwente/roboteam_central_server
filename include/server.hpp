#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <roboteam_proto/Setting.pb.h>
#include <roboteam_proto/State.pb.h>
#include <stx/option.h>

#include <thread>
#include <vector>

#include "connection.hpp"
#include "modulehandler.hpp"
#include "mutex.hpp"
#include "utils.hpp"

namespace rtt::central {

    struct Server {
        /**
         * @brief These are the modules connected.
         * They merely _receive_ data, they never send any, apart from inital handshake.
         */
        Mutex<ModuleHandler> modules;  // ->write() broadcasts
        // When a module broadcasts its handshare (Handshake.proto)
        // it's stored here, ModuleHandler basically backtracks it to this vector
        // this is why ModuleHandler takes an std::vector<proto::Handshake>* as
        // ctor argument
        Mutex<std::vector<proto::Handshake>> module_handshakes; 

        /**
         * @brief There is a direct conneciton to the AI and the interface.
         * Both are readwrite and continuously read and written to.
         */
        Mutex<Connection<zmqpp::socket_type::pair, 16970>> roboteam_ai;
        Mutex<Connection<zmqpp::socket_type::pair, 16971>> roboteam_interface;

        Mutex<std::thread> ai_thread;
        Mutex<std::thread> interface_thread;

        Mutex<stx::Option<proto::State>> current_ai_state;
        // placeholder type Setting
        Mutex<stx::Option<proto::Setting>> current_settings;

        Server();
        void handle_success_state_read(proto::State ok);
        void handle_roboteam_ai();

        void handle_interface();

        void run();
    };

}  // namespace rtt::central
#endif