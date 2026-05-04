#include "LinuxSocket.hpp"
#include "LinuxConnection.hpp"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

namespace raven::linuximpl {

LinuxSocket::LinuxSocket(SocketConfig&& conf) :
    Socket(std::move(conf)),
    fd(-1)
{}

LinuxSocket::~LinuxSocket() {
    close();
}
void LinuxSocket::close() {
    if (fd >= 0) {
        ::close(fd);
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
    fcntl(
        fd,
        F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK
    );

    if (fd < 0) {
        throw std::runtime_error(
            "Failed to initialize socket. errno=" + std::to_string(errno)
        );
    }
    in_addr resolved = this->conf.ip.and_then([](const auto& ip) {
        return std::optional<in_addr>(
            // in_addr contains an in_addr_t for some reason, and sin_addr expects an in_addr
            // Could technically do the conversion up in the sockaddr assignment, but might as well
            in_addr {
                inet_addr(ip.c_str())
            }
        );
    })
        .value_or(in_addr { INADDR_ANY });

    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(conf.port),
        .sin_addr = resolved
    };
    // TODO: Should this be togglable?
    const int enable = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        throw std::runtime_error("Failed to set reuse_addr");
    }
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        throw std::runtime_error("Failed to set reuse_port");
    }

    if (::bind(this->fd, (const sockaddr*) &addr, sizeof(addr)) != 0) {
        // TODO: this could be used to make better error messages in the future
        throw std::runtime_error(
            "Failed to bind socket. errno=" + std::string(std::strerror(errno))
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

    clientFd = ::accept(
        this->fd, (sockaddr*) &clientAddr,
        &clientAddrSize
    );

    if (clientFd < 0) {
        return nullptr;
    }

    return std::make_unique<LinuxConnection>(
        clientAddr,
        clientFd
    );
}

}
