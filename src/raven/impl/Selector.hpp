#pragma once

#ifdef __linux__
#include "raven/impl/linux/Connection.hpp"
#include "raven/impl/linux/Socket.hpp"
#elif defined _WIN32
#error "Windows not implemented yet"
#else
#error "Not supported"
#endif

namespace raven::impl {
#ifdef __linux__
using Connection = linuximpl::LinuxConnection;
using Socket = linuximpl::LinuxSocket;
#elif defined _WIN32
#error "Windows not implemented yet"
#else
#error "Not supported"
#endif

}
