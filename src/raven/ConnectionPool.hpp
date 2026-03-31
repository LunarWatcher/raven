#pragma once

#include "raven/PoolSync.hpp"
#include "raven/Socket.hpp"
#include "raven/conn/Connection.hpp"
#include <functional>
#include <list>
namespace raven {

struct ConnPoolConfig {
    /**
     * \brief Callback used when a connection (provided as a raw pointer) is ready to receive data.
     *
     * This function may be called multiple times per connection depending on keepalive-settings. Closing the connection
     * with this callback is well-defined behaviour.
     *
     * \note This callback must be set.
     */
    std::function<
        void(
            Connection*,
            const std::array<char, 16'384>& buff,
            size_t availableChars
        )
    > onRecv;

    /**
     * Signals that a write has completed.
     * This is an optional callback. If unset, the connection will be closed once the write queue is empty.
     */
    std::function<void(Connection*)> onWriteComplete;
};

/**
 * Manages the IO thread pool
 */
class ConnectionPool {
protected:
    ConnPoolConfig callbacks;
    PoolSync& sync;
    std::shared_ptr<Socket> socket;
    std::list<std::shared_ptr<Connection>> conns;
    std::vector<std::thread> threads;

    /**
     * Contains the business end of the connection pool. This is called by each individual thread. The implementation
     * mechanics are platform-specific, but it must meet the following criteria:
     *
     * 1. It must handle being called from multiple threads, and not hand out the same event to multiple threads
     * 2. It must be able to respect termination signals so the server shuts down when signaled.
     *    If termination signals are not available or not implemented, it must at minimum contain a timeout so tests
     *    don't deadlock.
     * 3. It must be able to handle accepts on the server socket
     * 4. It must issue read and write events when they're ready, or act blocking for the thread if no such feature
     *    exists on the implementation platform.
     * 5. It must handle all errors by itself, whether thrown or in the form of status ints or similar.
     *    No error handling is provided by the thread, as any error that propagates to the thread is assumed to be fatal.
     */
    virtual void poll() = 0;

    /**
     * Used to perform a read on a connection. This method exists so implementations of the ConnectionPool don't need to
     * separately be friends with the base Connection class.
     */
    virtual size_t proxyRead(
        Connection* conn,
        std::array<char, Connection::WindowSize>& buff,
        int& flags
    ) {
        return conn->read(buff, flags);
    }

    /**
     * Used to perform a write on a connection. This method exists so implementations of the ConnectionPool don't need
     * to separately be friends with the base Connection class.
     */
    virtual size_t proxyWrite(
        Connection* conn
    ) {
        return conn->writeBuffers();
    }
public:
    ConnectionPool(
        const ConnPoolConfig& config,
        PoolSync& syncObject,
        const std::shared_ptr<Socket>& socket
    );
    virtual ~ConnectionPool();

    virtual void start(
        size_t threadCount
    );
    virtual void close() {}

    bool started() { return threads.size() > 0; }
};

}
