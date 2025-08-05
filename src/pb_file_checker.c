#include <sys/stat.h>
#include <unistd.h>

#include "pb_main.h"
#include "pix.h"

extern PB_Builder pb;
extern PB_Context pc;

// Pretty sure this is similar to how make does it where they just check last
// time it was updated - means touch <file> can be used for testing \o/
// Function to check if file1 is newer than file2
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
