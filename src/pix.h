/* #ifndef PIX_H */
/* #define PIX_H 1 */

// Pix Global Helpers
// Standard STB style header boi, do what you like with it, slap my name at the top if you
// care that its from me, sorry if it explodes on you.
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
#ifndef PIX_TYPES
#define PIX_TYPES 1

// I need a smarter check then this.
/* #ifndef _LIBCPP_STDINT_H */
/* typedef signed char i8; */
/* typedef unsigned char u8; */
/* typedef signed short i16; */
/* typedef unsigned short u16; */
/* typedef signed int i32; */
/* typedef unsigned int u32; */
/**/
/* #else */
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
typedef u8 bool; // Convience bool.

typedef char *string;

// Define struct for String?
// typedef struct __string {
//  char *str;
//  i64 len;
// } String; // ???

#endif

#endif // PIX_TYPES_H

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

// gimme dat sweet size_t ptrdiff_t etc
#include <stddef.h>

#ifndef PIX_FREE
#include <stdlib.h>
#define PIX_FREE(ptr)                                                                    \
  do {                                                                                   \
    if (ptr != NULL) {                                                                   \
      free(ptr);                                                                         \
      ptr = NULL;                                                                        \
    }                                                                                    \
  } while (0)
#endif

#ifndef PIX_DEF
#define PIX_DEF static
#endif

PIX_DEF void *pix_memcpy(void *dst, const void *src, size_t n);
PIX_DEF void *pix_memset(void *s, i32 c, u64 n);
PIX_DEF void *pix_calloc(i64 sz);
PIX_DEF i64 pix_strlen(char *str);
PIX_DEF i64 _pix_strlen(const char *str); // This is just in case you don't want the + 1
PIX_DEF void pix_strncpy(char *dst, const char *src, i64 n);
PIX_DEF i32 pix_strcmp(const char *p1, const char *p2);
PIX_DEF char *shrinkwrap_string(char *in_str);
PIX_DEF void *byte_cpy(void *dst, const void *src, i32 len, u8 **pos);
PIX_DEF char *int_to_addr(u32 ip, char **addr);
PIX_DEF u32 addr_to_int(const char *addr, u32 *out);
PIX_DEF bool mkdir_if_not_exists(const char *path);

// A copy of my loggin file
#ifndef P_LOGGING_H
#define P_LOGGING_H 1

// NOTE: Update this to VERBOSE(A) A in local program to create verbose options
#ifndef VERBOSE
#define VERBOSE(A)
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

// NOTE: Update this in the local program to set the degree of logs being set.
#ifndef P_LOG_LEVEL
#define P_LOG_LEVEL 7
#endif

// NOTE: Update this in the local program to unset the values
#ifndef P_LOG_COLOUR
#define P_LOG_COLOUR 1
#endif

// NOTE: Update this in the local program to choose where log messages go out
// to.
#ifndef P_LOG_OUT
#define P_LOG_OUT stderr
#endif

// Same as syslog
#define EMERG 0
#define ALERT 1
#define CRIT 2
#define ERR 3
#define WARN 4
#define SUCCESS 5
#define INFO 6
#define DBG 7

// Better colour definition
#define NORM "\x1b[0m"
#define BLACK "\x1b[0;30m"
#define L_BLACK "\x1b[1;30m"
#define RED "\x1b[0;31m"
#define L_RED "\x1b[1;31m"
#define GREEN "\x1b[0;32m"
#define L_GREEN "\x1b[1;32m"
#define BROWN "\x1b[0;33m"
#define YELLOW "\x1b[1;33m"
#define BLUE "\x1b[0;34m"
#define L_BLUE "\x1b[1;34m"
#define PURPLE "\x1b[0;35m"
#define L_PURPLE "\x1b[1;35m"
#define CYAN "\x1b[0;36m"
#define L_CYAN "\x1b[1;36m"
#define GREY "\x1b[0;37m"
#define WHITE "\x1b[1;37m"

// Colour utils
#define BOLD "\x1b[1m"
#define UNDERLINE "\x1b[4m"
#define BLINK "\x1b[5m"
#define REVERSE "\x1b[7m"
#define HIDE "\x1b[8m"
#define CLEAR "\x1b[2J"
/* #define CLRLINE "\r\e[K" // or "\e[1K\r" */

// New hack to get the filename I found :D
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Could pull out errno here with this found in syslog:
#define err_to_str() (errno == 0 ? "" : strerror(errno))

// Log functions
#define p_emrg(msg, ...)                                                                 \
  do {                                                                                   \
    fprintf(P_LOG_OUT,                                                                   \
            RED "[!!!]: "                                                                \
                "%s (%s:%d) " NORM msg YELLOW " errno: %s\n" NORM,                       \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__, err_to_str());                  \
  } while (0)

