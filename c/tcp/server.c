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
# error Unknown OS.
#endif

#include "../utils/log.h"

#define BUFFER_SIZE (1024)
#define MODULE "gypsy.toy.echo.c.unix"
#define TAG "server"
#define PATH "gypsy.toy.echo.c.unix.sock"
#define VRB(fmt, ...) (vrb(MODULE, TAG, fmt, __VA_ARGS__))
#define WRN(fmt, ...) (wrn(MODULE, TAG, fmt, __VA_ARGS__))
#define ERR(fmt, ...) (err(MODULE, TAG, fmt, __VA_ARGS__))
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

typedef struct sockaddr addr_t;
typedef struct sockaddr_in addr_in_t;

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
    WRN("No path provided, using default path number =>  %s", PATH);
  }
  // int port = atoi(argv[1]);
  // if (port <= 0) {
  //   WRN("Bad port number argument provided [arg::%s], using default port number =>  %s", PATH);
  // }

  signal(SIGINT, sigint_handler);

#ifdef __APPLE__
  addr_in_t server_addr = {
    .sin_len = sizeof(addr_in_t),
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(9987),
  };
#else
  addr_in_t server_addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(9987),
  };
#endif
  // size_t addr_un_sun_path_size = sizeof(server_addr.sun_path);
  // size_t arg_path_len = argc > 1 ? strlen(argv[1]) : strlen(PATH);
  // char const* path = argc > 1 ? argv[1] : PATH;
  // if (arg_path_len + 1 > addr_un_sun_path_size) {
  //   ERR("ERROR: Path is to long, longer then %zu", arg_path_len);
  // }
  // strncpy(server_addr.sun_path, path, addr_un_sun_path_size);
  // server_addr.sun_path[addr_un_sun_path_size - 1] = '\0';

  // unlink
  // int unlink_result = unlink(server_addr.sun_path);
  // if (0 != unlink_result) {
  //   int const error = errno;
  //   WRN("Unlink path [path::%s] =>  [%d::%s]", path, error, strerror(error));
  // }

  // socket
  int const server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
  addr_in_t client_addr = {0};
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
    if (AF_INET == client_addr_ptr->sa_family) {
      VRB("Client address path =>  [%s]", inet_ntoa(client_addr.sin_addr));
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
