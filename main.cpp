#include "include/http_server.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#define BYTE_TO_BINARY_PATTERN "0b%c%c%c%c %c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'),                    \
      ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'),                \
      ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),                \
      ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

void get_byte_str(char byte, char *str) {
  for (int i = 0; i < 8; i++) {
    if (byte & (1 << i)) {
      str[i] = '1';
    } else {
      str[i] = '0';
    }
  }
  str[8] = '\0';
};

int main() {

  HttpServer server(8181);

  char mask = 0xFF;

  mask &= ~(1 << 3);

  // char str[9];
  // get_byte_str(mask, str);

  // printf("0b%s\n", str);
  printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(mask));

  while (1) {
  }
}
