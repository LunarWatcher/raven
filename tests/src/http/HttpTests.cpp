#include "raven/SocketServer.hpp"
#include "raven/conn/Connection.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("HTTP server") {
    raven::SocketServer serv {
        raven::SocketConfig{
            .type = raven::SocketType::Stream,
            .port = 62169,
            .ip = "127.0.0.1",
        },
        raven::ServerConfig {},
        raven::ConnPoolConfig {
            .onRecvReady = [](auto* conn) {
                std::cout << "onRecvReady" << std::endl;
                std::array<char, 16384> buff;
                int flags = 0;
                size_t read = 0;
                std::string compiled;
                while (
                    (read = conn->read(buff, flags)) > 0
                ) {
                    compiled += std::string(
                        buff.begin(), read
                    );
                }
                if (flags == 0 || flags == raven::Connection::ReadFlags::Eof) {
                    // Strip out input headers
                    compiled = compiled.substr(
                        compiled.find("\r\n\r\n") + 4
                    );
                    conn->write(
                        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"
                        + compiled,
                        flags
                    );
                }
            }
        }
    };

    std::thread runner(
        [&serv]() { serv.startConnectionPool(); }
    );

    SECTION("It should be possible to connect") {
        
    }

    serv.close();
    std::cout << "Pre-join" << std::endl;
    runner.join();
}
