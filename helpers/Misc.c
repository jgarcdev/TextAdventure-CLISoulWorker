#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Misc.h"
#include "Error.h"


/**
 * Compares the floats, with a provided epsilon for floating errors.
 * @param f1 Float 1
 * @param f2 Float 2
 * @return True if relativelt equal, false otherwise
 */
static bool floateq(float f1, float f2) {
  float EPS = 0.0001;

  return (fabs(f1 - f2) < EPS);
}

/**
 * Whether the two SoulWeapons are equal in all fields, except durability and upgrades.
 * @param sw1 SoulWeapon 1
 * @param sw2 SoulWeapon 2
 * @return True if equal, false otherwise
 */
static bool equalSoulWeapon(SoulWeapon* sw1, SoulWeapon* sw2) {
  return ((sw1->acc == sw2->acc) &&
          (sw1->atk == sw2->atk) &&
          (floateq(sw1->atk_crit, sw2->atk_crit)) &&
          (sw1->atk_crit_dmg == sw2->atk_crit_dmg) &&
          (sw1->lvl == sw2->lvl) &&
          (strcmp(sw1->name, sw2->name) == 0));
}

/**
 * Whether the two pieces of armor are equal in all fields.
 * @param arm1 Armor 1
 * @param arm2 Armor 2
 * @return True if equal, false otherwise
 */
static bool equalArmor(Armor* arm1, Armor* arm2) {
  return ((arm1->type == arm2->type) &&
          (arm1->acc == arm2->acc) &&
          (arm1->def == arm2->def) &&
          (arm1->lvl == arm2->lvl) &&
          (strcmp(arm1->name, arm2->name) == 0));
}

/**
 * Whether the two HP kits are equal in all fields, except description.
 * @param kit1 HP Kit 1
 * @param kit2 HP Kit 2
 * @return True if equal, false otherwise
 */
static bool equalHPKits(HPKit* kit1, HPKit* kit2) {
  return kit1->type == kit2->type;
}

/**
 * Whether the two upgrade materials are equal in all fields, except description.
 * @param up1 Upgrade material 1
 * @param up2 Upgrade material 2
 * @return True if equal, false otherwise
 */
static bool equalUpgrade(Upgrade* up1, Upgrade* up2) {
  return (up1->rank == up2->rank && up1->type == up2->type);
}

bool equalItems(Item* item1, Item* item2) {
  if (!item1 || !item2) return false;

  if (item1->type != item2->type) return false;

  // For each specific item type, check data equality
  switch (item1->type) {
    case SOULWEAPON_T:
      return equalSoulWeapon((SoulWeapon*)item1->_item, (SoulWeapon*)item2->_item);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      return equalArmor((Armor*)item1->_item, (Armor*)item2->_item);
      break;
    case HP_KITS_T:
      return equalHPKits((HPKit*)item1->_item, (HPKit*)item2->_item);
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
      return equalUpgrade((Upgrade*)item1->_item, (Upgrade*)item2->_item);
      break;
    case SLIME_T:
      return true;
      break;
    default:
      return false;
      break;
  }

  return false;
}

/**
 * Creates the name for the given HP kit.
 * @param hpKit The HP kit to get the name
 * @return The name
 */
static str getHPKitName(HPKit* hpKit) {
  // "**** HP Kit"
  str name = (str) malloc(12);
  if (!name) handleError(ERR_MEM, FATAL, "Could not allocate space for HP Kit name!\n");

  str type;
  switch (hpKit->type) {
    case DEKA:
      type = "Deka";
      break;
    case MEGA:
      type = "Mega";
      break;
    case PETA:
      type = "Peta";
      break;
    default:
      type = "ERR_";
      break;
  }

  sprintf(name, "%s HP Kit", type);  

  return name;
}

/**
 * Creates the name for the given upgrade material.
 * @param upgrade The upgrade material
 * @return The name
 */
static str getUpgradeName(Upgrade* upgrade) {
  // "* Weapon|Armor Upgrade Material"  
  str name = (str) malloc(26);
  if (!name) handleError(ERR_MEM, FATAL, "Could not allocate space for upgrade material name!\n");

  str type;
  switch (upgrade->type) {
    case WEAPON:
      type = "Weapon";
      break;
    case ARMOR:
      type = "Armor";
      break;
    default:
      type = "ERROR";
      break;
  }

  sprintf(name, "%c %s Upgrade Material", (char)upgrade->rank, type);

  return name;
}

str getItemName(Item* item) {
  str name;

  switch (item->type) {
    case SOULWEAPON_T:
      SoulWeapon* sw_t = (SoulWeapon*) item->_item;
      name = sw_t->name;
      break;
    case HELMET_T:
      Armor* helm_t = (Armor*) item->_item;
      name = helm_t->name;
      break;
    case SHOULDER_GUARD_T:
      Armor* guard_t = (Armor*) item->_item;
      name = guard_t->name;
      break;
    case CHESTPLATE_T:
      Armor* plate_t = (Armor*) item->_item;
      name = plate_t->name;
      break;
    case BOOTS_T:
      Armor* boots_t = (Armor*) item->_item;
      name = boots_t->name;
      break;
    case HP_KITS_T:
      HPKit* hp_t = (HPKit*) item->_item;
      name = getHPKitName(hp_t);
      break;
    case WEAPON_UPGRADE_MATERIALS_T:
      Upgrade* wu_t = (Upgrade*) item->_item;
      name = getUpgradeName(wu_t);
      break;
    case ARMOR_UPGRADE_MATERIALS_T:
      Upgrade* au_t = (Upgrade*) item->_item;
      name = getUpgradeName(au_t);
      break;
    case SLIME_T:
      name = "Slime";
      break;
    default:
      name = "NOT_AN_ITEM";
      break;
  }

  return name;
}

