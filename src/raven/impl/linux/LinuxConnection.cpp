#include "LinuxConnection.hpp"
#include "raven/Logging.hpp"
#include "raven/ssl/OpenSSLImplHelper.hpp"

#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <openssl/ssl.h>

namespace raven::linuximpl {

LinuxConnection::LinuxConnection(
    const sockaddr_in& clientAddr,
    int fd,
    SSL* ssl
) : Connection({
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
        std::string(inet_ntoa((const in_addr&) clientAddr.sin_addr))
    }), fd(fd), ssl(ssl) {

    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        RavenLog("Failed to set nonblocking: %s\n", strerror(errno));
    }
}

LinuxConnection::~LinuxConnection() {
    close();
}

void LinuxConnection::close() {
    if (ssl != nullptr) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
        open = false;
        closed = true;
    }
}

size_t LinuxConnection::read(
    Buffer& buff,
    int& flags
) {
    // TODO: I want to mix in poll() here, but I suspect that'll be one level up from the connection
    // Maybe the acceptor also needs to have platform-specific implementations to allow this?
    ssize_t read = ossl::read(
        ssl,
        this->fd,
        buff
    );

    if (read < 0) {
        // Nothing to receive
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            flags = Connection::ReadFlags::BufferEmpty;
        } else if (
            // TODO: is this enough? ECONNRESET is not listed for recv nor write, but the error list on the man page
            // says additional errors can come from underlying protocols, which as far as I can tell includes ECONNRESET
            // on both read and write
            errno == ENOTCONN
            || errno == ECONNRESET
        ) {
            flags = Connection::ReadFlags::Closed;
        } else {
            flags = Connection::ReadFlags::Error;
        }
        return 0;
    } else if (read == 0) {
        flags = Connection::ReadFlags::Closed;
        return 0;
    }

    flags = 0;
    return (size_t) read;
}

size_t LinuxConnection::write(
    Buffer& buff,
    size_t available,
    int& flags
) {
    ssize_t written = ossl::write(
        ssl,
        this->fd,
        buff,
        available
    );

    if (written < 0) {
        if (errno == ENOTCONN || errno == ECONNRESET) {
            flags = Connection::WriteFlags::Closed;
        } else if (errno == EWOULDBLOCK) {
            flags = Connection::WriteFlags::BufferFull;
        } else {
            flags = Connection::WriteFlags::Error;
        }
        return 0;
    } else if (written == 0) {
        flags = Connection::ReadFlags::Closed;
        return 0;
    }
    return (size_t) written;
}

}
