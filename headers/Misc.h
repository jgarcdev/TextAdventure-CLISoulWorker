#ifndef _MISC_H
#define _MISC_H


#include <stdbool.h>

#include "Colors.h"


#define ushort unsigned short
#define uchar unsigned char
#define uint unsigned int

#define STR char*
#define ubyte unsigned char


typedef struct Stats {                         // 12B
  ushort ATK; // The attack damage                 2B
  ushort DEF; // The defense                       2B
  ushort ACC; // The accuracy                      2B
  ushort ATK_CRIT_DMG; // The attack crit damage   2B
  float ATK_CRIT; // The attack crit percent       4B
} Stats;

// deal with -fshort-enums and pragma pack(2) later
// for now, enums are uints in gcc and msvc
typedef enum {
  NONE, // only when NO_ITEM
  SOULWEAPON_T,
  HELMET_T,
  SHOULDER_GUARD_T,
  CHESTPLATE_T,
  BOOTS_T,
  HP_KITS_T,
  WEAPON_UPGRADE_MATERIALS_T,
  ARMOR_UPGRADE_MATERIALS_T,
  SLIME_T
} item_t;
// The Item model.
typedef struct Item {      // 14B+2B(PAD) = 16B
  void* _item; // The item                   8B
  item_t type; // The type of the item       4B
  ushort count; // The amount of that item   2B
  // Maybe in future, store the price as a uchar or ushort????
} Item;

// 

typedef struct SoulWeapon {      // 21B+3B(PAD) = 24B
  STR name; // The name of the weapon              8B
  ushort atk; // The attack stat                   2B
  ushort acc; // The accuracy stat                 2B
  float atk_crit; // The attack crit percent       4B
  ushort atk_crit_dmg; // The attack crit damage   2B
  ubyte lvl; // The weapon level                    1B
  ubyte upgrades; // Upgrades done                  1B
  ubyte durability; // Weapon durability            1B
} SoulWeapon;

typedef enum {
  HELMET,
  SHOULDER_GUARD,
  CHESTPLATE,
  BOOTS
} armor_t;
typedef struct Armor { //                16B
  STR name; // The armor name             8B
  armor_t type; // The type of armor      4B
  ushort acc; // The accuracy it provides 2B
  ubyte def; // The defense stat           1B
  ubyte lvl; // The armor level            1B
} Armor;

typedef enum {
  DEKA,
  MEGA,
  PETA
} hpkit_t;
typedef struct HPKit {
  hpkit_t type;
  char* desc;
} HPKit;

typedef enum {
  B = 'B', // Lvl 1-30
  A = 'A', // Lvl 30-60
  S = 'S' // Lvl 60-100
} value_t;
typedef enum {
  WEAPON,
  ARMOR
} upgrade_t;
typedef struct Upgrade {
  value_t rank; // The rank of the upgrade
  upgrade_t type; // Weapon or armor upgrade
  char* desc; // Description
} Upgrade;

typedef struct Slime {
  char* desc;
} Slime;


/**
 * Checks whether the two items are the exact same.
 * Note, there is a small chance that the items were generated with the exact same data but as seperate.
 * In that case, it will detect them as the same. This shall be intended behavior.
 * @param item1 Item 1
 * @param item2 Item 2
 * @return True if both are the same, false otherwise
 */
bool equalItems(Item* item1, Item* item2);

/**
 * Gets the specific name for the item, depending on its type.
 * @param item The item
 * @return The name
 */
STR getItemName(Item* item);


/**
 * Maybe no reason to have these functions......
 */

/**
 * Displays SoulWeapon data.
 * @param sw The SoulWeapon to display
 */
void displaySoulWeapon(SoulWeapon* sw);

/**
 * Displays armor data.
 * @param armor The armor to display
 */
void displayArmor(Armor* armor);

/**
 * Displays HP kit data.
 * @param hpKit The HP kit to display
 */
void displayHPKit(HPKit* hpKit);

/**
 * Displays upgrade material data.
 * @param upgrade The upgrade material to display
 */
void displayUpgrade(Upgrade* upgrade);

/**
 * Displays slime data.
 * @param slime The slime to display
 */
void displaySlime(Slime* slime);

/**
 * Deletes the item, freeing the memory.
 * @param item The item to delete
 * @return True if the item was deleted, false otherwise
 */
bool deleteItem(Item* item);

/**
 * Deletes the given SoulWeapon, freeing memory.
 * @param sw The SoulWeapon to delete
 */
void deleteSoulWeapon(SoulWeapon* sw);

/**
 * Deletes the given armor piece, freeing memory.
 * @param armor The armor piece to delete
 */
void deleteArmor(Armor* armor);

/**
 * Deletes items that only have the description allocated.
 * Note, even if the type is HPKit*, it does the same for the upgrade materials.
 * @param item Any other item
 */
void deleteOther(HPKit* item);


typedef struct Gear {
  SoulWeapon* sw; //    8B
  Armor* helmet; //     8B
  Armor* guard; //      8B
  Armor* chestplate; // 8B
  Armor* boots; //      8B
} Gear;


typedef enum {
  ATK,
  ATK_CRIT_DMG,
  DEF,
  ACC,
  ATK_CRIT
} effect_t;

typedef struct Skill { // 34B+6B(PAD) = 40B
  STR name; //                           8B
  STR description; //                    8B
  ubyte lvl; //                           1B
  ubyte cooldown; //                      1B
  char cdTimer; //                       1B
  ubyte id; //                            1B
  union { //                             2B
    ushort atk; // 2B
    ushort atk_crit_dmg; // 2B
  } effect1;
  union { //                             4B
    ushort def; // 2B
    ushort acc; // 2B
    float atk_crit; // 4B
  } effect2;
  effect_t activeEffect1; //             4B
  effect_t activeEffect2; //             4B
} Skill;

typedef struct Enemy { // 25B+7B(PAD) = 32B
  STR name; //                           8B
  uint xpPoints; //                      4B
  uint hp; //                            4B
  ubyte lvl; //                          1B
  Stats* stats; //                       8B
} Enemy;

#define BOSS_SKILL_COUNT 5

typedef struct Boss { //               272B
  Enemy base; //                        32B
  Gear gearDrop; //                     40B
  Skill skills[BOSS_SKILL_COUNT]; //   200B
} Boss;

/**
 * Displays the stats of the enemy.
 * @param enemy The enemy to display
 */
void displayEnemyStats(Enemy* enemy);

// TODO: Better way to init skill
/**
 * Initiates/fills out the given skill slot.
 * Note, I DO NOT like how the information is passed.
 * Future me issue, figure out alt way.
 * @param skill 
 * @param name 
 * @param desc 
 * @param lvl 
 * @param cooldown 
 * @param effect1 
 * @param effect2 
 * @param active1 
 * @param active2 
 */
void initSkill(Skill* skill, STR name, STR desc, ubyte lvl, ubyte cooldown, 
    ushort effect1, float effect2, effect_t active1, effect_t active2, ubyte id);

/**
 * Deletes the skills' name and description
 * @param skills The skills to delete
 * @param len The length of the skill array
 */
void deleteSkills(Skill skills[], int len);

/**
 * Deletes the enemy, freeing the memory.
 * @param enemy The enemy to delete
 * @return True if the enemy was deleted, false otherwise
 */
bool deleteEnemy(Enemy* enemy);

/**
 * Deletes the boss, freeing the memory. Note, does not free the dropped gear.
 * @param boss 
 * @return 
 */
bool deleteBoss(Boss* boss, bool deleteGear);


















#endif