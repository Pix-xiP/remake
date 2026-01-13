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

// PIXTODO: Instantiate in main setup and pass around pointer?
PB_Builder pb = {0};
PB_Context pc = {0};

void print_da(size_t count, const char **items) {
  for (size_t i = 0; i < count; i++)
    fprintf(stdout, "%s ", items[i]);
  printf("\n");
}

void print_pb() {
  px_log(19, "Compiler: %s", pb.compiler);

  printf("Files:\n");
  print_da(pb.files.count, pb.files.items);
  printf("CFlags:\n");
  print_da(pb.cflags.count, pb.cflags.items);
  printf("Libs\n");
  print_da(pb.libs.count, pb.libs.items);
  printf("Library Dirs\n");
  print_da(pb.lib_dirs.count, pb.lib_dirs.items);
  printf("Include Dirs\n");
  print_da(pb.inc_dirs.count, pb.inc_dirs.items);
}

void print_pc() {
  px_log(19, "Install Dir: %s", pc.install_dir);
  px_log(19, "Build Dir: %s", pc.build_dir);
  if (pc.is_exe)
    px_log(19, "Is exe");
  if (pc.is_lib)
    px_log(19, "Is library");
}

i32 alloc_and_cpy_string(void **loc, const char *str) {
  u64 len = pix_strlen((char *)str);
  (*loc) = pix_calloc(len);
  pix_memcpy((*loc), str, len);
  return len;
}

i32 parse_opt_level(lua_State *L) {

  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: Expected a 'string' for 'optimisation_level', found '%s'",
           luat_to_string(lua_type(L, -1)));
    return 1;
  }

  const char *val = lua_tostring(L, -1);

  if (pix_strcmp(val, "none") == 0)
    pc.op_lvl = none;
  else if (pix_strcmp(val, "debug") == 0)
    pc.op_lvl = debug;
  else if (pix_strcmp(val, "debug_optimised") == 0)
    pc.op_lvl = debug_optimised;
  else if (pix_strcmp(val, "basic") == 0)
    pc.op_lvl = basic;
  else if (pix_strcmp(val, "default") == 0)
    pc.op_lvl = regular;
  else if (pix_strcmp(val, "size") == 0)
    pc.op_lvl = size;
  else if (pix_strcmp(val, "extreme") == 0)
    pc.op_lvl = extreme;
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

void parse_nested_table(lua_State *state) {
  return; // PIXTODO: Remove this when tackling the rest lmao.
  lua_pushvalue(state, -1);
  lua_pushnil(state);

  while (lua_next(state, -2) != 0) {
    if (lua_type(state, -2) == LUA_TSTRING) {
      px_log(19, "    This is a string'd key: %s", lua_tostring(state, -2));
    } else {
      px_log(19, "    This is a number'd key: %lld", lua_tointeger(state, -2));
    }
    px_log(19, "    With a value of: %s", luat_to_string(lua_type(state, -1)));
    lua_pop(state, 1); // Pop value from stack
  }
  lua_pop(state, 1); // Pop nested table from stack
}

bool parse_name(lua_State *L) {
  if (lua_type(L, -1) != LUA_TSTRING) {
    px_log(px_err, "[LUA]: Executable field expects 'string' found: '%s'",
           luat_to_string(lua_type(L, -1)));
    return true;
  }
  alloc_and_cpy_string((void **)&pb.name, lua_tostring(L, -1));

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

  alloc_and_cpy_string((void **)&pb.compiler, lua_tostring(state, -1));
  px_log(px_dbg, "compiler value: %s", pb.compiler);

  return false;
}

bool parse_generic_table(lua_State *L, DynamicArray *da, const char *who, char *prefix) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    px_log(px_err, "[LUA]: Expected a 'table' for '%s'", who);
    return true;
  }
  lua_pushvalue(L, -1);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_type(L, -1) != LUA_TSTRING) {
      px_log(px_err, "[LUA]: defines must be 'string'. Found: '%s'",
             luat_to_string(lua_type(L, -1)));
      return true;
    }
    if (prefix)
      pix_da_append(da, prefix);

    const char *val = NULL;
    alloc_and_cpy_string((void **)&val, lua_tostring(L, -1));
    pix_da_append(da, val);

    lua_pop(L, 1); // Pop value from stack
  }
  lua_pop(L, 1); // Pop nested table from stack

  return false;
}

