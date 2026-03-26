#ifndef _SETUP_H
#define _SETUP_H


#include <stdbool.h>

#include "Maze.h"
#include "cJSON.h"


#define FLUSH() do { int c; while ((c = getchar()) != '\n' && c != EOF); } while (0);


/**
 * Creates the maze.
 * @param filename The map to initiate
 * @return The Maze structure
 */
Maze* initMaze(const STR filename);

/**
 * Creates a SoulWeapon with the given cJSON data.
 * @param obj The data to create the SoulWeapon
 * @return The SoulWeapon
 */
SoulWeapon* createSoulWeapon(cJSON* obj);

/**
 * Creates an armor with the given cJSON data.
 * @param obj The data to create the armor
 * @return The armor
 */
Armor* createArmor(cJSON* obj);

/**
 * Creates an HP Kit with the given cJSON data.
 * @param obj The data to create the HP Kit
 * @return The HP Kit
 */
HPKit* createHPKit(cJSON* obj);

/**
 * Creates a upgrade material with the given cJSON data.
 * @param obj The data to create the upgrade material
 * @return The upgrade material
 */
Upgrade* createUpgrade(cJSON* obj);

/**
 * Creates a slime with the given cJSON data.
 * @param obj The data to create the slime
 * @return The slime
 */
Slime* createSlime(cJSON* obj);

/**
 * Creates an item object given the cJSON object.
 * @param obj The raw item data
 * @param type The type of item
 * @return The item
 */
Item* createItem(cJSON* obj, item_t type);

/**
 * Creates a skill object given the cJSON object.
 * @param obj The raw skill data
 * @return The skill
 */
Skill* createSkill(cJSON* obj);

#endif