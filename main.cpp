#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  uint16_t port;
  port = 8181;

  int listen_fd;
  int opt = 1;

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
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

  char buffer[1024 * 8];

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

    read(conn_fd, &buffer, sizeof(buffer));
    printf("Data from client:\n%s\n", buffer);
    char const *html = "This is my html";
    char const content_length = strlen(html);
    std::string res = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: " +
                      std::to_string(content_length) +
                      "\r\n"
                      "\r\n"
                      "This is my html";

    write(conn_fd, res.c_str(), res.length());
    close(conn_fd);
  }
  close(listen_fd);
  return 0;
}
