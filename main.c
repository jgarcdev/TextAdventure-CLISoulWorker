#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "Error.h"
#include "Keyboard.h"
#include "SaveLoad.h"
#include "Battle.h"


SoulWorker* player;
Maze* maze;

#define NUM_MAZES 2
int mazeIdx; // The index indicating the current maze name from mazes
// The array of the names of all possible mazes
STR mazes[NUM_MAZES] = {
  "control_zone",
  "r_square"
};


/**
 * 
 */
static void printSplashScreen() {
  printf("╔═════════════════════════════════════******═══════════════════════════════════════════════╗\n");
  printf("|                                                                                          |\n");
  printf("|%s                               ██████╗██╗     ██╗                                         %s|\n", BLINK, RESET);
  printf("║%s                              ██╔════╝██║     ██║                                         %s║\n", BLINK, RESET);
  printf("║%s                              ██║     ██║     ██║                                         %s║\n", BLINK, RESET);
  printf("*%s                              ██║     ██║     ██║                                         %s*\n", BLINK, RESET);
  printf("*%s                              ╚██████╗███████╗██║                                         %s*\n", BLINK, RESET);
  printf("║                                                                                          ║\n");
  printf("║%s  ███████╗ ██████╗ ██╗    ██╗██╗     ██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗███████╗██████╗   %s║\n", BLINK, RESET);
  printf("*%s  ██╔════╝██╔═══██╗██║    ██║██║     ██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝██╔════╝██╔══██╗  %s*\n", BLINK, RESET);
  printf("*%s  ███████╗██║   ██║██║    ██║██║     ██║ █╗ ██║██║   ██║██████╔╝█████╔╝ █████╗  ██████╔╝  %s*\n", BLINK, RESET);
  printf("║%s  ╚════██║██║   ██║██║    ██║██║     ██║███╗██║██║   ██║██╔══██╗██╔═██╗ ██╔══╝  ██╔══██╗  %s║\n", BLINK, RESET);
  printf("║%s  ███████║╚██████╔╝╚███████╔╝███████╗╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗███████╗██║  ██║  %s║\n", BLINK, RESET);
  printf("|%s  ╚══════╝ ╚═════╝  ╚══╝╚══╝ ╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝  %s|\n", BLINK, RESET);
  printf("|                                                                                          |\n");
  printf("╚══════════════════════════════════════******══════════════════════════════════════════════╝\n");

  printf("                           The Journey of the SoulWorker....                                \n");
  // Next two lines make sure the player only presses the enter key once
  // Using only getchar() makes it press two (for some reason)
  getchar();
  // Don't know why this works but it works
  ungetc('\n', stdin);

  FLUSH()
#ifdef _WIN64
  system("cls");
#else
  system("clear"); // I DO NOT like this, extra overhead???
#endif
}

/**
 * 
 */
static void printWelcome() {
  printf("✧~~~~~~~~~~~~~~~~~~✧\n");
  printf("Welcome to Cloudream\n");
  printf("✧~~~~~~~~~~~~~~~~~~✧\n");
  // Add delay in print characters
  printf("      . . . .\n");

  printf("Cloudream, divided.\n");
  ssleep(1000);
  printf("North. East. South. %sWest%s...\n", PURPLE, RESET);
  ssleep(1000);
  printf("Which %sRosca%s hath forsaken\n", YELLOW, RESET);
  ssleep(1000);
  printf("To which %sKent%s then envores\n", RED, RESET);
  ssleep(1000);
  printf("\n\n");
}

/**
 * 
 */
static void tutorial() {
  printf("*~%sRosca%s shows you how to navigate the New World~*\n", YELLOW, RESET);
  ssleep(500);

  // Maybe extract to a text file?

  printf("In Game Menu:\n");
  printf("\tTo view your stats: ('e')\n");
  printf("\tTo view the map: ('p')\n");
  printf("\tTo move ('m') to a direction: ('n', 'e', 's', 'w')\n");
  printf("\tTo save the game: ('s')\n");
  printf("\tTo open the inventory menu: ('k')\n");
  printf("\tTo open the skill menu: ('k')\n");
  printf("\tTo unequip all the gear: ('g')\n");
  printf("\tTo save and quit: ('q')\n");
  printf("\tTo view the main help message: ('h')\n");
  ssleep(500);

  printf("In Inventory Menu:\n");
  printf("\tTo view the item: ('i')\n");
  printf("\tTo view your inventory: ('v')\n");
  printf("\tTo use an item (opens item menu): ('u')\n");
  printf("\tTo close the inventory: ('q')\n");
  printf("\tTo view the inventory help message: ('h')\n");
  ssleep(500);

  printf("In Item Menu:\n");
  printf("\tTo sell the item (all items): ('s')\n");
  printf("\tTo equip (only for gear): ('e')\n");
  printf("\tTo upgrade (only for gear): ('u')\n");
  printf("\tTo heal (only for HP kits): ('h')\n");
  ssleep(500);

  printf("In Skill Menu:\n");
  printf("\tTo view skill info: ('i')\n");
  printf("\tTo view all skills and equipped skills: ('k')\n");
  printf("\tTo unlock a skill: ('n')\n");
  printf("\tTo set a skill to equipped skills: ('s')\n");
  printf("\tTo upgrade a skill: ('u')\n");
  printf("\tTo close the menu: ('q')\n");
  printf("\tTo view the skill menu help message: ('h')\n");
  ssleep(500);


  // Add more info????
  printf("%sRosca%s wishes you the best...\n\n", YELLOW, RESET);
  ssleep(500);
}

