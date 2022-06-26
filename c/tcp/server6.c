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
# error Unknown platform.
#endif

#include "../utils/log.h"

#define MODULE "gypsy.toy.echo.c.tcp"
#define TAG "server6"

#define VRB(fmt, ...) (vrb(MODULE, TAG, fmt, __VA_ARGS__))
#define WRN(fmt, ...) (wrn(MODULE, TAG, fmt, __VA_ARGS__))
#define ERR(fmt, ...) (err(MODULE, TAG, fmt, __VA_ARGS__))
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

#define BUFFER_SIZE (1024)

#define PORT (9987)
#define DATA ("hello")

#define ARG_PORT_PREFIX ("--port=")
#define ARG_PORT_PREFIX_LEN (strlen(ARG_PORT_PREFIX))

typedef struct sockaddr addr_t;
typedef struct sockaddr_in6 addr_in6_t;

static void sigint_handler(int sig) {
  RAW("%s1", "\n");
  WRN("Received [SIGINT::%d], exit with =>  [code::%d]", sig, EXIT_FAILURE);
  exit(EXIT_FAILURE);
}

static int parse_arg_port(int i, char const* arg) {
  char const* value = arg + ARG_PORT_PREFIX_LEN;
  VRB("Port is provided by argv[%d] =>  [value::%s]", i, value);
  int port = atoi(value);
  if (port > 0) {
    return port;
  }
  WRN("Bad port argument argv[%d] =>  [value::%s]", i, arg);
  return PORT;
}

int main(int argc, char** argv) {
  int port = PORT;

  // args
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
    if (0 == strncmp(ARG_PORT_PREFIX, argv[i], ARG_PORT_PREFIX_LEN)) {
      port = parse_arg_port(i, argv[i]);
      continue;
    }
  }

  VRB("Port is =>  [port::%d]", port);

  // SIGINT
  signal(SIGINT, sigint_handler);

#ifdef __APPLE__
  addr_in6_t server_addr = {
    .sin6_len = sizeof(addr_in6_t),
    .sin6_family = AF_INET6,
    .sin6_port = htons((in_port_t) port),
    .sin6_addr = IN6ADDR_ANY_INIT,
  };
#else
  addr_in6_t server_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = htons((in_port_t) port),
    .sin6_addr = IN6ADDR_ANY_INIT,
  };
#endif

  // socket
  int const server_sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (-1 == server_sockfd) {
    int const error = errno;
    ERR("Create server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // bind
  int bind_result = bind(
    server_sockfd,
    (addr_t *) &server_addr,
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
  addr_in6_t client_addr = {0};
  socklen_t client_addr_len = sizeof(client_addr);
  while (true) {
    VRB("%s", "");
    VRB("#%#011x Server waiting...", ++sn);
    memset(&client_addr, 0, client_addr_len);
    addr_t* client_addr_ptr = (addr_t*) &client_addr;
    int const client_sockfd = accept(
      server_sockfd, client_addr_ptr, &client_addr_len
    );
    if (-1 == client_sockfd) {
      int const error = errno;
      ERR("Accept client fail =>  [%d::%s]", error, strerror(error));
      continue;
    }
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
    VRB("%s", "Echoing ...");
    while (true) {
      // read
      ssize_t const read_n = read(client_sockfd, buffer, BUFFER_SIZE);
      if (-1 == read_n) {
        int const error = errno;
        WRN(
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
          "Write client socket#%d fail =>  [%d::%s]",
          client_sockfd,
          error,
          strerror(error)
        );
      }
    }
    close(client_sockfd);
  }

  return EXIT_SUCCESS;
}
