#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoulWorker.h"
#include "Error.h"

#define NO_ITEM NULL
#define NO_SKILL NULL
#define SKILL_DATA "./data/misc/skills.dat"

static SkillTree* initSkillTree();
static uint xpRequired(uint lvl);

SoulWorker* initSoulWorker(str name) {
  SoulWorker* sw = (SoulWorker*) malloc(sizeof(SoulWorker));
  if (!sw) handleError(ERR_MEM, FATAL, "Could not allocate space for the player!\n");  

  // Set basic stats
  sw->name = name;
  sw->hp = 5;
  sw->xp = 0;
  sw->xpReq = xpRequired(1);
  sw->lvl = 1;
  sw->invCount = 0;
  sw->dzenai = 0;
  sw->maxHP = 5;

  sw->room = NULL;

  // Set gear
  sw->gear.sw = NO_ITEM;
  sw->gear.helmet = NO_ITEM;
  sw->gear.guard = NO_ITEM;
  sw->gear.chestplate = NO_ITEM;
  sw->gear.boots = NO_ITEM;
  
  sw->hpSlot = NO_ITEM;

  // Set inv
  for (int i = 0; i < INV_CAP; i++) {
    sw->inv[i]._item = NO_ITEM;
    sw->inv[i].count = 0;
    sw->inv[i].type = NONE;
  }

  // Set adv stats
  Stats* stats = (Stats*) malloc(sizeof(Stats));
  if (!stats) handleError(ERR_MEM, FATAL, "Could not allocate space for player stats!\n");

  stats->ATK = 4;
  stats->DEF = 4;
  stats->ACC = 3;
  stats->ATK_CRIT_DMG = 2;
  stats->ATK_CRIT = 0.05;
  sw->stats = stats;

  // Set skill tree
  sw->skills = initSkillTree();

  return sw;
}


bool addToInv(SoulWorker* sw, Item* loot) {
  /**
   * When items are created, it creates an Item structure, keeping it as a ptr
   * However, when that item is picked up, the malloc'd struct (Item*) is kept but the Item struct in
   *  the inventory is filled with its data. However, that Item* struct is never freed.
   * When the player is deleted, it only frees its built-in Item structs and the structs of its void*
   *  but never the original malloc'd Item* structure, thus having a memory leak if original item is not freed.
   * Doing deleteItem when adding to inventory frees its void* as well, which is not what is intended.
   * Most of the time, the original Item* struct will be freed after this call
   */

  Item* item = NULL;

  // Look for item
  for (int i = 0; i < INV_CAP; i++) {
    if (equalItems(&(sw->inv[i]), loot)) {
      item = &(sw->inv[i]);
      break;
    }
  }

  if (!item) { // Item not in inventory
    // Make sure enough space
    if (sw->invCount == INV_CAP) {
      printf("Your inventory is full! You must remove some items!\n");

      return false;
    }

    // Gear are non-stackable
    // However, because each gear is different, it will get caught by the first equalItems check
    // So it then is guaranteed that the gear is different, thus it is handle by the item == NULL block

    // Enough space, add it
    for (int i = 0; i < INV_CAP; i++) {
      // Finding first slot with no item
      if (sw->inv[i].type == NONE) {
        sw->inv[i]._item = loot->_item;
        sw->inv[i].count = loot->count;
        sw->inv[i].type = loot->type;
        sw->invCount++;

        // printf("Item has been added to the inventory!\n"); // Change message based on scenario (print on scenario scope)
        return true;
      } else if (equalItems(&(sw->inv[i]), loot)) { // Updating existing item
        sw->inv[i].count++;

        printf("Item count has been increased!");

        /**
         * The item in the map does not get removed. When an existing item is picked up,
         * only the inv item count is increased, then the Item structure (*loot) is freed, but not its void* one (_item)
         * leading to a memory leak.
         * Thus, Item._item must be freed in this case
         * Weapons and armor should not really be here as it should (almost?) never be the case that there is
         * a duplicate one (leading to this if case) but for good practice, it is freed anyway
         */
        switch (sw->inv[i].type) {
          case SOULWEAPON_T:
            deleteSoulWeapon((SoulWeapon*) sw->inv[i]._item);
            break;
          case HELMET_T:
          case SHOULDER_GUARD_T:
          case CHESTPLATE_T:
          case BOOTS_T:
            deleteArmor((Armor*) sw->inv[i]._item);
            break;
          case HP_KITS_T:
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
          case SLIME_T:
            deleteOther((HPKit*) sw->inv[i]._item);
            break;
          default:
            break;
        }
        sw->inv[i]._item = NO_ITEM;

        return true;
      }
    }
  }

  return false;
}