void add_optimisation_flags() {
  // What other options by default?
  switch (pc.op_lvl) {
  case basic:
    pix_da_append(&pb.cflags, "-O1");
    break;
  case debug:
    pix_da_append(&pb.cflags, "-g3");
    break;
  case debug_optimised:
    pix_da_append(&pb.cflags, "-g2");
    pix_da_append(&pb.cflags, "-Og");
    break;
  case size:
    pix_da_append(&pb.cflags, "-Os");
    break;
  case regular:
    pix_da_append(&pb.cflags, "-O2");
    break;
  case extreme:
    pix_da_append(&pb.cflags, "-Ofast");
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

i32 run_build() {
  DynamicArray args = {0};
  bool rebuild_exe = false;

  // Some default includes
  pix_da_append(&pb.inc_dirs, "-I.");
  pix_da_append(&pb.inc_dirs, "-I..");

  // Add optimisation flags
  add_optimisation_flags();

  // Add the compiler - default to?..
  if (pb.compiler)
    pix_da_append(&args, pb.compiler);
  else
    pix_da_append(&args, "cc");

  // Append the flgs and directories that may be needed to each line.
  pix_da_append_multi(&args, pb.cflags.items, pb.cflags.count);
  pix_da_append_multi(&args, pb.defines.items, pb.defines.count);
  pix_da_append_multi(&args, pb.inc_dirs.items, pb.inc_dirs.count);

  for (size_t i = 0; i < pb.files.count; ++i) {
    // Store the current count of 'args' before adding file-specific arguments.
    size_t initial_args_count = args.count;
    char *path = realpath(pb.files.items[i], NULL);

    if (path == NULL) {
      px_log(px_err, "Unable to find file: %s", pb.files.items[i]);
      exit(1);
    }

    char *obj = pix_calloc(pix_strlen(path));
    strcpy(obj, path);
    obj[strlen(path) - 1] = 'o';

    // px_log(19, "%s", obj);
    // px_log(19, "%s", path);

    pix_da_append(&pb.obj_files, obj);

    if (is_file_newer(path, obj)) {
      // Append file to rebuild.
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

    PIX_FREE(path);
  }

  if (rebuild_exe) {
    if (pb.name) {
      pix_da_append(&args, "-o");
      pix_da_append(&args, pb.name);
    }

    pix_da_append_multi(&args, pb.obj_files.items, pb.obj_files.count);
    pix_da_append_multi(&args, pb.ldflags.items, pb.ldflags.count);
    pix_da_append_multi(&args, pb.lib_dirs.items, pb.lib_dirs.count);
    pix_da_append_multi(&args, pb.libs.items, pb.libs.count);
    pix_da_append(&args, NULL);

    px_log(px_info, "Compiling executable");
    exec_fork(&args);
  } else {
    px_log(px_info, "No changes detected.");
  }

  pix_da_free(args);

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
    lua_gettable(L, -1);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      // key is at idx -2 and value at -1
      if (lua_type(L, -2) == LUA_TSTRING) {
        // px_log(px_dbg,"This is a string'd key: %s", lua_tostring(L, -2));
        if (pix_strcmp("compiler", lua_tostring(L, -2)) == 0) {
          if (parse_compiler(L))
            return 1;
        } else if (pix_strcmp("defines", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.defines, "defines", "-D"))
            return 1;
        } else if (pix_strcmp("cflags", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.cflags, "cflags", NULL))
            return 1;
        } else if (pix_strcmp("src_files", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.files, "src_files", NULL))
            return 1;
        } else if (pix_strcmp("include_dirs", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.inc_dirs, "inc_dirs", "-I"))
            return 1;
        } else if (pix_strcmp("include_libs", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.libs, "libs", "-l"))
            return 1;
        } else if (pix_strcmp("library_dirs", lua_tostring(L, -2)) == 0) {
          if (parse_generic_table(L, &pb.lib_dirs, "lib_dirs", "-L"))
            return 1;
        } else if (pix_strcmp("optimisation_level", lua_tostring(L, -2)) == 0) {
          if (parse_opt_level(L))
            return 1;
        } else if (pix_strcmp("name", lua_tostring(L, -2)) == 0) {
          if (parse_name(L))
            return 1;
        }
      } else {
        // px_log(px_dbg,"This is a number'd key: %lld", lua_tointeger(L, -2));
      }
      // px_log(px_dbg,"With a value of: %s", luat_to_string(lua_type(L, -1)));
      if (lua_type(L, -1) == LUA_TTABLE)
        parse_nested_table(L);

      // Unless something gets added, this will always handle popping the last value.
      lua_pop(L, 1); // Pop total table from stack
    }
    // Finish with the lua state and cleanup.
    // lua_close(L);
  }

  // To have a peek at whats inside..
  print_pb();
  print_pc();

  run_build();

  // We close state here, because we use all the allocated strings lua has for us.
  lua_close(L);
  return 0;
}
