#include "include/http_server.h"
#include <cstdio>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {

  HttpServer server(8181);

  server.start();

  printf("After the server started");

  while (1) {
  }

  return 0;
}