void removeFromInv(SoulWorker* sw, Item* item, ushort count) {
  // Item* item is pointing to the slot in the inventory already

  item->count -= count;

  if (item->count == 0) {
    // Not using deleteItem() because Item struct is built-in the inv
    // not as a pointer
    // And deleteItem() frees and nulls the Item struct
    // Need to free and null the actual item itself (void*)

    void* _item = item->_item;

    switch (item->type) {
      case SOULWEAPON_T:
        deleteSoulWeapon((SoulWeapon*) _item);
        break;
      case HELMET_T:
      case SHOULDER_GUARD_T:
      case CHESTPLATE_T:
      case BOOTS_T:
        deleteArmor((Armor*) _item);
        break;
      case HP_KITS_T:
      case WEAPON_UPGRADE_MATERIALS_T:
      case ARMOR_UPGRADE_MATERIALS_T:
      case SLIME_T:
        deleteOther((HPKit*) _item);
        break;
      default:
        break;
    }

    item->_item = NO_ITEM;
    item->count = 0;
    item->type = NONE;

    sw->invCount--;
  }
}

void viewInventory(SoulWorker* sw) {
  if (sw->invCount == 0) printf("You have no items.\n");
  else {
    printf("You have %d item%s: \n", sw->invCount, (sw->invCount == 1) ? "" : "s");

    for (int i = 0; i < INV_CAP; i++) {
      if (sw->inv[i]._item != NO_ITEM) {
        str name = getItemName(&(sw->inv[i]));

        printf("%d: %s * %d\n", i+1, name, sw->inv[i].count);

        // Since some strings have been alloc'd, free them
        switch (sw->inv[i].type) {
          case HP_KITS_T:
          case WEAPON_UPGRADE_MATERIALS_T:
          case ARMOR_UPGRADE_MATERIALS_T:
            free(name);
            name = NULL;
          default:
            break;
        }
      }
    }
  }
}

/**
 * Displays the equipped gear (armor and weapon) of the player.
 * @param sw The player
 */
static void viewGear(SoulWorker* sw) {
  SoulWeapon* swp = sw->gear.sw;
  Armor* helmet = sw->gear.helmet;
  Armor* guard = sw->gear.guard;
  Armor* chestplate = sw->gear.chestplate;
  Armor* boots = sw->gear.boots;
  /**
   * [weapon]: (Unequipped)|([name], ATK: [atk], ACC: [acc], ATK CRIT: [atk_crit], ATK CRIT DMG: [atk_crit_dmg],\n
   * \tLVL: [lvl], UPGRADES: [upgrades], DURABILITY: [durability])
   * [armor piece]: (Unequipped)|([name]; LVL: [lvl], ACC: [acc], DEF: [def])
   */

  char *weaponStats, *helmetStats, *shoulderGuardStats, *chestplateStats, *bootsStats;
  int weaponNameLen, helmetNameLen, shoulderGuardNameLen, chestplateNameLen, bootsNameLen;
  if (swp != NO_ITEM) weaponNameLen = strlen(swp->name);
  if (helmet != NO_ITEM) helmetNameLen = strlen(helmet->name);
  if (guard != NO_ITEM) shoulderGuardNameLen = strlen(guard->name);
  if (chestplate != NO_ITEM) chestplateNameLen = strlen(chestplate->name);
  if (boots != NO_ITEM) bootsNameLen = strlen(boots->name);

  // If no gear piece equipped, only get space for "Unequipped\0"
  // Otherwise, space for name and its stats
  //                                                            [name] + rest + [lvl] + [acc] + [def] + \0
  helmetStats = (str) malloc((helmet == NO_ITEM) ? 11 : (helmetNameLen + 21 + 3 + 5 + 3 + 1));
  shoulderGuardStats = (str) malloc((guard == NO_ITEM) ? 11 : (shoulderGuardNameLen + 21 + 3 + 5 + 3 + 1));
  chestplateStats = (str) malloc((chestplate == NO_ITEM) ? 11 : (chestplateNameLen + 21 + 3 + 5 + 3 + 1));
  bootsStats = (str) malloc((boots == NO_ITEM) ? 11 : (bootsNameLen + 21 + 3 + 5 + 3 + 1));

  weaponStats = (str) malloc((swp == NO_ITEM) ? 11 : (weaponNameLen + 75 + 5 + 5 + 6 + 5 + 3 + 3 + 3 + 1));
  // atk_crit: ###.##

  if (swp == NO_ITEM) sprintf(weaponStats, "Unequipped");
  else sprintf(weaponStats, "%s, ATK: %d, ACC: %d, ATK CRIT: %3.2f, ATK CRIT DMG: %d,\n\t\tLVL: %d, UPGRADES: %d, DURABILITY, %d", 
                            swp->name, swp->atk, swp->acc, swp->atk_crit, swp->atk_crit_dmg, swp->lvl, swp->upgrades, swp->durability);

  if (helmet == NO_ITEM) sprintf(helmetStats, "Unequipped");
  else sprintf(helmetStats, "%s; LVL: %d, ACC: %d, DEF: %d", helmet->name, helmet->lvl, helmet->acc, helmet->def);

  if (guard == NO_ITEM) sprintf(shoulderGuardStats, "Unequipped");
  else sprintf(shoulderGuardStats, "%s; LVL: %d, ACC: %d, DEF: %d", guard->name, guard->lvl, guard->acc, guard->def);

  if (chestplate == NO_ITEM) sprintf(chestplateStats, "Unequipped");
  else sprintf(chestplateStats, "%s; LVL: %d, ACC: %d, DEF: %d", chestplate->name, chestplate->lvl, chestplate->acc, chestplate->def);

  if (boots == NO_ITEM) sprintf(bootsStats, "Unequipped");
  else sprintf(bootsStats, "%s; LVL: %d, ACC: %d, DEF: %d", boots->name, boots->lvl, boots->acc, boots->def);

  printf("SoulWeapon: %s\n", weaponStats);
  printf("Helmet: %s\n", helmetStats);
  printf("Shoulder Guard: %s\n", shoulderGuardStats);
  printf("Chestplate: %s\n", chestplateStats);
  printf("Boots: %s\n", bootsStats);

  free(weaponStats);
  free(helmetStats);
  free(shoulderGuardStats);
  free(chestplateStats);
  free(bootsStats);
}

