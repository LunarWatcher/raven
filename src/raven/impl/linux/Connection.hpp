#pragma once

#include "raven/conn/Connection.hpp"
#include "raven/ip/IP.hpp"
#include <netinet/in.h>
#include <unistd.h>

namespace raven::linuximpl {

class LinuxConnection : public Connection {
private:
    int fd;
    ip::IP ip;
public:
    LinuxConnection(
        const sockaddr_in& clientAddr,
        int fd
    );
    ~LinuxConnection();
    LinuxConnection(LinuxConnection&) = delete;
    LinuxConnection(LinuxConnection&& other) noexcept
        : fd(other.fd),
        ip(std::move(other.ip))
    {
        other.fd = -1;
    }

    void close() override {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    void write(const std::string& buff) override;
};

}
