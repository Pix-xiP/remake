// Pix Global Helpers
// Standard STB style header boi, do what you like with it, slap my name at the
// top if you care that its from me, sorry if it explodes on you.
//
// ============================================================================
// Usage:
//
// Include in ONE C file that includes this header, BEFORE the include like dis:
//
//    #define PIX_IMPLEMENTATION
//    #include "pix.h"
//
// All other files should just #include "pix.h" without the define as normal :>
// ============================================================================

// ===============================
// DEFINITIONS AND FORWARD DECS
// ===============================
#ifndef PIX_H
#define PIX_H 1

#ifndef PIX_DEF
#ifdef PIX_DEF_STATIC
#define PIX_DEF static
#else
#define PIX_DEF extern
#endif
#endif

#ifndef PIX_TYPES_H
#define PIX_TYPES_H 1

#ifndef NULL
#ifdef __cplusplus
#define NULL 0 // NULL 4 C++
#else
#define NULL ((void *)0)
#endif
#endif

#if !defined(false)
#define false 0 // Defines False.
#endif
#if !defined(true)
#define true !false // Anything that isn't false, is true
#endif

#if defined _WIN32
typedef unsigned int uint;
#endif

// I need a smarter check then this.
// #ifndef _LIBCPP_STDINT_H
// typedef signed char i8;
// typedef unsigned char u8;
// typedef signed short i16;
// typedef unsigned short u16;
// typedef signed int i32;
// typedef unsigned int u32;
// #else
// Use STDINT definitions for portability
#include <stdint.h>
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

// I also want access to size_t, ptrdiff_t and ssize_t
#include <stddef.h>

/* #endif */

typedef float f32;
typedef double f64;
typedef u8 b8;
typedef u16 b16;
typedef i32 b32;

#if !defined(__cplusplus)
#ifndef bool
#define bool _Bool
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true !false
#endif
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
/* Define _Bool as a GNU extension. */
#define _Bool bool
#if defined(__cplusplus) && __cplusplus < 201103L
#define bool bool
#define false false
#define true true
#endif
#endif

typedef char *string;

// Define struct for String?
// typedef struct __string {
//  char *str;
//  i64 len;
// } String; // ???

#ifdef __clang__
typedef char c8 __attribute__((ext_vector_type(8), aligned(1)));
typedef char c16 __attribute__((ext_vector_type(16), aligned(1)));
typedef char c32 __attribute__((ext_vector_type(32), aligned(1)));
typedef char c32a __attribute__((ext_vector_type(32), aligned(32)));

#else
// __GNUC__
typedef char c8 __attribute__((vector_size(8), aligned(1)));
typedef char c16 __attribute__((vector_size(16), aligned(1)));
typedef char c32 __attribute__((vector_size(32), aligned(1)));
typedef char c32a __attribute__((vector_size(32), aligned(32)));
#endif

#endif // PIX_TYPES_H

// gimme dat sweet size_t ptrdiff_t etc
#include <stddef.h>

#ifndef PIX_FREE
#include <stdlib.h>
#define PIX_FREE(ptr)                                                                              \
  do {                                                                                             \
    if (ptr != NULL) {                                                                             \
      free(ptr);                                                                                   \
      ptr = NULL;                                                                                  \
    }                                                                                              \
  } while (0)
#endif

#ifndef pix_assert
#include <signal.h>
#include <stdio.h>
#define pix_assert(expr, msg)                                                                      \
  if (!(expr)) {                                                                                   \
    perror(#expr);                                                                                 \
    msg;                                                                                           \
    kill(0, SIGTERM);                                                                              \
  }
#endif

// SOME ENDIAN HELPERS! Replacement for noths etc
// swap bytes in 16 bit value
#define pix_bswap16(x) ((u16)((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

// Swap bytes in 32 bit value
#define pix_bswap32(x)                                                                             \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8) | (((x) & 0x0000ff00u) << 8) |         \
   (((x) & 0x000000ffu) << 24))

// Swap bytes in 64 bit value
#define pix_bswap64(x)                                                                             \
  ((((x) & 0xff00000000000000ull) >> 56) | (((x) & 0x00ff000000000000ull) >> 40) |                 \
   (((x) & 0x0000ff0000000000ull) >> 24) | (((x) & 0x000000ff00000000ull) >> 8) |                  \
   (((x) & 0x00000000ff000000ull) << 8) | (((x) & 0x0000000000ff0000ull) << 24) |                  \
   (((x) & 0x000000000000ff00ull) << 40) | (((x) & 0x00000000000000ffull) << 56))

// ===============================
// FUNCTION DEFINITIONS
// ===============================