void viewSelf(SoulWorker* sw) {
  /**
   * [name], LVL [lvl]; [hp]/[maxHP]\n
   * XP: [xp]/[xpReq]; [dzenai] DZ\n
   * ATK: [atk]; DEF: [def]; ACC: [acc]; ATK CRIT DMG: [atk_crit_dmg]; ATK CRIT: [atk_crit]\n
   */

  Gear* gear = &(sw->gear);

  // Maybe something nicer looking or better??
  uint totalAtk = sw->stats->ATK + ((gear->sw != NO_ITEM) ? gear->sw->atk : 0);
  uint totalDef = sw->stats->DEF + ((gear->helmet != NO_ITEM) ? gear->helmet->def : 0) + 
                                   ((gear->guard != NO_ITEM) ? gear->guard->def : 0) + 
                                   ((gear->chestplate != NO_ITEM) ? gear->chestplate->def : 0) +
                                   ((gear->boots != NO_ITEM) ? gear->boots->def : 0);
  uint totalAcc = sw->stats->ACC + ((gear->sw != NO_ITEM) ? gear->sw->acc : 0) +                                   
                                   ((gear->helmet != NO_ITEM) ? gear->helmet->acc : 0) + 
                                   ((gear->guard != NO_ITEM) ? gear->guard->acc : 0) + 
                                   ((gear->chestplate != NO_ITEM) ? gear->chestplate->acc : 0) +
                                   ((gear->boots != NO_ITEM) ? gear->boots->acc : 0);
  uint totalCritDmg = sw->stats->ATK_CRIT_DMG + ((gear->sw != NO_ITEM) ? gear->sw->atk_crit_dmg : 0);
  float totalCrit = sw->stats->ATK_CRIT + ((gear->sw != NO_ITEM) ? gear->sw->atk_crit : 0);

  printf("%s, LVL %d; %s%d%s/%d\nXP: %d/%d; %d DZ\nATK: %d; DEF: %d; ACC: %d; ATK CRIT DMG: %d; ATK CRIT: %3.2f\n\n", 
          sw->name, sw->lvl, (sw->hp <= (sw->maxHP / 2)) ? RED : GREEN, sw->hp, RESET, sw->maxHP, 
          sw->xp, sw->xpReq, sw->dzenai,
          totalAtk, totalDef, totalAcc, totalCritDmg, totalCrit);
          
  viewGear(sw);
}

