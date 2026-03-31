#include "Connection.hpp"

namespace raven {

size_t Connection::writeBuffers() {
    if (writeQueue.empty()) {
        return 0;
    }

    auto& front = writeQueue.front();
    int flags = 0;
    size_t totalWritten = 0;
    // TODO: we should be centralising these to one buffer per thread. This would result in many fewer allocations
    std::array<char, 16'384> buff;

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

}