PIX_DEF void *pix_memcpy(void *dst, const void *src, size_t n);
PIX_DEF void *pix_memset(void *s, i32 c, u64 n);
PIX_DEF void *pix_calloc(i64 sz);
PIX_DEF i64 pix_strlen(const char *str);
PIX_DEF i64 _pix_strlen(const char *str); // Just in case you don't want the + 1
PIX_DEF void pix_strncpy(char *dst, const char *src, i64 n);
PIX_DEF i32 pix_strcmp(const char *p1, const char *p2);
PIX_DEF char *shrinkwrap_string(char *in_str);
PIX_DEF void *byte_cpy(void *dst, const void *src, i32 len, u8 **pos);
PIX_DEF char *int_to_addr(u32 ip, char **addr);
PIX_DEF i32 addr_to_int(const char *addr, u32 *out);
PIX_DEF bool mkdir_if_not_exists(const char *path);

// LOGGING
typedef enum {
  px_emrg,
  px_alert,
  px_crit,
  px_err,
  px_warn,
  px_success,
  px_info,
  px_verb,
  px_dbg,
  px_dbgv
} px_logtypes;

// PIXTODO: Improve general loggin and screen manipulation with escape codes.
#define _NRM "\x1b[0;0m"
#define _RED "\x1b[0;31m"
#define _GRN "\x1b[0;32m"
#define _YEL "\x1b[0;33m"
#define _BLU "\x1b[0;34m"
#define _MAG "\x1b[0;35m"
#define _CYN "\x1b[0;36m"
#define _WHT "\x1b[0;37m"

#define _B_NRM "\x1b[1;0m"
#define _B_RED "\x1b[1;31m"
#define _B_GRN "\x1b[1;32m"
#define _B_YEL "\x1b[1;33m"
#define _B_BLU "\x1b[1;34m"
#define _B_MAG "\x1b[1;35m"
#define _B_CYN "\x1b[1;36m"
#define _B_WHT "\x1b[1;37m"

#define _BOLD "\x1b[1m"
#define _UNDERLINE "\x1b[4m"
#define _BLINK "\x1b[5m"
#define _REVERSE "\x1b[7m"
#define _HIDE "\x1b[8m"
#define _CLEAR "\x1b[2J"
// #define _CLRLINE "\r\e[K // or \e[1K\r"

#define PX_LOG_OUT stderr

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// syslog uses this, pull out errno
#include <errno.h>
#define err_to_str() (errno == 0 ? "" : strerror(errno))

extern bool PX_VERBOSE_LOGGING; // Defaults FALSE
PIX_DEF void px_log(u16 type, const char *format, ...);
// END LOGGING

#define pix_unreachable()                                                                          \
  do {                                                                                             \
    px_log(px_emrg, "Unreachable content reached.");                                               \
    exit(119);                                                                                     \
  } while (0)

#endif // PIX_H
//
#ifndef PIX_SCUFFED_DA_H // {{
#define PIX_SCUFFED_DA_H 1

#ifndef PIX_REALLOC
#define PIX_REALLOC realloc
#endif

typedef struct _dynamic_array_t {
  const char **items;
  size_t count;
  size_t capacity;
} DynamicArray;

#define PIX_DYNARR_INIT_SZ 256

#define pix_da_free(da) PIX_FREE((da).items);

#define pix_da_append(da, item)                                                                    \
  do {                                                                                             \
    if ((da)->count >= (da)->capacity) {                                                           \
      (da)->capacity = (da)->capacity == 0 ? PIX_DYNARR_INIT_SZ : 2 * (da)->capacity;              \
      (da)->items = PIX_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));               \
      pix_assert((da)->items != NULL, px_log(px_err, "Unable to realloc"));                        \
    }                                                                                              \
    (da)->items[(da)->count++] = (item);                                                           \
  } while (0)

#define pix_da_append_multi(da, in_items, items_count)                                             \
  do {                                                                                             \
    if ((da)->count + items_count > (da)->capacity) {                                              \
      if ((da)->capacity == 0) {                                                                   \
        (da)->capacity = PIX_DYNARR_INIT_SZ;                                                       \
      }                                                                                            \
      while ((da)->count + items_count > (da)->capacity) {                                         \
        (da)->capacity *= 2;                                                                       \
      }                                                                                            \
      (da)->items = PIX_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items));               \
      pix_assert((da)->items != NULL, px_log(px_err, "Out of Memory"));                            \
    }                                                                                              \
    pix_memcpy((da)->items + (da)->count, (in_items), (items_count) * sizeof(*(da)->items));       \
    (da)->count += items_count;                                                                    \
  } while (0)

