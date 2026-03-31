#include "raven/ConnectionPool.hpp"
#include "raven/Socket.hpp"
#include "raven/SocketServer.hpp"
#include <iostream>

int main() {
    auto placeholder = 128;
    const std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGood girl :3\r\n";
    raven::SocketServer serv {
        raven::SocketConfig{
            .queue = placeholder,
            .type = raven::SocketType::Stream,
            .port = 62169,
            .ip = "127.0.0.1",
        },
        raven::ServerConfig {},
        raven::ConnPoolConfig {
            .onRecv = [&](auto* conn, auto& buff, size_t length) {
                std::cout << "Received:\n" << std::string(buff.data(), length) << std::endl;
                conn->queueWrite(
                    [&](std::array<char, 16'384>& buff, size_t lastIdx) -> size_t {
                        if (lastIdx >= response.size()) {
                            return 0;
                        }

                        return (size_t) response.copy(
                            buff.data(),
                            response.size() - lastIdx,
                            lastIdx
                        );
                    }
                );
            },
            .onWriteComplete = nullptr
        }
    };

    serv.start();
    serv.waitForDone();
}
