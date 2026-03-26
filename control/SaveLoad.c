#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SaveLoad.h"
#include "LoadJSON.h"
#include "Error.h"
#include "Setup.h"
#include "DArray.h"


#define NO_ITEM 0x0
#define NO_SKILL 0x0

#define NAME "name"
#define HP "hp"
#define MAX_HP "maxHP"
#define INV_COUNT "invCount"
#define XP "xp"
#define LVL "lvl"
#define DZ "dzenai"
#define ROOM "room"
#define INV "inventory"
#define ID "id"
#define MAP "map"
#define ITEM "item"
#define TYPE "type"
#define COUNT "count"
#define GEAR "gear"

#define IS_ENTRY "isEntry"
#define INFO "info"
#define EXITS "exits"
#define LOOT "loot"
#define ENEMY "enemy"
#define HAS_BOSS "hasBoss"

const str SAVE_DIR = "./data/saves";

#ifdef __linux__
// itoa is in assembly (just for fun), change it to C later
/**
 * Converts integer n into a character string.
 * @param n The integer
 * @param buffer The buffer to hold the string
 * @return The integer as a string
 */
extern str itoa(int n, char* buffer, int radix);
#endif

/**
 * Handles errors regarding cJSON objects.
 * @param obj The cJSON object
 * @param type The JSON object tag
 * @return NULL
 */
static str createError(cJSON* obj, const str type) {
  cJSON_Delete(obj);

  handleError(ERR_DATA, WARNING, "Could not create JSON for %s!\n", type);

  return NULL;
}

/**
 * Finds a room with the given ID.
 * @param room The room to look at
 * @param id The target ID
 * @return Room with matching ID
 */
static Room* findRoom(Room* room, byte id, DArray* visited) {
  if (room != (void*) ((long long) NO_EXIT)) {
    if (room->id == id) return room;

    if (dArrayExists(visited, room->id)) return NULL;

    dArrayAdd(visited, room->id);

    for (int i = 0; i < 4; i++) {
      Room* target = findRoom(room->exits[i], id, visited);
      if (target != NULL) return target;
    }
  }

  return NULL;
}

/**
 * 
 * @param parentObj 
 * @param data 
 * @return 
 */