#define pix_da_append_cstr(da, buf, sz) pix_da_append_multi(da, buf, sz)

#endif // PIX DA }}

// ===============================
// IMPLEMENTATION BAY-BEE
// ===============================

// {{ Mark: PIX_IMPLEMENTATION
// #define PIX_IMPLEMENTATION // Uncomment for quick easy syntax highlighting
#if defined(PIX_IMPLEMENTATION) && !defined(PIX_IMPLEMENTATION_DONE)
#define PIX_IMPLEMENTATION_DONE 1

// Make sure we have a null term'd string.
PIX_DEF void pix_strncpy(char *dst, const char *src, i64 n) {
  pix_memcpy(dst, src, n);
  if (n > 0)
    dst[n - 1] = '\0';
}

PIX_DEF i32 pix_strcmp(const char *p1, const char *p2) {
  const u8 *s1 = (const u8 *)p1;
  const u8 *s2 = (const u8 *)p2;
  u8 c1, c2;
  do {
    c1 = (u8)*s1++;
    c2 = (u8)*s2++;
    if (c1 == '\0')
      return c1 - c2;
  } while (c1 == c2);
  return c1 - c2;
}

// Takes in a string that is overallocated and reduces it down to the first
// null terminator.
PIX_DEF char *shrinkwrap_string(char *in_str) {
  char *new_str = NULL;
  i64 len = pix_strlen(in_str);
  new_str = (char *)pix_calloc(len);
  pix_memcpy(new_str, in_str, len - 1);
  PIX_FREE(in_str);
  return new_str;
}

// Does a memcpy and moves along the position pointer
// Position can be same as dst, just allows for versitily this way
PIX_DEF void *byte_cpy(void *dst, const void *src, i32 len, u8 **pos) {
  u8 *d = (u8 *)dst;
  const u8 *s = (u8 *)src;
  *pos += len; // Shift the pointer along before we modify the size var;
  while (len--)
    *d++ = *s++;
  return dst;
}

// Converts a string addr into the integer representation.
// If the err check fills it will return 5 not 4, indicating an invalid ip.
PIX_DEF i32 addr_to_int(const char *addr, u32 *out) {
  u32 b0, b1, b2, b3;
  char err_check[2];

  pix_assert(out != NULL, px_log(px_err, "addr_to_int: out param was not initilised"));

  if (sscanf(addr, "%u.%u.%u.%u%1s", &b3, &b2, &b1, &b0, err_check) == 4) {
    if (b3 < 256 && b2 < 256 && b1 < 256 && b0 < 256) {
      *out = (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
      return 0;
    }
  }
  return -1;
}

#define PIX_IP_MAX_STR_LEN 18
// Takes in an IP and conversts it to the string representation
// if addr is NULL, it will return the address.
PIX_DEF char *int_to_addr(u32 ip, char **addr) {
  u8 b[4];
  char *a;
  b[0] = ip & 0xFF;
  b[1] = (ip >> 8) & 0xFF;
  b[2] = (ip >> 16) & 0xFF;
  b[3] = (ip >> 24) & 0xFF;

  if (addr != NULL)
    a = *addr;
  else // If it is NULL, alloc.
    a = (char *)pix_calloc(PIX_IP_MAX_STR_LEN);

  snprintf(a, PIX_IP_MAX_STR_LEN, "%d.%d.%d.%d", b[3], b[2], b[1], b[0]);
  return a;
}

// I'm not actually if this is faster then LIBC standard anymore
// given they use inline asm now.. pray the compiler is dope? xD
PIX_DEF void *pix_memcpy(void *dst, const void *src, size_t n) {
  char *d = (char *)dst;
  const char *s = (char *)src;

  if (n < 5) {
    if (n == 0)
      return dst;
    d[0] = s[0];
    d[n - 1] = s[n - 1];
    if (n <= 2)
      return dst;
    d[1] = s[1];
    d[2] = s[2];
    return dst;
  }

  if (n <= 16) {
    if (n >= 8) {
      const char *first_s = s;
      const char *last_s = s + n - 8;
      char *first_d = d;
      char *last_d = d + n - 8;
      *((u64 *)first_d) = *((u64 *)first_s);
      *((u64 *)last_d) = *((u64 *)last_s);
      return dst;
    }

    const char *first_s = s;
    const char *last_s = s + n - 4;
    char *first_d = d;
    char *last_d = d + n - 4;
    *((u32 *)first_d) = *((u32 *)first_s);
    *((u32 *)last_d) = *((u32 *)last_s);
    return dst;
  }

  if (n <= 32) {
    const char *first_s = s;
    const char *last_s = s + n - 16;
    char *first_d = d;
    char *last_d = d + n - 16;

    *((c16 *)first_d) = *((c16 *)first_s);
    *((c16 *)last_d) = *((c16 *)last_s);
    return dst;
  }

  const char *last_word_s = s + n - 32;
  char *last_word_d = d + n - 32;

  // Stamp the 32-byte chunks.
  do {
    *((c32 *)d) = *((c32 *)s);
    d += 32;
    s += 32;
  } while (d < last_word_d);

  // Stamp the last unaligned 32 bytes of the buffer.
  *((c32 *)last_word_d) = *((c32 *)last_word_s);
  return dst;
}

// Faster MEMSET implementation from:
// https://github.com/nadavrot/memset_benchmark/blob/main/src/memset/impl.c
PIX_DEF void *_pix_smol_memset(void *s, i32 c, u64 n) {
  if (n < 5) {
    if (n == 0)
      return s;
    char *p = s;
    p[0] = c;
    p[n - 1] = c;
    if (n <= 2)
      return s;
    p[1] = c;
    p[2] = c;
    return s;
  }

  if (n <= 16) {
    u64 val8 = ((u64)0x0101010101010101L * ((u8)c));
    if (n >= 8) {
      char *first = s;
      char *last = s + n - 8;
      *((u64 *)first) = val8;
      *((u64 *)last) = val8;
      return s;
    }

    u32 val4 = val8;
    char *first = s;
    char *last = s + n - 4;
    *((u32 *)first) = val4;
    *((u32 *)last) = val4;
    return s;
  }

  char X = c;
  char *p = s;
  c16 val16 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};
  char *last = s + n - 16;
  *((c16 *)last) = val16;
  *((c16 *)p) = val16;
  return s;
}

