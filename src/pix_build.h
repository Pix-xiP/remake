#ifndef PIX_BUILD_H
#define PIX_BUILD_H
#pragma once

#include <unistd.h>

// Sane Defaults?
#define COMPILER "cc"
#define OPTIMISED "-O2"
#define VERY_OPTIMISED "-O3"
/* #define DEBUG "-g" */
/* #define VERBOSE "-v" */
#define C_STD "std=c99"

typedef struct _cmd_list_t {
  const char **cmds;
  size_t count;
  size_t capacity;
} CmdList;

#endif
