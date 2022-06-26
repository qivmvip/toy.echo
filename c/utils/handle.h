// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : YanJibin <qivmvip AT gmail DOT net>
// + date   : 2022-05-01
// + desc   : log utilities header


#ifndef OOO_LOONG_TOY_ECHO_C_HANDLE_H
#define OOO_LOONG_TOY_ECHO_C_HANDLE_H

#define HADNLE(handle) \
  struct handle##__ { \
    int unused; \
  }; \
  typedef struct handle##__* handle

#endif // OOO_LOONG_TOY_ECHO_C_LOG_H
