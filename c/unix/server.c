/**
 * @file server.c
 * @author YanJibin (qivmvip AT gmail DOT com)
 * @brief unix socket server
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
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
#elif defined (__linux__)
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
#else
# error Unknown OS.
#endif

#include "../utils/log.h"
#include "../utils/sock.h"

#define BUFFER_SIZE (1024)
#define MODULE "gypsy.toy.echo.c.unix"
#define TAG "server"
#define PATH "gypsy.toy.echo.c.unix.sock"

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

static void sigint_handler(int sig) {
  RAW("%s1", "\n");
  WRN("Received [SIGINT::%d], exit with =>  [code::%d]", sig, EXIT_FAILURE);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
  }

  if (argc < 2) {
    WRN("No path provided, using default path =>  %s", PATH);
  }

  signal(SIGINT, sigint_handler);

#ifdef __APPLE__
  x_sockaddr_un_t server_addr = {
    .sun_len = sizeof(x_sockaddr_un_t),
    .sun_family = AF_UNIX,
  };
#else
  x_sockaddr_un_t server_addr = {
    .sun_family = AF_UNIX,
  };
#endif
  size_t addr_un_sun_path_size = sizeof(server_addr.sun_path);
  size_t arg_path_len = argc > 1 ? strlen(argv[1]) : strlen(PATH);
  char const* path = argc > 1 ? argv[1] : PATH;
  if (arg_path_len + 1 > addr_un_sun_path_size) {
    ERR("ERROR: Path is to long, longer then %zu", arg_path_len);
  }
  strncpy(server_addr.sun_path, path, addr_un_sun_path_size);
  server_addr.sun_path[addr_un_sun_path_size - 1] = '\0';

  // unlink
  int unlink_result = unlink(server_addr.sun_path);
  if (0 != unlink_result) {
    int const error = errno;
    WRN("Unlink path [path::%s] =>  [%d::%s]", path, error, strerror(error));
  }

  // socket
  int const server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == server_sockfd) {
    int const error = errno;
    ERR("Create server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // bind
  int bind_result = bind(
    server_sockfd,
    (x_sockaddr_t *) &server_addr,
    sizeof(server_addr)
  );
  if (0 != bind_result) {
    int const error = errno;
    ERR("Bind server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // listen
  int backlog = 666;
  if (argc > 2) {
    backlog = atoi(argv[2]);
  }
  if (backlog < 1) {
    backlog = 666;
  }
  int listen_result = listen(server_sockfd, backlog);
  if (0 != listen_result) {
    int const error = errno;
    ERR("Listen server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // accept
  int sn = 0;
  char buffer[BUFFER_SIZE] = {0};
  x_sockaddr_un_t client_addr = {0};
  socklen_t client_addr_len = sizeof(client_addr);
#if defined(X_MULTICLIENT_FORK)
  signal(SIGCHLD, SIG_IGN);
#endif
  while (true) {
    VRB("%s", "");
    VRB("#%#011x Server waiting...", ++sn);
    memset(&client_addr, 0, client_addr_len);
    x_sockaddr_t* client_addr_ptr = (x_sockaddr_t*) &client_addr;
    int const client_sockfd = accept(
      server_sockfd, client_addr_ptr, &client_addr_len
    );
    if (-1 == client_sockfd) {
      int const error = errno;
      ERR("Accept client fail =>  [%d::%s]", error, strerror(error));
      continue;
    }
    if (AF_UNIX == client_addr_ptr->sa_family) {
      VRB("Client address path =>  [%s]", client_addr.sun_path);
    }
# if defined(X_MULTICLIENT_FORK)
  for (int ac_retries = 0; ac_retries < 3; ++ac_retries) {
      if (sock_accept(MODULE, TAG, client_sockfd, buffer, BUFFER_SIZE)) {
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
