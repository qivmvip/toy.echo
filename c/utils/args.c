// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : Yan Jibin <qivmvip AT gmail DOT net>
// + date   : 2022-06-27
// + desc   : log utilities implemetation.


#include <stdlib.h>
#include <string.h>
#include "./log.h"
#include "./args.h"

#define VRB(module, tag, fmt, ...) ( \
  vrb(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define WRN(module, tag, fmt, ...) ( \
  wrn(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define ERR(module, tag, fmt, ...) ( \
  err(module, tag, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__) \
)
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

#define ARG_BACKLOG_PREFIX ("--backlog=")
#define ARG_BACKLOG_PREFIX_LEN (strlen(ARG_IP_PREFIX))

#define ARG_IP_PREFIX ("--ip=")
#define ARG_IP_PREFIX_LEN (strlen(ARG_IP_PREFIX))

#define ARG_PORT_PREFIX ("--port=")
#define ARG_PORT_PREFIX_LEN (strlen(ARG_PORT_PREFIX))

#define ARG_DATA_PREFIX ("--data=")
#define ARG_DATA_PREFIX_LEN (strlen(ARG_DATA_PREFIX))

bool args_is_backlog(
  char const* module,
  char const* tag,
  int index,
  char const* arg
) {
  return 0 == strncmp(ARG_BACKLOG_PREFIX, arg, ARG_BACKLOG_PREFIX_LEN);
}

int
args_parse_backlog(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  int failover
) {
  char const* value = arg + ARG_BACKLOG_PREFIX_LEN;
  VRB(
    module,
    tag,
    "Backlog is provided by argv[%d] =>  [value::%s]",
    index,
    value
  );
  int backlog = atoi(value);
  if (backlog > 1) {
    return backlog;
  }
  WRN(
    module,
    tag,
    "Bad backlog argument argv[%d] =>  [value::%s]",
    index,
    value
  );
  return failover;
}

bool
args_is_ip(
  char const* module,
  char const* tag,
  int i,
  char const* arg
) {
  return 0 == strncmp(ARG_IP_PREFIX, arg, ARG_IP_PREFIX_LEN);
}

char const*
args_parse_ip(
  char const* module,
  char const* tag,
  int index,
  char const* arg,
  char const* failover
) {
  char const* value = arg + ARG_IP_PREFIX_LEN;
  VRB(
    module,
    tag,
    "IP is provided by argv[%d] =>  [value::%s]",
    index,
    value
  );
  return value;
}

bool
args_is_port(
  char const* module,
  char const* tag,
  int i,
  char const* arg
) {
  return 0 == strncmp(ARG_PORT_PREFIX, arg, ARG_PORT_PREFIX_LEN);
}

int
args_parse_port(
  char const* module,
  char const* tag,
  int i,
  char const* arg,
  int failover
) {
  char const* value = arg + ARG_PORT_PREFIX_LEN;
  VRB(module, tag, "Port is provided by argv[%d] =>  [value::%s]", i, value);
  int port = atoi(value);
  if (port > 0) {
    return port;
  }
  WRN(module, tag, "Bad port argument argv[%d] =>  [value::%s]", i, value);
  return failover;
}

bool
args_is_data(
  char const* module,
  char const* tag,
  int i,
  char const* arg
) {
  return 0 == strncmp(ARG_DATA_PREFIX, arg, ARG_DATA_PREFIX_LEN);
}

char const*
args_parse_data(
  char const* module,
  char const* tag,
  int i,
  char const* arg,
  char const* failover
) {
  char const* value = arg + ARG_DATA_PREFIX_LEN;
  VRB(module, tag, "Data is provided by argv[%d] =>  [value::%s]", i, value);
  return value;
}
