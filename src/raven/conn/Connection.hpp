#pragma once

#include "raven/ip/IP.hpp"
#include <queue>
#include <string>

#include "CommonDefs.hpp"

namespace raven {

class Connection {
protected:
    // we differentiate between closed and not opened yet.
    // isOpen == false && isCLosed == false => not initialised
    // isOpen == true && isClosed == false => open
    // isOpen == false && isClosed == true => closed
    // both true: you did a dumdum
    bool closed = false;
    bool open = false;

    std::queue<WriteBuffer> writeQueue;

    ip::IP ip;

    /**
     * Attempts to read from the socket. This interacts directly with the socket, and must not be used by implementing
     * applications. Use the other form of read instead.
     *
     * \param buff          the buffer to read into
     * \param flags[out]    Additional information about the read. If nothing noteworthy, this is set to 0.
     *                      This is usually used to signal errors. For valid values, see ReadFlags.
     * \returns             The size of the bytes read. 0 means no bytes read, but this does not necessarily convey an
     *                      error.
     */
    virtual size_t read(
        Buffer& buff,
        int& flags
    ) = 0;

    /**
     * Attempts to write to the socket. This interacts directly with the socket, and must not be used by implementing
     * applications. Use the other form of write instead.
     *
     * \param buff          The buffer to write from
     * \param length        The number of characters available in the buffer
     * \param flags[out]    Additional information about the write. If nothing noteworthy, this is set to 0.
     *                      This is usually used to signal errors. For valid values, see WriteFlags
     * \returns             The size of the bytes written. 0 means no bytes written, but this does not necessarily
     *                      convey an error
     */
    virtual size_t write(
        Buffer& buff,
        size_t length,
        int& flags
    ) = 0;

    /**
     * Writes blocks until the OS buffers are full, and returns the total size written. Returns 0 if nothing can be
     * written.
     */
    virtual size_t writeBuffers(Buffer& buff);
public:
    /**
     * Contains flags specific to reads.
     */
    struct ReadFlags {
        static constexpr int BufferEmpty = 1;
        static constexpr int Error = 2;
        /**
         * Signals that the connection has been closed
         */
        static constexpr int Closed = 4;
    };
    /**
     * Contains flags specific to writes.
     *
     * Note that compatibility with ReadFlags is attempted, but cannot be guaranteed.
     */
    struct WriteFlags {
        static constexpr int BufferFull = 1;
        static constexpr int Error = 2;
        static constexpr int Closed = 4;
    };

    Connection(ip::IP&& ip)
        : ip(std::move(ip)) {}
    virtual ~Connection() = default;

    virtual bool isOpen() { return open; }
    virtual bool isClosed() { return closed; }

    virtual void close() = 0;
    virtual int getNativeHandle() = 0;

    /**
     * Queues a write. The write is performed asynchronously, so this function returns instantly.
     */
    virtual void queueWrite(const WriteCallback& callback);

    const std::string& getIP() { return ip.dotNotation; }
    bool hasWriteableBuffers() { return !writeQueue.empty(); }

    friend class ConnectionPool;
};


}
