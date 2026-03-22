#pragma once

#include "raven/Socket.hpp"
#include "raven/conn/Connection.hpp"
#include <cstdint>
#include <memory>

namespace raven::linuximpl {
class LinuxSocket : public Socket {
private:
    int fd;
public:
    LinuxSocket(SocketConfig&& conf);
    ~LinuxSocket();
    void bind() override;
    std::unique_ptr<Connection> accept() override;
};

}
