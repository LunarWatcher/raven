#pragma once

#include "raven/Logging.hpp"
#include "raven/conn/CommonDefs.hpp"
#include "raven/conn/Connection.hpp"
#include "raven/ip/IP.hpp"
#include <netinet/in.h>
#include <unistd.h>
#include <array>

namespace raven::linuximpl {

class LinuxConnection : public Connection {
private:
    int fd;

    size_t read(
        Buffer& buff,
        int& flags
    ) override;
    size_t write(
        Buffer& buff,
        size_t length,
        int& flags
    ) override;

public:
    LinuxConnection(
        const sockaddr_in& clientAddr,
        int fd
    );
    LinuxConnection(LinuxConnection&) = delete;
    LinuxConnection(LinuxConnection&& other) noexcept
        : Connection(std::move(other.ip)), fd(other.fd)
    {
        other.fd = -1;
    }

    ~LinuxConnection();

    void close() override {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
            open = false;
            closed = true;
        }
    }

    int getNativeHandle() override { return fd; }
};

}
