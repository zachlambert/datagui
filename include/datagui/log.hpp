#pragma once

#ifdef DATAGUI_DEBUG

#include <cstdarg>
#include <cstdio>

namespace datagui {

extern const char* log_hostname;
extern const char* log_port;

void log_init();

void log_write(
    const char* file,
    int line_number,
    const char* label,
    const char* fmt,
    va_list args);

inline void log_write(
    const char* file,
    int line_number,
    const char* label,
    const char* fmt,
    ...) {
  va_list args;
  va_start(args, fmt);
  log_write(file, line_number, label, fmt, args);
  va_end(args);
}

void log_indent(int change);

} // namespace datagui

// https://stackoverflow.com/questions/31050113/how-to-extract-the-source-filename-without-path-and-suffix-at-compile-time
#define __FILENAME__ \
  (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 \
                                    : __FILE__)

#define DATAGUI_LOG_INIT() ::datagui::log_init();

#define DATAGUI_LOG(label, fmt, ...) \
  ::datagui::log_write(__FILENAME__, __LINE__, label, fmt, ##__VA_ARGS__)

#define DATAGUI_LOG_INDENT(change) ::datagui::log_indent(change)

#else
#define DATAGUI_LOG_INIT() \
  { \
  }
#define DATAGUI_LOG(msg, ...) \
  { \
  }
#define DATAGUI_LOG2(label, fmt, ...) \
  { \
  }
#define DATAGUI_INDENT(change) \
  { \
  }
#endif
