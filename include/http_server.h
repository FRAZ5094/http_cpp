#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

class HttpServer {
public:
  HttpServer(int port);
  ~HttpServer();
  int start();

private:
  int _server_fd;
  int _port;

  int _accept_request();
  int _handle_request(int conn_fd);
};

#endif // HTTP_SERVER_H
