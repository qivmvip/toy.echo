// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : YanJibin <qivmvip AT gmail DOT net>
// + date   : 2022-06-27
// + desc   : args utilities header


#ifndef OOO_LOONG_TOY_ECHO_C_UTILS_ARGS_H
#define OOO_LOONG_TOY_ECHO_C_UTILS_ARGS_H

#include <stdbool.h>

bool
args_is_backlog(
  char const* module,
  char const* tag,
  int index,
  char const* arg
);

int
args_parse_backlog(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  int failover
);


bool args_is_ip(
  char const* module,
  char const* tag,
  int index,
  char const* arg
);

char const*
args_parse_ip(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  char const* failover
);

bool
args_is_port(
  char const* module,
  char const* tag,
  int index,
  char const* arg
);

int
args_parse_port(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  int failover
);

bool args_is_data(
  char const* module,
  char const* tag,
  int index,
  char const* arg
);

char const*
args_parse_data(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  char const* failover
);

#endif // OOO_LOONG_TOY_ECHO_C_UTILS_ARGS_H
