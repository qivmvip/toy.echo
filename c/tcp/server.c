/**
 * @file server.c
 * @author YanJibin (qivmvip AT gmail DOT com)
 * @brief TCP socket server
 * @version 0.1
 * @date 2022-04-30
 *
 * @copyright Copyright (c) 2022
 *
 */


#if !defined(__APPLE__)
# if defined(__linux__)
#   if !defined(_GNU_SOURCE)
#     define _GNU_SOURCE
#   endif
# endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#if defined(__APPLE__)
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#elif defined (__linux__)
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# error Unknown platform...
#endif

#include "../utils/log.h"
#include "../utils/args.h"
#include "../utils/sock.h"

#define MODULE "gypsy.toy.echo.c.tcp"
#define TAG "server"

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

#define BUFFER_SIZE (1024)

#define PORT (9987)
#define BACKLOG (666)
#define DATA ("hello")

static void sigint_handler(int sig) {
  RAW("%s1", "\n");
  WRN("Received [SIGINT::%d], exit with =>  [code::%d]", sig, EXIT_FAILURE);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  int port = PORT;
  int backlog = BACKLOG;

  // args
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
    if (args_is_port(MODULE, TAG, i, argv[i])) {
      port = args_parse_port(MODULE, TAG, i, argv[i], PORT);
      continue;
    }
    if (args_is_backlog(MODULE, TAG, i, argv[i])) {
      backlog = args_parse_backlog(MODULE, TAG, i, argv[i], BACKLOG);
      continue;
    }
  }

  VRB("Port is =>  [port::%d]", port);

  // SIGINT
  signal(SIGINT, sigint_handler);

#ifdef __APPLE__
# if defined(X_USING_IPV6)
    x_sockaddr_in6_t server_addr = {
      .sin6_len = sizeof(x_sockaddr_in6_t),
      .sin6_family = AF_INET6,
      .sin6_port = htons((in_port_t) port),
      .sin6_addr = IN6ADDR_ANY_INIT,
    };
# else
   x_sockaddr_in_t server_addr = {
      .sin_len = sizeof(x_sockaddr_in_t),
      .sin_family = AF_INET,
      .sin_port = htons((in_port_t) port),
      .sin_addr.s_addr = INADDR_ANY,
  };
# endif
#else
# if defined(X_USING_IPV6)
    x_sockaddr_in6_t server_addr = {
      .sin6_family = AF_INET6,
      .sin6_port = htons((in_port_t) port),
      .sin6_addr = IN6ADDR_ANY_INIT,
    };
# else
    x_sockaddr_in_t server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons((in_port_t) port),
      .sin_addr.s_addr = INADDR_ANY,
    };
# endif
#endif

  // socket
#if defined(X_USING_IPV6)
  int const server_sockfd = socket(AF_INET6, SOCK_STREAM, 0);
# else
  int const server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
  if (-1 == server_sockfd) {
    int const error = errno;
    ERR("Create server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // bind
  x_sockaddr_t const* server_addr_ptr = (x_sockaddr_t const*) &server_addr;
  socklen_t server_addr_len = (socklen_t) sizeof(server_addr);
  if (!sock_bind(
    MODULE, TAG, server_sockfd, server_addr_ptr, server_addr_len
  )) {
    return EXIT_FAILURE;
  }

  // listen
  if (!sock_listen(MODULE, TAG, server_sockfd, backlog)) {
    return EXIT_FAILURE;
  }

  int sn = 0;
  char buffer[BUFFER_SIZE] = {0};

#if defined(X_USING_IPV6)
  struct sockaddr_storage client_addr = {0};
#else
  struct sockaddr_storage client_addr = {0};
#endif
  x_sockaddr_t* client_addr_ptr = (x_sockaddr_t*) &client_addr;
  socklen_t client_addr_len = (socklen_t) sizeof(client_addr);

#if defined (X_MULTICLIENT_SELECT)
  fd_set active_fds;
  FD_ZERO(&active_fds);
  FD_SET(server_sockfd, &active_fds);
  int max_fd = server_sockfd + 1;

  while (true) {
    VRB("%s", "");
    VRB("#%#011x Server waiting...", ++sn);
    int const select_result = select(FD_SETSIZE, &active_fds, NULL, NULL, NULL);
    if (-1 == select_result) {
      return EXIT_FAILURE;
    }
    if (0 == select_result) {
      return EXIT_FAILURE;
    }
    VRB("Select max_fd =>  [%d]", max_fd);
    for (int fd = 0; fd < max_fd; ++fd) {
      if (FD_ISSET(fd, &active_fds)) {
        VRB("Select FD_ISSET =>  [%d]", fd);
        if (fd == server_sockfd) {
          VRB("Select is server [fd::%d]", fd);
          int const client_sockfd = sock_accept(
            MODULE, TAG, server_sockfd, client_addr_ptr, &client_addr_len
          );
          if (-1 == client_sockfd) {
            continue;
          }
          sock_dump_peer_addr(MODULE, TAG, client_addr_ptr);
          FD_SET(client_sockfd, &active_fds);
          if (client_sockfd >= max_fd) {
            max_fd = client_sockfd + 1;
          }
        } else {
          VRB("Select go is peer [fd::%d]", fd);
          sock_echo(MODULE, TAG, fd, buffer, BUFFER_SIZE);
          FD_CLR(fd, &active_fds);
          close(fd);
        }
      }
    }
  }
#endif

#if defined(X_MULTICLIENT_FORK)
  signal(SIGCHLD, SIG_IGN);
#endif

  while (true) {
    VRB("%s", "");
    VRB("#%#011x Server waiting...", ++sn);
    int const client_sockfd = sock_accept(
      MODULE,
      TAG,
      server_sockfd,
      client_addr_ptr,
      &client_addr_len
    );
    if (-1 == client_sockfd) {
      continue;
    }
    sock_dump_peer_addr(MODULE, TAG, client_addr_ptr);
# if defined(X_MULTICLIENT_FORK)
  for (int ac_retries = 0; ac_retries < 3; ++ac_retries) {
      if (sock_fork(MODULE, TAG, client_sockfd, buffer, BUFFER_SIZE)) {
        break;
      }
      usleep(11111);
  }
  close(client_sockfd);
# else
    sock_echo(MODULE, TAG, client_sockfd, buffer, BUFFER_SIZE);
    close(client_sockfd);
#endif
  }
  return EXIT_SUCCESS;
}
