#include "catch2/benchmark/catch_benchmark.hpp"
#include "raven/SocketServer.hpp"
#include "raven/config/SSLConfig.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <cpr/ssl_options.h>

TEST_CASE("Test HTTPS", "[ssl]") {
    const std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGood girl :3";
    raven::SocketServer serv {
        raven::SocketConfig{
            .type = raven::SocketType::Stream,
            .port = 62262,
            .ip = "127.0.0.1",
            .sslConfig = raven::SSLConfig(
                std::filesystem::path{"certs/tests/cert.pem"},
                std::filesystem::path{"certs/tests/pk.pem"},
                true
            ),
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
        },
    };

    serv.start();

    SECTION("Responses should be received") {
        auto res = cpr::Get(
            cpr::Url{"https://localhost:62262"},
            cpr::VerifySsl{false}
        );
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.status_line == "HTTP/1.1 200 OK");
        REQUIRE(res.text == "Good girl :3");
    }
}

TEST_CASE("Naive benchmark (HTTPS)", "[ssl][benchmark]") {
    const std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGood girl :3";
    raven::SocketServer serv {
        raven::SocketConfig{
            .type = raven::SocketType::Stream,
            .port = 62262,
            .ip = "127.0.0.1",
            .sslConfig = raven::SSLConfig(
                std::filesystem::path{"certs/tests/cert.pem"},
                std::filesystem::path{"certs/tests/pk.pem"},
                true
            ),
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
            cpr::Url{"https://localhost:62262"},
            cpr::VerifySsl{false}
        );
        REQUIRE(res.status_code == 200);
        return res;
    };

    serv.close();
}
