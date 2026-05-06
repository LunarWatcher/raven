#include "raven/SocketServer.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <catch2/benchmark/catch_benchmark.hpp>

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
                    [&](raven::Buffer& buff, size_t lastIdx) -> size_t {
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

    SECTION("Test IP extraction") {
        REQUIRE(serv.getPort() == 62262);
        REQUIRE(serv.getAssignedAddr() == "127.0.0.1");
    }

    SECTION("Responses should be received") {
        auto res = cpr::Get(
            cpr::Url{"http://localhost:62262"}
        );
        REQUIRE(res.status_code == 200);
        REQUIRE(res.status_line == "HTTP/1.1 200 OK");
        REQUIRE(res.text == "Good girl :3");
    }
}

TEST_CASE("Naive benchmark", "[benchmark]") {
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
                    [&](raven::Buffer& buff, size_t lastIdx) -> size_t {
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

    BENCHMARK("Single-threaded bomb") {
        auto res = cpr::Get(
            cpr::Url{"http://localhost:62262"}
        );
        REQUIRE(res.status_code == 200);
        return res;
    };

    serv.close();
}

TEST_CASE("Test random assignment") {
    const std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGood girl :3";
    raven::SocketServer serv {
        raven::SocketConfig{
            .type = raven::SocketType::Stream,
            .port = 0,
            .ip = std::nullopt,
        },
        raven::ServerConfig {},
        raven::ConnPoolConfig {
            .onRecv = [&](auto* conn, auto&, size_t) {
                conn->queueWrite(
                    [&](raven::Buffer& buff, size_t lastIdx) -> size_t {
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

    REQUIRE(serv.getPort() != 0);
    REQUIRE(serv.getAssignedAddr() != "");

    auto res = cpr::Get(
        cpr::Url{
            std::format(
                "http://{}:{}",
                serv.getAssignedAddr(),
                serv.getPort()
            )
        }
    );
    REQUIRE(res.status_code == 200);
    REQUIRE(res.status_line == "HTTP/1.1 200 OK");
    REQUIRE(res.text == "Good girl :3");
}
