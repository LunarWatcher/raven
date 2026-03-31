#include "raven/SocketServer.hpp"
#include "raven/conn/Connection.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <cpr/cpr.h>

TEST_CASE("HTTP server") {
    const std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGood girl :3";
    raven::SocketServer serv {
        raven::SocketConfig{
            .type = raven::SocketType::Stream,
            .port = 62262,
            .ip = "127.0.0.1",
        },
        raven::ServerConfig {},
        raven::ConnPoolConfig {
            .onRecv = [&](auto* conn, auto&, size_t) {
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

    SECTION("Responses should be received") {
        auto res = cpr::Get(
            cpr::Url{"http://localhost:62262"}
        );
        REQUIRE(res.status_code == 200);
        REQUIRE(res.status_line == "HTTP/1.1 200 OK");
        REQUIRE(res.text == "Good girl :3");
    }
}
