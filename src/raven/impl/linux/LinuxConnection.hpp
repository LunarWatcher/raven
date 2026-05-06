#pragma once

#include "raven/Logging.hpp"
#include "raven/conn/CommonDefs.hpp"
#include "raven/conn/Connection.hpp"
#include "raven/ip/IP.hpp"
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <unistd.h>
#include <array>

namespace raven::linuximpl {

class LinuxConnection : public Connection {
private:
    int fd;
    SSL* ssl;

    size_t read(
        Buffer& buff,
        int& flags
    ) override;

public:
    LinuxConnection(
        const sockaddr_in& clientAddr,
        int fd,
        SSL* ssl
    );
    LinuxConnection(LinuxConnection&) = delete;
    LinuxConnection(LinuxConnection&& other) noexcept
        : Connection(std::move(other.ip)),
          fd(other.fd),
          ssl(other.ssl)
    {
        other.fd = -1;
        other.ssl = nullptr;
    }

    ~LinuxConnection();

    size_t write(
        Buffer& buff,
        size_t length,
        int& flags
    ) override;

    void close() override;

    int getNativeHandle() override { return fd; }
};

}
