#include "Xprogs.h"
 
#define BITMAP_PATH	"/afs/rpi.edu/home/33/lordj/SparcWorks/xmoria/pixmaps/"
#define BITMAP_TYPE	".bmp"
#define OPENING_HEADER_STRING	"XMoria 1.07"
#define WIDTH		560
#define HEIGHT		205
 
#ifdef MAIN
int Done = 0;
XFontStruct *BigFont;
GC BFGC, redGC, goldGC, doorGC, wallGC, silverGC, tanGC, electricblueGC;
GC woodGC, clearGC, greenGC, floorGC;
unsigned BigFontheight;
unsigned Height, Width;
unsigned long newcolor, tancolor, silvercolor;
Pix bigscroll, amulet, bow, coins, closed_door, open_door, scroll, chest;
Pix gem, potion, shield, down_stairs, sword, wand, bigsword, arrow;
Pix monsters[51], magicbook, polearm, prayerbook, staff, up_stairs;
Pix bigbook, stone, stipple, bigperson, backpack, worldmap, exit_pix;
Pix bigpotion, mage, warrior, ring, floor, rubble, solid, question;
Pix magicstore, weaponstore, trap, alchemistshop, armorer, marmor, paladin;
Pix priest, ranger, rogue, templeshop, loose_rock, food, generalstore;
Pix miscarmor, quartz, softarmor, hafted_weapon, misc_item;
XWIN *buttons[15];
DWin Stats_window, Map_window, Status_window, Spell_win;
DWin Char_window, Inven_window, Flags_window, Store_window;
DWin mapscreen, Memory_window;
 
static char *stat_names[] = { "STR", "INT", "WIS",
				 "DEX", "CON", "CHR" };
 
static char *monster_files[] =
{
  "ant",
  "bat",
  "centipede",
  "smalldragon",
  "eye",
  "frog",
  "golem",
  "harpy",
  "ickything",
  "jackal",
  "kobold",
  "louse",
  "mold",
  "naga",
  "ogre",
  "person",
  "quasit",
  "rat",
  "skeleton",
  "tick",
  " ",
  " ",
  "worm",
  " ",
  "yeek",
  "zombie",
  "antlion",
  "balrog",
  "cube",
  "bigdragon",
  "elemental",
  "fly",
  "ghost",
  "hobgoblin",
  " ",
  "jelly",
  "beetle",
  "lich",
  "mummy",
  " ",
  "ooze",
  "giant",
  "quylthulg",
  "snake",
  "scorpion",
  "troll",
  "umberhulk",
  "vampire",
  "wraith",
  "xorn",
  "yeti"
  };
#else
extern int Done;
extern XFontStruct *BigFont;
extern GC BFGC, redGC, goldGC, doorGC, wallGC, silverGC, tanGC, electricblueGC;
extern GC woodGC, clearGC, greenGC, floorGC;
extern unsigned BigFontheight;
extern unsigned Height, Width;
extern unsigned long newcolor, tancolor, silvercolor;
extern Pix bigscroll, amulet, bow, coins, closed_door, open_door, scroll;
extern Pix gem, potion, shield, down_stairs, sword, wand, bigsword, arrow;
extern Pix monsters[51], magicbook, polearm, prayerbook, staff, up_stairs;
extern Pix bigbook, stone, stipple, bigperson, backpack, worldmap, exit_pix;
extern Pix bigpotion, mage, warrior, ring, floor, rubble, solid, question;
extern Pix magicstore, weaponstore, trap, alchemistshop, armorer, marmor;
extern Pix priest, ranger, rogue, templeshop, paladin, loose_rock, food;
extern Pix generalstore, miscarmor, quartz, softarmor, hafted_weapon;
extern Pix misc_item, chest;
extern XWIN buttons[15];
extern DWin Stats_window, Map_window, Status_window, Spell_win;
extern DWin Char_window, Inven_window, Flags_window, Store_window;
extern DWin mapscreen, Memory_window;
#endif