/**
 * Prints the skills for a single row.
 * @param skillTree The skill tree
 * @param start Starting index of the skills array
 * @param end Ending index (exclusive) of the skills array
 * @param COL_WIDTH Width of a column
 */
void skillRow(SkillTree* skillTree, int start, int end, int COL_WIDTH) {
  const str UNLOCKED = GREEN;
  const str LOCKED = RED;

// Print slot/id
  for (int i = start; i < end; i++) {
    printf("|%s%*d%*s%s", isSkillUnlocked(skillTree, i + 1) ? UNLOCKED : LOCKED, COL_WIDTH / 2, i + 1, COL_WIDTH / 2, " ", RESET);
  }
  printf("|\n");

  // Print name and level
  for (int i = start; i < end; i++) {
    printf("| %-*s LVL %d", COL_WIDTH - 7, skillTree->skills[i].name, skillTree->skills[i].lvl);
  }
  printf("|\n");

  // Print cooldown
  for (int i = start; i < end; i++) {
    printf("| CD: %-*d", COL_WIDTH - 5, skillTree->skills[i].cooldown);
  }
  printf("|\n");


  str activeEffect;
  effect_t _activeEffect;
  ushort effect;
  float effectF;

  // Print effect 1
  for (int i = start; i < end; i++) {
    _activeEffect = skillTree->skills[i].activeEffect1;

    switch (_activeEffect) {
      case ATK: // "ATK"
        // activeEffect = (str) malloc(4);
        activeEffect = "ATK";
        effect = skillTree->skills[i].effect1.atk;
        break;
      case ATK_CRIT_DMG: // "ATK CRIT DMG"
        activeEffect = "ATK CRIT DMG";
        effect = skillTree->skills[i].effect1.atk_crit_dmg;
        break;
      default:
        break;
    }

    printf("| %s: %-*d", activeEffect, COL_WIDTH - 6, effect);
  }
  printf("|\n");

  // Print effect 2
  for (int i = start; i < end; i++) {
    _activeEffect = skillTree->skills[i].activeEffect2;

    switch (_activeEffect) {
      case DEF: // "DEF"
        activeEffect = "DEF";
        effect = skillTree->skills[i].effect2.def;
        break;
      case ACC: // "ACC"
        activeEffect = "ACC";
        effect = skillTree->skills[i].effect2.acc;
        break;
      case ATK_CRIT: // "ATK CRIT"
        activeEffect = "ATK CRIT";
        effectF = skillTree->skills[i].effect2.atk_crit;
        break;
      default:
        break;
    }

    if (_activeEffect == ATK_CRIT) printf("| %s: %-*.2f", activeEffect, (COL_WIDTH / 2) - 1, effectF);
    else printf("| %s: %-*d", activeEffect, COL_WIDTH - 6, effect);
  }
  printf("|\n");

  // Print seperator
  printf("|");
  for (int i = start; i < end; i++) {
    for (int j = 0; j < COL_WIDTH + 1; j++) {
      printf("-");
    }
  }
  printf("|\n");
}

void viewSkills(SkillTree* skillTree) {
  const int COL_WIDTH = 20;

  /**
   * Skills:
   * |            1               |             2              |
   * | [name], LVL [lvl]          | [name], LVL [lvl]          | ...
   * | CD: [cooldown]             | CD: [cooldown]             |
   * | [activeEffect1]: [effect1] | [activeEffect1]: [effect1] |
   * | [activeEffect2]: [effect2] | [activeEffect2]: [effect2] |
   * -------------------------------------------------------------------
   * [repeat]
   * ...
   * Active skills:
   * | [skill 1] | [skill2] | [skill3] | [skill4] | [skill5] |
   * Current Skill Points: []
   */

  printf("%sSkills:%s\n", CYAN, RESET);

  skillRow(skillTree, 0, TOTAL_SKILLS / 2, COL_WIDTH);
  skillRow(skillTree, TOTAL_SKILLS / 2, TOTAL_SKILLS, COL_WIDTH);

  printf("Active skills:\n");
  for (int i = 0; i < EQUIPPED_SKILL_COUNT; i++) {
    if (skillTree->equippedSkills[i] == NO_SKILL) printf("| %-*s", COL_WIDTH - 1, "");
    else printf("| %-*s", COL_WIDTH - 1, skillTree->equippedSkills[i]->name);
  }
  printf("|\n");

  printf("Current Skill Points: %d\n", skillTree->totalSkillPoints);

}

