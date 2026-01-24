#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pb_main.h"
#include "pix.h"

i32 is_file_newer(const char *f, const char *f2) {
  struct stat f_stat, f2_stat;
  // Didn't exist? Probably new then
  if (stat(f, &f_stat) != 0) {
    return 1;
  }

  // Didn't exist? Probably newer then.
  if (stat(f2, &f2_stat) != 0)
    return 1;

  return f_stat.st_mtime > f2_stat.st_mtime;
}

bool does_file_exist(const char *f) {
  if (access(f, F_OK))
    return false;
  return true;
}

static void normalize_dep_file(char *buf, size_t len) {
  for (size_t i = 0; i + 1 < len; i++) {
    if (buf[i] == '\\' && buf[i + 1] == '\n') {
      buf[i] = ' ';
      buf[i + 1] = ' ';
      i++;
      continue;
    }
    if (buf[i] == '\r')
      buf[i] = ' ';
  }
}

char *make_dep_path(const char *obj_path) {
  size_t path_len = (size_t)pix_strlen(obj_path) - 1;
  const char *last_dot = strrchr(obj_path, '.');
  const char *last_slash = strrchr(obj_path, '/');
  bool replace_ext = false;

  // TODO: Refactor this and the one in main.c to use a function that does the same thing, plus the
  // replace_ext check.
  if (last_dot && (!last_slash || last_dot > last_slash)) {
    const char *ext = last_dot + 1;
    size_t ext_len = path_len - (size_t)(ext - obj_path);
    replace_ext = (ext_len == 1 && ext[0] == 'o');
  }

  if (replace_ext) {
    size_t base_len = (size_t)(last_dot - obj_path);
    size_t new_len = base_len + 2; // ".d"

    char *dep = pix_calloc((i64)(new_len + 1));
    pix_memcpy(dep, obj_path, base_len);

    dep[base_len] = '.';
    dep[base_len + 1] = 'd';
    dep[new_len] = '\0';

    return dep;
  }

  size_t new_len = path_len + 2; // ".d"

  char *dep = pix_calloc((i64)(new_len + 1));
  pix_memcpy(dep, obj_path, path_len);

  dep[path_len] = '.';
  dep[path_len + 1] = 'd';
  dep[new_len] = '\0';

  return dep;
}

bool deps_require_rebuild(const char *obj_path, const char *dep_path) {
  if (!does_file_exist(dep_path))
    return true;

  FILE *fp = fopen(dep_path, "rb");
  if (!fp)
    return true;

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return true;
  }

  long size = ftell(fp);
  if (size < 0) {
    fclose(fp);
    return true;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    return true;
  }

  char *buf = pix_calloc((i64)size + 1);
  size_t read_count = fread(buf, 1, (size_t)size, fp);
  fclose(fp);

  if (read_count != (size_t)size) {
    PIX_FREE(buf);
    return true;
  }

  buf[size] = '\0';
  normalize_dep_file(buf, (size_t)size);

  char *p = buf;
  bool first_token = true;
  while (*p) {
    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p)
      break;

    char *tok = p;
    while (*p && !isspace((unsigned char)*p))
      p++;
    if (*p)
      *p++ = '\0';

    if (first_token) {
      first_token = false;
      continue;
    }

    if (tok[0] == '\0')
      continue;

    if (is_file_newer(tok, obj_path)) {
      PIX_FREE(buf);

      return true;
    }
  }

  PIX_FREE(buf);

  return false;
}
