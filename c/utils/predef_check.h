// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : YanJibin <qivmvip AT gmail DOT net>
// + date   : 2022-07-02
// + desc   : predef.h Predefined check macros.


#ifndef OOO_LOONG_TOY_ECHO_C_PREDEF_CHECK_H
#define OOO_LOONG_TOY_ECHO_C_PREDEF_CHECK_H

#if (defined(X_MULTICLIENT_FORK) && defined(X_MULTICLIENT_SELECT))
# error "X_MULTICLIENT_FORK and X_MULTICLIENT_SELECT are mutually exclusive."
#endif

#endif // OOO_LOONG_TOY_ECHO_C_PREDEF_CHECK_H
