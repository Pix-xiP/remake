#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "pb_parsing.h"
#define PIX_IMPLEMENTATION
#include "pix.h"
#include "pix_scuffed_da.h"
#include "src/pix_logging.h"

// v1 -- Makes this smarter :>
// This will just handle C for the time being
typedef struct PB_Builder {

  const char *compiler;
  const char *build_path;
  const char *exec_name;

  DynamicArray libs;
  DynamicArray files;
  DynamicArray cflags;
  DynamicArray ldflags;
  DynamicArray inc_dirs;

} PB_Builder;

// PIXTODO: Instantiate in main setup and pass around pointer?
PB_Builder pb = {0};

void print_da(size_t count, const char **items) {
  for (size_t i = 0; i < count; i++)
    fprintf(stdout, "%s ", items[i]);
  printf("\n");
}
void print_pb() {
  p_log("Compiler: %s", pb.compiler);

  printf("Files:\n");
  print_da(pb.files.count, pb.files.items);
  printf("CFlags:\n");
  print_da(pb.cflags.count, pb.cflags.items);
  printf("Libs\n");
  print_da(pb.libs.count, pb.libs.items);
  printf("Include Dirs\n");
}

i32 alloc_and_cpy_string(void **loc, const char *str) {
  u64 len = pix_strlen((char *)str);
  (*loc) = pix_calloc(len);
  pix_memcpy((*loc), str, len);
  return len;
}

void parse_nested_table(lua_State *state) {
  return; // PIXTODO: Remove this when tackling the rest lmao.
  lua_pushvalue(state, -1);
  lua_pushnil(state);
  while (lua_next(state, -2) != 0) {
    if (lua_type(state, -2) == LUA_TSTRING) {
      p_log("    This is a string'd key: %s", lua_tostring(state, -2));
    } else {
      p_log("    This is a number'd key: %lld", lua_tointeger(state, -2));
    }
    p_log("    With a value of: %s", luat_to_string(lua_type(state, -1)));
    lua_pop(state, 1); // Pop value from stack
  }
  lua_pop(state, 1); // Pop nested table from stack
}

bool parse_executable(lua_State *L) {
  if (lua_type(L, -1) != LUA_TSTRING) {
    p_err("[LUA]: Executable field expects 'string' found: '%s'",
          luat_to_string(lua_type(L, -1)));
    return true;
  }
  alloc_and_cpy_string((void **)&pb.exec_name, lua_tostring(L, -1));
  return false;
}

bool parse_compiler(lua_State *state) {
  // Parse function that could return a string, or just a string.
  if (lua_type(state, -1) != LUA_TFUNCTION && lua_type(state, -1) != LUA_TSTRING) {
    p_err("[LUA]: Compiler field expects 'string' or 'function' found: '%s'",
          luat_to_string(lua_type(state, -1)));
    return true;
  }
  if (lua_type(state, -1) == LUA_TFUNCTION) {
    // parse the function
    if (lua_pcall(state, 0, 1, 0)) {
      p_err("Error in compiler function: '%s'", lua_tostring(state, -1));
      lua_pop(state, 1);
      return true;
    }
    if (lua_type(state, -1) != LUA_TSTRING) {
      p_err("Compiler variable expecting a 'string', function returned: '%s'",
            luat_to_string(lua_type(state, -1)));
    }
    alloc_and_cpy_string((void **)&pb.compiler, lua_tostring(state, -1));
    // I don't understand why I don't need to pop here..
    // state of lua is local to this function and its not being modified? Or it is being
    //
    // modified but not by the pcall? lua_pop(state, 1); // Remvoe what the function just
    // added to top of the stack
  } else {
    alloc_and_cpy_string((void **)&pb.compiler, lua_tostring(state, -2));
  }
  p_dbg("compiler value: %s", pb.compiler);
  return false;
}

bool parse_build_defines(lua_State *L) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    p_err("[LUA]: Expected a 'table' for 'build_defines'");
    return true;
  }
  lua_pushvalue(L, -1);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_type(L, -1) != LUA_TSTRING) {
      p_log("Type was: %s", luat_to_string(lua_type(L, -1)));
      p_err("[LUA]: defines must be 'string'");
      return true;
    }
    pix_da_append(&pb.cflags, "-D");
    pix_da_append(&pb.cflags, lua_tostring(L, -1));
    lua_pop(L, 1); // Pop value from stack
  }
  lua_pop(L, 1); // Pop nested table from stack

  return false;
}

