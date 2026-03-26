#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "Error.h"
#include "Colors.h"

#define str char*

static char buffer[150]; // Maybe increase size depending on future error text

static str errnames[ERR_IO+1] = {
  "DATA FORMAT ERROR",
  "MEMORY ERROR",
  "IO ERROR"
};

/**
 * Includes the arguments passed to handleError into the format string fmsg
 * @param fmsg The format string
 * @param args The variable arguments
 */
static void formatMessage(const str fmsg, va_list args) {
  vsnprintf(buffer, 150, fmsg, args);
}

void handleError(errType err, sevType sev, const str fmsg, ...) {
  va_list args;
  va_start(args, fmsg);

  formatMessage(fmsg, args);

  if (sev == FATAL) {
    fprintf(stdout, RED "%s: %s" RESET, errnames[err], buffer);
    // TODO: write/log errors to files in future if exiting
    exit(-1);
  }

  fprintf(stdout, YELLOW "%s: %s" RESET, errnames[err], buffer);
}