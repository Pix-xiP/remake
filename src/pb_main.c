#define _GNU_SOURCE // allows for use of realpath.. should probably just implement it myself.
#define PIX_IMPLEMENTATION
#include "pix.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h> // fork

#include "pb_file_checker.h"
#include "pb_main.h"
#include "pb_parsing.h"

static build_config_t bc = {0};
static build_target_t *current_target = NULL;

static build_target_t *add_build_target(void) {
  if (bc.target_count >= bc.target_capacity) {
    bc.target_capacity = bc.target_capacity == 0 ? 4 : bc.target_capacity * 2;
    bc.targets = PIX_REALLOC(bc.targets, bc.target_capacity * sizeof(*bc.targets));
    pix_assert(bc.targets != NULL, px_log(px_err, "Out of Memory"));
  }

  build_target_t *target = &bc.targets[bc.target_count++];
  pix_memset(target, 0, sizeof(*target));
  target->kind = target_kind_exe;
  target->op_lvl = none;
  return target;
}

static void free_string_array(DynamicArray *da) {
  for (size_t i = 0; i < da->count; i++) {
    void *item = (void *)da->items[i];
    PIX_FREE(item);
    da->items[i] = NULL;
  }
  pix_da_free((*da));
  *da = (DynamicArray){0};
}

static void free_build_target(build_target_t *target) {
  void *compiler = (void *)target->compiler;
  PIX_FREE(compiler);
  target->compiler = NULL;

  void *name = (void *)target->name;
  PIX_FREE(name);
  target->name = NULL;
  free_string_array(&target->lib_dirs);
  free_string_array(&target->libs);
  free_string_array(&target->sources);
  free_string_array(&target->cflags);
  free_string_array(&target->defines);
  free_string_array(&target->ldflags);
  free_string_array(&target->inc_dirs);
  free_string_array(&target->obj_files);
}

static void free_build_config(void) {
  void *install_dir = (void *)bc.install_dir;
  PIX_FREE(install_dir);
  bc.install_dir = NULL;

  void *build_dir = (void *)bc.build_dir;
  PIX_FREE(build_dir);
  bc.build_dir = NULL;
  for (size_t i = 0; i < bc.target_count; i++) {
    free_build_target(&bc.targets[i]);
  }
  PIX_FREE(bc.targets);
  bc = (build_config_t){0};
  current_target = NULL;
}

void print_da(size_t count, const char **items) {
  for (size_t i = 0; i < count; i++) {
    if (items[i] == NULL)
      break;
    fprintf(stdout, "%s ", items[i]);
  }
  printf("\n");
}

void print_target(const build_target_t *target) {
  px_log(px_info, "Target: %s", target->name);
  px_log(px_info, "Compiler: %s", target->compiler);

  px_log(px_info, "Files:");
  print_da(target->sources.count, target->sources.items);
  px_log(px_info, "CFlags:");
  print_da(target->cflags.count, target->cflags.items);
  px_log(px_info, "Libs");
  print_da(target->libs.count, target->libs.items);
  px_log(px_info, "Library Dirs");
  print_da(target->lib_dirs.count, target->lib_dirs.items);
  px_log(px_info, "Include Dirs");
  print_da(target->inc_dirs.count, target->inc_dirs.items);
}

void print_bc() {
  px_log(px_info, "Install Dir: %s", bc.install_dir);
  px_log(px_info, "Build Dir: %s", bc.build_dir);
  for (size_t i = 0; i < bc.target_count; i++) {
    print_target(&bc.targets[i]);
  }
}

i32 alloc_and_cpy_string(void **loc, const char *str) {
  u64 len = pix_strlen((char *)str);
  (*loc) = pix_calloc(len);
  pix_memcpy((*loc), str, len);
  return len;
}

