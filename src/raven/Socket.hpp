#pragma once

#include "raven/conn/Connection.hpp"
#include <cstdint>
#include <memory>
#include <string>
namespace raven {

enum class SocketType {
    Stream,
    Dgram
};

struct SocketConfig {
    int queue = 128;
    SocketType type;
    uint16_t port;
    std::string ip;
};

class Socket {
protected:
    SocketConfig conf;
public:
    Socket(SocketConfig&& conf) : conf(std::move(conf)) {}
    virtual ~Socket() = default;

    virtual void bind() = 0;

    virtual std::unique_ptr<Connection> accept() = 0;
};

}
