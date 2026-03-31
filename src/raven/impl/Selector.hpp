#pragma once

#ifdef __linux__
#include "raven/impl/linux/LinuxConnection.hpp"
#include "raven/impl/linux/LinuxConnectionPool.hpp"
#include "raven/impl/linux/LinuxSocket.hpp"
#elif defined _WIN32
#error "Windows not implemented yet"
#else
#error "Not supported"
#endif

namespace raven::impl {
#ifdef __linux__
using Connection = linuximpl::LinuxConnection;
using Socket = linuximpl::LinuxSocket;
using ConnectionPool = linuximpl::LinuxConnectionPool;
#elif defined _WIN32
#error "Windows not implemented yet"
#else
#error "Not supported"
#endif

}