void displaySoulWeapon(SoulWeapon* sw) {
  printf("SoulWeapon %s; Lvl %d, Upgrade %d/5; %d/100\n\tATK: %d, ACC: %d, ATK CRIT: %2.3f%%, ATK CRIT DMG: %d\n",
      sw->name, sw->lvl, sw->upgrades, sw->durability,
      sw->atk, sw->acc, sw->atk_crit, sw->atk_crit_dmg);
}

void displayArmor(Armor* armor) {
  str type;

  if (armor->type == HELMET) type = "Helmet";
  else if (armor->type == SHOULDER_GUARD) type = "Shoulder Guard";
  else if (armor->type == CHESTPLATE) type = "Chestplate";
  else type = "Boots";

  printf("%s %s; Lvl %d\n\tDEF: %d, ACC: %d\n", 
      type, armor->name, armor->lvl, armor->def, armor->acc);
}

void displayHPKit(HPKit* hpKit) {
  printf("%s\n%s\n", getHPKitName(hpKit), hpKit->desc);
}

void displayUpgrade(Upgrade* upgrade) {
  printf("%s\n%s\n", getUpgradeName(upgrade), upgrade->desc);
}

void displaySlime(Slime* slime) {
  printf("Slime\n%s\n", slime->desc);
}

void deleteSoulWeapon(SoulWeapon* sw) {
  if (!sw) return;

  free(sw->name);
  free(sw);
  sw = NULL;
}

void deleteArmor(Armor* armor) {
  if (!armor) return;

  free(armor->name);
  free(armor);
  armor = NULL;
}

void deleteOther(HPKit* item) {
  if (!item) return;

  free(item->desc);
  free(item);
  item = NULL;
}

bool deleteItem(Item* item) {
  if (!item) return false;

  // Maybe turn to if statements
  switch (item->type) {
    case SOULWEAPON_T:
      deleteSoulWeapon((SoulWeapon*) item->_item);
      break;
    case HELMET_T:
    case SHOULDER_GUARD_T:
    case CHESTPLATE_T:
    case BOOTS_T:
      deleteArmor((Armor*) item->_item);
      break;
    case HP_KITS_T:
    case WEAPON_UPGRADE_MATERIALS_T:
    case ARMOR_UPGRADE_MATERIALS_T:
    case SLIME_T:
      deleteOther((HPKit*)item->_item);
      break;
    default:
      break;
  }

  free(item);
  item = NULL;

  return true;
}

void displayEnemyStats(Enemy* enemy) {
  printf("%s, LVL %d; HP %d\nATK: %d; DEF: %d; ACC: %d; ATK CRIT DMG: %d; ATK CRIT: %3.2f\n", 
      enemy->name, enemy->lvl, enemy->hp,
      enemy->stats->ATK, enemy->stats->DEF, enemy->stats->ACC, enemy->stats->ATK_CRIT_DMG, enemy->stats->ATK_CRIT);
}

void initSkill(Skill *skill, str name, str desc, byte lvl, byte cooldown, 
    ushort effect1, float effect2, effect_t active1, effect_t active2, byte id) {
  skill->name = name;
  skill->description = desc;
  skill->lvl = lvl;
  skill->cooldown = cooldown;
  // cdTimer works by starting at 0
  // When the skill is used, it gets set to cooldown
  // As each turn finishes, all skills have their cdTimer reduced by 1
  skill->cdTimer = 0;
  skill->id = id;

  switch (active1) {
    case ATK:
      skill->effect1.atk = effect1;
    case ATK_CRIT_DMG:
      skill->effect1.atk_crit_dmg = effect1;
      break;
    default:
      break;
  }

  switch (active2) {
    case DEF:
      skill->effect2.def = effect2;
      break;
    case ACC:
      skill->effect2.acc = effect2;
      break;
    case ATK_CRIT:
      skill->effect2.atk_crit = effect2;
      break;
    default:
      break;
  }

  skill->activeEffect1 = active1;
  skill->activeEffect2 = active2;
}

void deleteSkills(Skill skills[], int len) {
  for (int i = 0; i < len; i++) {
    free(skills[i].name);
    free(skills[i].description);
  }
}

bool deleteEnemy(Enemy *enemy) {
  if (!enemy) return false;

  free(enemy->name);
  free(enemy->stats);
  free(enemy);

  return true;
}

bool deleteBoss(Boss* boss, bool deleteGear) {
  if (!boss) return false;

  free(boss->base.name);
  free(boss->base.stats);
  // Do not delete the gear. Transfer ownership (copy addresses) to player
  // Default to not delete the gear so the ownership (addresses) are transferred to player
  // But when ending the game, do delete the gear if boss is still present
  if (deleteGear) {
    deleteSoulWeapon(boss->gearDrop.sw);
    deleteArmor(boss->gearDrop.helmet);
    deleteArmor(boss->gearDrop.guard);
    deleteArmor(boss->gearDrop.chestplate);
    deleteArmor(boss->gearDrop.boots);
  }

  deleteSkills(boss->skills, BOSS_SKILL_COUNT);

  free(boss);

  return true;
}