// #include "include/http_server.h"
// #include <cstdint>
#include "include/http_server.h"
#include <chrono>
#include <cstdio>
#include <fcntl.h>
// #include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

void write_sendfile() {
  int fd = open("index.html", O_RDONLY);
  struct stat st;
  fstat(fd, &st);

  //  Write file to stdout
  sendfile(1, fd, NULL, st.st_size);
  close(fd);
}
void write_read_write() {
  int fd = open("index.html", O_RDONLY);
  struct stat st;
  fstat(fd, &st);

  int file_size = st.st_size;

  char buffer[file_size];

  read(fd, buffer, file_size);

  write(1, buffer, file_size);
  close(fd);
}

int N_ITERATIONS = 10000;

float bench_mark(void (*func)()) {
  long times[N_ITERATIONS];

  for (int i = 0; i < N_ITERATIONS; i++) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    times[i] = duration.count();
  }
  long sum = 0;

  for (auto time : times) {
    sum += time;
  }

  float avg = (float)sum / N_ITERATIONS;

  return avg;
}

int main() {

  HttpServer server(8181);

  server.start();

  // float sendfile_avg = bench_mark(&write_sendfile);
  // float read_write_avg = bench_mark(&write_read_write);

  // printf("sendfile avg: %.1f\n", sendfile_avg);
  // printf("read_write avg: %.1f\n", read_write_avg);

  return 0;
}
