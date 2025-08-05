#ifndef PB_FILE_CHECKER_H
#define PB_FILE_CHECKER_H 1

#include "pix.h"

i32 is_file_newer(const char *f, const char *f2);
bool does_file_exist(const char *f);

#endif
