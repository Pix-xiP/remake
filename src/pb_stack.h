#ifndef PB_STACK_H
#define PB_STACK_H 1

#pragma once

// Implementation taken from FelixKratz's stack.h implementation
// inside the Sketchybar Lua repository.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pix_logging.h"
#include "pix_types.h"

#define m_clone(dst, src)                                                                \
  {                                                                                      \
    dst = malloc(strlen(src) + 1);                                                       \
    memcpy(dst, src, strlen(src) + 1);                                                   \
  }

typedef struct _stack_t {
  char **value;
  u32 num_values;
} pb_stack;

static inline pb_stack *stack_create() { return malloc(sizeof(pb_stack)); }

static inline void stack_init(pb_stack *stack) { memset(stack, 0, sizeof(pb_stack)); }

static inline void stack_push(pb_stack *stack, const char *value) {
  stack->value = realloc(stack->value, (sizeof(char *) * ++stack->num_values));
  m_clone(stack->value[stack->num_values - 1], value);
}

static inline void stack_copy(pb_stack *stack, pb_stack *copy) {
  stack_init(copy);

  for (i32 i = 0; i < stack->num_values; i++)
    stack_push(copy, stack->value[i]);
}

static inline void stack_pop(pb_stack *stack) {
  if (stack->num_values == 0)
    return;

  if (stack->value[stack->num_values - 1])
    free(stack->value[stack->num_values - 1]);

  stack->value[stack->num_values - 1] = NULL;
  stack->num_values--;
}

static inline void stack_clean(pb_stack *stack) {
  for (u32 i = 0; i < stack->num_values; i++) {
    if (stack->value[i])
      free(stack->value[i]);
  }

  if (stack->value)
    free(stack->value);
  stack_init(stack);
}

static inline void stack_destroy(pb_stack *stack) {
  stack_clean(stack);
  free(stack);
}

static inline char *stack_flatten_ttb(pb_stack *stack, u32 *length) {
  if (!stack)
    return NULL;
  if (stack->num_values == 0)
    return NULL;
  *length = 0;
  for (i32 i = stack->num_values - 1; i >= 0; i--) {
    *length += strlen(stack->value[i]) + 1;
  }
  char *out = malloc(*length);
  u32 caret = 0;

  for (i32 i = stack->num_values - 1; i >= 0; i--) {
    u32 entry_size = strlen(stack->value[i]) + 1;
    memcpy(out + caret, stack->value[i], entry_size);
    caret += entry_size;
  }

  return out;
}

static inline void stack_print(pb_stack *stack) {
  for (u32 i = 0; i < stack->num_values - 1; i++)
    fprintf(stdout, "%s -> ", stack->value[i]);
  if (stack->num_values > 0)
    fprintf(stdout, "%s\n", stack->value[stack->num_values - 1]);
}

#endif