void viewSkill(Skill* skill) {
  const int COL_WIDTH = 30;

  /**
   * |            #               |
   * | [name], LVL [lvl]          |
   * | [activeEffect1]: [effect1] |
   * | [activeEffect2]: [effect2] |
   * | [desc]                     |
   * | CD: [cooldown]             |
   */

  
  printf("|%*d%-*s|\n", COL_WIDTH / 2, skill->id, COL_WIDTH / 2 - 1, " ");

  printf("| %s, LVL %-*d|\n", skill->name, COL_WIDTH / 2, skill->lvl);

  str activeEffect;
  effect_t _activeEffect;
  ushort effect;
  float effectF;

  _activeEffect = skill->activeEffect1;

  switch (_activeEffect) {
    case ATK: // "ATK"
      // activeEffect = (str) malloc(4);
      activeEffect = "ATK";
      effect = skill->effect1.atk;
      break;
    case ATK_CRIT_DMG: // "ATK CRIT DMG"
      activeEffect = "ATK CRIT DMG";
      effect = skill->effect1.atk_crit_dmg;
      break;
    default:
      break;
  }

  printf("| %s: %-*d|\n", activeEffect, COL_WIDTH - 7, effect);

  _activeEffect = skill->activeEffect2;

  switch (_activeEffect) {
    case DEF: // "DEF"
      activeEffect = "DEF";
      effect = skill->effect2.def;
      break;
    case ACC: // "ACC"
      activeEffect = "ACC";
      effect = skill->effect2.acc;
      break;
    case ATK_CRIT: // "ATK CRIT"
      activeEffect = "ATK CRIT";
      effectF = skill->effect2.atk_crit;
      break;
    default:
      break;
  }

  if (_activeEffect == ATK_CRIT) printf("| %s: %-*.2f|\n", activeEffect, (COL_WIDTH / 2) - 2, effectF);
  else printf("| %s: %-*d|\n", activeEffect, COL_WIDTH - 7, effect);

  // description goes here

  printf("| CD: %-*d|\n", COL_WIDTH - 6, skill->cooldown);
}

// Keep player and equipped gear stats seperate (for leveling sake)
//  but when displaying stats, include all stats (combine)

void setSkill(SkillTree* skillTree, Skill* skill, uint slot) {
  // Check that the slot and skill is appropriate beforehand

  // Set skill
  skillTree->equippedSkills[slot - 1] = skill;

  printf("Skill %s set at slot %d!\n", skill->name, slot);
}

bool isSkillUnlocked(SkillTree* skillTree, uint skillNum) {
  ushort skillStatus = skillTree->skillStatus;

  ushort bitmask = 0x1 << (skillNum - 1);

  return (bool) ((skillStatus & bitmask) >> (skillNum - 1));
}

void skillUnlock(SkillTree* skillTree, uint skillNum) {
  const int SKILL_COST = 10;

  if (isSkillUnlocked(skillTree, skillNum)) {
    printf("Skill has already been unlocked!\n");
    return;
  }

  if (skillTree->totalSkillPoints - SKILL_COST < 0) {
    printf("You do not have sufficient skill points!\n");
    return;
  }

  // The cost of unlocking a skill
  skillTree->totalSkillPoints -= SKILL_COST;

  ushort bitmask = 0x1 << (skillNum - 1);

  skillTree->skillStatus |= bitmask;

  // printf("0x%x\n", skillTree->skillStatus);

  printf("Skill %s is now unlocked!\n", skillTree->skills[skillNum - 1].name);

}