#define p_alert(msg, ...)                                                                \
  do {                                                                                   \
    fprintf(P_LOG_OUT, PURPLE "[A!]: " NORM msg "\n", ##__VA_ARGS__);                    \
  } while (0)

#define p_crit(msg, ...)                                                                 \
  do {                                                                                   \
    fprintf(P_LOG_OUT,                                                                   \
            RED "[!!]: "                                                                 \
                "%s (%s:%d) " NORM msg YELLOW " errno: %s\n" NORM,                       \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__, err_to_str());                  \
  } while (0)

#define p_err(msg, ...)                                                                  \
  do {                                                                                   \
    fprintf(P_LOG_OUT, RED "[!]: " NORM msg "\n", ##__VA_ARGS__);                        \
  } while (0)

#define p_warn(msg, ...)                                                                 \
  do {                                                                                   \
    fprintf(P_LOG_OUT, BLUE "[W]: " NORM msg "\n", ##__VA_ARGS__);                       \
  } while (0)

#define p_success(msg, ...)                                                              \
  do {                                                                                   \
    fprintf(P_LOG_OUT, GREEN "[S]: " NORM msg "\n", ##__VA_ARGS__);                      \
  } while (0)

#define p_info(msg, ...)                                                                 \
  do {                                                                                   \
    fprintf(P_LOG_OUT, CYAN "[+]: " NORM msg "\n", ##__VA_ARGS__);                       \
  } while (0)

// The ability to pull out lines and func can be extended to others if required
#define p_dbg(msg, ...)                                                                  \
  do {                                                                                   \
    fprintf(P_LOG_OUT, GREY "[D]: " NORM msg "\n", ##__VA_ARGS__);                       \
  } while (0)

#define p_dbgv(msg, ...)                                                                 \
  do {                                                                                   \
    fprintf(P_LOG_OUT, GREY "[D]: %s (%s:%d) " NORM msg "\n", __func__, __FILE__,        \
            __LINE__, ##__VA_ARGS__);                                                    \
  } while (0)

#define p_log(msg, ...)                                                                  \
  do {                                                                                   \
    fprintf(P_LOG_OUT, "[>]: " msg "\n", ##__VA_ARGS__);                                 \
  } while (0)

// Log level controls -- Syslog style
//
#ifndef DEBUG
#undef p_dbg
#define p_dbg(msg, ...)                                                                  \
  do {                                                                                   \
  } while (0)
#undef p_dbgv
#define p_dbgv(msg, ...)                                                                 \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < INFO
#undef p_info
#define p_dbg(msg, ...)                                                                  \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < SUCCESS
#undef p_p_success
#define p_success(msg, ...)                                                              \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < WARN
#undef p_warn
#define p_warn(msg, ...)                                                                 \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < ERR
#undef p_err
#define p_err(msg, ...)                                                                  \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < ALERT
#undef p_alert
#define p_alert(msg, ...)                                                                \
  do {                                                                                   \
  } while (0)
#endif

#if P_LOG_LEVEL < EMERG
#undef p_emrg
#define p_emrg(msg, ...)                                                                 \
  do {                                                                                   \
  } while (0)
#endif

// Turn off colors.
#if P_LOG_COLOUR < 1
#undef NORM
#define NORM
#undef RED
#define RED
#undef PURPLE
#define PURPLE
#undef YELLOW
#define YELLOW
#undef BROWN
#define BROWN
#undef GREEN
#define GREEN
#undef CYAN
#define CYAN
#undef BLUE
#define BLUE
#undef GREY
#define GREY
#endif

#define unreachable()                                                                    \
  do {                                                                                   \
    p_emrg("UNREACHABLE CONTENT REACHED");                                               \
    exit(119);                                                                           \
  } while (0)

#endif

// ===============================
// IMPLEMENTATION BAY-BEE
// ===============================

#if defined(PIX_IMPLEMENTATION) && !defined(PIX_IMPLEMENTATION_DONE)
#define PIX_IMPLEMENTATION_DONE

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
  new_str = calloc(len, sizeof(char));
  pix_memcpy(new_str, in_str, len - 1);
  PIX_FREE(in_str);
  return new_str;
}

// Does a memcpy and moves along the position pointer
// Position can be same as dst, just allows for versitily this way
PIX_DEF void *byte_cpy(void *dst, const void *src, i32 len, u8 **pos) {
  char *d = dst;
  const char *s = src;
  *pos += len; // Shift the pointer along before we modify the size var;
  while (len--)
    *d++ = *s++;
  return dst;
}

// Converts a string addr into the integer representation.
// If the err check fills it will return 5 not 4, indicating an invalid ip.
PIX_DEF u32 addr_to_int(const char *addr, u32 *out) {
  u32 b0, b1, b2, b3, *o;
  char err_check[2];

  if (out != NULL)
    o = out;

  if (sscanf(addr, "%u.%u.%u.%u%1s", &b3, &b2, &b1, &b0, err_check) == 4) {
    if (b3 < 256 && b2 < 256 && b1 < 256 && b0 < 256) {
      *o = (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
      return *o;
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

  a = pix_calloc(PIX_IP_MAX_STR_LEN);
  snprintf(a, PIX_IP_MAX_STR_LEN, "%d.%d.%d.%d", b[3], b[2], b[1], b[0]);
  return a;
}

// I'm not actually if this is faster then LIBC standard anymore
// given they use inline asm now.. pray the compiler is dope? xD
void *pix_memcpy(void *dst, const void *src, size_t n) {
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
#define TRY_STAMP_32_BYTES                                                               \
  if (p < last_word) {                                                                   \
    *((c32a *)p) = val32;                                                                \
    p += 32;                                                                             \
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

// TEMP INCLUEDE FOR FIXBELOW
#include <string.h>
// Returns the length of a string including the null term.
PIX_DEF i64 pix_strlen(char *str) { return strlen((char *)str) + 1; }

// MEMORY HEAP OVERFLOW HERE FOR SOME REASON.
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
  if (sizeof(longword) > 8)
    abort();

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
      p_info("Directory '%s' already exists", path);
      return false;
    }
    p_alert("Failed to create directory: '%s'. Error: %s", path, strerror(errno));
    return true;
  }
  p_info("Directory: '%s' created", path);
  return false;
}

#endif // PIX_IMPLEMENTATION

/* #endif // PIX_H */