static bool saveSoulWeapon(cJSON* parentObj, SoulWeapon* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (!name) { createError(parentObj, "SoulWeapon name"); return false; }

  cJSON* atk = cJSON_AddNumberToObject(parentObj, "atk", data->atk);
  if (!atk) { createError(parentObj, "SoulWeapon atk"); return false; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (!acc) { createError(parentObj, "SoulWeapon acc"); return false; }

  cJSON* atkCrit = cJSON_AddNumberToObject(parentObj, "atk_crit", data->atk_crit);
  if (!atkCrit) { createError(parentObj, "SoulWeapon atk crit"); return false; }

  cJSON* critDmg = cJSON_AddNumberToObject(parentObj, "atk_crit_dmg", data->atk_crit_dmg);
  if (!critDmg) { createError(parentObj, "SoulWeapon atk crit dmg"); return false; }

  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (!lvl) { createError(parentObj, "SoulWeapon lvl"); return false; }

  cJSON* upgrades = cJSON_AddNumberToObject(parentObj, "upgrades", data->upgrades);
  if (!upgrades) { createError(parentObj, "SoulWeapon upgrades"); return false; }

  cJSON* durability = cJSON_AddNumberToObject(parentObj, "durability", data->durability);
  if (!durability) { createError(parentObj, "SoulWeapon durability"); return false; }

  return true;
}

/**
 * 
 * @param parentObj 
 * @param data 
 * @return 
 */
static bool saveArmor(cJSON* parentObj, Armor* data) {
  cJSON* name = cJSON_AddStringToObject(parentObj, NAME, data->name);
  if (!name) { createError(parentObj, "armor name"); return false; }

  cJSON* type = cJSON_AddNumberToObject(parentObj, TYPE, data->type);
  if (!type) { createError(parentObj, "armor type"); return false; }

  cJSON* acc = cJSON_AddNumberToObject(parentObj, "acc", data->acc);
  if (!acc) { createError(parentObj, "armor acc"); return false; }

  cJSON* def = cJSON_AddNumberToObject(parentObj, "def", data->def);
  if (!def) { createError(parentObj, "armor def"); return false; }
  
  cJSON* lvl = cJSON_AddNumberToObject(parentObj, LVL, data->lvl);
  if (!lvl) { createError(parentObj, "armor lvl"); return false; }

  return true;
}

/**
 * 
 * @param parentObj 
 * @param data 
 * @return 
 */
static bool saveGear(cJSON* parentObj, Gear* data) {
  cJSON* _null = (void*) ((long long) 0xFEEDBEAD); // For when a gear piece does not exist to be saved

  cJSON* gear = cJSON_AddObjectToObject(parentObj, "gear");
  if (!gear) { createError(parentObj, GEAR); return false; }

  if (!data->sw) _null = cJSON_AddNullToObject(gear, "soulweapon");
  else {
    cJSON* sw = cJSON_AddObjectToObject(gear, "soulweapon");
    if (!sw) { createError(parentObj, "soulweapon"); return false; }

    bool _sw = saveSoulWeapon(sw, data->sw);
    if (!_sw) { createError(parentObj, "soulweapon"); return false; }
  }

  if (!data->helmet) _null = cJSON_AddNullToObject(gear, "helmet");
  else {
    cJSON* helmet = cJSON_AddObjectToObject(gear, "helmet");
    if (!helmet) { createError(parentObj, "helmet"); return false; }

    bool _helmet = saveArmor(helmet, data->helmet);
    if (!_helmet) { createError(parentObj, "helmet"); return false; }
  }

  if (!data->guard) _null = cJSON_AddNullToObject(gear, "shoulder_guard");
  else {
    cJSON* guard = cJSON_AddObjectToObject(gear, "shoulder_guard");
    if (!guard) { createError(parentObj, "shoulder_guard"); return false; }

    bool _guard = saveArmor(guard, data->guard);
    if (!_guard) { createError(parentObj, "shoulder_guard"); return false; }
  }

  if (!data->chestplate) _null = cJSON_AddNullToObject(gear, "chestplate");
  else {
    cJSON* chestplate = cJSON_AddObjectToObject(gear, "chestplate");
    if (!chestplate) { createError(parentObj, "chestplate"); return false; }

    bool _chestplate = saveArmor(chestplate, data->chestplate);
    if (!_chestplate) { createError(parentObj, "chestplate"); return false; }
  }

  if (!data->boots) _null = cJSON_AddNullToObject(gear, "boots");
  else {
    cJSON* boots = cJSON_AddObjectToObject(gear, "boots");
    if (!boots) { createError(parentObj, "boots"); return false; }

    bool _boots = saveArmor(boots, data->boots);
    if (!_boots) { createError(parentObj, "boots"); return false; }
  }

  // It would be better to check if AddNullToObject is checked for nullity
  //  right after it is created in order to avoid processing for the other data
  //  but I am not going to add more if statements because I'm too lazy and will not like how it will look
  if (!_null) { createError(parentObj, "null"); return false; }

  return true;
}

/**
 * 
 * @param parentObj 
 * @param lootItem 
 * @param lootType 
 * @return 
 */
static cJSON* saveLootItem(cJSON* parentObj, void* lootItem, item_t lootType) {
  cJSON* _lootItem = cJSON_AddObjectToObject(parentObj, ITEM);
  if (!_lootItem) { createError(parentObj, "loot item"); return NULL; }

  switch (lootType) {
    case SOULWEAPON_T:
      saveSoulWeapon(_lootItem, (SoulWeapon*) lootItem);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      saveArmor(_lootItem, (Armor*) lootItem);
      break;
    case HP_KITS_T:
      HPKit* hpKit = (HPKit*) lootItem;

      cJSON* hpKitDesc = cJSON_AddStringToObject(_lootItem, "description", hpKit->desc);
      if (!hpKitDesc) { createError(parentObj, "item HP Kit description"); return NULL; }

      cJSON* hpKitT = cJSON_AddNumberToObject(_lootItem, TYPE, hpKit->type);
      if (!hpKitT) { createError(parentObj, "item HP Kit type"); return NULL; }
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      Upgrade* upgrade = (Upgrade*) lootItem;

      cJSON* upgradeDesc = cJSON_AddStringToObject(_lootItem, "description", upgrade->desc);
      if (!upgradeDesc) { createError(parentObj, "item upgrade material description"); return NULL; }

      cJSON* upgradeT = cJSON_AddNumberToObject(_lootItem, TYPE, upgrade->type);
      if (!upgradeT) { createError(parentObj, "item upgrade material type"); return NULL; }

      cJSON* upgradeR = cJSON_AddNumberToObject(_lootItem, "rank", upgrade->rank);
      if (!upgradeR) { createError(parentObj, "item upgrade material rank"); return NULL; }
      break;
    case SLIME_T:
      Slime* slime = (Slime*) lootItem;

      cJSON* slimeDesc = cJSON_AddStringToObject(_lootItem, "description", slime->desc);
      if (!slimeDesc) { createError(parentObj, "item slime description"); return NULL; }
      break;
    default:
      break;
  }

  return _lootItem;
}

/**
 * 
 * @param loot 
 * @return 
 */
static cJSON* saveLoot(Item* loot) {
  cJSON* _loot = cJSON_CreateObject();
  if (!_loot) { createError(_loot, "loot"); return NULL; }

  cJSON* item = saveLootItem(_loot, loot->_item, loot->type);
  if (!item) { createError(_loot, "loot item"); return NULL; }

  cJSON* type = cJSON_AddNumberToObject(_loot, TYPE, loot->type);
  if (!type) { createError(_loot, "loot type"); return NULL; }

  cJSON* count = cJSON_AddNumberToObject(_loot, "count", loot->count);
  if (!count) { createError(_loot, "loot count"); return NULL; }

  return _loot;
}

/**
 * 
 * @param parentObj 
 * @param data 
 * @return 
 */
static bool saveStats(cJSON* parentObj, Stats* data) {
  cJSON* stats = cJSON_CreateObject();
  if (!stats) { createError(parentObj, "stats"); return false; }

  cJSON* atkArr = cJSON_AddArrayToObject(stats, "ATK");
  if (!atkArr) { createError(parentObj, "atk container"); return false; }
  cJSON* atk = cJSON_CreateNumber(data->ATK);
  if (!atk) { createError(parentObj, "atk value"); return false; }
  if (!cJSON_AddItemToArray(atkArr, atk)) { createError(parentObj, "atk"); return false; }

  cJSON* defArr = cJSON_AddArrayToObject(stats, "DEF");
  if (!defArr) { createError(parentObj, "def container"); return false; }
  cJSON* def = cJSON_CreateNumber(data->DEF);
  if (!def) { createError(parentObj, "def value"); return false; }
  if (!cJSON_AddItemToArray(defArr, def)) { createError(parentObj, "def"); return false; }

  cJSON* accArr = cJSON_AddArrayToObject(stats, "ACC");
  if (!accArr) { createError(parentObj, "acc container"); return false; }
  cJSON* acc = cJSON_CreateNumber(data->ACC);
  if (!acc) { createError(parentObj, "acc value"); return false; }
  if (!cJSON_AddItemToArray(accArr, acc)) { createError(parentObj, "acc"); return false; }

  cJSON* critDmgArr = cJSON_AddArrayToObject(stats, "ATK_CRIT_DMG");
  if (!critDmgArr) { createError(parentObj, "atk crit dmg container"); return false; }
  cJSON* critDmg = cJSON_CreateNumber(data->ATK_CRIT_DMG);
  if (!critDmg) { createError(parentObj, "atk crit dmg value"); return false; }
  if (!cJSON_AddItemToArray(critDmgArr, critDmg)) { createError(parentObj, "atk crit dmg"); return false; }

  cJSON* atkCritArr = cJSON_AddArrayToObject(stats, "ATK_CRIT");
  if (!atkCritArr) { createError(parentObj, "atk crit container"); return false; }
  cJSON* atkCrit = cJSON_CreateNumber(data->ATK_CRIT);
  if (!atkCrit) { createError(parentObj, "atk crit value"); return false; }
  if (!cJSON_AddItemToArray(atkCritArr, atkCrit)) { createError(parentObj, "atk crit"); return false; }

  return cJSON_AddItemToObject(parentObj, "stats", stats);
}

/**
 * 
 * @param skill 
 * @return 
 */
static cJSON* saveSkill(Skill* skill) {
  cJSON* _skill = cJSON_CreateObject();

  cJSON* name = cJSON_AddStringToObject(_skill, "name", skill->name);
  if (!name) { createError(_skill, "skill name"); return NULL; }

  cJSON* desc = cJSON_AddStringToObject(_skill, "description", skill->description);
  if (!desc) { createError(_skill, "skill description"); return NULL; }

  cJSON* lvl = cJSON_AddNumberToObject(_skill, "lvl", skill->lvl);
  if (!lvl) { createError(_skill, "skill lvl"); return NULL; }

  cJSON* cooldown = cJSON_AddNumberToObject(_skill, "cooldown", skill->cooldown);
  if (!cooldown) { createError(_skill, "skill cooldown"); return NULL; }

  cJSON* id = cJSON_AddNumberToObject(_skill, "id", skill->id);
  if (!id) { createError(_skill, "skill id"); return NULL; }

  // Since atk and atk_dmg are both ushort, they occupy the same region
  // So it doesn't matter which one is which
  cJSON* effect1 = cJSON_AddNumberToObject(_skill, "effect1", skill->effect1.atk);
  if (!effect1) { createError(_skill, "skill effect 1"); return NULL; }

  cJSON* effect2 = NULL;
  if (skill->activeEffect2 == DEF || skill->activeEffect2 == ACC) {
    // Same logic applies from effect1
    effect2 = cJSON_AddNumberToObject(_skill, "effect2", skill->effect2.def);
  } else if (skill->activeEffect2 == ATK_CRIT) {
    effect2 = cJSON_AddNumberToObject(_skill, "effect2", skill->effect2.atk_crit);
  } else {
    handleError(ERR_DATA, WARNING, "skill active effect 2 is not a possible option! Option is %d\n", skill->activeEffect2);
  }
  if (!effect2) { createError(_skill, "skill effect 2"); return NULL; }


  cJSON* activeEffect1 = cJSON_AddNumberToObject(_skill, "activeEffect1", skill->activeEffect1);
  if (!activeEffect1) { createError(_skill, "skill active effect 1"); return NULL; }

  cJSON* activeEffect2 = cJSON_AddNumberToObject(_skill, "activeEffect2", skill->activeEffect2);
  if (!activeEffect2) { createError(_skill, "skill active effect 2"); return NULL; }

  return _skill;
}

/**
 * 
 * @param parentObj 
 * @param data 
 * @return 
 */
static bool saveSkillTree(cJSON* parentObj, SkillTree* data) {
  cJSON* skillTree = cJSON_CreateObject();
  if (!skillTree) { createError(parentObj, "skill tree"); return false; }

  cJSON* skillStatus = cJSON_AddNumberToObject(skillTree, "skillStatus", data->skillStatus);
  if (!skillStatus) { createError(parentObj, "skill status"); return false; }

  cJSON* totalSkillPoints = cJSON_AddNumberToObject(skillTree, "totalSkillPoints", data->totalSkillPoints);
  if (!totalSkillPoints) { createError(parentObj, "total skill points"); return false; }

  cJSON* skills = cJSON_AddArrayToObject(skillTree, "skills");
  if (!skills) { createError(parentObj, "skills"); return false; }
  for (int i = 0; i < TOTAL_SKILLS; i++) {
    cJSON* skill = saveSkill(&data->skills[i]);
    if (!skill) return false;

    if (!cJSON_AddItemToArray(skills, skill)) return createError(parentObj, "skill in skills");
  }

  cJSON* equippedSkills = cJSON_AddArrayToObject(skillTree, "equippedSkills");
  if (!equippedSkills) { createError(parentObj, "equipped skills"); return false; }
  for (int i = 0; i < EQUIPPED_SKILL_COUNT; i++) {
    char skillId;

    if (data->equippedSkills[i] == NO_SKILL) skillId = -1;
    else skillId = data->equippedSkills[i]->id;

    if (!cJSON_AddNumberToObject(equippedSkills, "id", skillId)) {
      createError(parentObj, "skill number in equipped skills");
      return false;
    }
  }

  return cJSON_AddItemToObject(parentObj, "skills", skillTree);
}

/**
 * 
 * @param _enemy 
 * @param hasBoss 
 * @return 
 */
static cJSON* saveEnemy(EnemyU* _enemy, bool hasBoss) {
  /**
   * NOTE
   * enemy union contains an address, either to Enemy or Boss,
   * but an address nonetheless. Thus no matter which type,
   * it'll still lead to the object.
   * Considering Boss has the Enemy structure at the beginning,
   * its data will be in the same relative order as a regular Enemy
   * Thus, doing _enemy->enemy->[] will do the same pointer arithmetic
   * as _enemy->boss->base->[].
   * However, since Boss has 240B of data more than Enemy,
   * that will need to be saved using the lens as Boss.
   */


  cJSON* enemy = cJSON_CreateObject();
  if (!enemy) { createError(enemy, "enemy"); return NULL; }

  cJSON* name = cJSON_AddStringToObject(enemy, NAME, _enemy->enemy->name);
  if (!name) { createError(enemy, "enemy name"); return NULL; }

  cJSON* xpPoints = cJSON_AddNumberToObject(enemy, "xpPoints", _enemy->enemy->xpPoints);
  if (!xpPoints) { createError(enemy, "enemy xpPoints"); return NULL; }

  cJSON* hpArr = cJSON_AddArrayToObject(enemy, HP);
  if (!hpArr) { createError(enemy, "enemy hp container"); return NULL; }
  cJSON* hp = cJSON_CreateNumber(_enemy->enemy->hp);
  if (!hp) { createError(enemy, "enemy hp value"); return NULL; }
  if (!cJSON_AddItemToArray(hpArr, hp)) { createError(enemy, "enemy hp"); return NULL; }

  cJSON* lvl = cJSON_AddNumberToObject(enemy, LVL, _enemy->enemy->lvl);
  if (!lvl) { createError(enemy, "enemy lvl"); return NULL; }

  bool stats = saveStats(enemy, _enemy->enemy->stats);
  if (!stats) { createError(enemy, "enemy stats"); return NULL; }

  if (hasBoss) {
    bool gear = saveGear(enemy, &_enemy->boss->gearDrop);
    if (!gear) return NULL;

    cJSON* skills = cJSON_AddArrayToObject(enemy, "skills");
    if (!skills) { createError(enemy, "boss skills"); return NULL; }
    for (int i = 0; i < 5; i++) {
      cJSON* skill = saveSkill(&_enemy->boss->skills[i]);
      if (!skill) return NULL;

      if (!cJSON_AddItemToArray(skills, skill)) { createError(enemy, "skill in boss skills"); return NULL; }
    }
  }

  return enemy;
}

/**
 * Creates the maze state for saving.
 * @return The maze as a JSON string
 */
static str createMapState() {
  Table* table = initTableL(maze->size);
  if (!table) handleError(ERR_MEM, FATAL, "Could not allocate space for the table!\n");

  Room* room = maze->entry;
  addAndRecurse(room, table);
  room = NULL;

  if (table->len != maze->size) { handleError(ERR_DATA, WARNING, "Table size %d does not equal maze size! %d\n", table->len, maze->size); return NULL; }

  // The table SHOULD have all of the rooms
  // Time to create the JSON object
  cJSON* mapObj = cJSON_CreateObject();
  if (!mapObj) return createError(mapObj, "map");

  cJSON* mapName = cJSON_AddStringToObject(mapObj, "name", maze->name);
  if (!mapName) return createError(mapName, "map name");

  for (int i = 0; i < table->len; i++) {
    room = table->rooms[i];
    // Creating each room

    char buffer[2];
    str idAsChar = itoa(room->id, buffer, 10);

    cJSON* roomObj = cJSON_AddObjectToObject(mapObj, idAsChar);
    if (!roomObj) return createError(mapObj, idAsChar);

    cJSON* storyfile = cJSON_AddStringToObject(roomObj, "storyfile", (room->storyFile != NULL) ? room->storyFile : "");
    if (!storyfile) return createError(mapObj, "storyfile");

    cJSON* isEntry = cJSON_AddNumberToObject(roomObj, IS_ENTRY, (room->id == 0) ? 1 : 0);
    if (!isEntry) return createError(mapObj, IS_ENTRY);

    cJSON* info = cJSON_AddStringToObject(roomObj, INFO, room->info);
    if (!info) return createError(mapObj, INFO);

    cJSON* hasBoss = cJSON_AddNumberToObject(roomObj, HAS_BOSS, (room->hasBoss) ? 1 : 0);
    if (!hasBoss) return createError(mapObj, HAS_BOSS);

    cJSON* exits = cJSON_AddArrayToObject(roomObj, EXITS);
    if (!exits) return createError(mapObj, EXITS);
    for (int i = 0; i < 4; i++) {
      Room* roomExit = room->exits[i];

      cJSON* exit = cJSON_CreateNumber((roomExit == (void*)((long long)NO_EXIT) ? -1 : roomExit->id));
      if (!exit) return createError(mapObj, "exit");

      if (!cJSON_AddItemToArray(exits, exit)) return createError(mapObj, "exit in exits");
    }

    // Create the arrays just to be in compliance to format
    // But no need to add when none are present

    cJSON* loot = cJSON_AddArrayToObject(roomObj, LOOT);
    if (!loot) return createError(mapObj, LOOT);
    if (room->loot != NULL) {
      cJSON* _loot = saveLoot(room->loot);
      if (!_loot) return createError(mapObj, "item loot");
      if (!cJSON_AddItemToArray(loot, _loot)) return createError(mapObj, "item loot in loot");
    }

    cJSON* enemy = cJSON_AddArrayToObject(roomObj, ENEMY);
    if (!enemy) return createError(mapObj, ENEMY);
    if (room->enemy.enemy != NULL) {
      cJSON* enemyEntity = saveEnemy(&(room->enemy), room->hasBoss);
      if (!enemyEntity) return createError(mapObj, "enemy entity");
      if (!cJSON_AddItemToArray(enemy, enemyEntity)) return createError(mapObj, "enemy entity in enemy");
    }
  }

  str mapState = cJSON_Print(mapObj);

  cJSON_Delete(mapObj);
  deleteTable(table);

  return mapState;
}

/**
 * Saves the current state of the maze.
 * @return True if the map was saved, false otherwise
 */
static bool saveMap() {
  str mapState = createMapState();
  if (!mapState) handleError(ERR_DATA, WARNING, "Could not create map state!\n");

  if (mapState) {
    char filename[30];
    sprintf(filename, "%s/map_save.json", SAVE_DIR);

    FILE* file = fopen(filename, "w");
    if (!file) { handleError(ERR_IO, WARNING, "Unable to create the save!\n"); return false; }
    else {
      int written = fprintf(file, "%s", mapState);
      if (written <= 0) { handleError(ERR_IO, WARNING, "Unable to save the map data!\n"); return false; }
    }

    fclose(file);

    cJSON_free(mapState);

    return true;
  }

  return false;
}

/**
 * Creates the player state.
 * @return The player state as JSON string
 */
static str createPlayerState() {
  cJSON* playerObj = cJSON_CreateObject();
  if (!playerObj) return createError(playerObj, "player");

  cJSON* playerName = cJSON_AddStringToObject(playerObj, NAME, player->name);
  if (!playerName) return createError(playerObj, NAME);

  cJSON* playerXP = cJSON_AddNumberToObject(playerObj, XP, player->xp);
  if (!playerXP) return createError(playerObj, XP);

  cJSON* playerXPReq = cJSON_AddNumberToObject(playerObj, "xpReq", player->xpReq);
  if (!playerXPReq) return createError(playerObj, "xpReq");

  cJSON* playerLvl = cJSON_AddNumberToObject(playerObj, LVL, player->lvl);
  if (!playerLvl) return createError(playerObj, LVL);

  cJSON* playerHP = cJSON_AddNumberToObject(playerObj, HP, player->hp);
  if (!playerHP) return createError(playerObj, HP);

  cJSON* playerMaxHP = cJSON_AddNumberToObject(playerObj, MAX_HP, player->maxHP);
  if (!playerMaxHP) return createError(playerObj, MAX_HP);

  cJSON* playerDZ = cJSON_AddNumberToObject(playerObj, DZ, player->dzenai);
  if (!playerDZ) return createError(playerObj, DZ);

  cJSON* playerInvCount = cJSON_AddNumberToObject(playerObj, INV_COUNT, player->invCount);
  if (!playerInvCount) return createError(playerObj, INV_COUNT);

  cJSON* playerRoom = cJSON_AddObjectToObject(playerObj, ROOM);
  if (!playerRoom) return createError(playerObj, ROOM);
  cJSON* roomId = cJSON_AddNumberToObject(playerRoom, ID, player->room->id);
  if (!roomId) return createError(playerObj, ID);
  // Look at comment of loading map stuff when loading player
  // // Create the map filename
  // str base = "../maps/";
  // str mapFile = (str) malloc(14 + strlen(maze->name));
  // if (!mapFile) {
  //   cJSON_Delete(playerObj);
  //   handleError(ERR_MEM, WARNING, "Could not allocate space for map filename!\n");
  //   return NULL;
  // }
  // sprintf(mapFile, "%s%s.json", base, maze->name);
  // cJSON* roomMap = cJSON_AddStringToObject(playerRoom, MAP, mapFile);
  // if (roomMap == NULL) return createError(playerObj, MAP);

  cJSON* playerInv = cJSON_AddArrayToObject(playerObj, INV);
  if (!playerInv) createError(playerObj, INV);
  for (int i = 0; i < INV_CAP; i++) {
    if (player->inv[i]._item != NO_ITEM) {
      cJSON* item = cJSON_CreateObject();
      if (!item) return createError(playerObj, ITEM);

      cJSON* itemItem = saveLootItem(item, player->inv[i]._item, player->inv[i].type);
      if (!itemItem) return createError(playerObj, "item item");

      cJSON* itemType = cJSON_AddNumberToObject(item, TYPE, player->inv[i].type);
      if (!itemType) return createError(playerObj, TYPE);

      cJSON* itemCount = cJSON_AddNumberToObject(item, COUNT, player->inv[i].count);
      if (!itemCount) return createError(playerObj, COUNT);

      if (!cJSON_AddItemToArray(playerInv, item)) return createError(playerObj, "item in inventory");
    }
  }

  // HP Kit at hp slot is saved as a pointer, pointing to the same object as the one in the inv
  // Save it as the index to the kit it refers to
  // That is, search through inv, comparing addresses until they match, saving that index
  // This only works provided that the indices of the inventory items match between the saved json and when loading it up
  int hpSlotVal;

  if (player->hpSlot != NO_ITEM) {
    for (int i = 0; i < INV_CAP; i++) {
      printf("Searching address %p\n", &player->inv[i]);
      if (&player->inv[i] == player->hpSlot) {
        printf("Found hp kit!\n");
        hpSlotVal = i;
        break;
      }
    }
  } else hpSlotVal = -1;

  cJSON* hpSlot = cJSON_AddNumberToObject(playerObj, "hpSlot", hpSlotVal);
  if (!hpSlot) return createError(playerObj, "hp slot");


  bool gear = saveGear(playerObj, &player->gear);
  if (!gear) return createError(playerObj, GEAR);

  bool stats = saveStats(playerObj, player->stats);
  if (!stats) return createError(playerObj, "player stats");

  bool skillTree = saveSkillTree(playerObj, player->skills);
  if (!skillTree) return createError(playerObj, "player skill tree");

  str playerState = cJSON_Print(playerObj);

  cJSON_Delete(playerObj);
  return playerState;
}

/**
 * Saves the current state of the player.
 * @return True if the player was saved, false otherwise
 */
static bool savePlayer() {
  str playerState = createPlayerState();
  if (!playerState) { handleError(ERR_DATA, WARNING, "Could not create player state!\n"); return false; }
  
  char filename[32];
  sprintf(filename, "%s/player_save.json", SAVE_DIR);

  FILE* file = fopen(filename, "w");
  if (!file) { handleError(ERR_IO, WARNING, "Unable to create the save!\n"); return false; } 
  else {
    int written = fprintf(file, "%s", playerState);
    fclose(file);

    if (written <= 0) { handleError(ERR_IO, WARNING, "Unable to save the player data!\n"); return false; }
  }

  cJSON_free(playerState);

  return true;
}

void saveGame() {
  if (savePlayer() && saveMap()) printf("The game has been saved!\n");
  else printf("The game could not be saved!\n");
}

/**
 * Loads the player data.
 * @return The player
 */
static SoulWorker* loadPlayer() {
  char filename[30];
  sprintf(filename, "%s/player_save.json", SAVE_DIR);


  cJSON* root = readData(filename);
  if (!root) handleError(ERR_DATA, FATAL, "Could not parse JSON!\n");


  cJSON* name = cJSON_GetObjectItemCaseSensitive(root, NAME);
  if (!name) handleError(ERR_DATA, FATAL, "No name data found!\n");

  cJSON* hp = cJSON_GetObjectItemCaseSensitive(root, HP);
  if (!hp) handleError(ERR_DATA, FATAL, "No hp data found!\n");

  cJSON* maxHP = cJSON_GetObjectItemCaseSensitive(root, MAX_HP);
  if (!maxHP) handleError(ERR_DATA, FATAL, "No maxHP data found!\n");

  cJSON* invCount = cJSON_GetObjectItemCaseSensitive(root, INV_COUNT);
  if (!invCount) handleError(ERR_DATA, FATAL, "No invCount data found!\n");

  cJSON* xp = cJSON_GetObjectItemCaseSensitive(root, XP);
  if (!xp) handleError(ERR_DATA, FATAL, "No xp data found!\n");

  cJSON* xpReq = cJSON_GetObjectItemCaseSensitive(root, "xpReq");
  if (!xpReq) handleError(ERR_DATA, FATAL, "No xp req data found!\n");

  cJSON* dzenai = cJSON_GetObjectItemCaseSensitive(root, DZ);
  if (!dzenai) handleError(ERR_DATA, FATAL, "No dzenai data found!\n");

  cJSON* lvl = cJSON_GetObjectItemCaseSensitive(root, LVL);
  if (!lvl) handleError(ERR_DATA, FATAL, "No lvl data found!\n");

  cJSON* room = cJSON_GetObjectItemCaseSensitive(root, ROOM);
  if (!room) handleError(ERR_DATA, FATAL, "No room data found!\n");
  cJSON* roomId = cJSON_GetObjectItemCaseSensitive(room, ID);
  if (!roomId) handleError(ERR_DATA, FATAL, "No roomID data found!\n");
  // Just realized this is not used and it is not needed
  // The map is obviously saved on its own but alongside player
  // So by logic, the saved map (which gets loaded before the player)
  // is the map that the player is to be in
  // cJSON* roomMap = cJSON_GetObjectItemCaseSensitive(room, MAP);
  // if (roomMap == NULL) handleError(ERR_DATA, FATAL, "No roomMap data found!\n");

  cJSON* inv = cJSON_GetObjectItemCaseSensitive(root, INV);
  if (!inv) handleError(ERR_DATA, FATAL, "No inventory data found!\n");


  int nameLen = strlen(name->valuestring);
  str playerName = (str) malloc(nameLen + 1);
  if (!playerName) handleError(ERR_MEM, FATAL, "Could not allocate space for the player name!\n");
  strncpy(playerName, name->valuestring, nameLen+1);

  SoulWorker* player = initSoulWorker(playerName);
  player->hp = hp->valueint;
  player->xp = xp->valueint;
  player->xpReq = xpReq->valueint;
  player->invCount = invCount->valueint;
  player->maxHP = maxHP->valueint;
  player->dzenai = dzenai->valueint;
  player->lvl = lvl->valueint;

  DArray* visited = initDArray(maze->size);

  player->room = findRoom(maze->entry, (char) roomId->valueint, visited);

  dArrayFree(visited);

  if (!player->room|| player->room == (void*)((long long) NO_EXIT)) handleError(ERR_DATA, FATAL, "Could not find room!\n");

  // Make sure room is the same
  if (player->room->id != (byte)(roomId->valueint)) handleError(ERR_DATA, FATAL, "Room ID does not match!\n");


  for (int i = 0; i < cJSON_GetArraySize(inv); i++) {
    cJSON* invItem = cJSON_GetArrayItem(inv, i);

    cJSON* itemType = cJSON_GetObjectItemCaseSensitive(invItem, TYPE);

    Item* item = createItem(invItem, itemType->valueint);
    player->inv[i]._item = item->_item;
    player->inv[i].type = item->type;
    player->inv[i].count = item->count;
    free(item);
    item = NULL;
  }

  // Saved hp slot is the index in the inventory

  cJSON* hpSlot = cJSON_GetObjectItemCaseSensitive(root, "hpSlot");
  if (!hpSlot) handleError(ERR_DATA, FATAL, "No HP slot data found!\n");

  int hpSlotIdx = hpSlot->valueint;
  // printf("Saved HP slot index is %d\n", hpSlotIdx);

  if (hpSlotIdx != -1) {
    for (int i = 0; i < INV_CAP; i++) {
      if (i == hpSlotIdx) {
        player->hpSlot = &player->inv[i];
        break;
      }
    }
  } else player->hpSlot = NO_ITEM;

  // TODO: Make loadGear and loadStats, use with loading enemies???

  cJSON* stats = cJSON_GetObjectItemCaseSensitive(root, "stats");
  if (!stats) handleError(ERR_DATA, FATAL, "No stats data found!\n");

  cJSON* atk = cJSON_GetObjectItemCaseSensitive(stats, "ATK");
  if (!atk) handleError(ERR_DATA, FATAL, "No atk stats data found!\n");
  player->stats->ATK = (cJSON_GetArrayItem(atk, 0))->valueint;

  cJSON* def = cJSON_GetObjectItemCaseSensitive(stats, "DEF");
  if (!def) handleError(ERR_DATA, FATAL, "No def stats data found!\n");
  player->stats->DEF = (cJSON_GetArrayItem(def, 0))->valueint;

  cJSON* acc = cJSON_GetObjectItemCaseSensitive(stats, "ACC");
  if (!acc) handleError(ERR_DATA, FATAL, "No acc stats data found!\n");
  player->stats->ACC = (cJSON_GetArrayItem(acc, 0))->valueint;

  cJSON* atkCrit = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT");
  if (!atkCrit) handleError(ERR_DATA, FATAL, "No atk_crit stats data found!\n");
  player->stats->ATK_CRIT = (cJSON_GetArrayItem(atkCrit, 0))->valuedouble;

  cJSON* critDmg = cJSON_GetObjectItemCaseSensitive(stats, "ATK_CRIT_DMG");
  if (!critDmg) handleError(ERR_DATA, FATAL, "No atk_crit_dmg stats data found!\n");
  player->stats->ATK_CRIT_DMG = (cJSON_GetArrayItem(critDmg, 0))->valueint;


  cJSON* gear = cJSON_GetObjectItemCaseSensitive(root, "gear");
  if (!gear) handleError(ERR_DATA, FATAL, "No gear data found!\n");

  cJSON* sw = cJSON_GetObjectItemCaseSensitive(gear, "soulweapon");
  if (!sw) handleError(ERR_DATA, FATAL, "No SoulWeapon data found!\n");
  if (!cJSON_IsNull(sw)) player->gear.sw = createSoulWeapon(sw);

  cJSON* helmet = cJSON_GetObjectItemCaseSensitive(gear, "helmet");
  if (!helmet) handleError(ERR_DATA, FATAL, "No helmet data found!\n");
  if (!cJSON_IsNull(helmet)) player->gear.helmet = createArmor(helmet);

  cJSON* guard = cJSON_GetObjectItemCaseSensitive(gear, "shoulder_guard");
  if (!guard) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(guard)) player->gear.guard = createArmor(guard);

  cJSON* chestplate = cJSON_GetObjectItemCaseSensitive(gear, "chestplate");
  if (!chestplate) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(chestplate)) player->gear.chestplate = createArmor(chestplate);

  cJSON* boots = cJSON_GetObjectItemCaseSensitive(gear, "boots");
  if (!boots) handleError(ERR_DATA, FATAL, "No  data found!\n");
  if (!cJSON_IsNull(boots)) player->gear.boots = createArmor(boots);

  cJSON* skillTree = cJSON_GetObjectItemCaseSensitive(root, "skills");
  if (!skillTree) handleError(ERR_DATA, FATAL, "No skill tree data found!\n");

  cJSON* skillStatus = cJSON_GetObjectItemCaseSensitive(skillTree, "skillStatus");
  if (!skillStatus) handleError(ERR_DATA, FATAL, "No skill status data found!\n");
  player->skills->skillStatus = skillStatus->valueint;

  cJSON* totalSkillPoints = cJSON_GetObjectItemCaseSensitive(skillTree, "totalSkillPoints");
  if (!totalSkillPoints) handleError(ERR_DATA, FATAL, "No total skill points data found!\n");
  player->skills->totalSkillPoints = totalSkillPoints->valueint;

  cJSON* skills = cJSON_GetObjectItemCaseSensitive(skillTree, "skills");
  if (!skills) handleError(ERR_DATA, FATAL, "No skills data found!\n");

  for (int i = 0; i < cJSON_GetArraySize(skills); i++) { // size should be TOTAL_SKILLS
    cJSON* _skill = cJSON_GetArrayItem(skills, i);

    Skill* skill = createSkill(_skill);
    
    // strcpy(player->skills->skills[i].name, skill->name);
    player->skills->skills[i].name = skill->name;
    // strcpy(player->skills->skills[i].description, skill->description);
    player->skills->skills[i].description = skill->description;
    player->skills->skills[i].lvl = skill->lvl;
    player->skills->skills[i].cooldown = skill->cooldown;
    player->skills->skills[i].id = skill->id;
    player->skills->skills[i].effect1 = skill->effect1;
    player->skills->skills[i].effect2 = skill->effect2;
    player->skills->skills[i].activeEffect1 = skill->activeEffect1;
    player->skills->skills[i].activeEffect2 = skill->activeEffect2;

    free(skill);
  }

  cJSON* equippedSkills = cJSON_GetObjectItemCaseSensitive(skillTree, "equippedSkills");
  if (!equippedSkills) handleError(ERR_DATA, FATAL, "No equipped skills data found!\n");

  for (int i = 0; i < cJSON_GetArraySize(equippedSkills); i++) { // size should be EQUIPPED_SKILL_COUNT
    cJSON* id = cJSON_GetArrayItem(equippedSkills, i);

    if (id->valueint == -1) player->skills->equippedSkills[i] = NO_SKILL;
    else {
      // Since the actual player object stores pointers to the skill in the skill array
      // It needs to iterate through that array, finding for a matching id, and grabbing the pointer
      // and storing it
      // However, since the id of the skill is just (its array pos + 1), no need to traverse it
      // Just get it
      // Thus for skills[i], i = id - 1
      player->skills->equippedSkills[i] = &player->skills->skills[id->valueint - 1];
    }
  }

  cJSON_Delete(root);

  return player;
}

/**
 * Loads the maze data. Just a wrapper to initMaze.
 * @return The maze
 */
static Maze* loadMap() {
  char filename[30];
  sprintf(filename, "%s/map_save.json", SAVE_DIR);

  return initMaze(filename);
}

/**
 * Loads a saved game.
 */
void loadGame() {
  printf("Loading map...\n");
  maze = loadMap();
  printf("Map loaded!\n");

  printf("Loading player...\n");
  player = loadPlayer();
  printf("Player loaded!\n");

  printf("The game has been loaded!\n");
}