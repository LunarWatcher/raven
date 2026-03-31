#include "ConnectionPool.hpp"
#include "raven/Logging.hpp"

namespace raven {

ConnectionPool::ConnectionPool(
    const ConnPoolConfig& config,
    PoolSync& syncObject,
    const std::shared_ptr<Socket>& socket
): callbacks(config), sync(syncObject), socket(socket) {
    if (callbacks.onRecv == nullptr) {
        throw std::runtime_error("onRecvReady cannot be nullptr");
    }
}

ConnectionPool::~ConnectionPool() {
    for (auto& thread : threads) {
        thread.join();
    }
}

void ConnectionPool::start(size_t threadCount) {
    if (threadCount == 0) {
        [[unlikely]]
        throw std::runtime_error(
            "Must start with a non-zero amount of threads"
        );
    }
    for (size_t i = 0; i < threadCount; ++i) {
        threads.push_back(
            std::thread(
                [this, i]() {
                    // TODO: this should be RAII'd
                    RavenLog("Thread %ld online\n", i);
                    this->sync.newConnPool();
                    while (this->sync.isRunning) {
                        poll();
                    }
                    RavenLog("Thread %ld dying\n", i);
                    this->sync.destroyConnPool();
                }
            )
        );
    }
}

}
