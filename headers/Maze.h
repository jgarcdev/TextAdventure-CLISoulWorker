#ifndef _MAZE_H
#define _MAZE_H

#include <stdbool.h>

#include "Misc.h"


#define NO_EXIT 0xFEEDFAED

// Better name
typedef union EnemyU {
  Enemy* enemy;
  Boss* boss;
} EnemyU;
// A structure representing a room within a maze. Has connections to other possible rooms.
typedef struct Room {                                       // 74B+6B(PAD) = 80B
  struct Room* exits[4]; // The possible exits that a room can have          32B
  STR info; // The description of the room.                                   8B
  STR storyFile; // The name of the story text file.                          8B
  FILE* file; // The file for the story text file.                            8B
  // At initialization, the pointers will be the room ids in hex (-1 -> 0xFEEDFAED; 0 -> 0x0; 1 -> 0x1; ...; 10 -> 0xa; etc..)
  EnemyU enemy; // The possible enemy that the room can have                  8B
  Item* loot;// The possible loot item that the room can have                 8B
  bool hasBoss; // Whether the room holds a normal enemy or a boss            1B
  ubyte id; // The room id, note: it is not a string, just a number of 1 byte  1B
} Room;

// A structure representing a single maze with an entry.
typedef struct Maze {                     // 17B+7B(PAD) = 24B
  char* name; // The name of the maze/directory for story   8B
  Room* entry; // The entrance of the maze                  8B  
  ubyte size; // The number of rooms that the maze has       1B
} Maze;

// A temporary hashtable to store the rooms for saving, loading, and deleting
typedef struct Table {                              // 16B
  Room** rooms; // The array of room pointers           8B
  uint cap; // The current capacity of the table        4B
  uint len; // Number of items that the table contains  4B
} Table;


/**
 * Displays
 * @param maze 
 */
void showMap(Maze* maze, Room* playerRoom);

/**
 * Deletes the given room, freeing the memory.
 * @param room The room to delete
 */
void deleteRoom(Room* room);

/**
 * Removes an item from the given room. Note, this only frees the item structure, and not _item.
 * All data must be copied before removing and the _item pointer must be in the player's hands.
 * Thus, the contents of _item must be freed by the player.
 * @param room The target room
 */
void removeItemFromMap(Room* room);

/**
 * Deletes an enemy from the given room.
 * @param room The target room
 * @param deleteGear Whether to delete the boss gear
 * @return True if it was deleted, false otherwise
 */
bool deleteEnemyFromMap(Room* room, bool deleteGear);

/**
 * Inititates the temporary table to store the room
 * @return The table
 */
Table* initTable();

/**
 * Initiates the temporary table to store the room with a set size.
 * @param size The size to set the table.
 * @return The table.
 */
Table* initTableL(int size);

/**
 * Inserts the given room into the given table
 * @param table The table
 * @param room The room to store
 * @param overwrite Whether to overwrite the room if one exists
 * @return Whether the room was put
 */
bool putRoom(Table* table, Room* room, bool overwrite);

/**
 * Recursively, adds the room to the table.
 * Note: Depending on size of maze, may or may not rework to avoid stack overflow
 * @param room The room to add
 * @param table The table to add to
 */
void addAndRecurse(Room* room, Table* table);

/**
 * Connects the created rooms to form the maze.
 * @param table The table containing the rooms
 * @return The entry room
 */
Room* connectRooms(Table* table);

/**
 * Deletes the temporary table.
 * @param table The table to delete
 */
void deleteTable(Table* table);

/**
 * Deletes the maze.
 * @param maze The maze to delete
 */
void deleteMaze(Maze* maze);


#endif