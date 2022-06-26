/**
 * @file client.c
 * @author YanJibin (qivmvip AT gmail DOT com)
 * @brief TCP socket client
 * @version 0.1
 * @date 2022-05-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#if defined(__APPLE__)
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#elif defined (__linux__)
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# error Unknown OS.
#endif

#include "../utils/log.h"


#define BUFFER_SIZE (1024)
#define MODULE "gypsy.toy.echo.c.unix"
#define TAG "client"
#define PATH "gypsy.toy.echo.c.unix.sock"
#define DATA "hello"
#define VRB(fmt, ...) (vrb(MODULE, TAG, fmt, __VA_ARGS__))
#define WRN(fmt, ...) (wrn(MODULE, TAG, fmt, __VA_ARGS__))
#define ERR(fmt, ...) (err(MODULE, TAG, fmt, __VA_ARGS__))

typedef struct sockaddr addr_t;
typedef struct sockaddr_in addr_in_t;

int main(int argc, char** argv) {
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
  }

  // path
  if (argc < 2) {
    WRN("No path provided, unsing default path =>  %s", PATH);
  }
#ifdef __APPLE__
  addr_in_t server_addr = {
    .sin_len = sizeof(addr_in_t),
    .sin_family = AF_INET,
    .sin_addr.s_addr = inet_addr("127.0.0.1"),
    .sin_port = htons(9987),
  };
#else
  addr_in_t server_addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = inet_addr("127.0.0.1"),
    .sin_port = htons(9987),
  };
#endif
  // size_t addr_un_path_size = sizeof(server_addr.sun_path);
  // char const* path = argc > 1 ? argv[1] : PATH;
  // size_t arg_path_len = strlen(path);
  // if (arg_path_len + 1 > addr_un_path_size) {
  //   ERR("Path [path::%s] is to long, longer then %zu", path, arg_path_len);
  // }
  // strncpy(server_addr.sun_path, path, addr_un_path_size);
  // server_addr.sun_path[addr_un_path_size - 1] = '\0';

  // socket
  int const sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sockfd) {
    int const error = errno;
    ERR("Create socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // connect
  int connect_result = connect(
    sockfd,
    (addr_t *) &server_addr,
    sizeof(server_addr)
  );
  if (0 != connect_result) {
    int const error = errno;
    ERR("Connect server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // data
  if (argc < 3) {
    WRN("No data provided, unsing default data =>  %s", DATA);
  }
  char const* data = (argc > 2) ? argv[2] : DATA;
  ssize_t const size = strlen(data) + 1;

  // write
  char const* w_buffer = data;
  ssize_t w_pending_size = size;
  while (true) {
    ssize_t const n = write(sockfd, w_buffer, w_pending_size);
    if (-1 == n) {
      int const error = errno;
      WRN(
        "Write server#%d fail, error =>  [%d::%s]",
         sockfd,
         error,
         strerror(error)
      );
      break;
    }
    w_pending_size = w_pending_size - n;
    if (0 == w_pending_size) {
      break;
    }
  }

  // sleep(3);

  // read
  char r_buffer[BUFFER_SIZE] = {0};
  ssize_t r_pending_size = size;
  while (true) {
    ssize_t const n = read(sockfd, r_buffer, BUFFER_SIZE);
    if (-1 == n) {
      int const error = errno;
      WRN(
        "Read socket#%d fail, error =>  [%d::%s]",
         sockfd,
         error,
         strerror(error)
      );
      break;
    }
    for (int i = 0; i < n; ++i) {
      fputc(r_buffer[i], stdout);
    }
    fflush(stdout);
    r_pending_size = r_pending_size - n;
    if (0 == r_pending_size) {
      fputc('\n', stdout);
      fflush(stdout);
      break;
    }
  }
  close(sockfd);
  return EXIT_SUCCESS;
}
