#include "include/http_server.h"
#include <cstdint>

int main() {
  uint16_t port;
  port = 8181;

  HttpServer server(port);
  server.start();
  return 0;
}
