#include "datagui/log.hpp"
#ifdef DATAGUI_LOG

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <assert.h>
#include <cstring>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace datagui {

static bool log_init_called = false;
static int socket_fd = -1;
static int indent = 0;
static int message_num = 0;

const char* log_hostname = "localhost";
const char* log_port = "5678";

#define _RED "\033[0;31m"
#define _GREEN "\033[0;32m"
#define _BLUE "\033[0;34m"
#define _RESET "\033[0m"
char indent_buffer[1024];
char pad_buffer[1024];
const char pad_char = '.';
const char pad_end_char = '|';

constexpr std::size_t buffer_size = 256;
char buffer[buffer_size];

const char* indent_str(std::size_t length) {
  length = std::min(length, sizeof(indent_buffer) - 1);
  return indent_buffer + sizeof(indent_buffer) - 1 - length;
}
const char* pad_str(std::size_t length) {
  length = std::min(length, sizeof(pad_buffer) - 1);
  return pad_buffer + sizeof(pad_buffer) - 1 - length;
}

static void* get_in_addr(struct sockaddr* addr) {
  if (addr->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)addr)->sin_addr);
  }
  return &(((struct sockaddr_in6*)addr)->sin6_addr);
}

void log_init() {
  log_init_called = true;

  struct addrinfo hints;
  struct addrinfo* servinfo;
  struct addrinfo* p;
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; // TCP

  {
    int rc = getaddrinfo(log_hostname, log_port, &hints, &servinfo);
    if (rc != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
      return;
    }
  }

  for (struct addrinfo* iter = servinfo; iter; iter = iter->ai_next) {
    socket_fd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
    if (socket_fd == -1) {
      perror("client: socket");
      continue;
    }
    if (connect(socket_fd, iter->ai_addr, iter->ai_addrlen) == -1) {
      close(socket_fd);
      socket_fd = -1;
      continue;
    }

    // Success

    char ip_addr[INET6_ADDRSTRLEN];
    inet_ntop(
        iter->ai_family,
        get_in_addr((struct sockaddr*)iter->ai_addr),
        ip_addr,
        sizeof(ip_addr));
    break;
  }
  freeaddrinfo(servinfo);

  if (socket_fd == -1) {
    perror(_RED "Failed to start log client" _RESET);
    return;
  }

  indent = 0;
  memset(indent_buffer, ' ', sizeof(indent_buffer));
  indent_buffer[sizeof(indent_buffer) - 1] = '\0';
  memset(pad_buffer, pad_char, sizeof(pad_buffer));
  pad_buffer[sizeof(pad_buffer) - 1] = '\0';

  printf(_GREEN "Log client started successfully\n" _RESET);
}

void log_write(
    const char* file,
    int line_number,
    const char* label,
    const char* fmt,
    va_list args) {

  if (socket_fd == -1) {
    if (!log_init_called) {
      fprintf(
          stderr,
          _RED "Failed to start log client:" _RESET
               " Logger was not initialized\n");
      log_init_called = true;
    }
    return;
  }

  // location = label + line number (ignoring file)
  const std::size_t line_num_width = 4;
  const std::size_t label_width = 30;
  const std::size_t indent_width = 16;

  std::size_t pos = 0;
  std::size_t change = 0;

  pos += snprintf(buffer + pos, buffer_size - pos, "%i ", message_num);
  message_num++;

  pos += snprintf(buffer + pos, buffer_size - pos, _BLUE);
  change = snprintf(buffer + pos, buffer_size - pos, "%i", line_number);
  pos += change;
  pos += snprintf(
      buffer + pos,
      buffer_size - pos,
      "%s",
      indent_str(line_num_width - std::min(change, line_num_width)));

  pos += snprintf(buffer + pos, buffer_size - pos, _GREEN);
  change = snprintf(buffer + pos, buffer_size - pos, " %s", label);
  pos += change;
  pos += snprintf(
      buffer + pos,
      buffer_size - pos,
      "%s",
      indent_str(label_width - std::min(change, label_width)));
  pos += snprintf(buffer + pos, buffer_size - pos, _RESET " | ");

  pos += snprintf(buffer + pos, buffer_size - pos, _RED);
  if (indent == 0) {
    pos +=
        snprintf(buffer + pos, buffer_size - pos, "%s", pad_str(indent_width));
  } else {
    pos += snprintf(
        buffer + pos,
        buffer_size - pos,
        "%s%c%s",
        pad_str(indent),
        pad_end_char,
        indent_str(indent_width - indent - 1));
  }
  pos += snprintf(buffer + pos, buffer_size - pos, _RESET " | ");

  pos += vsnprintf(buffer + pos, buffer_size - pos, fmt, args);

  std::size_t sent = send(socket_fd, buffer, pos + 1, 0);
  if (sent != pos + 1) {
    printf(_RED "Failed to send message" _RESET "\n");
  }

  // Note: Need to throttle the output rate otherwise messages will be skipped
  std::this_thread::sleep_for(std::chrono::microseconds(500));
}

void log_indent(int change) {
  indent += change;
  assert(indent >= 0);
}

} // namespace datagui

#endif
