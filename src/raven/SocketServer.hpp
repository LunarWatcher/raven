#pragma once

#include "raven/Socket.hpp"
#include <memory>

namespace raven {

class SocketServer {
private:
    std::unique_ptr<Socket> sock;

public:
    SocketServer(
        SocketConfig&& conf
    );

    /**
     * Starts an acceptor. This is a blocking function that can be called from multiple threads.
     */
    void startAcceptor();

};

}
