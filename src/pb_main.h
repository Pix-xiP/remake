#ifndef PB_MAIN_H
#define PB_MAIN_H 1

#include "pix_scuffed_da.h"

typedef enum Optimisation {
  none = 0,
  debug,
  debug_optimised,
  basic,
  regular,
  size,
  extreme,
} Optimisation;

typedef struct PB_Builder {

  const char *compiler;
  const char *build_path;
  const char *name;

  DynamicArray libs;
  DynamicArray files;
  DynamicArray cflags;
  DynamicArray defines;
  DynamicArray ldflags;
  DynamicArray inc_dirs;

  DynamicArray obj_files;

} PB_Builder;

typedef struct PB_Context {
  const char *install_dir;
  const char *build_dir;

  Optimisation op_lvl;

  bool is_exe;
  const char *exe;

  bool is_lib;
  const char *lib;
} PB_Context;

#endif
