#include "SocketServer.hpp"
#include "raven/Socket.hpp"
#include "raven/impl/Selector.hpp"
#include <memory>

namespace raven {

SocketServer::SocketServer(
    SocketConfig&& conf
) :
    sock(std::make_unique<impl::Socket>(
        std::move(conf)
    ))
{
    // Not sure if I want to do this here, but this is good enough for now
    sock->bind();
}

void SocketServer::startAcceptor() {
    // TODO: this should be using proper acceptors that can keep their own queues of living connections
    while (true) {
        auto conn = this->sock->accept();
        conn->write("Good girl :3");
        conn->close();
    }

}

}
