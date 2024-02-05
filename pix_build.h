#ifndef PIX_BUILD_H
#define PIX_BUILD_H
#pragma once

#include <unistd.h>

typedef struct _cmd_list_t {
  const char **cmds;
  size_t count;
  size_t capacity;
} CmdList;

#endif
