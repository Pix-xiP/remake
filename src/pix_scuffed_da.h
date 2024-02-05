#ifndef PIX_SCUFFED_DA_H
#define PIX_SCUFFED_DA_H 1

#include <stdlib.h>
#include <unistd.h>

#ifndef PIX_FREE
#define PIX_FREE(ptr)                                                                    \
  do {                                                                                   \
    if (ptr != NULL) {                                                                   \
      free(ptr);                                                                         \
      ptr = NULL;                                                                        \
    }                                                                                    \
  } while (0)
#endif

#ifndef PIX_REALLOC
#define PIX_REALLOC realloc
#endif

typedef struct _dynamic_array_t {
  const char **items;
  size_t count;
  size_t capacity;
} DynamicArray;

// Initial size of DA
#define PIX_DYNARR_INIT_SZ 256

#define pix_da_free(da) PIX_FREE((da).items);

#define pix_da_append(da, item)                                                          \
  do {                                                                                   \
    if ((da)->count >= (da)->capacity) {                                                 \
      (da)->capacity = (da)->capacity == 0 ? PIX_DYNARR_INIT_SZ : 2 * (da)->capacity;    \
      (da)->items = PIX_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));     \
      (assert((da)->items != NULL && "Unable to realloc"));                              \
    }                                                                                    \
    (da)->items[(da)->count++] = (item);                                                 \
  } while (0)

#define pix_da_append_multi(da, in_items, items_count)                                   \
  do {                                                                                   \
    if ((da)->count + items_count > (da)->capacity) {                                    \
      if ((da)->capacity == 0) {                                                         \
        (da)->capacity = NOB_DA_INIT_CAP;                                                \
      }                                                                                  \
      while ((da)->count + items_count > (da)->capacity) {                               \
        (da)->capacity *= 2;                                                             \
      }                                                                                  \
      (da)->items = PIX_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));     \
      assert((da)->items != NULL && "Buy more RAM lol");                                 \
    }                                                                                    \
    memcpy((da)->items + (da)->count, in_items, items_count * sizeof(*(da)->items));     \
    (da)->count += items_count;                                                          \
  } while (0)

#endif
