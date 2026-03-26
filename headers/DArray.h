#include <stdbool.h>

#define ubyte unsigned char


typedef struct DArray {
  char* ids; // The array containing room ids
  ubyte size; // The size of the array
} DArray;


/**
 * Initiates the dynamic array with the given size.
 * @param size The size for the array.
 * @return The dynamic array
 */
DArray* initDArray(ubyte size);

/**
 * Adds an id to the given array.
 * @param arr The array to add to
 * @param id The id to add
 */
void dArrayAdd(DArray* arr, ubyte id);

/**
 * Checks whether the given id exists in the array.
 * @param arr The array to check
 * @param id The target id
 * @return True if if exists, false otherwise
 */
bool dArrayExists(DArray* arr, ubyte id);

/**
 * Frees the array and nulls the pointer.
 * @param arr The array to free
 */
void dArrayFree(DArray* arr);