static void append_copied_arg(DynamicArray *da, const char *prefix, const char *str) {
  size_t prefix_len = prefix ? (size_t)pix_strlen(prefix) - 1 : 0;
  size_t str_len = (size_t)pix_strlen(str) - 1;
  char *arg = pix_calloc((i64)(prefix_len + str_len + 1));

  if (prefix_len)
    pix_memcpy(arg, prefix, prefix_len);
  pix_memcpy(arg + prefix_len, str, str_len);
  arg[prefix_len + str_len] = '\0';

  pix_da_append(da, arg);
}

i32 parse_opt_level(lua_State *L) {

  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: Expected a 'string' for 'optimisation_level', found '%s'",
           luat_to_string(lua_type(L, -1)));
    return 1;
  }

  const char *val = lua_tostring(L, -1);

  if (pix_strcmp(val, "none") == 0)
    current_target->op_lvl = none;
  else if (pix_strcmp(val, "debug") == 0)
    current_target->op_lvl = debug;
  else if (pix_strcmp(val, "debug_optimised") == 0)
    current_target->op_lvl = debug_optimised;
  else if (pix_strcmp(val, "basic") == 0)
    current_target->op_lvl = basic;
  else if (pix_strcmp(val, "default") == 0)
    current_target->op_lvl = regular;
  else if (pix_strcmp(val, "size") == 0)
    current_target->op_lvl = size;
  else if (pix_strcmp(val, "extreme") == 0)
    current_target->op_lvl = extreme;
  else {
    px_log(px_err,
           "Invalid optiisation level: '%s'. \n   Please choose from: none, basic, default, "
           "extreme",
           val);
    return 1;
  }

  px_log(px_info, "Optimisation Level: %s", val);
  return 0;
}

// void parse_nested_table(lua_State *state) {
//   return; // PIXTODO: Remove this when tackling the rest lmao.
//   lua_pushvalue(state, -1);
//   lua_pushnil(state);
//
//   while (lua_next(state, -2) != 0) {
//     if (lua_type(state, -2) == LUA_TSTRING) {
//       px_log(19, "    This is a string'd key: %s", lua_tostring(state, -2));
//     } else {
//       px_log(19, "    This is a number'd key: %lld", lua_tointeger(state, -2));
//     }
//     px_log(19, "    With a value of: %s", luat_to_string(lua_type(state, -1)));
//     lua_pop(state, 1); // Pop value from stack
//   }
//   lua_pop(state, 1); // Pop nested table from stack
// }

bool parse_name(lua_State *L) {
  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: Executable field expects 'string' found: '%s'",
           luat_to_string(lua_type(L, -1)));
    return true;
  }
  alloc_and_cpy_string((void **)&current_target->name, lua_tostring(L, -1));

  return false;
}

static bool parse_build_dir(lua_State *L) {
  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: build_dir expects 'string' found: '%s'",
           luat_to_string(lua_type(L, -1)));
    return true;
  }
  alloc_and_cpy_string((void **)&bc.build_dir, lua_tostring(L, -1));
  return false;
}

static bool parse_install_table(lua_State *L) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    px_log(px_err, "[LUA]: install expects 'table' found: '%s'", luat_to_string(lua_type(L, -1)));
    return true;
  }

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_type(L, -2) != LUA_TSTRING) {
      px_log(px_err, "[LUA]: install keys must be 'string'. Found: '%s'",
             luat_to_string(lua_type(L, -2)));
      lua_pop(L, 1);
      return true;
    }

    const char *key = lua_tostring(L, -2);
    if (pix_strcmp(key, "directory") != 0) {
      px_log(px_err, "[LUA]: Unknown install field '%s'", key);
      lua_pop(L, 1);
      return true;
    }

    if (lua_type(L, -1) != LUA_TSTRING) {
      px_log(px_err, "[LUA]: install.directory expects 'string' found: '%s'",
             luat_to_string(lua_type(L, -1)));
      lua_pop(L, 1);
      return true;
    }

    alloc_and_cpy_string((void **)&bc.install_dir, lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  return false;
}

