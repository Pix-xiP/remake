#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <string.h>
#include <unistd.h>

#include "pb_stack.h"
#include "pix_build.h"
#include "pix_logging.h"

// helper printer function.
char *luat_to_string(int type) {
  switch (type) {
  case LUA_TNONE:
    return "none";
  case LUA_TNIL:
    return "nil";
  case LUA_TBOOLEAN:
    return "boolean";
  case LUA_TNUMBER:
    return "number";
  case LUA_TSTRING:
    return "string";
  case LUA_TTABLE:
    return "table";
  case LUA_TFUNCTION:
    return "function";
  case LUA_TUSERDATA:
    return "userdata";
  case LUA_TTHREAD:
    return "thread";
  default:
    return "unmapped";
  }

  unreachable();
}

void parse_build_file(lua_State *state, char *prefix, pb_stack *stack, CmdList *cmds) {
  p_info("Parsing build file function");

  lua_pushnil(state); // The first key

  const char *key, value;
  char hex[16];

  while (lua_next(state, -2)) {
    if (lua_isnil(state, -2)) {
      p_err("Nil State found");
      return;
    }

    key = lua_tostring(state, -2); // The assumes the table only has k, v pairs currently.
  }
}

void parse_kv_table(lua_State *state, char *prefix, pb_stack *stack) {
  lua_pushnil(state);
  const char *key, *value;
  char hex[16];

  while (lua_next(state, -2)) {
    if (lua_isnil(state, -2)) {
      return;
    }

    key = lua_tostring(state, -2);

    if (lua_type(state, -1) == LUA_TTABLE) {
      if (prefix) {
        u32 new_prefix_len = (prefix ? strlen(prefix) : 0) + strlen(key) + 2;

        char new_prefix[new_prefix_len];
        snprintf(new_prefix, new_prefix_len, "%s.%s", prefix, key);
        parse_kv_table(state, new_prefix, stack);
      } else {
        parse_kv_table(state, (char *)key, stack);
      }
    } else {
      if (lua_type(state, -1) == LUA_TBOOLEAN) {
        value = lua_toboolean(state, -1) ? "on" : "off";
      } else {
        if ((strcmp(key, "color") == 0 || strcmp(key, "border_color") == 0) &&
            lua_type(state, -1) == LUA_TNUMBER) {
          u32 number = lua_tonumber(state, -1);
          snprintf(hex, 16, "0x%x", number);
          value = hex;
        } else {
          value = lua_tostring(state, -1);
        }
      }

      u32 kv_pair_len =
          (prefix ? strlen(prefix) + 1 : 0) + strlen(value) + strlen(key) + 5;

      char kv_pair[kv_pair_len];
      if (prefix) {
        snprintf(kv_pair, kv_pair_len, "%s.%s=%s", prefix, key, value);
      } else {
        snprintf(kv_pair, kv_pair_len, "%s=%s", key, value);
      }
      stack_push(stack, kv_pair);
    }
    lua_pop(state, 1);
  }
}
