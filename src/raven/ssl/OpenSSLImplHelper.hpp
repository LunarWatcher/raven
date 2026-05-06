#pragma once

#include "raven/conn/CommonDefs.hpp"
#include <openssl/crypto.h>
namespace raven::ossl {

extern int64_t write(SSL* ssl, int fd, Buffer& buff, size_t available);
extern int64_t write(SSL* ssl, int fd, const char* buff, size_t available);
extern int64_t read(SSL* ssl, int fd, Buffer& buff);

}