bool parse_compiler(lua_State *state) {
  // Parse function that could return a string, or just a string.
  if (lua_type(state, -1) == LUA_TFUNCTION) {
    // parse the function
    if (lua_pcall(state, 0, 1, 0)) {
      px_log(px_err, "Error in compiler function: '%s'", lua_tostring(state, -1));
      lua_pop(state, 1);
      return true;
    }
  }

  if (lua_type(state, -1) != LUA_TSTRING) {
    px_log(px_err,
           "[LUA]: Compiler field expects 'string' or a 'function' that returns a 'string', found: "
           "'%s'",
           luat_to_string(lua_type(state, -1)));
    return true;
  }

  alloc_and_cpy_string((void **)&current_target->compiler, lua_tostring(state, -1));

  return false;
}

static bool parse_kind(lua_State *L) {
  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: kind expects 'string' found: '%s'", luat_to_string(lua_type(L, -1)));
    return true;
  }

  const char *val = lua_tostring(L, -1);
  if (pix_strcmp(val, "exe") == 0) {
    current_target->kind = target_kind_exe;
    return false;
  }

  px_log(px_err, "[LUA]: Unsupported target kind '%s'. Only 'exe' is implemented", val);
  return true;
}

bool parse_generic_table(lua_State *L, DynamicArray *da, const char *who, char *prefix);

static bool parse_defines_table(lua_State *L) {
  return parse_generic_table(L, &current_target->defines, "defines", "-D");
}

static bool parse_cflags_table(lua_State *L) {
  return parse_generic_table(L, &current_target->cflags, "cflags", NULL);
}

static bool parse_sources_table(lua_State *L) {
  return parse_generic_table(L, &current_target->sources, "sources", NULL);
}

static bool parse_inc_dirs_table(lua_State *L) {
  return parse_generic_table(L, &current_target->inc_dirs, "inc_dirs", "-I");
}

static bool parse_libs_table(lua_State *L) {
  return parse_generic_table(L, &current_target->libs, "libs", "-l");
}

static bool parse_lib_dirs_table(lua_State *L) {
  return parse_generic_table(L, &current_target->lib_dirs, "lib_dirs", "-L");
}

static bool parse_ldflags_table(lua_State *L) {
  return parse_generic_table(L, &current_target->ldflags, "ldflags", NULL);
}

static bool parse_opt_level_field(lua_State *L) { return parse_opt_level(L) != 0; }

typedef bool (*field_handler_t)(lua_State *L);

typedef struct field_handler_entry_t {
  const char *key;
  field_handler_t handler;
} field_handler_entry_t;

static const field_handler_entry_t target_field_handlers[] = {
    {"compiler", parse_compiler},
    {"name", parse_name},
    {"kind", parse_kind},
    {"defines", parse_defines_table},
    {"cflags", parse_cflags_table},
    {"sources", parse_sources_table},
    {"include_dirs", parse_inc_dirs_table},
    {"libs", parse_libs_table},
    {"library_dirs", parse_lib_dirs_table},
    {"ldflags", parse_ldflags_table},
    {"optimisation_level", parse_opt_level_field},
};

static bool handle_field(lua_State *L, const char *key, const field_handler_entry_t *handlers,
                         size_t handler_count, const char *scope) {
  for (size_t i = 0; i < handler_count; i++) {
    if (pix_strcmp(handlers[i].key, key) == 0) {
      return handlers[i].handler(L);
    }
  }

  px_log(px_err, "[LUA]: Unknown %s field '%s'", scope, key);
  return true;
}

static bool parse_target_table(lua_State *L) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    px_log(px_err, "[LUA]: targets entries must be 'table'. Found: '%s'",
           luat_to_string(lua_type(L, -1)));
    return true;
  }

  build_target_t *previous_target = current_target;
  current_target = add_build_target();

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_type(L, -2) != LUA_TSTRING) {
      px_log(px_err, "[LUA]: target keys must be 'string'. Found: '%s'",
             luat_to_string(lua_type(L, -2)));
      lua_pop(L, 1);
      current_target = previous_target;
      return true;
    }

    const char *key = lua_tostring(L, -2);
    if (handle_field(L, key, target_field_handlers,
                     sizeof(target_field_handlers) / sizeof(target_field_handlers[0]), "target")) {
      current_target = previous_target;
      return true;
    }

    lua_pop(L, 1);
  }

  if (!current_target->name) {
    px_log(px_err, "[LUA]: target is missing required field 'name'");
    current_target = previous_target;
    return true;
  }

  if (current_target->sources.count == 0) {
    px_log(px_err, "[LUA]: target '%s' is missing required field 'sources'", current_target->name);
    current_target = previous_target;
    return true;
  }

  current_target = previous_target;
  return false;
}

