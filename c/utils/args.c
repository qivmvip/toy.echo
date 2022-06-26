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

#define MODULE "gypsy.toy.echo.c.utils"
#define TAG "args"

#define VRB(fmt, ...) (vrb(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define WRN(fmt, ...) (wrn(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define ERR(fmt, ...) (err(MODULE, TAG, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__))
#define RAW(fmt, ...) (raw(fmt, __VA_ARGS__))

#define ARG_IP_PREFIX ("--ip=")
#define ARG_IP_PREFIX_LEN (strlen(ARG_IP_PREFIX))
#define ARG_PORT_PREFIX ("--port=")
#define ARG_PORT_PREFIX_LEN (strlen(ARG_PORT_PREFIX))
#define ARG_DATA_PREFIX ("--data=")
#define ARG_DATA_PREFIX_LEN (strlen(ARG_DATA_PREFIX))

bool args_is_ip(int i, char const* arg) {
  return 0 == strncmp(ARG_IP_PREFIX, arg, ARG_IP_PREFIX_LEN);
}

char const* args_parse_ip(int i, char const* arg, char const* failover) {
  char const* value = arg + ARG_IP_PREFIX_LEN;
  VRB("IP is provided by argv[%d] =>  [value::%s]", i, value);
  return value;
}

bool args_is_port(int i, char const* arg) {
  return 0 == strncmp(ARG_PORT_PREFIX, arg, ARG_PORT_PREFIX_LEN);
}

int args_parse_port(int i, char const* arg, int failover) {
  char const* value = arg + ARG_PORT_PREFIX_LEN;
  VRB("Port is provided by argv[%d] =>  [value::%s]", i, value);
  int port = atoi(value);
  if (port > 0) {
    return port;
  }
  WRN("Bad port argument argv[%d] =>  [value::%s]", i, value);
  return failover;
}

bool args_is_data(int i, char const* arg) {
  return 0 == strncmp(ARG_DATA_PREFIX, arg, ARG_DATA_PREFIX_LEN);
}

char const* args_parse_data(int i, char const* arg, char const* failover) {
  char const* value = arg + ARG_DATA_PREFIX_LEN;
  VRB("Data is provided by argv[%d] =>  [value::%s]", i, value);
  return value;
}
