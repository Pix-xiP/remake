#ifndef PB_PARSING_H
#define PB_PARSING_H 1
#pragma once

#include "pb_stack.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

void parse_kv_table(lua_State *state, char *prefix, pb_stack *stack);
char *luat_to_string(int type);

#endif
