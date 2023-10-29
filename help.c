/* help.c: identify a symbol

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "xmoria.h"
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"


void ident_char()
{
  char command, query;
  register int i, n;

  if (get_com(Status_window, "Enter character to be identified :", &command))
    switch(command)
      {
	/* every printing ASCII character is listed here, in the order in which
	   they appear in the ASCII character set */
      case ' ':
	msg_print("  - An open pit.");
	break;
      case '!':
	msg_print("! - A potion.");
	break;
      case '"':
	msg_print("\" - An amulet, periapt, or necklace.");
	break;
      case '#':
	msg_print("# - A stone wall.");
	break;
      case '$':
	msg_print("$ - Treasure.");
	break;
      case '%':
	if (highlight_seams == TRUE)
	  msg_print("% - A magma or quartz vein.");
	else
	  msg_print("% - Not used.");
	break;
      case '&':
	msg_print("& - Treasure chest.");
	break;
      case '\'':
	msg_print("' - An open door.");
	break;
      case '(':
	msg_print("( - Soft armor.");
	break;
      case ')':
	msg_print(") - A shield.");
	break;
      case '*':
	msg_print("* - Gems.");
	break;
      case '+':
	msg_print("+ - A closed door.");
	break;
      case ',':
	msg_print(", - Food or mushroom patch.");
	break;
      case '-':
	msg_print("- - A wand");
	break;
      case '.':
	msg_print(". - Floor.");
	break;
      case '/':
	msg_print("/ - A pole weapon.");
	break;
	/* case '0': msg_print("0 - Not used.", 0, 0); b*/
      case '1':
	msg_print("1 - Entrance to General Store.");
	break;
      case '2':
	msg_print("2 - Entrance to Armory.");
	break;
      case '3':
	msg_print("3 - Entrance to Weaponsmith.");
	break;
      case '4':
	msg_print("4 - Entrance to Temple.");
	break;
      case '5':
	msg_print("5 - Entrance to Alchemy shop.");
	break;
      case '6':
	msg_print("6 - Entrance to Magic-Users store.");
	break;
      case ':':
	msg_print(": - Rubble.");
	break;
      case ';':
	msg_print("; - A loose rock.");
	break;
      case '<':
	msg_print("< - An up staircase.");
	break;
      case '=':
	msg_print("= - A ring.");
	break;
      case '>':
	msg_print("> - A down staircase.");
	break;
      case '?':
	msg_print("? - A scroll.");
	break;
      case '@':
	msg_print(py.misc.name);
	break;
      case 'A':
	msg_print("A - Giant Ant Lion.");
	break;
      case 'B':
	msg_print("B - The Balrog.");
	break;
      case 'C':
	msg_print("C - Gelatinous Cube.");
	break;
      case 'D':
	msg_print("D - An Ancient Dragon (Beware).");
	break;
      case 'E':
	msg_print("E - Elemental.");
	break;
      case 'F':
	msg_print("F - Giant Fly.");
	break;
      case 'G':
	msg_print("G - Ghost.");
	break;
      case 'H':
	msg_print("H - Hobgoblin.");
	break;
      case 'J':
	msg_print("J - Jelly.");
	break;
      case 'K':
	msg_print("K - Killer Beetle.");
	break;
      case 'L':
	msg_print("L - Lich.");
	break;
      case 'M':
	msg_print("M - Mummy.");
	break;
      case 'O':
	msg_print("O - Ooze.");
	break;
      case 'P':
	msg_print("P - Giant humanoid.");
	break;
      case 'Q':
	msg_print("Q - Quylthulg (Pulsing Flesh Mound).");
	break;
      case 'R':
	msg_print("R - Reptile.");
	break;
      case 'S':
	msg_print("S - Giant Scorpion.");
	break;
      case 'T':
	msg_print("T - Troll.");
	break;
      case 'U':
	msg_print("U - Umber Hulk.");
	break;
      case 'V':
	msg_print("V - Vampire.");
	break;
      case 'W':
	msg_print("W - Wight or Wraith.");
	break;
      case 'X':
	msg_print("X - Xorn.");
	break;
      case 'Y':
	msg_print("Y - Yeti.");
	break;
      case '[':
	msg_print("[ - Hard armor.");
	break;
      case '\\':
	msg_print("\\ - A hafted weapon.");
	break;
      case ']':
	msg_print("] - Misc. armor.");
	break;
      case '^':
	msg_print("^ - A trap.");
	break;
      case '_':
	msg_print("_ - A staff.");
	break;
      case 'a':
	msg_print("a - Giant Ant.");
	break;
      case 'b':
	msg_print("b - Giant Bat.");
	break;
      case 'c':
	msg_print("c - Giant Centipede.");
	break;
      case 'd':
	msg_print("d - Dragon.");
	break;
      case 'e':
	msg_print("e - Floating Eye.");
	break;
      case 'f':
	msg_print("f - Giant Frog");
	break;
      case 'g':
	msg_print("g - Golem.");
	break;
      case 'h':
	msg_print("h - Harpy.");
	break;
      case 'i':
	msg_print("i - Icky Thing.");
	break;
      case 'j':
	msg_print("j - Jackal.");
	break;
      case 'k':
	msg_print("k - Kobold.");
	break;
      case 'l':
	msg_print("l - Giant Louse.");
	break;
      case 'm':
	msg_print("m - Mold.");
	break;
      case 'n':
	msg_print("n - Naga.");
	break;
      case 'o':
	msg_print("o - Orc or Ogre.");
	break;
      case 'p':
	msg_print("p - Person (Humanoid).");
	break;
      case 'q':
	msg_print("q - Quasit.");
	break;
      case 'r':
	msg_print("r - Rodent.");
	break;
      case 's':
	msg_print("s - Skeleton.");
	break;
      case 't':
	msg_print("t - Giant Tick.");
	break;
      case 'w':
	msg_print("w - Worm or Worm Mass.");
	break;
      case 'y':
	msg_print("y - Yeek.");
	break;
      case 'z':
	msg_print("z - Zombie.");
	break;
      case '{':
	msg_print("{ - Arrow, bolt, or bullet.");
	break;
      case '|':
	msg_print("| - A sword or dagger.");
	break;
      case '}':
	msg_print("} - Bow, crossbow, or sling.");
	break;
      case '~':
	msg_print("~ - Miscellaneous item.");
	break;
      default:
	msg_print("Not Used.");
	break;
      }

  /* Allow access to monster memory. -CJS- */
  n = 0;
  for (i = MAX_CREATURES-1; i >= 0; i--)
    if ((c_list[i].cchar == command) && bool_roff_recall (i))
      {
	if (n == 0)
	  {
	    put_buffer (Status_window, "You recall those details? [y/n]",
			1, 40);
	    query = inkey(TRUE);
	    if (query != 'y' && query != 'Y')
	      break;
	  }
	n++;
	query = roff_recall (i);
	if (query == ESCAPE)
	  break;
      }
}
