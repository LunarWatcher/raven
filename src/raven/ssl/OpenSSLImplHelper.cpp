#include "OpenSSLImplHelper.hpp"

#include <openssl/ssl.h>
#include <unistd.h>

namespace raven::ossl {

int64_t write(SSL* ssl, int fd, Buffer& buff, size_t available) {
    return write(ssl, fd, buff.data(), available);
}

int64_t write(SSL* ssl, int fd, const char* buff, size_t available) {
    if (ssl == nullptr) {
        return ::write(fd, buff, available);
    }
    return SSL_write(
        ssl,
        buff,
        available
    );
}

int64_t read(SSL* ssl, int fd, Buffer& buff) {
    if (ssl == nullptr) {
        return ::read(fd, buff.data(), buff.size());
    }
    return SSL_read(
        ssl,
        buff.data(),
        buff.size()
    );
}

}
