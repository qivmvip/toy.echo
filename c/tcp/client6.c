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

#define IP ("::1")
#define PORT (9987)
#define DATA ("hello")

#define ARG_IP_PREFIX ("--ip=")
#define ARG_IP_PREFIX_LEN (strlen(ARG_IP_PREFIX))
#define ARG_PORT_PREFIX ("--port=")
#define ARG_PORT_PREFIX_LEN (strlen(ARG_PORT_PREFIX))
#define ARG_DATA_PREFIX ("--data=")
#define ARG_DATA_PREFIX_LEN (strlen(ARG_DATA_PREFIX))

typedef struct sockaddr addr_t;
typedef struct sockaddr_in6 addr_in6_t;
typedef struct in6_addr in6_addr_t;

static void sigint_handler(int sig) {
  RAW("%s1", "\n");
  WRN("Received [SIGINT::%d], exit with =>  [code::%d]", sig, EXIT_FAILURE);
  exit(EXIT_FAILURE);
}

static char const* parse_arg_ip(int i, char const* arg) {
  char const* value = arg + ARG_IP_PREFIX_LEN;
  VRB("IP is provided by argv[%d] =>  [value::%s]", i, value);
  return value;
}

static int parse_arg_port(int i, char const* arg) {
  char const* value = arg + ARG_PORT_PREFIX_LEN;
  VRB("Port is provided by argv[%d] =>  [value::%s]", i, value);
  int port = atoi(value);
  if (port > 0) {
    return port;
  }
  WRN("Bad port argument argv[%d] =>  [value::%s]", i, value);
  return PORT;
}

static char const* parse_arg_data(int i, char const* arg) {
  char const* value = arg + ARG_DATA_PREFIX_LEN;
  VRB("Data is provided by argv[%d] =>  [value::%s]", i, value);
  return value;
}

int main(int argc, char** argv) {
  char const* ip = IP;
  int port = PORT;
  char const* data = DATA;

  // args
  VRB("argc =>  %d", argc);
  for (int i = 0; i < argc; ++i) {
    VRB("argv[%d]: %s", i, argv[i]);
    if (0 == strncmp(ARG_IP_PREFIX, argv[i], ARG_IP_PREFIX_LEN)) {
      ip = parse_arg_ip(i, argv[i]);
      continue;
    }
    if (0 == strncmp(ARG_PORT_PREFIX, argv[i], ARG_PORT_PREFIX_LEN)) {
      port = parse_arg_port(i, argv[i]);
      continue;
    }
    if (0 ==  strncmp(ARG_DATA_PREFIX, argv[i], ARG_DATA_PREFIX_LEN)) {
      data = parse_arg_data(i, argv[i]);
      continue;
    }
  }

  VRB("IP   is =>  [ip::%s]", ip);
  VRB("Port is =>  [port::%d]", port);
  VRB("Data is =>  [data::%s]", data);

  // SIGINT
  signal(SIGINT, sigint_handler);

  // server_addr
in6_addr_t server_in6_addr = {};
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
#ifdef __APPLE__
  addr_in6_t server_addr = {
    .sin6_len = sizeof(addr_in6_t),
    .sin6_family = AF_INET6,
    .sin6_addr = server_in6_addr,
    .sin6_port = htons((in_port_t) port),
  };
#else
  addr_in6_t server_addr = {
    .sin6_family = AF_INET6,
    .sin6_addr = server_in6_addr,
    .sin6_port = htons((in_port_t) port),
  };
#endif

  // socket
  int const sockfd = socket(AF_INET6, SOCK_STREAM, 0);
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
