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
# include <netinet/in.h>
# include <arpa/inet.h>
#elif defined (__linux__)
# include <unistd.h>
# include <sys/socket.h>
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

#if defined(X_MULTICLIENT_FORK)
static bool accept_client(int client_sockfd, char* buffer, size_t buffer_size) {
  int const fork_result = fork();
  if (0 == fork_result) {
    sock_echo(MODULE, TAG, client_sockfd, buffer, BUFFER_SIZE);
    close(client_sockfd);
    exit(EXIT_SUCCESS);
  } else {
    if (-1 == fork_result) {
      int error = errno;
      ERR("Fork child fail =>  [%d::%s]", error, strerror(error));
      return false;
    } else {
      return true;
    }
  }
}
#endif

int main(int argc, char** argv) {
  int port = PORT;
  int backlog = BACKLOG;

  // args
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
    if (args_is_port(i, argv[i])) {
      port = args_parse_port(i, argv[i], PORT);
      continue;
    }
    if (args_is_backlog(i, argv[i])) {
      backlog = args_parse_backlog(i, argv[i], BACKLOG);
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
  int listen_result = listen(server_sockfd, backlog);
  if (0 != listen_result) {
    int const error = errno;
    ERR("Listen server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // accept
  int sn = 0;
  char buffer[BUFFER_SIZE] = {0};
#if defined(X_USING_IPV6)
  x_sockaddr_in6_t client_addr = {0};
#else
  x_sockaddr_in_t client_addr = {0};
#endif
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
# if defined(X_USING_IPV6)
    if (AF_INET6 == client_addr_ptr->sa_family) {
      char addr_text[BUFFER_SIZE] = {0};
      char const* addr_text_result = inet_ntop(
        AF_INET6,
        &client_addr.sin6_addr,
        addr_text,
        BUFFER_SIZE
      );
      if (NULL == addr_text_result) {
        int const error = errno;
        WRN("inet_ntop fail =>  [%d::%s]", error, strerror(error));
      } else {
        VRB("Client address IPV6 =>  [%s]", addr_text_result);
      }
    }
#else
    if (AF_INET == client_addr_ptr->sa_family) {
      char addr_text[BUFFER_SIZE] = {0};
      char const* addr_text_result = inet_ntop(
        AF_INET,
        &client_addr.sin_addr,
        addr_text,
        BUFFER_SIZE
      );
      if (NULL == addr_text_result) {
        int const error = errno;
        WRN("inet_ntop fail =>  [%d::%s]", error, strerror(error));
      } else {
        VRB("Client address IPV4 =>  [%s]", addr_text_result);
      }
    }
# endif
# if defined(X_MULTICLIENT_FORK)
  for (int ac_retries = 0; ac_retries < 3; ++ac_retries) {
      if (accept_client(client_sockfd, buffer, BUFFER_SIZE)) {
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
