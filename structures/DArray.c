#include <stdlib.h>

#include "DArray.h"
#include "Error.h"


DArray* initDArray(byte size) {
  DArray* arr = (DArray*) malloc(sizeof(DArray));
  if (!arr) handleError(ERR_MEM, FATAL, "Could not allocate space for the dynamic array structure!\n");

  arr->ids = (char*) malloc(size);
  if (!arr->ids) handleError(ERR_MEM, FATAL, "Could not allocate space for the dyanmic array contents!\n");

  arr->size = size;

  for (int i = 0; i < arr->size; i++) arr->ids[i] = -1;

  return arr;
}

void dArrayAdd(DArray* arr, byte id) {
  if (!arr) return;

  for (int i = 0; i < arr->size; i++) {
    if (arr->ids[i] == -1) {
      arr->ids[i] = id;
      return;
    }
  }
}

bool dArrayExists(DArray* arr, byte id) {
  if (!arr) return false;

  for (int i = 0; i < arr->size; i++) {
    if (arr->ids[i] == id) return true;
  }

  return false;
}

void dArrayFree(DArray* arr) {
  free(arr->ids);
  free(arr);
  arr = NULL;
}