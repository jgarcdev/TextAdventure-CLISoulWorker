#include "unistd.h"
#include "Error.h"

ssize_t getline(char** restrict lineptr, size_t* restrict n, FILE* restrict stream) {
  register char c;
  register char* cs = NULL;
  register int length = 0;

  c = _getc_nolock(stream);
  // if (c == EOF) return -1;

  *lineptr = (char*) malloc(64);
  if (!*lineptr) handleError(ERR_MEM, FATAL, "Could not allocate space for lineptr!\n");
  *n = 64;

  while (c != EOF) {
    if (length + 1 >= *n) {
      size_t newsize = *n + (*n >> 2);
      if (newsize < 64) newsize = 64;

      cs = (char*) realloc(*lineptr, newsize);
      if (!cs) handleError(ERR_MEM, FATAL, "Could not reallocate space for lineptr!\n");

      *n = newsize;
      *lineptr = cs;
    }

    ((unsigned char*) *lineptr)[length++] = c;

    if (c == '\n') break;

    c = _getc_nolock(stream);
  }

  (*lineptr)[length] = '\0';

  return (ssize_t) length;
}