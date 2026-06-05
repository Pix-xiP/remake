#ifndef PB_MAIN_H
#define PB_MAIN_H 1

#include "pix.h"

#define DEFAULT_FILE_NAME "build.lua"
#define DEFAULT_BUILD_DIR "./build"

typedef enum optimisation_t {
  none = 0,
  debug,
  debug_optimised,
  basic,
  regular,
  size,
  extreme,
} optimisation_t;

typedef enum target_kind_t {
  target_kind_exe = 0,
} target_kind_t;

typedef struct build_target_t {
  const char *compiler;
  const char *name;
  target_kind_t kind;
  optimisation_t op_lvl;

  DynamicArray lib_dirs;
  DynamicArray libs;
  DynamicArray sources;
  DynamicArray cflags;
  DynamicArray defines;
  DynamicArray ldflags;
  DynamicArray inc_dirs;

  DynamicArray obj_files;
} build_target_t;

typedef struct build_config_t {
  const char *install_dir;
  const char *build_dir;

  build_target_t *targets;
  size_t target_count;
  size_t target_capacity;
} build_config_t;

#endif
