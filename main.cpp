#include <cinttypes>
#include <cstdint>
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
#include <sys/un.h>
#include <unistd.h>

void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

struct Param {
  const char *key;
  const char *value;
};

size_t create_str_param_list(struct Param *params, int len, char *out) {
  // start at one to include the null terminator
  int total_len = 1;
  for (int i = 0; i < len; i++) {
    Param param = params[i];

    // for postgres params are in the form "{key}\0{value}\0"

    int key_len = strlen(param.key);
    strcpy(out, param.key);
    out += key_len;
    *out++ = '\0';
    int value_len = strlen(param.value);
    strcpy(out, param.value);
    out += value_len;
    *out++ = '\0';
    total_len += key_len + 1 + value_len + 1;
  }
  *out++ = '\0';
  return total_len;
};

int send_startup_message(int fd) {
  int16_t protocol_major_version = htons(3);
  int16_t protocol_minor_version = htons(0);
  char user_name[] = "postgres";
  char db[] = "mux_dev";
  char application_name[] = "raw_c_postgres";
  char encoding[] = "UTF8";
  // This won't work because the null terminations make strlen think the string
  // ends early
  struct Param params[]{{"user", "fraser"},
                        {"database", "mux_dev"},
                        {"application_name", "raw_c_postgres"}};
  char param_str[1000];
  int param_len =
      create_str_param_list(params, sizeof(params) / sizeof(Param), param_str);
  int32_t msg_len =
      sizeof(int32_t) + sizeof(int16_t) + sizeof(int16_t) + param_len;

  char msg[msg_len];
  char *ptr = msg;

  int32_t network_msg_len = htonl(msg_len);

  memcpy(ptr, &network_msg_len, sizeof(msg_len));
  ptr += sizeof(msg_len);

  memcpy(ptr, &protocol_major_version, sizeof(protocol_minor_version));
  ptr += sizeof(protocol_major_version);

  memcpy(ptr, &protocol_minor_version, sizeof(protocol_minor_version));
  ptr += sizeof(protocol_minor_version);

  memcpy(ptr, &param_str, param_len);

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
  char buff[1024];
  int n = read(sock_fd, buff, sizeof(buff));
  for (int i = 0; i < n; i++) {
    printf("%c", buff[i]);
  }
  printf("\n");

  close(sock_fd);
};
