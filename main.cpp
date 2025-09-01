#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>

int main() {

  int listen_fd;

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  struct in_addr addr{.s_addr = htonl(INADDR_ANY)};

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET, .sin_port = htons(8181), .sin_addr = addr

  };

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind");
    return 1;
  }

  if (listen(listen_fd, 10) == -1) {
    perror("listen");
    return 1;
  }

  while (1) {
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_size = sizeof(conn_addr);
    int conn_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL);

    printf("Client connected!\n");

    if (conn_fd == -1) {
      perror("accept");
      return 1;
    }
  }

  return 0;
}