// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : YanJibin <qivmvip AT gmail DOT net>
// + date   : 2022-05-01
// + desc   : log utilities header


#ifndef OOO_LOONG_TOY_ECHO_C_LOG_H
#define OOO_LOONG_TOY_ECHO_C_LOG_H

void vrb(char const* module, char const* tag, char const* fmt, ...);

void dbg(char const* module, char const* tag, char const* fmt, ...);

void inf(char const* module, char const* tag, char const* fmt, ...);

void wrn(char const* module, char const* tag, char const* fmt, ...);

void err(char const* module, char const* tag, char const* fmt, ...);

void raw(char const* fmt, ...);

#endif // OOO_LOONG_TOY_ECHO_C_UNIX_LOG_H