PIX_DEF void *_pix_big_memset(void *s, i32 c, u64 n) {
  char *p = s;
  char X = c;
  c32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
               X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

  // Stamp the first 32byte store.
  *((c32 *)p) = val32;

  char *first_aligned = p + 32 - ((u64)p % 32);
  char *buffer_end = p + n;
  char *last_word = buffer_end - 32;

  // Align the next stores.
  p = first_aligned;

  // Unroll the body of the loop to increase parallelism.
  while (p + (32 * 5) < buffer_end) {
    *((c32a *)p) = val32;
    p += 32;
    *((c32a *)p) = val32;
    p += 32;
    *((c32a *)p) = val32;
    p += 32;
    *((c32a *)p) = val32;
    p += 32;
    *((c32a *)p) = val32;
    p += 32;
  }

// Complete the last few iterations:
#define TRY_STAMP_32_BYTES                                                                         \
  if (p < last_word) {                                                                             \
    *((c32a *)p) = val32;                                                                          \
    p += 32;                                                                                       \
  }

  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES

  // Stamp the last unaligned word.
  *((c32 *)last_word) = val32;
  return s;
}

PIX_DEF void *pix_memset(void *s, i32 c, u64 n) {
  if (n < 32)
    return _pix_smol_memset(s, c, n);
  if (n > 160)
    return _pix_big_memset(s, c, n);

  char *p = s;
  char X = c;

  c32 v32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
             X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

  char *last_word = s + n - 32;

  // Stamp the 32-byte chunks.
  do {
    *((c32 *)p) = v32;
    p += 32;
  } while (p < last_word);

  // Stamp the last unaligned 32 bytes of the buffer.
  *((c32 *)last_word) = v32;
  return s;
}

// For when you really need it zero'd no questions asked.
PIX_DEF void *pix_calloc(i64 sz) {
  void *mem = malloc(sz);
  pix_memset((char *)mem, 0, sz);
  return (void *)mem;
}

// Temp include <string.h> to fix strlen problem for now.
#include <string.h>
PIX_DEF i64 pix_strlen(const char *str) { return strlen((char *)str) + 1; }

/* // Returns the length of a string including the null term. */
/* PIX_DEF i64 pix_strlen(char *str) { return _pix_strlen((char *)str) + 1; } */

