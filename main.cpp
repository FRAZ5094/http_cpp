#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <optional>
#include <stdio.h>
#include <string>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

void handle_perror(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
void handle_error(const char *msg) {
  printf("%s\n", msg);
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
  struct Param params[]{{"user", "postgres"},
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
    handle_perror("send");
    return -1;
  }
  return 0;
}

int send_simple_query(int conn_fd, char *query) {
  int query_len = strlen(query);

  // Q + int32_t + strlen (Postgres doesn't want the null added onto the length
  // but still wants it to be transferred (see +1 below))
  int msg_len = 1 + sizeof(int32_t) + query_len;
  int32_t net_msg_len = htonl(msg_len);

  char msg[msg_len];
  char *msg_p = msg;

  *msg_p++ = 'Q';

  memcpy(msg_p, &net_msg_len, sizeof(int32_t));
  msg_p += sizeof(int32_t);

  strcpy(msg_p, query);
  // + 1 to include the null terminator on the end of the string
  if (send(conn_fd, msg, sizeof(msg) + 1, 0) == -1) {
    handle_perror("send");
    return -1;
  }

  return 0;
}

struct Field {
  std::string name;
  // int32_t table_object_id;
  // int16_t col_attribute_number;
  // int32_t data_type_object_id;
  // int16_t data_type_size;
  // int32_t type_modifier;
  // int16_t format_code;
};

std::vector<struct Field> parse_row_dec(char **p) {
  char *p_in = *p;
  if (*p_in != 'T') {
    handle_error("Response is not a row description");
  }
  p_in++;

  int32_t row_desc_len = ntohl(*(int32_t *)p_in);
  p_in += sizeof(int32_t);

  int16_t n_fields = ntohs(*(int16_t *)p_in);
  p_in += sizeof(int16_t);

  std::vector<struct Field> fields(n_fields);

  for (int i = 0; i < n_fields; i++) {
    struct Field field;
    field.name = p_in;
    p_in += field.name.size() + 1;

    // // SELECT 'table_name'::regclass::oid;
    // memcpy(&field.table_object_id, p, sizeof(int32_t));
    // field.table_object_id = ntohl(field.table_object_id);
    // p += sizeof(int32_t);

    // memcpy(&field.col_attribute_number, p, sizeof(int16_t));
    // // select * from information_schema.columns where table_name =
    // 'table_name' field.col_attribute_number =
    // ntohs(field.col_attribute_number); p += sizeof(int16_t);

    p_in += 18;

    fields[i] = field;
  }

  // move the outer pointer to the point we ended at, this is to move to the
  // next message
  *p = p_in;
  return fields;
};

std::vector<std::optional<std::string>> parse_data_row(char **p) {
  char *p_in = *p;

  if (*p_in++ != 'D') {
    handle_error("This is not a DataRow message");
  }

  int32_t msg_len;
  memcpy(&msg_len, p_in, sizeof(int32_t));
  msg_len = ntohl(msg_len);
  p_in += sizeof(int32_t);

  // printf("Message length: %u\n", msg_len);

  int16_t n_values;
  memcpy(&n_values, p_in, sizeof(int16_t));
  n_values = ntohs(n_values);
  p_in += sizeof(int16_t);

  // printf("Number of values: %u\n", n_values);

  std::vector<std::optional<std::string>> values(n_values);

  for (int i = 0; i < n_values; i++) {
    int32_t value_len;
    memcpy(&value_len, p_in, sizeof(int32_t));
    value_len = ntohl(value_len);
    p_in += sizeof(int32_t);

    if (value_len > 0) {
      std::string value = std::string(p_in, p_in + value_len);
      values[i] = value;
      p_in += value.size();
    } else {
      values[i] = {};
    }
  }

  *p = p_in;

  return values;
}
int main() {
  // std::vector<char> vec;

  // char buff2[10];
  // for (int i = 0; i < 10; i++) {
  //   buff2[i] = i;
  // }

  // vec.insert(vec.end(), buff2, buff2 + 10);

  auto main_start = std::chrono::high_resolution_clock::now();
  int pg_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr{.sun_family = AF_UNIX,
                          .sun_path = "/var/run/postgresql/.s.PGSQL.5432"};

  if (connect(pg_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    handle_perror("connect");
  }

  // printf("Connected to postgres\n");

  int ret = send_startup_message(pg_fd);
  char buff[1024];
  int n = read(pg_fd, buff, sizeof(buff));

  if (n < 0) {
    handle_perror("read");
  }

  char *buff_p = buff;

  if (buff_p[0] != 'R') {
    printf("Recieved an unexpected server response\n");
  }
  buff_p++;

  int32_t msg_len = ntohl(*(int32_t *)buff_p);
  buff_p += sizeof(int32_t);

  int32_t error = ntohl(*(int32_t *)buff_p);
  buff_p += sizeof(int32_t);

  if (error != 0) {
    handle_error("Failed to authenticate");
  }

  // printf("Successfully authenticated\n");

  // ignore the rest of the ParameterStatus (S) message because I don't care
  // https://wp.keploy.io/wp-content/uploads/2024/12/ReadyForQuery.png

  char query[] = "SELECT * FROM performed_set_metric;";
  auto start = std::chrono::high_resolution_clock::now();
  send_simple_query(pg_fd, query);

  const int read_buff_size = 1024;

  int n2 = read_buff_size;
  char read_buff[read_buff_size];

  std::vector<char> data_buff;
  data_buff.reserve(read_buff_size);

  while (n2 == read_buff_size) {
    n2 = read(pg_fd, read_buff, sizeof(read_buff));
    data_buff.insert(data_buff.end(), read_buff, read_buff + n2);
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

  printf("Duration of query: %ld us\n", dt);

  error = close(pg_fd);
  if (error == -1) {
    handle_perror("close");
  }

  start = std::chrono::high_resolution_clock::now();
  char *res_buff_p = data_buff.data();

  std::vector<Field> fields = parse_row_dec(&res_buff_p);
  std::vector<std::vector<::std::optional<std::string>>> rows;
  while (*res_buff_p == 'D') {
    std::vector<std::optional<std::string>> values =
        parse_data_row(&res_buff_p);
    rows.push_back(values);
  }

  printf("Number of rows: %zu\n", rows.size());
  end = std::chrono::high_resolution_clock::now();

  dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
           .count();

  printf("Duration of parse: %ld us\n", dt);

  auto main_end = std::chrono::high_resolution_clock::now();

  dt = std::chrono::duration_cast<std::chrono::microseconds>(main_end -
                                                             main_start)
           .count();

  printf("Duration of whole program: %ld us\n", dt);

  return 0;
}
