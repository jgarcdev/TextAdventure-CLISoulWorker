#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Maze.h"
#include "Error.h"

#define ROOM_MULT 5 // By how much should room count increase

Table* initTable() {
  Table* table = (Table*) malloc(sizeof(Table));

  if (!table) return NULL;

  table->rooms = (Room**) calloc(ROOM_MULT, sizeof(Room*));
  if (!table->rooms) {
    free(table);
    return NULL;
  }

  table->cap = ROOM_MULT;
  table->len = 0;

  return table;
}

Table* initTableL(int size) {
  Table* table = (Table*) malloc(sizeof(Table));

  if (!table) return NULL;

  table->rooms = (Room**) calloc(size, sizeof(Room*));
  if (!table->rooms) {
    free(table);
    return NULL;
  }

  table->cap = size;
  table->len = 0;

  return table;
}

bool putRoom(Table* table, Room* room, bool overwrite) {
  // Once the length matches the capacity, increase the capacity
  // Note that there might not be a reason to increase the size, if there are only current [cap] rooms in the maze
  //   which it will increase the size no matter what (will add or no longer will add).
  // However, it is only 40 bytes extra, plus some space and time overhead, and it will all get freed anyway
  if (table->len == table->cap) {
    Room** temp = (Room**) realloc(table->rooms, (table->cap+ROOM_MULT)*sizeof(Room*));
    if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate space!\n");
    table->rooms = temp;

    void* startingPoint = (table->rooms) + (table->len);
    memset(startingPoint, 0x0, ROOM_MULT*sizeof(Room*));

    table->cap += ROOM_MULT;
  }

  byte id = room->id;
  Room* _room = table->rooms[(int) id];

  if (!_room) { // Room does not exist, add it
    table->rooms[(int) id] = room;
    table->len++;

    return true;
  } else if (overwrite) {
    // A room with the same id already exists. Do not replace.
    // Note that even though the rooms may have the same id, their contents (loot table, enemy table, etc) may be different.
    // This only happens when initializing the maze and there are two rooms in the json file with the same id (maybe be unlikely)
    // Current behaviour is just to not add it and keep the room

    deleteRoom(room);
    return false;
  }

  return false;
}

void deleteTable(Table* table) {
  if (!table) return;

  free(table->rooms);
  free(table);
  table = NULL;
}

void addAndRecurse(Room* room, Table* table) {
  if (room != (void*) ((long long) NO_EXIT)) {
    bool inTable = putRoom(table, room, false);

    // Prevent stack overflow by not going to rooms that have been inserted to the table
    //  although their exits may not be in table
    //  they'll be check by other paths
    // NOTE: Need to see behaviour for larger maps!!
    if (inTable) {
      for (int i = 0; i < 4; i++) {
        addAndRecurse(room->exits[i], table);
      }
    }
  }
}