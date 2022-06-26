// +------------------------------------------------------------------------+
// | Copyright (c) YanJibin <qivmvip AT gmail DOT net> All rights reserved. |
// | Licensed under the MIT license,  see LICENSE file in the project root. |
// +------------------------------------------------------------------------+
// + author : Yan Jibin <qivmvip AT gmail DOT net>
// + date   : 2022-05-01
// + desc   : log utilities implemetation.

#if !defined(__APPLE__)
# if defined(__linux__)
#   if !defined(_GNU_SOURCE)
#     define _GNU_SOURCE
#   endif
# endif
#endif

#ifdef __STDC_LIB_EXT1__
# ifndef  __STDC_WANT_LIB_EXT1__
#   undef __STDC_WANT_LIB_EXT1__
#   define __STDC_WANT_LIB_EXT1__ (1)
# endif // __STDC_WANT_LIB_EXT1__
#else
# include <stdarg.h>
# include <stdio.h>
# include <string.h>
# include <time.h>
  static int sprintf_s(char* buffer, size_t bufsz, const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    int result = vsprintf(buffer, format, vlist);
    va_end(vlist);
    return result;
  }
  static struct tm *localtime_s(time_t const* restrict time,
                                struct tm* restrict buffer) {
    struct tm* tmp = localtime(time);
    return (struct tm*) memcpy(buffer, tmp, sizeof(struct tm));
  }
#endif // __STDC_LIB_EXT1__
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <memory.h>
#if !defined(__STDC_NO_ATOMICS__)
# include <stdatomic.h>
#endif
#if !defined(__STDC_NO_THREADS__)
# include <threads.h>
#else
# include <pthread.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__APPLE__)
# include <pthread.h>
#endif

#include "./log.h"

typedef struct tm tm_t;
typedef unsigned long long int ull_t;

#if defined(__STDC_NO_ATOMICS__)
# warning "__STDC_NO_ATOMICS__ defined"
#endif

// #if defined(__STDC_NO_THREADS__)
// # warning "__STDC_NO_THREADS__ defined"
// #endif

#if defined(__STDC_NO_ATOMICS__)
  static ull_t sn = 0ull;
#else
  static _Atomic(ull_t) sn = 0ull;
#endif

static char* get_timestamp(char* out_buffer, size_t size) {
  struct timeval tv = {0};
  int result = gettimeofday(&tv, NULL);
  if (0 != result) {
    memset(&tv, 0, sizeof(tv));
  }
  time_t time = time = (time_t) tv.tv_sec;
  tm_t tm_buffer = {0};
  tm_t* tm = localtime_s(&time, &tm_buffer);
  char buffer[32] = {0};
  sprintf_s(
    buffer,
    size,
    "%04d-%02d-%02d %02d:%02d:%02d.%03d",
    (1900 + tm->tm_year),
    tm->tm_mon,
    tm->tm_mday,
    tm->tm_hour,
    tm->tm_min,
    tm->tm_sec,
    (int) ((tv.tv_usec / 1000.0) + 0.5)
  );
  strncpy(out_buffer, buffer, size);
  out_buffer[size - 1] = '\0';
  return out_buffer;
}

#if defined(__STDC_NO_ATOMICS__)
  static ull_t log_pid_value = 0;
  static ull_t get_log_pid() {
    return (ull_t) getpid();
  }
#else
  static atomic_flag pid_flag = ATOMIC_FLAG_INIT;
  static ull_t log_pid_value = 0;
  static ull_t get_log_pid() {
    if (!atomic_flag_test_and_set(&pid_flag)) {
      log_pid_value = (ull_t) getpid();
    }
    return log_pid_value;
  }
#endif

static _Thread_local ull_t log_tid_value = 0;
# if defined(__APPLE__)
  static ull_t get_log_tid() {
    if (0 == log_tid_value) {
      __uint64_t id = 0;
      pthread_threadid_np(NULL, &id);
      log_tid_value = id;
    }
    return log_tid_value;
  }
# else
  static ull_t get_log_tid() {
    if (0 == log_tid_value) {
      log_tid_value = gettid();
    }
    return log_tid_value;
  }
# endif


static void log(FILE* dest,
                char const* level,
                char const* module,
                char const* tag,
                char const* file,
                char const* func,
                int line,
                char const* format,
                va_list vlist) {
  char timestamp[32] = {0};
  fprintf(
    dest,
  #if defined(__APPLE__)
    "#%#011llx %s %s [%#6llx::%#-7llx] [%s::%s] [%s::%s::%d] ",
  #else
    "#%#011llx %s %s [%#6llx::%#-6llx] [%s::%s] [%s::%s::%d] ",
  #endif
    ++sn,
    get_timestamp(timestamp, sizeof(timestamp)),
    level,
    get_log_pid(),
    get_log_tid(),
    module,
    tag,
    file,
    func,
    line
  );
	vfprintf(dest, format, vlist);
	fprintf(dest, "\n");
}

void vrb(char const* module, char const* tag, char const* file, char const* func, int line, char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  log(stdout, "VRB", module, tag, file, func, line, format, vlist);
	va_end(vlist);
}

void dbg(char const* module, char const* tag, char const* file, char const* func, int line, char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  log(stdout, "DBG", module, tag, file, func, line, format, vlist);
	va_end(vlist);
}

void inf(char const* module, char const* tag, char const* file, char const* func, int line, char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  log(stdout, "INF", module, tag, file, func, line, format, vlist);
	va_end(vlist);
}

void wrn(char const* module, char const* tag, char const* file, char const* func, int line, char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  log(stderr, "WRN", module, tag, file, func, line, format, vlist);
	va_end(vlist);
}

void err(char const* module, char const* tag, char const* file, char const* func, int line, char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  log(stderr, "ERR", module, tag, file, func, line, format, vlist);
	va_end(vlist);
}

void raw(char const* format, ...) {
	va_list vlist;
	va_start(vlist, format);
  vfprintf(stdout, format, vlist);
  fflush(stdout);
	va_end(vlist);
}
