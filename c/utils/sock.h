// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : YanJibin <qivmvip AT gmail DOT net>
// + date   : 2022-06-17
// + desc   : sock utilities header


#ifndef OOO_LOONG_TOY_ECHO_C_UTILS_SOCK_H
#define OOO_LOONG_TOY_ECHO_C_UTILS_SOCK_H

#if defined(__APPLE__)
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#elif defined (__linux__)
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/select.h>
#else
# error Unknown platform...
#endif

typedef struct sockaddr x_sockaddr_t;
typedef struct sockaddr_un x_sockaddr_un_t;

typedef struct sockaddr_in6 x_sockaddr_in6_t;
typedef struct in6_addr x_in6_addr_t;

typedef struct sockaddr_in x_sockaddr_in_t;
typedef struct in_addr x_in_addr_t;


bool
sock_bind(
  char const* module,
  char const* tag,
  int sockfd,
  x_sockaddr_t const* addr,
  socklen_t addr_len
);

bool
sock_listen(
  char const* module,
  char const* tag,
  int sockfd,
  int backlog
);

int
sock_accept(
  char const* module,
  char const* tag,
  int sockfd,
  x_sockaddr_t* addr,
  socklen_t* addr_len
);

bool
sock_fork(
  char const* module,
  char const* tag,
  int client_sockfd,
  char* buffer,
  size_t buffer_size
);

int
sock_select(
  char const* module,
  char const* tag,
  int nfds,
  fd_set *restrict readfds,
  fd_set *restrict writefds,
  fd_set *restrict exceptfds,
  struct timeval *restrict timeout
);

void
sock_dump_peer_addr(
  char const* module,
  char const* tag,
  x_sockaddr_t const* addr
);

void
sock_echo(
  char const* module,
  char const* tag,
  int sockfd,
  char* buffer,
  size_t buffer_size
);

#endif // OOO_LOONG_TOY_ECHO_C_UTILS_SOCK_H
