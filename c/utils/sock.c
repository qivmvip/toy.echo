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

#define BUFFER_SIZE (1024)

bool
sock_bind(
  char const* module,
  char const* tag,
  int sockfd,
  x_sockaddr_t const* addr,
  socklen_t addr_len
) {
  int bind_result = bind(sockfd, addr, addr_len);
  if (0 != bind_result) {
    int const error = errno;
    ERR(
      module,
      tag,
      "Bind socket#%d fail =>  [%d::%s]", sockfd, error, strerror(error)
    );
    return false;
  }
  return true;
}

bool
sock_listen(
  char const* module,
  char const* tag,
  int sockfd,
  int backlog
) {
  int result = listen(sockfd, backlog);
  if (0 != result) {
    int const error = errno;
    ERR(
      module,
      tag,
      "Listen socket#%d fail =>  [%d::%s]", sockfd, error, strerror(error)
    );
    return false;
  }
  return true;
}

int
sock_accept(
  char const* module,
  char const* tag,
  int sockfd,
  x_sockaddr_t* addr,
  socklen_t* addr_len
) {
  if (NULL != addr && NULL != addr_len) {
    memset(addr, 0, (size_t) *addr_len);
  }
  int accepted_sockfd = accept(sockfd, addr, addr_len);
  if (-1 == accepted_sockfd) {
    int const error = errno;
    ERR(
      module,
      tag,
      "Accept socket#%d fail =>  [%d::%s]", sockfd, error, strerror(error)
    );
  }
  return accepted_sockfd;
}

#if defined(X_MULTICLIENT_FORK)
bool
sock_fork(
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

int
sock_select(
  char const* module,
  char const* tag,
  int nfds,
  fd_set* readfds,
  fd_set* writefds,
  fd_set* exceptfds,
  struct timeval* timeout
) {
  int const selected = select(nfds, readfds, writefds, exceptfds, timeout);
  if (-1 == selected) {
    int const error = errno;
    ERR(module, tag, "Select fail =>  [%d::%s]", error, strerror(error));
  } else if (0 == selected) {
    ERR(module, tag, "%s", "Select timeout");
  } else {
    VRB(module, tag, "Select success =>  [selected::%d]", selected);
  }
  return selected;
}

void
sock_dump_peer_addr(
  char const* module,
  char const* tag,
  x_sockaddr_t const* addr
) {
  if (AF_INET == addr->sa_family) {
    x_sockaddr_in_t const* peer_addr = (x_sockaddr_in_t const*) addr;
    char addr_text[BUFFER_SIZE] = {0};
    char const* addr_text_result = inet_ntop(
      AF_INET,
      &(peer_addr->sin_addr),
      addr_text,
      BUFFER_SIZE
    );
    if (NULL == addr_text_result) {
      int const error = errno;
      WRN(
        module,
        tag,
        "inet_ntop AF_INET fail =>  [%d::%s]", error, strerror(error)
      );
    } else {
      VRB(
        module,
        tag,
        "Peer address AF_INET =>  [%s]", addr_text_result
      );
    }
  }
  else if (AF_INET6 == addr->sa_family) {
    x_sockaddr_in6_t const* peer_addr = (x_sockaddr_in6_t const*) addr;
    char addr_text[BUFFER_SIZE] = {0};
    char const* addr_text_result = inet_ntop(
      AF_INET6,
      &(peer_addr->sin6_addr),
      addr_text,
      BUFFER_SIZE
    );
    if (NULL == addr_text_result) {
      int const error = errno;
      WRN(
        module,
        tag,
        "inet_ntop AF_INET6 fail =>  [%d::%s]", error, strerror(error)
      );
    } else {
      VRB(
        module,
        tag,
        "Peer address AF_INET6 =>  [%s]", addr_text_result
      );
    }
  } else {
    ERR(
      module,
      tag,
      "Unknown Peer address [family::%d]", addr->sa_family
    );
  }
}

void
sock_echo(
  char const* module,
  char const* tag,
  int sockfd,
  char* buffer,
  size_t buffer_size
) {
  VRB(module, tag, "%s", "Echoing ...");
  while (true) {
    // read
    ssize_t const read_n = read(sockfd, buffer, buffer_size);
    if (-1 == read_n) {
      int const error = errno;
      WRN(
        module,
        tag,
        "Read socket#%d fail =>  [%d::%s]", sockfd, error, strerror(error)
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
    ssize_t const write_n = write(sockfd, buffer, read_n);
    if (read_n != write_n) {
      int const error = errno;
      WRN(
        module,
        tag,
        "Write socket#%d fail =>  [%d::%s]", sockfd, error, strerror(error)
      );
    }
  }
}
