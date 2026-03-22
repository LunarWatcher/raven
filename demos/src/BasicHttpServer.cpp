#include "raven/Socket.hpp"
#include "raven/SocketServer.hpp"

int main() {
  auto placeholder = 128;
  raven::SocketServer serv{
      raven::SocketConfig{
          .queue = placeholder,
          .type = raven::SocketType::Stream,
          .port = 62169,
          .ip = "127.0.0.1",
      }
  };

  serv.startAcceptor();
}
