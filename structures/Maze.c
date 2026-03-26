#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "Maze.h"
#include "Error.h"


void removeItemFromMap(Room* room) {
  if (!room && !room->loot) {
    free(room->loot);
    room->loot = NULL;

    return;
  }

  if (!room && room->loot) {
    fprintf(stderr, "Could not remove item %p from map!\n", room->loot); // Add to error handling???
  }

  return;
}

bool deleteEnemyFromMap(Room* room, bool deleteGear) {
  if (!room || !room->enemy.enemy || !room->enemy.boss) return false;

  bool del;

  if (room->hasBoss) {
    del = deleteBoss(room->enemy.boss, deleteGear);
    room->enemy.boss = NULL;
    room->hasBoss = false;

    return del;
  }
  
  del = deleteEnemy(room->enemy.enemy);
  room->enemy.enemy = NULL;

  return del;
}

/**
 * Prints the cleaned-up version of the maze name
 * @param name The maze name to print
 */
static void printMazeName(str name) {
  int nameLen = strlen(name);

  str _name = (str) malloc(nameLen + 1);
  if (!_name) {
    handleError(ERR_MEM, WARNING, "Could not allocate space for map name display! Defaulting to maze name.\n"); 
    _name = name;
  } else {
    strcpy(_name, name);

    str temp = _name;

    // First non-null character will always get capitalized
    if (*temp != '\0') *temp = toupper(*temp);
    temp++;

    while (*temp != '\0') {
      // Change the current character of _ to space
      if (*temp == '_') *temp = ' ';
      // Uppercase characters that followed _ (now space)
      // Guaranteed that *(temp - 1) will not be out of bounds by the increase prior to looping
      if (*(temp - 1) == ' ') *temp = toupper(*temp);

      temp++;
    }
  }
    
  printf("%s%s%s: ", PURPLE, _name, RESET);

  // In the case that _name is not malloc'd, _name is assigned to name, thus they point to the same thing
  if (_name != name) free(_name);
}

static void placeRoomOnGrid(Room* room, char** grid, int x, int y, bool** visited, uint gridSize, Room* playerRoom) {
  if (room == ((void*) ((long long) NO_EXIT)) || visited[y][x]) return;

  visited[y][x] = true;

  if (room->hasBoss) grid[y][x] = '!';
  else if (room->enemy.enemy) grid[y][x] = '%';
  else if (room->loot) grid[y][x] = '$';
  else if (room->id == 0) grid[y][x] = '@';
  else grid[y][x] = '#';

  if (room == playerRoom) grid[y][x] = 'o';


  if (room->exits[0] != ((void*) ((long long) NO_EXIT)) && y > 1) {
    grid[y-1][x] = '|';
    placeRoomOnGrid(room->exits[0], grid, x, y-2, visited, gridSize, playerRoom);
  }
  if (room->exits[1] != ((void*) ((long long) NO_EXIT)) && x < gridSize-2) {
    grid[y][x+1] = '-';
    placeRoomOnGrid(room->exits[1], grid, x+2, y, visited, gridSize, playerRoom);
  }
  if (room->exits[2] != ((void*) ((long long) NO_EXIT)) && y < gridSize-2) {
    grid[y+1][x] = '|';
    placeRoomOnGrid(room->exits[2], grid, x, y+2, visited, gridSize, playerRoom);
  }
  if (room->exits[3] != ((void*) ((long long) NO_EXIT)) && x > 1) {
    grid[y][x-1] = '-';
    placeRoomOnGrid(room->exits[3], grid, x-2, y, visited, gridSize, playerRoom);
  }
}

void showMap(Maze* maze, Room* playerRoom) {
  // TODO: Fix map size printing
  uint gridSize = (int) (sqrt(maze->size) * 6);

  if (gridSize < 5) gridSize = 5;

  char** grid = malloc(gridSize * sizeof(char*));
  if (!grid) handleError(ERR_MEM, FATAL, "Could not allocate memory for map grid!\n");

  bool** visited = malloc(gridSize * sizeof(bool*));
  if (!visited) handleError(ERR_MEM, FATAL, "Could not allocate memory for visisted array!\n");


  for (uint i = 0; i < gridSize; i++) {
    grid[i] = malloc(gridSize * sizeof(char));
    if (!grid[i]) handleError(ERR_MEM, FATAL, "Could not allocatr memory for map grid i!\n");

    visited[i] = malloc(gridSize * sizeof(bool));
    if (!visited[i]) handleError(ERR_MEM, FATAL, "Could not allocate memory for visited i!\n");

    memset(grid[i], ' ', gridSize * sizeof(char));
    memset(visited[i], false, gridSize * sizeof(bool));
  }
  
  placeRoomOnGrid(maze->entry, grid, gridSize/2, gridSize/2, visited, gridSize, playerRoom);
  
  printMazeName(maze->name);

  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      char room = grid[i][j];

      if (room == 'o') { // player
        printf("%s%c%s", GREEN, room, RESET);
      } else if (room == '$') { // loot
        printf("%s%c%s", YELLOW, room, RESET);
      } else if (room == '@') { // entry
        printf("%s%c%s", CYAN, room, RESET);
      } else if (room == '%') { // enemy
        printf("%s%c%s", PURPLE, room, RESET);
      } else if (room == '!') { // boss
        printf("%s%c%s", RED, room, RESET);
      } else { // connections and empty rooms
        printf("%s%c", RESET, room);
      }
      // else putchar(room);
    }
    putchar('\n');
  }
}

void deleteRoom(Room* room) {
  if (!room->loot) deleteItem(room->loot);
  deleteEnemyFromMap(room, true);
  free(room->info);
  if (room->storyFile) free(room->storyFile);
  if (room->file) fclose(room->file);
  free(room);

  room = NULL;
}

void deleteMaze(Maze* maze) {
  Table* table = initTableL(maze->size);
  if (!table) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  for (int i = 0; i < table->len; i++) {
    deleteRoom(table->rooms[i]);
  }

  deleteTable(table);

  free(maze->name);
  free(maze);
}