#pragma once

#include "raven/ConnectionPool.hpp"
#include "raven/Socket.hpp"
#include <memory>

namespace raven {

struct ServerConfig {
    int threadConnectionLimit = 128;
    // TODO: this should be max, but min is pretty convenient for some initial debugging, so fix later
    size_t threads = std::min<size_t>(
        1, std::thread::hardware_concurrency() / 2
    );
};

class SocketServer {
private:
    std::shared_ptr<Socket> sock;
    ServerConfig conf;
    PoolSync sync;

    std::unique_ptr<ConnectionPool> pool;

public:
    SocketServer(
        SocketConfig&& socketConf,
        ServerConfig&& serverConf,
        ConnPoolConfig&& poolConf
    );
    ~SocketServer();

    /**
     * Starts the threadpool. This is a non-blocking function  
     */
    void start();

    void waitForDone();

    /**
     * Closes the server. Normally, you won't need to call this manually. If you have threads joining before the
     * SocketServer is destroyed that you would like to actually join, then you need to call this function.
     *
     * Not calling this function results in it being automagically called in the destructor.
     */
    void close();

};

}
