#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int send_startup_message(int fd) {
  int16_t protocol_major_version = 3;
  int16_t protocol_minor_version = 0;
  char user_name[] = "postgres";
  char db[] = "mux_dev";
  char application_name[] = "raw_c_postgres";
  char encoding[] = "UTF8";

  char params[] = "user\0fraser\0"
                  "database\0mux_dev\0"
                  "application_name\0"
                  "psql\0"
                  "client_encoding\0UTF8";
  // This won't work because the null terminations make strlen think the string
  // ends early
  // int32_t msg_len =
  //     sizeof(int32_t) + sizeof(int16_t) + sizeof(int16_t) + strlen(params);
  int32_t msg_len = sizeof(int32_t) + sizeof(int16_t) + sizeof(int16_t) + 72;
  char msg[msg_len];
  sprintf(msg, "%u%u%u%s", msg_len, protocol_major_version,
          protocol_minor_version, params);
  if (send(fd, msg, sizeof(msg), 0) == -1) {
    handle_error("send");
    return -1;
  }
  return 0;
}

int main() {
  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr{.sun_family = AF_UNIX,
                          .sun_path = "/var/run/postgresql/.s.PGSQL.5432"};

  if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    handle_error("connect");
  }

  printf("Connected to postgres\n");

  int ret = send_startup_message(sock_fd);
  sleep(1);
  char buff[1024];
  int n = read(sock_fd, buff, sizeof(buff));

  close(sock_fd);
};