bool parse_generic_table(lua_State *L, DynamicArray *da, const char *who, char *prefix) {
  if (lua_type(L, -1) != LUA_TTABLE) {
    p_err("[LUA]: Expected a 'table' for '%s'", who);
    return true;
  }
  lua_pushvalue(L, -1);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_type(L, -1) != LUA_TSTRING) {
      p_err("[LUA]: defines must be 'string'. Found: '%s'",
            luat_to_string(lua_type(L, -1)));
      return true;
    }
    if (prefix)
      pix_da_append(da, prefix);

    pix_da_append(da, lua_tostring(L, -1));
    lua_pop(L, 1); // Pop value from stack
  }
  lua_pop(L, 1); // Pop nested table from stack

  return false;
}

i32 run_build() {
  pid_t cpid = fork();
  if (cpid < 0) {
    p_err("Could not fork child process: %s", strerror(errno));
    return 1;
  }

  if (cpid == 0) {
    DynamicArray args = {0};

    pix_da_append(&args, pb.compiler);

    if (pb.exec_name) {
      pix_da_append(&args, "-o");
      pix_da_append(&args, pb.exec_name);
    }

    pix_da_append_multi(&args, pb.libs.items, pb.libs.count);
    pix_da_append_multi(&args, pb.cflags.items, pb.cflags.count);
    pix_da_append_multi(&args, pb.ldflags.items, pb.ldflags.count);
    pix_da_append_multi(&args, pb.inc_dirs.items, pb.inc_dirs.count);
    pix_da_append_multi(&args, pb.files.items, pb.files.count);
    // print_pb();

    // Ready to run, add NULL term.
    pix_da_append(&args, NULL);

    p_log("Running the following command:");
    for (size_t i = 0; i < args.count; i++)
      fprintf(stdout, "%s ", args.items[i]);
    printf("\n");

    if (execvp(args.items[0], (char *const *)args.items) < 0) {
      p_err("Could not exec child process: %s", strerror(errno));
      exit(1);
    }

  } else {
    wait(NULL);
  }

  return cpid;
}

i32 main(i32 argc, char **argv) {

  // Instantiate the Lua state
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  // Load in the build file. || call lua, expect 1 result
  if (luaL_loadfile(L, "./build.lua") || lua_pcall(L, 0, 1, 0)) {
    p_err("Error: %s", lua_tostring(L, -1));
    lua_pop(L, 1); // Pop error message from the stack
  } else {
    // ensure data we get is a table!
    if (!lua_istable(L, -1)) {
      p_err("Error: Expected a table, got %s", lua_typename(L, lua_type(L, -1)));
    }
    lua_gettable(L, -1);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      // key is at idx -2 and value at -1
      if (lua_type(L, -2) == LUA_TSTRING) {
        // p_dbg("This is a string'd key: %s", lua_tostring(L, -2));
        if (pix_strcmp("compiler", lua_tostring(L, -2)) == 0) {
          if (parse_compiler(L))
            return 1;
        } else if (pix_strcmp("build_defines", lua_tostring(L, -2)) == 0) {
          if (parse_build_defines(L))
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
        } else if (pix_strcmp("executable", lua_tostring(L, -2)) == 0) {
          if (parse_executable(L))
            return 1;
        }
      } else {
        // p_dbg("This is a number'd key: %lld", lua_tointeger(L, -2));
      }
      // p_dbg("With a value of: %s", luat_to_string(lua_type(L, -1)));
      if (lua_type(L, -1) == LUA_TTABLE)
        parse_nested_table(L);

      // Unless something gets added, this will always handle popping the last value.
      lua_pop(L, 1); // Pop total table from stack
    }

    // Finish with the lua state and cleanup.
    // lua_close(L);
  }

  print_pb();

  run_build();

  // We close state here, because we use all the allocated strings lua has already done
  // for us.
  lua_close(L);

  return 0;
}
