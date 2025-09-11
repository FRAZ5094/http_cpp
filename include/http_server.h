#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <thread>
class HttpServer {
public:
  HttpServer(int port);
  ~HttpServer();
  int start();

private:
  int _server_fd;
  int _port;
  std::thread _handle_thread;
  int _accept_request();
  void _handle_clients();

  int _handle_request(int conn_fd);
};

#endif // HTTP_SERVER_H