// PIXTODO: This needs to be fixed - has a head overflow according to fsantise
// addrs.
PIX_DEF i64 _pix_strlen(const char *str) {
  const char *char_ptr;
  const u64 *longword_ptr;
  u64 longword, himagic, lomagic;

  // Handle the first few characters by reading one character at a time.
  // Do this until CHAR_PTR is aligned on a longword boundary.
  for (char_ptr = str; ((u64)char_ptr & (sizeof(longword) - 1)) != 0; ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  // All these elucidatory comments refer to 4-byte longwords,
  // but the theory applies equally well to 8-byte longwords.

  longword_ptr = (u64 *)char_ptr;

  // Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
  //   the "holes."  Note that there is a hole just to the left of
  //   each byte, with an extra at the end:
  //
  //   bits:  01111110 11111110 11111110 11111111
  //   bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD
  //
  //   The 1-bits make sure that carries propagate to the next 0-bit.
  //   The 0-bits provide holes for carries to fall into.
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof(longword) > 4) {
    // 64-bit version of the magic.  */
    // Do the shift in two steps to avoid a warning if long has 32 bits.
    himagic = ((himagic << 16) << 16) | himagic;
    lomagic = ((lomagic << 16) << 16) | lomagic;
  }

  // Instead of the traditional loop which tests each character,
  // we will test a longword at a time.  The tricky part is testing
  // if *any of the four* bytes in the longword in question are zero.
  for (;;) {
    longword = *longword_ptr++;

    if (((longword - lomagic) & ~longword & himagic) != 0) {
      /* Which of the bytes was the zero?  If none of them were, it was
         a misfire; continue the search.  */

      const char *cp = (const char *)(longword_ptr - 1);

      if (cp[0] == 0)
        return cp - str;
      if (cp[1] == 0)
        return cp - str + 1;
      if (cp[2] == 0)
        return cp - str + 2;
      if (cp[3] == 0)
        return cp - str + 3;
      if (sizeof(longword) > 4) {
        if (cp[4] == 0)
          return cp - str + 4;
        if (cp[5] == 0)
          return cp - str + 5;
        if (cp[6] == 0)
          return cp - str + 6;
        if (cp[7] == 0)
          return cp - str + 7;
      }
    }
  }
}

// --------------------
// FILE MANIPULATION
// --------------------
#include <sys/stat.h>
PIX_DEF bool mkdir_if_not_exists(const char *path) {
  i32 result = mkdir(path, 0755);
  if (result < 0) {
    if (errno == EEXIST) {
      px_log(px_info, "Directory '%s' already exists", path);
      return false;
    }
    px_log(px_alert, "Failed to create directory: %s", path);
    return true;
  }
  px_log(px_info, "Directory: '%s' created", path);
  return false;
}

/// PIXTODO: Wrap this in a #define PIX_LOGGING
// Logging implementation
#include <stdarg.h>
#include <stdio.h>

bool PX_VERBOSE_LOGGING = false;

PIX_DEF void px_log(u16 type, const char *format, ...) {
  switch (type) {
  case px_emrg:
    fprintf(PX_LOG_OUT, _B_RED "[E]: %s (%s:%d) " _NRM, __func__, __FILE__, __LINE__);
    break;
  case px_alert:
    fprintf(PX_LOG_OUT, _B_MAG "[A]: %s (%s:%d) " _NRM, __func__, __FILE__, __LINE__);
    break;
  case px_crit:
    fprintf(PX_LOG_OUT, _RED "[X]: %s (%s:%d) " _NRM, __func__, __FILE__, __LINE__);
    break;
  case px_err:
    fprintf(PX_LOG_OUT, _RED "[!]: ");
    break;
  case px_warn:
    fprintf(PX_LOG_OUT, _B_YEL "[W]: " _YEL);
    break;
  case px_success:
    fprintf(PX_LOG_OUT, _B_GRN "[S]: " _GRN);
    break;
  case px_info:
    fprintf(PX_LOG_OUT, _B_CYN "[+]: " _NRM);
    break;
  case px_verb:
    fprintf(PX_LOG_OUT, _MAG "[V]: " _NRM);
    break;
  case px_dbg:
    fprintf(PX_LOG_OUT, _WHT "[DBG]: " _WHT);
    break;
  case px_dbgv:
    fprintf(PX_LOG_OUT, _WHT "[DBG]: %s (%s:%d)" _WHT, __func__, __FILE__, __LINE__);
    break;
  default:
    fprintf(PX_LOG_OUT, "[LOG]: ");
    break;
  }
  va_list args;
  va_start(args, format);
  vfprintf(PX_LOG_OUT, format, args);
  va_end(args);
  if (type == px_emrg || type == px_alert || type == px_crit)
    fprintf(PX_LOG_OUT, "errno: %s", err_to_str());
  fprintf(PX_LOG_OUT, "\n" _NRM);
}

#ifndef PIX_TIME
#define PIX_TIME 1

#ifndef PX_MS_IN_SECOND
#define PX_MS_IN_SECOND 1000
#endif

#endif

#endif // PIX_IMPLEMENTATION
       // }}