static bool parse_targets_table(lua_State *L) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    px_log(px_err, "[LUA]: targets expects 'table' found: '%s'", luat_to_string(lua_type(L, -1)));
    return true;
  }

  if (bc.target_count != 0) {
    px_log(px_err, "[LUA]: targets may only be defined once");
    return true;
  }

  lua_Unsigned len = (lua_Unsigned)lua_rawlen(L, -1);
  if (len == 0) {
    px_log(px_err, "[LUA]: targets must contain at least one target");
    return true;
  }

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (!lua_isinteger(L, -2)) {
      px_log(px_err, "[LUA]: targets must be an array of target tables. Found key type: '%s'",
             luat_to_string(lua_type(L, -2)));
      lua_pop(L, 1);
      return true;
    }

    lua_Integer index = lua_tointeger(L, -2);
    if (index < 1 || (lua_Unsigned)index > len) {
      px_log(px_err, "[LUA]: targets has non-contiguous array index: %lld", index);
      lua_pop(L, 1);
      return true;
    }

    lua_pop(L, 1);
  }

  for (lua_Unsigned i = 1; i <= len; i++) {
    lua_rawgeti(L, -1, (lua_Integer)i);
    if (parse_target_table(L)) {
      lua_pop(L, 1);
      return true;
    }
    lua_pop(L, 1);
  }

  return false;
}

bool parse_generic_table(lua_State *L, DynamicArray *da, const char *who, char *prefix) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    px_log(px_err, "[LUA]: Expected a 'table' for '%s'", who);
    return true;
  }

  lua_Unsigned len = (lua_Unsigned)lua_rawlen(L, -1);

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (!lua_isinteger(L, -2)) {
      px_log(px_err, "[LUA]: '%s' must be an array of strings. Found key type: '%s'", who,
             luat_to_string(lua_type(L, -2)));
      lua_pop(L, 1);
      return true;
    }

    lua_Integer index = lua_tointeger(L, -2);
    if (index < 1 || (lua_Unsigned)index > len) {
      px_log(px_err, "[LUA]: '%s' has non-contiguous array index: %lld", who, index);
      lua_pop(L, 1);
      return true;
    }

    lua_pop(L, 1);
  }

  lua_pushvalue(L, -1);
  for (lua_Unsigned i = 1; i <= len; i++) {
    lua_rawgeti(L, -1, (lua_Integer)i);
    if (lua_type(L, -1) != LUA_TSTRING) {
      px_log(px_err, "[LUA]: '%s' values must be 'string'. Found at index %llu: '%s'", who, i,
             luat_to_string(lua_type(L, -1)));

      lua_pop(L, 2); // Pop value + table copy
      return true;
    }

    append_copied_arg(da, prefix, lua_tostring(L, -1));

    lua_pop(L, 1); // Pop value from stack
  }
  lua_pop(L, 1); // Pop nested table from stack

  return false;
}

static bool is_c_like_ext(const char *ext, size_t ext_len) {
  if (ext_len == 1)
    return ext[0] == 'c' || ext[0] == 'C' || ext[0] == 'S' || ext[0] == 's';

  if (ext_len == 2)
    return memcmp(ext, "cc", 2) == 0 || memcmp(ext, "CC", 2) == 0;

  if (ext_len == 3) {
    return memcmp(ext, "cpp", 3) == 0 || memcmp(ext, "cxx", 3) == 0 || memcmp(ext, "CPP", 3) == 0 ||
           memcmp(ext, "CXX", 3) == 0;
  }

  return false;
}

