#ifndef _ERROR_H
#define _ERROR_H


#define STR char*

// The type of error encountered
typedef enum {
  ERR_DATA, // Data format error (improper map/save format)
  ERR_MEM, // Unable to allocate memory
  ERR_IO // Cannot open/write file
} errType;

typedef enum {
  FATAL, // Fatal, terminate program
  WARNING // Do not terminate program
} sevType;


/**
 * Handles the error. If the error is fatal, terminates the program.
 * @param err The type of error
 * @param sev The severity of the error
 * @param fmsg The message to print
 */
void handleError(errType err, sevType sev, const STR fmsg, ...);


#endif