/**
 * Detects whether there is a saved game so the player can resume.
 * @return True if a saved game exists, false otherwise
 */
static bool detectSave() {
  FILE* mapSave = fopen("./data/saves/map_save.json", "r");
  FILE* playerSave = fopen("./data/saves/player_save.json", "r");

  if (mapSave != NULL && playerSave != NULL) {
    fclose(mapSave);
    fclose(playerSave);

    return true;
  }

  return false;
}

/**
 * Prints the story. If room is true, it means print the story for the room (either entry or boss)
 * @param room 
 */
static void story(bool room) {
  STR base = "./data/story";
  STR filename = NULL;

  // If the story to be printed is for a room
  if (room) {
    size_t mazeNameLen = strlen(maze->name);
    size_t roomStoryLen = strlen(player->room->storyFile);

    filename = (STR) malloc(14 + mazeNameLen + roomStoryLen + 1);
    if (!filename) handleError(ERR_MEM, FATAL, "Could not allocate space for filename!\n");

    sprintf(filename, "%s/%s/%s", base, maze->name, player->room->storyFile);
  } else { // The story is the introduction story
    filename = (STR) malloc(25);
    if (!filename) handleError(ERR_MEM, FATAL, "Could not allocate space for filename!\n");

    sprintf(filename, "%s/intro.story", base);
  }

  FILE* story = fopen(filename, "r");
  if (!story) handleError(ERR_IO, FATAL, "Could not open story!\n");
  free(filename);

  if (room) player->room->file = story;

  STR line = NULL;
  size_t n = 0;

  printf("\n");
  while (!feof(story)) {
    getline(&line, &n, story);
    // if (feof(intro)) break;

    if (strncmp(line, "FIGHT\n", 6) == 0) break;

    ssleep(1000);
    printf("%s", line);
  }

  if (line != NULL) free(line);

  printf("\n\n");
  ssleep(500);

  if (!room) { fclose(story); player->room->file = NULL; 
  } else { free(player->room->storyFile); player->room->storyFile = NULL; }
}

/**
 * Gets the next maze to load up, or null if no more
 * @return Next maze
 */
static STR getNextMaze() {
  STR nextMaze = NULL;

  // Out of bounds, as in no more mazes
  if (mazeIdx >= NUM_MAZES) return nextMaze;

  STR maze = mazes[mazeIdx];

  nextMaze = (STR) malloc(18 + strlen(maze));
  if (!nextMaze) handleError(ERR_MEM, FATAL, "Could not allocate space for maze name when loading new!\n");

  sprintf(nextMaze, "./data/maps/%s.json", maze);

  return nextMaze;
}

/**
 * Handles what happens when the game is finished.
 */
void endOfGame() {
  // Print some conclusion story??

  // Note that Kent will not drop gear (no need to)
  // But b/c of how game is structured, every boss needs to have it
  // So on game time, do not give out armor
  // THis will be done after boss fight
  // Maybe...just maybe the boss fight with Kent will be different, who knows

  printf("You have beaten Kent! Now you can go back to where you came from!\n");
  printf("Bye bye!!!\n");

  exit(1);
}

/**
 * Main game loop
 */
