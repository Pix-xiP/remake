#include <stdio.h>
#include <string.h>

// For package linking testing.
#include "../../mimalloc/include/mimalloc.h"
#include <pthread.h>

int main(void) {
  printf("Hello World\n");

  fprintf(stdout, "Using Malloc Stuff\n");

  char *test = mi_calloc(1000, sizeof(char));
  char *src = "Hello Extra World";
  memcpy(test, src, 18);
#ifdef DEBUG
  printf("Only print if DEBUG defined!\n");
#endif

  test[100] = '\0';
  fprintf(stdout, "%s\n", test);
  return 0;
}
