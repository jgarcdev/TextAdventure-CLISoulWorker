#ifndef _LOADJSON_H
#define _LOADJSON_H

#include "cJSON.h"


/**
 * Parses the given json file into cJSON
 * @param filename The json file to parse
 * @return The root cJSON structure
 */
cJSON* readData(const STR filename);

/**
 * Reads in the map json file as raw.
 * @param filename The json file
 * @return The raw json text
 */
char* readJSON(const STR filename);


#endif