static void copy_sanitized_path(char *dst, const char *src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    char c = src[i];
    if (c == '/' || c == '\\' || c == ':')
      dst[i] = '_';
    else
      dst[i] = c;
  }
}

static const char *skip_leading_path_markers(const char *path) {
  while (path[0] == '.' && path[1] == '/')
    path += 2;
  while (*path == '/' || *path == '\\')
    path++;
  return path;
}

static char *make_target_build_dir(const build_target_t *target) {
  const char *build_dir = bc.build_dir ? bc.build_dir : "build";
  const char *target_name = target->name ? target->name : "target";
  size_t dir_len = (size_t)pix_strlen(build_dir) - 1;
  size_t name_len = (size_t)pix_strlen(target_name) - 1;
  size_t new_len = dir_len + 1 + name_len;

  char *out = pix_calloc((i64)(new_len + 1));
  pix_memcpy(out, build_dir, dir_len);
  out[dir_len] = '/';
  copy_sanitized_path(out + dir_len + 1, target_name, name_len);
  out[new_len] = '\0';
  return out;
}

static char *make_obj_path(const build_target_t *target, const char *path) {
  char *target_build_dir = make_target_build_dir(target);
  const char *path_rooted = skip_leading_path_markers(path);
  size_t path_len = (size_t)pix_strlen(path_rooted) - 1;

  const char *last_dot = strrchr(path_rooted, '.');
  const char *last_slash = strrchr(path_rooted, '/');
  bool replace_ext = false;

  if (last_dot && (!last_slash || last_dot > last_slash)) {
    const char *ext = last_dot + 1;
    size_t ext_len = path_len - (size_t)(ext - path_rooted);

    replace_ext = is_c_like_ext(ext, ext_len);
  }

  size_t dir_len = (size_t)pix_strlen(target_build_dir) - 1;
  size_t stem_len = path_len;
  if (replace_ext) {
    stem_len = (size_t)(last_dot - path_rooted);
  }

  if (replace_ext) {
    size_t new_len = dir_len + 1 + stem_len + 2; // dir + "/" + stem + ".o"

    char *obj = pix_calloc((i64)(new_len + 1));
    pix_memcpy(obj, target_build_dir, dir_len);
    obj[dir_len] = '/';
    copy_sanitized_path(obj + dir_len + 1, path_rooted, stem_len);

    obj[dir_len + 1 + stem_len] = '.';
    obj[dir_len + 1 + stem_len + 1] = 'o';
    obj[new_len] = '\0';

    PIX_FREE(target_build_dir);
    return obj;
  }

  size_t new_len = dir_len + 1 + stem_len + 2; // dir + "/" + stem + ".o"

  char *obj = pix_calloc((i64)(new_len + 1));
  pix_memcpy(obj, target_build_dir, dir_len);
  obj[dir_len] = '/';
  copy_sanitized_path(obj + dir_len + 1, path_rooted, stem_len);

  obj[dir_len + 1 + stem_len] = '.';
  obj[dir_len + 1 + stem_len + 1] = 'o';
  obj[new_len] = '\0';

  PIX_FREE(target_build_dir);
  return obj;
}

void add_optimisation_flags(build_target_t *target) {
  // What other options by default?
  switch (target->op_lvl) {
  case basic:
    append_copied_arg(&target->cflags, NULL, "-O1");
    break;
  case debug:
    append_copied_arg(&target->cflags, NULL, "-g3");
    break;
  case debug_optimised:
    append_copied_arg(&target->cflags, NULL, "-g2");
    append_copied_arg(&target->cflags, NULL, "-Og");
    break;
  case size:
    append_copied_arg(&target->cflags, NULL, "-Os");
    break;
  case regular:
    append_copied_arg(&target->cflags, NULL, "-O2");
    break;
  case extreme:
    append_copied_arg(&target->cflags, NULL, "-Ofast");
    break;
  case none:
  default:
    break;
  }
}

