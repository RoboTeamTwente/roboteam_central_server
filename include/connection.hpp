#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__
#include <proto/State.pb.h>
#include <stx/result.h>

#define ZMQ_BUILD_DRAFT_API 1
#include <zmqpp/zmqpp.hpp>

#include "type_traits.hpp"

namespace rtt::central {
    template <zmqpp::socket_type ct>
    struct Connection {
        zmqpp::context context;
        zmqpp::socket socket;

        Connection(size_t port)
            : socket{ context, ct } {
            socket.bind("tcp://*:" + std::to_string(port));
        }

        template <typename T>
        stx::Result<T, std::string> read_next(bool dont_block = true) {
            static_assert(type_traits::is_serializable_v<T>, "T is not serializable to string in Connection::read_next()");
            zmqpp::message msg;
            std::string data{};
            if (!socket.receive(msg, dont_block)) {
                return stx::Err(std::move(data));
            }
            msg >> data;
            T object;
            auto succ = object.ParseFromString(data);
            if (succ) {
                return stx::Ok(std::move(object));
            }
            return stx::Err(std::move(data));
        }

        stx::Result<std::string, std::string> read(bool dont_block = true) {
            zmqpp::message msg;
            std::string data{};

            if (!socket.receive(msg, dont_block)) {
                return stx::Err(std::move(data));
            }
            msg >> data;

            return stx::Ok(std::move(data));
        }

        template <typename T>
        void write(T const& s) {
            static_assert(type_traits::is_serializable_v<T>, "T is not serializable to string in Connection::write()");
            std::string out;
            s.SerializeToString(&out);
            socket.send(out);
        }

        void write_str(const std::string& s) {
            socket.send(s);
        }

        bool is_ok() {
            return static_cast<bool>(socket);
        }
    };

}  // namespace rtt::central

#endif