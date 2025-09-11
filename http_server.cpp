#include "include/http_server.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

HttpServer::HttpServer(int port) : _port(port) {}

HttpServer::~HttpServer() { close(_server_fd); }
int HttpServer::start() {
  int opt = 1;

  if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  struct in_addr addr{.s_addr = htonl(INADDR_ANY)};

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET, .sin_port = htons(_port), .sin_addr = addr

  };

  if (bind(_server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ==
      -1) {
    perror("bind");
    return 1;
  }

  if (listen(_server_fd, 10) == -1) {
    perror("listen");
    return 1;
  }

  printf("Server started on http://localhost:%d\n", _port);
  _handle_thread = std::thread(&HttpServer::_handle_clients, this);
  return 0;
}

void HttpServer::_handle_clients() {
  printf("Starting to handle clients\n");
  while (1) {
    int conn_fd = _accept_request();
    _handle_request(conn_fd);
  }
}

int HttpServer::_accept_request() {

  struct sockaddr_in conn_addr;
  socklen_t conn_addr_size = sizeof(conn_addr);
  int conn_fd =
      accept(_server_fd, (struct sockaddr *)&conn_addr, &conn_addr_size);
  printf("Client connected!\n");
  printf("Client addr: %s\n", inet_ntoa(conn_addr.sin_addr));
  printf("Client port: %u\n", ntohs(conn_addr.sin_port));
  return conn_fd;
}

int HttpServer::_handle_request(int conn_fd) {
  char buffer[1024 * 8];
  read(conn_fd, &buffer, sizeof(buffer));
  int file_fd = open("index.html", O_RDONLY);

  struct stat st;

  fstat(file_fd, &st);

  int file_len = st.st_size;

  std::string res = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " +
                    std::to_string(file_len) +
                    "\r\n"
                    "\r\n";
  auto start = std::chrono::high_resolution_clock::now();
  write(conn_fd, res.c_str(), res.length());
  sendfile(conn_fd, file_fd, NULL, file_len);
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  printf("Duration to write response: %ldus\n", duration.count());

  close(file_fd);
  close(conn_fd);
  return 0;
}