i32 exec_fork(DynamicArray *da) {
  print_da(da->count, da->items); // Debuging <3
  i32 status;
  pid_t pid = fork();
  if (pid == 0) { // Child Run
    if (execvp(da->items[0], (char *const *)da->items) < 0) {
      px_log(px_err, "Could not exec child process: %s", strerror(errno));
      exit(1);
    }
  } else if (pid < 0) { // Error
    px_log(px_err, "Could not fork child process: %s", strerror(errno));
    exit(1);
  } else { // Parent
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      px_log(px_err, "Failing in compiliation");
      exit(1);
    }
  }

  return status;
}

static bool run_target_build(build_target_t *target, bool force_rebuild) {
  DynamicArray args = {0};
  bool rebuild_exe = false;

  char *target_build_dir = make_target_build_dir(target);
  mkdir_if_not_exists(target_build_dir);

  append_copied_arg(&target->inc_dirs, NULL, "-I.");
  append_copied_arg(&target->inc_dirs, NULL, "-I..");

  // Add optimisation flags
  add_optimisation_flags(target);

  // Add the compiler - default to?..
  if (target->compiler)
    pix_da_append(&args, target->compiler);
  else
    pix_da_append(&args, "cc");

  // Append the flgs and directories that may be needed to each line.
  pix_da_append_multi(&args, target->cflags.items, target->cflags.count);
  pix_da_append_multi(&args, target->defines.items, target->defines.count);
  pix_da_append_multi(&args, target->inc_dirs.items, target->inc_dirs.count);

  for (size_t i = 0; i < target->sources.count; ++i) {
    // Store the current count of 'args' before adding file-specific arguments.
    size_t initial_args_count = args.count;
    const char *path_spec = target->sources.items[i];
    char *path = realpath(path_spec, NULL);

    if (path == NULL) {
      px_log(px_err, "Unable to find file: %s", target->sources.items[i]);
      exit(1);
    }

    char *obj = make_obj_path(target, path_spec);

    pix_da_append(&target->obj_files, obj);

    char *dep = make_dep_path(obj);
    bool rebuild_obj = force_rebuild || is_file_newer(path, obj) || deps_require_rebuild(obj, dep);

    if (rebuild_obj) {
      // Append file to rebuild.
      // -MMD etc generate dependency ".d" files to check if header files have changed.
      pix_da_append(&args, "-MMD");
      pix_da_append(&args, "-MP");
      pix_da_append(&args, "-MF");
      pix_da_append(&args, dep);
      pix_da_append(&args, path);

      // Add out directory to same path as object
      pix_da_append(&args, "-o");
      pix_da_append(&args, obj);

      // We don't want to link.
      pix_da_append(&args, "-c");

      // Finish command.
      pix_da_append(&args, NULL);

      // We fork now to do the compiliation:
      px_log(px_info, "Compiling %s", path);
      exec_fork(&args);

      // Reset DA to its state before adding temporary arguments for this file.
      args.count = initial_args_count;

      // Easy way to check if need to update exec instead of stat
      rebuild_exe = true;
    }

    if (is_file_newer(obj, target->name)) {
      rebuild_exe = true;
    }

    PIX_FREE(dep);
    PIX_FREE(path);
  }

  if (!does_file_exist(target->name)) {
    rebuild_exe = true;
  }

  if (rebuild_exe) {
    pix_da_append(&args, "-o");
    pix_da_append(&args, target->name);

    pix_da_append_multi(&args, target->obj_files.items, target->obj_files.count);
    pix_da_append_multi(&args, target->ldflags.items, target->ldflags.count);
    pix_da_append_multi(&args, target->lib_dirs.items, target->lib_dirs.count);
    pix_da_append_multi(&args, target->libs.items, target->libs.count);
    pix_da_append(&args, NULL);

    px_log(px_info, "Compiling executable %s", target->name);
    exec_fork(&args);
  } else {
    px_log(px_info, "No changes detected for target '%s'.", target->name);
  }

  PIX_FREE(target_build_dir);
  pix_da_free(args);
  return rebuild_exe;
}

