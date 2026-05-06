#pragma once

#include "raven/Socket.hpp"
#include "raven/conn/Connection.hpp"
#include <memory>
#include <cstdint>

namespace raven::linuximpl {
class LinuxSocket : public Socket {
private:
    int fd;

    uint16_t port;
    ip::IP assignedAddr;
public:
    LinuxSocket(SocketConfig&& conf);
    ~LinuxSocket();
    void bind() override;
    void close() override;
    std::unique_ptr<Connection> accept() override;

    int getNativeHandle() override { return fd; }

    uint16_t getPort() override {
        return port;
    }
    const std::string& getAddr() override {
        return assignedAddr.dotNotation;
    }
};

}
