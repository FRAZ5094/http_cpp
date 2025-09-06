#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

struct test {
  char s;
  char s2;
};

int main() {
  int port = 8181;

  int listen_fd;

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  struct in_addr addr{.s_addr = htonl(INADDR_ANY)};

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET, .sin_port = htons(port), .sin_addr = addr

  };

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind");
    return 1;
  }

  if (listen(listen_fd, 10) == -1) {
    perror("listen");
    return 1;
  }

  printf("Server started on http://localhost:%d\n", port);

  char buffer[256];

  while (1) {
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_size = sizeof(conn_addr);
    int conn_fd =
        accept(listen_fd, (struct sockaddr *)&conn_addr, &conn_addr_size);

    if (conn_fd == -1) {
      perror("accept");
      return 1;
    }

    printf("Client connected!\n");

    printf("Client addr: %s\n", inet_ntoa(conn_addr.sin_addr));
    printf("Client port: %u\n", ntohs(conn_addr.sin_port));

    uint16_t version;
    read(conn_fd, &version, sizeof(version));
    printf("Data from client=%u\n", ntohs(version));
    return 0;
  }
}
