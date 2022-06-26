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
#define TAG "client"

#define VRB(fmt, ...) (vrb(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define WRN(fmt, ...) (wrn(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define ERR(fmt, ...) (err(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

#define BUFFER_SIZE (1024)

#if defined(X_USING_IPV6)
# define IP ("::1")
#else
# define IP ("127.0.0.1")
#endif
#define PORT (9987)
#define DATA ("hello")

static void sigint_handler(int sig) {
  RAW("%s1", "\n");
  WRN("Received [SIGINT::%d], exit with =>  [code::%d]", sig, EXIT_FAILURE);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  char const* ip = IP;
  int port = PORT;
  char const* data = DATA;

  // args
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
    if (args_is_ip(i, argv[i])) {
      ip = args_parse_ip(i, argv[i], IP);
      continue;
    }
    if (args_is_port(i, argv[i])) {
      port = args_parse_port(i, argv[i], PORT);
      continue;
    }
    if (args_is_data(i, argv[i])) {
      data = args_parse_data(i, argv[i], DATA);
      continue;
    }
  }

  VRB("IP   is =>  [ip::%s]", ip);
  VRB("Port is =>  [port::%d]", port);
  VRB("Data is =>  [data::%s]", data);

  // SIGINT
  signal(SIGINT, sigint_handler);

  // server_addr
#if defined(X_USING_IPV6)
  x_in6_addr_t server_in6_addr = {};
  int inet_pton_result = inet_pton(AF_INET6, ip, &server_in6_addr);
  if (1 != inet_pton_result) {
    if (0 == inet_pton_result) {
      ERR("inet_pton, invalid AF_INET6 IP =>  [ip::%s]", ip);
      return EXIT_FAILURE;
    } else {
      int const error = errno;
      ERR(
        "inet_pton, [ip::%s] fail =>  [%d::%s]",
        ip,
        error,
        strerror(error)
      );
      return EXIT_FAILURE;
    }
  }
#else
  x_in_addr_t server_in_addr = {};
  int inet_pton_result = inet_pton(AF_INET, ip, &server_in_addr);
  if (1 != inet_pton_result) {
    if (0 == inet_pton_result) {
      ERR("inet_pton, invalid AF_INET IP =>  [ip::%s]", ip);
      return EXIT_FAILURE;
    } else {
      int const error = errno;
      ERR(
        "inet_pton, [ip::%s] fail =>  [%d::%s]",
        ip,
        error,
        strerror(error)
      );
      return EXIT_FAILURE;
    }
  }
#endif
#ifdef __APPLE__
# if defined(X_USING_IPV6)
  x_addr_in6_t server_addr = {
    .sin6_len = sizeof(x_addr_in6_t),
    .sin6_family = AF_INET6,
    .sin6_addr = server_in6_addr,
    .sin6_port = htons((in_port_t) port),
  };
# else
  x_addr_in_t server_addr = {
    .sin_len = sizeof(x_addr_in_t),
    .sin_family = AF_INET,
    .sin_addr = server_in_addr,
    .sin_port = htons((in_port_t) port),
  };
# endif
#else
# if defined(X_USING_IPV6)
  x_addr_in6_t server_addr = {
    .sin6_family = AF_INET6,
    .sin6_addr = server_in6_addr,
    .sin6_port = htons((in_port_t) port),
  };
# else
  x_addr_in_t server_addr = {
    .sin_family = AF_INET,
    .sin_addr = server_in_addr,
    .sin_port = htons((in_port_t) port),
  };
# endif
#endif

  // socket
#if defined(X_USING_IPV6)
  int const sockfd = socket(AF_INET6, SOCK_STREAM, 0);
#else
  int const sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
  if (-1 == sockfd) {
    int const error = errno;
    ERR("Create socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // connect
  int connect_result = connect(
    sockfd,
    (x_addr_t *) &server_addr,
    sizeof(server_addr)
  );
  if (0 != connect_result) {
    int const error = errno;
    ERR("Connect server socket fail =>  [%d::%s]", error, strerror(error));
    return EXIT_FAILURE;
  }

  // data_size
  ssize_t const data_size = strlen(data) + 1;

  // write
  char const* w_buffer = data;
  ssize_t w_pending_size = data_size;
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
  ssize_t r_pending_size = data_size;
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
