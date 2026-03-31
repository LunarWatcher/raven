#include "raven/ConnectionPool.hpp"
#include "raven/Socket.hpp"
#include <sys/epoll.h>

namespace raven::linuximpl {

class LinuxConnectionPool : public ConnectionPool {
protected:
    int epollFd = -1;
    int eventFd = -1;

    void poll() override;
public:
    LinuxConnectionPool(
        const ConnPoolConfig& config,
        PoolSync& syncObject,
        const std::shared_ptr<Socket>& socket
    );
    ~LinuxConnectionPool();

    void close() override;
    void start(size_t threadCount) override;
};

}