void upgradeSkill(SkillTree* skillTree, uint skillNum) {
  if (!isSkillUnlocked(skillTree, skillNum)) {
    printf("You can't upgrade a locked skill!\n");
    return;
  }

  Skill* skill = &skillTree->skills[skillNum - 1];

  if (skill->lvl == 10) {
    printf("Skill is maxed out!\n");
    return;
  }

  skill->lvl++;

  // TODO: Play around with numbers
  float temp;
  float scaleFactor = 1.0f + 0.1f * skill->lvl;

  if (skill->activeEffect1 == ATK) {
    temp = skill->effect1.atk;
    skill->effect1.atk = (ushort) skill->effect1.atk * scaleFactor;
    printf("ATK: %d -> %d\n", (uint) temp, skill->effect1.atk);
  } else { // activeEffect1 == atk_crit_dmg
    temp = skill->effect1.atk_crit_dmg;
    skill->effect1.atk_crit_dmg = (ushort) skill->effect1.atk_crit_dmg * scaleFactor;
    printf("ATK CRIT DMG: %d -> %d\n", (uint) temp, skill->effect1.atk_crit_dmg);
  }

  if (skill->activeEffect2 == DEF) {
    temp = skill->effect2.def;
    skill->effect2.def = (ushort) skill->effect2.def * scaleFactor;
    printf("DEF: %d -> %d\n", (uint) temp, skill->effect2.def);
  } else if (skill->activeEffect2 == ACC) {
    temp = skill->effect2.acc;
    skill->effect2.acc = (ushort) skill->effect2.acc * scaleFactor;
    printf("ACC: %d -> %d\n", (uint) temp, skill->effect2.acc);
  } else { // activeEffect2 = atk_crit
    temp = skill->effect2.atk_crit;
    skill->effect2.atk_crit = skill->effect2.atk_crit * scaleFactor;
    printf("ATK CRIT: %.2f -> %.2f\n", temp, skill->effect2.atk_crit);
  }

  printf("Skill upgraded to level %d!\n", skill->lvl);
}

void unequipGear(SoulWorker* sw) {
  Item* gear = (Item*) malloc(sizeof(Item));
  if (!gear) {
    // Unsure whether this should be a fatal error or just a warning, allowing gameplay
    // to continue but not allow unequipping
    handleError(ERR_MEM, WARNING, "Could not allocate space for gear when unequipping!\n");
    return;
  }

  gear->count = 1;

  // Do not try unequipping when gear already unequipped
  if (sw->gear.sw != NO_ITEM) {
    gear->type = SOULWEAPON_T;
    gear->_item = sw->gear.sw;
    if(!addToInv(sw, gear)) { free(gear); return; }
  }

  if (sw->gear.helmet != NO_ITEM) {
    gear->type = HELMET_T;
    gear->_item = sw->gear.helmet;
    if(!addToInv(sw, gear)) { free(gear); return; }
  }

  if (sw->gear.guard != NO_ITEM) {
    gear->type = SHOULDER_GUARD_T;
    gear->_item = sw->gear.guard;
    if(!addToInv(sw, gear)) { free(gear); return; }
  }

  if (sw->gear.chestplate != NO_ITEM) {
    gear->type = CHESTPLATE_T;
    gear->_item = sw->gear.chestplate;
    if(!addToInv(sw, gear)) { free(gear); return; }
  }

  if (sw->gear.boots != NO_ITEM) {
    gear->type = BOOTS_T;
    gear->_item = sw->gear.boots;
    if(!addToInv(sw, gear)) { free(gear); return; }
  }

  sw->gear.sw = NO_ITEM;
  sw->gear.helmet = NO_ITEM;
  sw->gear.guard = NO_ITEM;
  sw->gear.chestplate = NO_ITEM;
  sw->gear.boots = NO_ITEM;

  free(gear);
  gear = NULL;
}

void equipGear(SoulWorker* sw, Item* item) {
  Gear* gear = &(sw->gear);
  
  // When equipping a gear piece that already is filled in the player's gear
  //  ie. equipping a SoulWeapon when there is one already equipped
  // simply swap the void* between both
  // But when equipping in a blank slot, transfer/copy the void* pointer from inv
  //  to gear.*, then "blanking" the inv slot (null out void* and 0 out type and count)

  void* temp = item->_item; // Hold the requested gear piece to equip

  switch (item->type) {
    case SOULWEAPON_T:
      if (gear->sw != NO_ITEM) { // One equipped already, swap
        item->_item = gear->sw; // Inv item slot will now hold equipped soulweapon
        gear->sw = temp; // Soulweapon gear slot will now hold requested inv item soulweapon
      } else { gear->sw = (SoulWeapon*) temp; temp = NULL; }
      break;
    case HELMET_T:
      if (gear->helmet != NO_ITEM) {
        item->_item = gear->helmet;
        gear->helmet = temp;
      } else { gear->helmet = (Armor*) temp; temp = NULL; }
      break;
    case SHOULDER_GUARD_T:
      if (gear->guard != NO_ITEM) {
        item->_item = gear->guard;
        gear->guard = temp;
      } else { gear->guard = (Armor*) temp; temp = NULL; }
      break;
    case CHESTPLATE_T:
      if (gear->chestplate != NO_ITEM) {
        item->_item = gear->chestplate;
        gear->chestplate = temp;
      } else { gear->chestplate = (Armor*) temp; temp = NULL; }
      break;
    case BOOTS_T:
      if (gear->boots != NO_ITEM) {
        item->_item = gear->boots;
        gear->boots = temp;
      } else { gear->boots = (Armor*) temp; temp = NULL; }
      break;
    default:
      break;
  }

  if (!temp) { // NULL only when not swapping, meaning inv slot is to be empty
    item->_item = NO_ITEM;
    item->type = NONE;
    item->count = 0;
    sw->invCount--;
  }

  printf("Equipped!\n");
}

