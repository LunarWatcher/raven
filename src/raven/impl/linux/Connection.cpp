#include "Connection.hpp"

#include <arpa/inet.h>

namespace raven::linuximpl {

LinuxConnection::LinuxConnection(
    const sockaddr_in& clientAddr,
    int fd
) : fd(fd), ip(
    ip::IPVersion::IPv4,
    // Not sure if this is actually universally thread-safe. 
    // It uses a statically allocated buffer, but one source I could find says the implementation uses a __thread-scoped
    // buffer, so copying it _should_ be enough.
    // https://github.com/kraj/glibc/blob/05d00ade8e8ae53d5087e986c9bbda449ff6be28/inet/inet_ntoa.c#L23-L34
    //
    // Need to test this properly though. I also assume it won't be portable outside linux, but that's unlikely to ever
    // be a problem.
    // (Windows gets its own implementation, and everything else is obscure enough that it'll only start to matter in
    // the _very_ unlikely scenario this library actually gets widely used)
    std::string(inet_ntoa((const in_addr&) clientAddr))
) {

}

LinuxConnection::~LinuxConnection() {
    close();
}

void LinuxConnection::write(const std::string& buff) {
    // TODO: remove, temporary hack for the write to work (at least curl expects to be read before it's written)
    char buffer[512];
    ssize_t bytesReceived = recv(fd, buffer, sizeof(buffer) - 1, 0);
    buffer[bytesReceived] = '\0';

    ::write(
        this->fd,
        buff.c_str(),
        buff.size()
    );
}

}
