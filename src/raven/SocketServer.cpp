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
    close();
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
    // TODO: the sync closure here has got somewhat out of hand. There's too many moving parts, but sync has to be both
    // created first and shut down first, so I doubt there's a better way to do it. The close methods should be made
    // more consistent though, a fair few of them could look like outwards API methods while they're functionally just
    // internal destructor logic.
    sock->close();
    RavenLog("Socket closed\n");
    sync.close(false);
    pool->close();
    RavenLog("Connection pool shutdown requested\n");
    sync.close(true);
    RavenLog("Connection pool shutdown completed\n");
}

}
