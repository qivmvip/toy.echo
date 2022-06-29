// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : Yan Jibin <qivmvip AT gmail DOT net>
// + date   : 2022-06-28
// + desc   : sock utilities implemetation.


#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#if defined(__APPLE__)
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#elif defined (__linux__)
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# error Unknown platform...
#endif

#include "./sock.h"
#include "./log.h"

#define MODULE "gypsy.toy.echo.c.utils"
#define TAG "sock"

#define VRB(fmt, ...) ( \
  vrb(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define WRN(fmt, ...) ( \
  wrn(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define ERR(fmt, ...) ( \
  err(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

#define VRB_X(module, tag, fmt, ...) ( \
  vrb(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define WRN_X(module, tag, fmt, ...) ( \
  wrn(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define ERR_X(module, tag, fmt, ...) ( \
  err(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)


void
sock_echo(
  char const* module,
  char const* tag,
  int client_sockfd,
  char* buffer,
  size_t buffer_size
) {
  VRB("%s", "Echoing ...");
  while (true) {
    // read
    ssize_t const read_n = read(client_sockfd, buffer, buffer_size);
    if (-1 == read_n) {
      int const error = errno;
      WRN_X(
        module,
        tag,
        "Read socket#%d fail =>  [%d::%s]",
        client_sockfd,
        error,
        strerror(error)
      );
      break;
    }
    if (0 == read_n) {
      fputc('\n', stdout);
      fflush(stdout);
      break;
    }
    for (int i = 0; i < read_n; ++i) {
      fputc(buffer[i], stdout);
      fflush(stdout);
    }
    // write
    ssize_t const write_n = write(client_sockfd, buffer, read_n);
    if (read_n != write_n) {
      int const error = errno;
      WRN_X(
        module,
        tag,
        "Write client socket#%d fail =>  [%d::%s]",
        client_sockfd,
        error,
        strerror(error)
      );
    }
  }
}
