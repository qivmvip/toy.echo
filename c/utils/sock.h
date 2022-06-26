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
#else
# error Unknown platform...
#endif

typedef struct sockaddr x_sockaddr_t;
typedef struct sockaddr_un x_sockaddr_un_t;
#if defined(X_USING_IPV6)
  typedef struct sockaddr_in6 x_sockaddr_in6_t;
  typedef struct in6_addr x_in6_addr_t;
# else
  typedef struct sockaddr_in x_sockaddr_in_t;
  typedef struct in_addr x_in_addr_t;
#endif

#endif // OOO_LOONG_TOY_ECHO_C_UTILS_SOCK_H
