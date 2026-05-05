#include "Connection.hpp"
#include "raven/conn/CommonDefs.hpp"

namespace raven {

size_t Connection::writeBuffers(Buffer& buff) {
    if (writeQueue.empty()) {
        return 0;
    }

    auto& front = writeQueue.front();
    int flags = 0;
    size_t totalWritten = 0;

    do {
        size_t available = front.func(buff, front.lastIndex);
        if (available == 0) {
            writeQueue.pop();
            return totalWritten;
        }
        size_t realWrite = write(buff, available, flags);

        front.lastIndex += realWrite;
        totalWritten += realWrite;

        if (flags == Connection::WriteFlags::Closed) {
            this->close();
            break;
        } else if (flags != 0 || realWrite == 0) {
            break;
        }
    } while (true);
    return totalWritten;
}

void Connection::queueWrite(const WriteCallback& callback) {
    // TODO: this is not thread-safe, but I don't think it matters. Since the connections are not to be persisted,
    // concurrent writes should never happen
    this->writeQueue.push(WriteBuffer {
        callback, 0
    });
}

}
