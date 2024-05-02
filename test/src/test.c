#include <stdio.h>
#include <string.h>

// For package linking testing.
#include "../../mimalloc/include/mimalloc.h"
#include <pthread.h>

#include "../hdr/test.h"

int main(void) {
  printf("Hello World\n");

  fprintf(stdout, "Using Malloc Stuff\n");

  char *test = mi_calloc(1000, sizeof(char));
  char *src = "Hello Extra World";
  memcpy(test, src, 18);
#ifdef DEBUG
  printf("Only print if DEBUG defined!\n");
#endif

  extend_function(10);
  extend_function(2);

  test[100] = '\0';
  fprintf(stdout, "%s\n", test);
  // fprintf(stdout, "Testing something new for incremental!");
  return 0;
}
