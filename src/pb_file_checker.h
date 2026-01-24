#ifndef PB_FILE_CHECKER_H
#define PB_FILE_CHECKER_H 1

#include "pix.h"

i32 is_file_newer(const char *f, const char *f2);
bool does_file_exist(const char *f);
char *make_dep_path(const char *obj_path);
bool deps_require_rebuild(const char *obj_path, const char *dep_path);
char *make_config_stamp_path(const char *build_dir);
bool write_empty_file(const char *path);

#endif
