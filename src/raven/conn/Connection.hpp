#pragma once

#include <string>
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
public:
    virtual ~Connection() = default;

    virtual bool isOpen() { return open; }
    virtual bool isClosed() { return closed; }

    virtual void close() = 0;

    virtual void write(const std::string& buff) = 0;
};

}