void heal(SoulWorker* sw, Item* item) {
  // item is guaranteed to contain the hp kit
  HPKit* hpKit = (HPKit*) item->_item;

  uint hpIncr;

  // TODO: Synch how much each level heals with player's HP level and max increase
  switch (hpKit->type) {
    case DEKA:
      hpIncr = 10;
      break;
    case MEGA:
      hpIncr = 240;
      break;
    case PETA:
      hpIncr = 1000;
      break;
    default:
      break;
  }

  sw->hp = (sw->hp + hpIncr < sw->maxHP) ? sw->hp + hpIncr : sw->maxHP;
  printf("+%d!\n", hpIncr);

  removeFromInv(sw, item, 1);
}

/**
 * Formula for how much xp is required based on the given level for the next level.
 * @param lvl The current level
 * @return XP required for the level
 */
static uint xpRequired(uint lvl) {
  uint xpReq;

  uint A = 15; // Acceleration of XP req growth
  uint B = 0; // Base XP req increment
  uint C = 0; // Min XP req'd at level 1

  xpReq = A * (lvl * lvl) + B + lvl + C;

  return xpReq;
}

/**
 * Levels up the player, increasing the stats as necessary.
 * @param sw The player
 */
static void levelUp(SoulWorker* sw) {
  // TODO: play around with numbers and formula

  sw->lvl++;

  sw->skills->totalSkillPoints += 5;

  int baseHPGain = -2;
  // sw->maxHP = sw->hp = sw->maxHP + baseHPGain + (sw->lvl * 2);
  sw->maxHP = sw->hp = sw->maxHP + baseHPGain + (2 * sw->lvl * sw->lvl);

  float growthFactor = 1.01 + 0.01 * (1.0 / sw->lvl);

  Stats* stats = sw->stats;

  stats->ATK += stats->ATK * growthFactor * (1 + (1) / 100.0);
  stats->ACC += stats->ACC * growthFactor * (1 + (1) / 100.0);
  stats->ATK_CRIT += stats->ATK_CRIT * growthFactor; // * (1 + (rand() % 1) / 100.0);
  stats->ATK_CRIT_DMG += stats->ATK_CRIT_DMG * growthFactor * (1 + (rand() % 1) / 100.0);
  stats->DEF += stats->DEF * growthFactor * (1 + (1) / 100.0);
}

void updateXP(SoulWorker* sw, uint xp) {
  uint xpLeftover = xp;

  bool leveledUp = false;

  // Keep leveling up until the acquired xp is no longer sufficient
  while (true) {
    sw->xp += xpLeftover;

    uint xpReqd = xpRequired(sw->lvl);
    // printf("Total XP required for next level (%d): %d\n", (sw->lvl)+1, xpReqd);
    // printf("Need %d more\n", xpReqd - sw->xp);

    // Current XP passes XP req'd for next level, time to increase level
    if (sw->xp >= xpReqd) {
      levelUp(sw);
      leveledUp = true;

      xpLeftover = sw->xp - xpReqd;
      // printf("XP leftover from level up: %d\n", xpLeftover);
      sw->xp = 0;
      sw->xpReq = xpRequired(sw->lvl);
      // printf("New XP required: %d\n", sw->xpReq);
      // sw->xp = xpLeftover;
    } else break;
  }

  if (leveledUp) printf("Leveled up to LVL %d!\n", sw->lvl);
}

/**
 * Initiates the skill tree for the player.
 * The skills are predetermined. It is only a matter of unlocking.
 * @return The skill tree
 */