i32 run_build() {
  bool rebuilt_any = false;
  bool force_rebuild = false;

  if (!bc.build_dir) {
    px_log(px_info,
           "No custom build directory found in '%s'. Setting build directory to default '%s'",
           DEFAULT_FILE_NAME, DEFAULT_BUILD_DIR);
    alloc_and_cpy_string((void **)&bc.build_dir, "build");
  }
  mkdir_if_not_exists(bc.build_dir);

  char *config_stamp = make_config_stamp_path(bc.build_dir);
  if (is_file_newer(DEFAULT_FILE_NAME, config_stamp)) {
    px_log(px_info, "Config changed: %s", DEFAULT_FILE_NAME);
    force_rebuild = true;
  }

  for (size_t i = 0; i < bc.target_count; i++) {
    rebuilt_any = run_target_build(&bc.targets[i], force_rebuild) || rebuilt_any;
  }

  if (rebuilt_any) {
    if (write_empty_file(config_stamp)) {
      px_log(px_warn, "Failed to write config stamp: %s", config_stamp);
    }
  } else {
    px_log(px_info, "No changes detected.");
  }

  PIX_FREE(config_stamp);

  return 0;
}

i32 main(i32 argc, char **argv) {
  if (!does_file_exist(DEFAULT_FILE_NAME)) {
    px_log(px_err, "Unable to locate '%s', aborting.", DEFAULT_FILE_NAME);
    exit(EXIT_FAILURE);
  }

  // Instantiate the Lua state
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  // Load in the build file. || call lua, expect 1 result
  if (luaL_loadfile(L, DEFAULT_FILE_NAME) || lua_pcall(L, 0, 1, 0)) {
    px_log(px_err, "Error: %s", lua_tostring(L, -1));
    lua_pop(L, 1); // Pop error message from the stack
    return 1;
  } else {
    // ensure data we get is a table!
    if (!lua_istable(L, -1)) {
      px_log(px_err, "[LUA]: Expected a 'table', got '%s'", lua_typename(L, lua_type(L, -1)));
      return 1;
    }

    lua_pushnil(L);

    while (lua_next(L, -2) != 0) {
      // key is at idx -2 and value at -1
      if (lua_type(L, -2) == LUA_TSTRING) {
        const char *key = lua_tostring(L, -2);

        static const field_handler_entry_t field_handlers[] = {
            {"build_dir", parse_build_dir},
            {"install", parse_install_table},
            {"targets", parse_targets_table},
        };

        if (handle_field(L, key, field_handlers, sizeof(field_handlers) / sizeof(field_handlers[0]),
                         "top-level")) {
          return 1;
        }
      } else {
        px_log(px_err, "[LUA]: Top-level keys must be 'string'. Found: '%s'",
               luat_to_string(lua_type(L, -2)));
        return 1;
      }

      // Unless something gets added, this will always handle popping the last value.
      lua_pop(L, 1); // Pop total table from stack
    }
    // Finish with the lua state and cleanup.
    // lua_close(L);
  }

  if (bc.target_count == 0) {
    px_log(px_err, "[LUA]: build file must define 'targets'");
    lua_close(L);
    free_build_config();
    return 1;
  }

  for (size_t i = 0; i < bc.target_count; i++) {
    if (!bc.targets[i].name) {
      px_log(px_err, "[LUA]: target is missing required field 'name'");
      lua_close(L);
      free_build_config();
      return 1;
    }

    if (bc.targets[i].sources.count == 0) {
      px_log(px_err, "[LUA]: target '%s' is missing required field 'sources'", bc.targets[i].name);
      lua_close(L);
      free_build_config();
      return 1;
    }
  }

  // To have a peek at whats inside..
  print_bc();

  run_build();

  // We close state here, because we use all the allocated strings lua has for us.
  lua_close(L);
  free_build_config();

  return 0;
}
