#include "SocketServer.hpp"
#include "raven/Logging.hpp"
#include "raven/Socket.hpp"
#include "raven/impl/Selector.hpp"
#include <memory>

namespace raven {

SocketServer::SocketServer(
    SocketConfig&& socketConf,
    ServerConfig&& serverConf,
    ConnPoolConfig&& poolConf
) :
    sock(std::make_shared<impl::Socket>(
            std::move(socketConf)
        )),
    conf(std::move(serverConf)),
    pool(
        std::make_unique<impl::ConnectionPool>(
            poolConf,
            sync,
            this->sock
        )
    )
{
    // Not sure if I want to do this here, but this is good enough for now
    sock->bind();
}

SocketServer::~SocketServer() {
    sync.close();
}

void SocketServer::start() {
    if (!pool->started()) {
        pool->start(this->conf.threads);
    }
}

void SocketServer::waitForDone() {
    sync.subscribeToTermination();
}

void SocketServer::close() {
    sock->close();
    sync.close();
}

}