static SkillTree* initSkillTree() {
  const int SIZE = 4096;

  SkillTree* skillTree = (SkillTree*) malloc(sizeof(SkillTree));
  if (!skillTree) handleError(ERR_MEM, FATAL, "Could not allocate space for skill tree!\n");

  for (int i = 0; i < EQUIPPED_SKILL_COUNT; i++) {
    skillTree->equippedSkills[i] = NO_SKILL;
  }


  FILE* skillsFile = fopen(SKILL_DATA, "r");
  if (!skillsFile) handleError(ERR_IO, FATAL, "Could not open skills data!\n");

  str buffer = (str) malloc(SIZE);
  if (!buffer) handleError(ERR_MEM, FATAL, "Could not allocate space for skills buffer!\n");

  str name = NULL, *description = NULL;
  byte lvl, cooldown, id;
  ushort effect1;
  float effect2;
  effect_t activeEffect1, activeEffect2;

  char* pos;

  for (int i = 0; i < TOTAL_SKILLS; i++) {
    // printf("Iteration %d\n", i);

    fgets(buffer, SIZE, skillsFile);
    if ((pos = strchr(buffer, '\n')) != NULL) *pos = '\0';
#ifdef _WIN64
    name = _strdup(buffer);
#else
    name = strdup(buffer);
#endif
    // printf("name: %s\n", name);

    fgets(buffer, SIZE, skillsFile);
    if ((pos = strchr(buffer, '\n')) != NULL) *pos = '\0';
    // printf("Buffer for desc: %s\n", buffer);
#ifdef _WIN64
    description = _strdup(buffer);
#else
    description = strdup(buffer);
#endif
    // printf("description: %s\n", description);

    fgets(buffer, 5, skillsFile);
    lvl = (byte) atoi(buffer);
    // printf("lvl: %d\n", lvl);

    fgets(buffer, 5, skillsFile);
    cooldown = (byte) atoi(buffer);
    // printf("cooldown: %d\n", cooldown);

    fgets(buffer, 5, skillsFile);
    id = (byte) atoi(buffer);
    // printf("id: %d\n", id);

    fgets(buffer, 7, skillsFile);
    effect1 = (ushort) atoi(buffer);
    // printf("effect 1: %d\n", effect1);

    fgets(buffer, SIZE, skillsFile);
    if (strchr(buffer, '.')) effect2 = (float) atof(buffer);
    else effect2 = (float) ((ushort) atoi(buffer));
    // printf("effect 2: \n");

    fgets(buffer, 3, skillsFile);
    activeEffect1 = (effect_t) atoi(buffer);
    // printf("active effect 1: %d\n", activeEffect1);

    fgets(buffer, 3, skillsFile);
    activeEffect2 = (effect_t) atoi(buffer);
    // printf("active effect 2: %d\n", activeEffect2);

    // For empty newline
    fgets(buffer, 2, skillsFile);
    // printf("\n");

    initSkill(&skillTree->skills[i], name, description, lvl, cooldown, effect1, effect2, activeEffect1, activeEffect2, id);
  }

  free(buffer);
  fclose(skillsFile);

  skillTree->skillStatus = 0x0000;
  skillTree->totalSkillPoints = 20; // OG is 0

  return skillTree;
}

/**
 * 
 * @param skillTree 
 */
static void deleteSkillTree(SkillTree* skillTree) {
  deleteSkills(skillTree->skills, TOTAL_SKILLS);

  free(skillTree);
}

void deleteSoulWorker(SoulWorker* sw) {
  if (!sw) return;

  free(sw->name);
  free(sw->stats);
  // room will be taken care of by Maze cleanup
  deleteSoulWeapon(sw->gear.sw);
  deleteArmor(sw->gear.helmet);
  deleteArmor(sw->gear.guard);
  deleteArmor(sw->gear.chestplate);
  deleteArmor(sw->gear.boots);
  
  for (int i = 0; i < INV_CAP; i++) {
    void* _item = sw->inv[i]._item;

    switch (sw->inv[i].type) {
      case SOULWEAPON_T:
        deleteSoulWeapon((SoulWeapon*) _item);
        break;
      case HELMET_T:
      case SHOULDER_GUARD_T:
      case CHESTPLATE_T:
      case BOOTS_T:
        deleteArmor((Armor*) _item);
        break;
      case HP_KITS_T:
      case WEAPON_UPGRADE_MATERIALS_T:
      case ARMOR_UPGRADE_MATERIALS_T:
      case SLIME_T:
        deleteOther((HPKit*) _item);
        break;
      default:
        break;
    }
  }

  deleteSkillTree(sw->skills);

  free(sw);
}