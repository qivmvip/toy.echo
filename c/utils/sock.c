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
#include <stdlib.h>
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

#define VRB(module, tag, fmt, ...) ( \
  vrb(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define WRN(module, tag, fmt, ...) ( \
  wrn(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define ERR(module, tag, fmt, ...) ( \
  err(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)

#if defined(X_MULTICLIENT_FORK)
bool sock_accept(
  char const* module,
  char const* tag,
  int client_sockfd,
  char* buffer,
  size_t buffer_size
) {
  int const fork_result = fork();
  if (0 == fork_result) {
    sock_echo(module, tag, client_sockfd, buffer, buffer_size);
    close(client_sockfd);
    exit(EXIT_SUCCESS);
  } else {
    if (-1 == fork_result) {
      int error = errno;
      ERR(
        module,
        tag,
        "Fork child fail =>  [%d::%s]",
        error,
        strerror(error)
      );
      return false;
    } else {
      return true;
    }
  }
}
#endif

void
sock_echo(
  char const* module,
  char const* tag,
  int client_sockfd,
  char* buffer,
  size_t buffer_size
) {
  VRB(module, tag, "%s", "Echoing ...");
  while (true) {
    // read
    ssize_t const read_n = read(client_sockfd, buffer, buffer_size);
    if (-1 == read_n) {
      int const error = errno;
      WRN(
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
      WRN(
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
