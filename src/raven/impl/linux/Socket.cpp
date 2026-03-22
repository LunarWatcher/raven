#include "Socket.hpp"
#include "raven/impl/linux/Connection.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

namespace raven::linuximpl {

LinuxSocket::LinuxSocket(SocketConfig&& conf) : Socket(std::move(conf)) {

}

LinuxSocket::~LinuxSocket() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

void LinuxSocket::bind() {
    fd = ::socket(
        // TODO: decouple from ipv4
        PF_INET,
        conf.type == SocketType::Stream ? SOCK_STREAM : SOCK_DGRAM,
        0
    );

    if (fd < 0) {
        throw std::runtime_error(
            "Failed to initialize socket. errno=" + std::to_string(errno)
        );
    }

    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(conf.port),
        .sin_addr = INADDR_ANY
    };
    if (::bind(this->fd, (const sockaddr*) &addr, sizeof(addr)) != 0) {
        // TODO: this could be used to make better error messages in the future
        throw std::runtime_error(
            "Failed to bind socket. errno=" + std::to_string(errno)
        );
    }

    if (::listen(this->fd, this->conf.queue) != 0) {
        throw std::runtime_error(
            "Failed to listen to socket. errno=" + std::to_string(errno)
        );
    }
}

std::unique_ptr<Connection> LinuxSocket::accept() {
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientFd;
    while (true) {
        clientFd = ::accept(
            this->fd, (sockaddr*) &clientAddr,
            &clientAddrSize
        );

        if (clientFd < 0) {
            // TODO: log error?
            continue;
        }
        break;
    }

    return std::make_unique<LinuxConnection>(
        clientAddr,
        clientFd
    );
}

}
