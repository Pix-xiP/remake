#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Lua files
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "pb_parsing.h"
#include "pb_stack.h"
#include "pix_build.h"
#include "pix_logging.h"

void *byte_cpy(void *dst, const void *src, size_t len, unsigned char **pos) {
  char *d = dst;
  const char *s = src;
  *pos += len; // Shift the pointer along before we modify the size var;
  while (len--)
    *d++ = *s++;
  return dst;
}

char *flatten_cmd(CmdList *cmd) {

  char *flat_cmd = calloc(cmd->capacity, sizeof(char));
  char *pos = flat_cmd;
  for (i32 i = 0; i < cmd->count; i++) {
    byte_cpy(pos, cmd->cmds[i], strlen(cmd->cmds[i]) + 1, (u8 **)&pos);
    byte_cpy(pos, " ", 1, (u8 **)&pos);
  }

  return flat_cmd;
}

int pb_cmd_run() {
  pid_t cpid = fork();
  if (cpid < 0) {
    p_err("Could not fork child process: %s", strerror(errno));
    return 1;
  }

  char *args[] = {"gcc", "-o", "working", "test.c", NULL};
  if (cpid == 0) {
    p_log("Hello, running gcc");
    if (execvp("gcc", (char *const *)&args) < 0) {
      p_err("Could not exec child process: %s", strerror(errno));
      exit(1);
    }
  }
  return cpid;
}

void error(lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
  lua_close(L);
  exit(EXIT_FAILURE);
}

static void stackDump(lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) { /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

    case LUA_TSTRING: /* strings */
      printf("`%s'", lua_tostring(L, i));
      break;

    case LUA_TBOOLEAN: /* booleans */
      printf(lua_toboolean(L, i) ? "true" : "false");
      break;

    case LUA_TNUMBER: /* numbers */
      printf("%g", lua_tonumber(L, i));
      break;

    default: /* other values */
      printf("%s", lua_typename(L, t));
      break;
    }
    printf("  "); /* put a separator */
  }
  printf("\n"); /* end the listing */
}

void pb_load(const char *filename) {
  lua_State *state = luaL_newstate();
  pb_stack *stack = stack_create();
  stack_init(stack);
  luaL_openlibs(state);

  p_info("Loading in configuration file: %s", filename);
  // int res = luaL_loadfile(L, filename);
  // p_log("res %d", res);
  if (luaL_loadfile(state, filename) || lua_pcall(state, 0, 2, 0))
    error(state, "Cannot load pb configuration file :%s", lua_tostring(state, -1));

  // lua_getglobal(state, "width"); // get globals..
  // lua_getglobal(state, "height");
  if (lua_type(state, 1) == LUA_TTABLE) {
    p_log("Found a table, parsing...");
    luaL_checktype(state, 1, LUA_TTABLE);

    lua_pushnil(state); // First Key;
    while (lua_next(state, 2)) {
      // uses 'key' (at indext -x) and 'value' (at index -1)
      printf("%s - %s\n", lua_typename(state, lua_type(state, -2)),
             lua_typename(state, lua_type(state, -3)));
    }
    // parse_kv_table(state, NULL, stack);
    // stack_print(stack);
  }

  p_log("getting top most value of lua");
  p_log("%d", lua_gettop(state));

  p_log("%s", luat_to_string(lua_type(state, 1)));
  p_log("%d", lua_type(state, 1));

  p_log("%s", luat_to_string(lua_type(state, 2)));
  p_log("%d", lua_type(state, 2));

  p_log("Second string: %s", lua_tostring(state, -1));
  p_log("Second string: %s", lua_tostring(state, 2));

  lua_getglobal(state, "target");
  const char *foo = lua_tostring(state, -1);
  p_log("This is target: %s", foo);

  lua_close(state);
}

void better_load(char *filename) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 1, 0)) {
    printf("error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1); // pop error message from the stack
  } else {
    if (lua_type(L, -1) != LUA_TTABLE) {
      printf("Error: Unexpected return type of build file. Found: %s\n",
             luat_to_string(lua_type(L, -1)));
    }
    // luaL_checktype(L, -1, LUA_TTABLE); // Check that the first result is a
    // table
    pb_stack *stack = stack_create();
    stack_init(stack);

    parse_kv_table(L, NULL, stack);
    stack_print(stack);

    // This also works for just parsing through a table very basic.
    // lua_pushnil(L); // First key
    // while (lua_next(L, -2) != 0) {
    //   // Uses 'key' (at index -2) and 'value' (at index -1)
    //   printf("Key(%s): %s - Value(%s): %s\n", lua_typename(L, lua_type(L,
    //   -2)),
    //          lua_tostring(L, -2), lua_typename(L, lua_type(L, -1)),
    //          lua_tostring(L, -1));
    //
    //   lua_pop(L, 1); // Removes 'value'; keeps 'key' for next iteration
    // }
    //
    // lua_pop(L, 1); // Remove the table
  }

  lua_close(L);
}

int example(void) {
  char buff[256];
  int error;
  lua_State *L = luaL_newstate(); // opens lua
  luaL_openlibs(L);
  // Replaced by openLibs.
  // luaopen_base(L);   /* opens the basic library */
  // luaopen_table(L);  /* opens the table library */
  // luaopen_io(L);     /* opens the I/O library */
  // luaopen_string(L); /* opens the string lib. */
  // luaopen_math(L);   /* opens the math lib. */

  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s", lua_tostring(L, -1));
      lua_pop(L, 1); /* pop error message from the stack */
    }
  }

  lua_close(L);
  return 0;
}

int main(int argc, char **argv) {
  // TODO: Parse the arguments, maybe not using getopts for once.

  // Load the build file.
  // TODO: Make this look to the root of a project directory?
  // pb_load("./test/build.lua");
  better_load("./test/build.lua");

  // printf("Hello, I am your main!\n");
  // pb_cmd_run();
  //
  // sleep(5);

  return 0;
}