void loop() {
  Room* currRoom = player->room;
  Commands choice;

  // Code only runs from here only if it's the start of a new maze 
  // Or when the save file is loaded 
  START:
  
  printf("You find yourself in %s...\n", currRoom->info);

  if (currRoom->storyFile != NULL) {
    // story(true);
    // fclose(currRoom->file);
    // currRoom->file = NULL;
  }

  while (true) {
    if (!currRoom->hasBoss && currRoom->enemy.enemy != NULL) {
      battleEnemy(currRoom->enemy.enemy);
      // Update currRoom in case player respawned at entrance
      // Prevent from getting the loot, if one exists
      currRoom = player->room;
    }

    if (currRoom->hasBoss && currRoom->enemy.boss != NULL) {
      story(true);

      bool win = bossBattle(currRoom->enemy.boss);

      // If the battle was a win, then print story and move on to next maze
      // Otherwise, player is respawned to current maze entry
      if (win) {
        // File for boss story is already opened
        // Can just do the printing
        // Assuming the pointer to text is past FIGHT

        STR line = NULL;
        size_t n = 0;

        printf("\n");
        while (!feof(currRoom->file)) {
          getline(&line, &n, currRoom->file);
          ssleep(1000);
          printf("%s", line);
          line = NULL;
        }
        ssleep(1000);
        printf("\n\n");

        
        // Transport to next maze
        // printf("GOING TO NEXT MAZE\n");

        deleteMaze(maze);

        mazeIdx++;
        STR mazeFile = getNextMaze();

        if (!mazeFile) endOfGame();

        maze = initMaze(mazeFile);

        player->room = maze->entry;
        currRoom = player->room;

        goto START;
      }
    }

    if (currRoom->loot != NULL) {
      STR name = getItemName(currRoom->loot);
      printf("You found %d * %s!\n", currRoom->loot->count, name);
      bool added = addToInv(player, currRoom->loot);

      // Since some strings have been alloc'd, free them
      switch (currRoom->loot->type) {
        case HP_KITS_T:
        case WEAPON_UPGRADE_MATERIALS_T:
        case ARMOR_UPGRADE_MATERIALS_T:
          free(name);
          name = NULL;
        default:
          break;
      }

      if (added) {
        printf("ADDED TO INV! REMOVING %p!\n", currRoom->loot);
        removeItemFromMap(currRoom);
      }
    }

    printf("What are you going to do?... ");
    choice = getchar();
    choice = tolower(choice);
    FLUSH()

    while(!performAction(choice)) {
      printf("That is not an action. Try again! For a list of acceptable actions, type 'h'!\n");
      choice = getchar();
      choice = tolower(choice);
      FLUSH()
    };

    currRoom = player->room;
  }
}


int main(int argc, char const *argv[]) {
  if (argc != 2) exit(1); // running without launcher, exit silently

  // if ran in cmd, check it was done by the launcher
  const char* arg = argv[1];
  if (strncmp(arg, "-l", 2) != 0) exit(1);

  // Funky utf8 windows stuff
#ifdef _WIN64
  SetConsoleOutputCP(CP_UTF8);
#endif

  // printSplashScreen();
  // printWelcome();

  bool saveExists = detectSave();

  if (saveExists) {
    printf("~~~~The %sAkashic Records%s indicate change~~~~\n", YELLOW, RESET);
    printf("Do you wish to follow the Records? (yes|no) ");

    // char buff[5];
    // fgets(buff, sizeof(buff), stdin);
    STR buff = (STR) malloc(5); // max is 3 for "yes", plus newline, plus null
    fgets(buff, 5, stdin);

    STR in = buff; // better name
    for(;*buff != '\0';buff++) *buff = tolower(*buff);

    if (strncmp(in, "yes", 3) == 0) {
      free(in);

      printf("Reading Akashic Records...\n");
      loadGame();

      printf("%sRosca%s welcomes you back, %s...\n", YELLOW, RESET, player->name);

      // When loading from a save, the mazeIdx for the progression is lost
      // Reverse search to get the mazeIdx and reset so proper progression can happen
      for (int i = 0; i < NUM_MAZES; i++) {
        if (strcmp(mazes[i], maze->name) == 0) mazeIdx = i;
      }

      loop();
    } else if (strncmp(in, "no", 2) == 0) {
      printf("The Akashic Records shall start anew...\n");
    } else {
      printf("No such response exists for the Records. Starting anew...\n\n");
    }

    free(in);
  }

  mazeIdx = 0;
  maze = initMaze(getNextMaze());

  printf("What shall the Records name you, birthing %sSoul%s? ", CYAN, RESET);

  STR name = NULL;
  size_t n = 0;
  ssize_t nameLen = getline(&name, &n, stdin);
  *(name + nameLen - 1) = '\0';

  player = initSoulWorker(name);
  player->room = maze->entry;

  // printf("");
  // printf("Is\n\t%s\t\tyour name?...\n", name);
  // Do a yes or no, repeat type of thing
  // printf("")

  printf("%sRosca%s cordially welcomes you, %sSoulWorker %s%s, to Cloudream...\n\n", YELLOW, RESET, CYAN, player->name, RESET);
  ssleep(1000);

  // tutorial();
  // story(false);
  loop();

  return 0;
}