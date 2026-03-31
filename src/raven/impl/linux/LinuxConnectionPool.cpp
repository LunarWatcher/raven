#include "LinuxConnectionPool.hpp"
#include <cstring>
#include <sys/epoll.h>
#include <raven/Logging.hpp>

namespace raven::linuximpl {

LinuxConnectionPool::LinuxConnectionPool(
    const ConnPoolConfig& config,
    PoolSync& syncObject,
    const std::shared_ptr<Socket>& socket
): ConnectionPool(config, syncObject, socket) {
    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        throw std::runtime_error("epoll init error. errno=" + std::string(std::strerror(errno)));
    }
}

void LinuxConnectionPool::start(size_t threadCount) {
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLEXCLUSIVE;
    ev.data.ptr = socket.get();
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, this->socket->getNativeHandle(), &ev) < 0) {
        RavenLog(
            "Error: %s (fd=%d)\n",
            strerror(errno),
            socket->getNativeHandle()
        );
        throw std::runtime_error("Failed to queue socket handle to epoll. errno=" + std::string(strerror(errno)));
    }

    ConnectionPool::start(threadCount);
}

LinuxConnectionPool::~LinuxConnectionPool() {
    if (epollFd >= 0) {
        close(epollFd);
        epollFd = -1;
    }
}

void LinuxConnectionPool::poll() {
    std::array<epoll_event, 10> ready;
    auto eventCount = epoll_wait(
        this->epollFd,
        ready.data(),
        ready.size(),
        10000
    );

    if (eventCount > 0) {
        for (size_t i = 0; i < (size_t) eventCount; ++i) {
            auto& ev = ready.at(i);
            if (ev.data.ptr == socket.get()) { // This is gross, but if it works
                auto conn = socket->accept();

                RavenLog("Queueing %d\n", conn->getNativeHandle());
                epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLET | EPOLLEXCLUSIVE;
                ev.data.ptr = conn.get();
                if (epoll_ctl(
                    epollFd,
                    EPOLL_CTL_ADD,
                    conn->getNativeHandle(),
                    &ev
                ) < 0) {
                    RavenLog("Error: epollctl failed: %d", errno);
                } else {
                    // The connection is copied to epoll_ctl. If epoll_ctl fails, this won't run, and the pointer will
                    // be freed as part of the unique_ptr's logic.
                    // If this does run, we've already copied it to an event that'll be passed to the other segment of
                    // this if statement, where there are systems in place to delete it. If an exception escapes, the
                    // program dies and we don't care.
                    std::ignore = conn.release();
                }
            } else {
                RavenLog("Entered\n");
                auto* conn = (Connection*) ev.data.ptr;

                if (ev.events & EPOLLHUP || ev.events & EPOLLERR) {
                    RavenLog("EPOLLHUP signaled\n"); 
                    conn->close();
                } else {
                    if (ev.events & EPOLLIN) {
                        std::array<char, Connection::WindowSize> buff;
                        try {
                            int flags = 0;
                            size_t bytesRead = proxyRead(
                                conn,
                                buff,
                                flags
                            );
                            if (flags == Connection::ReadFlags::Closed) {
                                conn->close();
                            } else if (flags == 0 && bytesRead > 0) {
                                this->callbacks.onRecv(
                                    conn,
                                    buff,
                                    bytesRead
                                );
                            }
                        } catch (std::exception& e) {
                            RavenLog("Raven caught uncaught exception: %s\n", e.what());
                            conn->close();
                        } catch (...) {
                            RavenLog("Raven caught uncaught exception of non-exception type\n");
                            conn->close();
                        }
                    }

                    if (
                        !conn->isClosed()
                        && conn->hasWriteableBuffers()
                        && ev.events & EPOLLOUT
                    ) {
                        proxyWrite(conn);

                        if (!conn->hasWriteableBuffers()) {
                            if (callbacks.onWriteComplete) {
                                callbacks.onWriteComplete(conn);
                            } else {
                                conn->close();
                            }
                        }
                    }
                }

                if (conn->isClosed()) {
                    RavenLog("Connection closed; deleting\n");
                    delete conn;
                    continue;
                }

                // if (conn->hasWriteableBuffers()) {
                //     ev.events = EPOLLOUT | EPOLLHUP | EPOLLONESHOT;
                // } else {
                //     ev.events = EPOLLIN | EPOLLHUP | EPOLLET | EPOLLONESHOT;
                // }
                // if (epoll_ctl(
                //     epollFd,
                //     EPOLL_CTL_MOD,
                //     conn->getNativeHandle(),
                //     &ev
                // ) < 0) {
                //     RavenLog("Failed to queue for next read and/or write: %s\n", strerror(errno));
                //     delete conn;
                // }
            }
        }
    }
}

}
