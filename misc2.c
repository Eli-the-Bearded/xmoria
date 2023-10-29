#include "xmoria.h"
/* misc2.c: misc code for maintaining the dungeon, printing player info

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "config.h"

/* include before constant.h because param.h defines NULL incorrectly */
#ifndef USG
#include <sys/types.h>
#include <sys/param.h>
#endif

#include "constant.h"
#include "types.h"
#include "externs.h"

#include <ctype.h>

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#else
char *index();
#endif
#else
#include <strings.h>
#endif

#ifdef AIXV3
char *index(char *, char);
#else
char *index();
#endif

static char *stat_names[] = { "STR : ", "INT : ", "WIS : ",
				 "DEX : ", "CON : ", "CHR : " };
#define BLANK_LENGTH	24
static char blank_string[] = "                        ";

char *malloc();

/* Places a particular trap at location y, x		-RAK-	*/
void place_trap(y, x, subval)
int y, x, subval;
{
  register int cur_pos;

  cur_pos = popt();
  cave[y][x].tptr  = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_TRAP_LIST + subval);
}


/* Places rubble at location y, x			-RAK-	*/
void place_rubble(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  cave_ptr->fval = BLOCKED_FLOOR;
  invcopy(&t_list[cur_pos], OBJ_RUBBLE);
}


/* Places a treasure (Gold or Gems) at given row, column -RAK-	*/
void place_gold(y, x)
int y, x;
{
  register int i, cur_pos;
  register inven_type *t_ptr;

  cur_pos = popt();
  i = ((randint(dun_level+2)+2) / 2) - 1;
  if (randint(OBJ_GREAT) == 1)
    i += randint(dun_level+1);
  if (i >= MAX_GOLD)
    i = MAX_GOLD - 1;
  cave[y][x].tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_GOLD_LIST+i);
  t_ptr = &t_list[cur_pos];
  t_ptr->cost += (8L * (long)randint((int)t_ptr->cost)) + randint(8);
  if (cave[y][x].cptr == 1)
    msg_print ("You feel something roll beneath your feet.");
}


/* Returns the array number of a random object		-RAK-	*/
int get_obj_num(level)
int level;
{
  register int i, j;

  if (level == 0)
    i = randint(t_level[0]) - 1;
  else
    {
      if (level >= MAX_OBJ_LEVEL)
	level = MAX_OBJ_LEVEL;
      else if (randint(OBJ_GREAT) == 1)
	{
	  level = level * MAX_OBJ_LEVEL / randint (MAX_OBJ_LEVEL) + 1;
	  if (level > MAX_OBJ_LEVEL)
	    level = MAX_OBJ_LEVEL;
	}

      /* This code has been added to make it slightly more likely to get the
	 higher level objects.	Originally a uniform distribution over all
	 objects less than or equal to the dungeon level.  This distribution
	 makes a level n objects occur approx 2/n% of the time on level n,
	 and 1/2n are 0th level. */

      if (randint(2) == 1)
	i = randint(t_level[level]) - 1;
      else /* Choose three objects, pick the highest level. */
	{
	  i = randint(t_level[level]) - 1;
	  j = randint(t_level[level]) - 1;
	  if (i < j)
	    i = j;
	  j = randint(t_level[level]) - 1;
	  if (i < j)
	    i = j;
	  j = object_list[sorted_objects[i]].level;
	  if (j == 0)
	    i = randint(t_level[0]) - 1;
	  else
	    i = randint(t_level[j]-t_level[j-1]) - 1 + t_level[j-1];
	}
    }
  return(i);
}


/* Places an object at given row, column co-ordinate	-RAK-	*/
void place_object(y, x)
int y, x;
{
  register int cur_pos, tmp;

  cur_pos = popt();
  cave[y][x].tptr = cur_pos;
  /* split this line up to avoid a reported compiler bug */
  tmp = get_obj_num(dun_level);
  invcopy(&t_list[cur_pos], sorted_objects[tmp]);
  magic_treasure(cur_pos, dun_level);
  if (cave[y][x].cptr == 1)
    msg_print ("You feel something roll beneath your feet.");	/* -CJS- */
}


/* Allocates an object for tunnels and rooms		-RAK-	*/
void alloc_object(alloc_set, typ, num)
int (*alloc_set)();
int typ, num;
{
  register int i, j, k;

  for (k = 0; k < num; k++)
    {
      do
	{
	  i = randint(cur_height) - 1;
	  j = randint(cur_width) - 1;
	}
      /* don't put an object beneath the player, this could cause problems
	 if player is standing under rubble, or on a trap */
      while ((!(*alloc_set)(cave[i][j].fval)) ||
	     (cave[i][j].tptr != 0) || (i == char_row && j == char_col));
      if (typ < 4) {	/* typ == 2 not used, used to be visible traps */
	if (typ == 1) place_trap(i, j, randint(MAX_TRAP)-1); /* typ == 1 */
	else	      place_rubble(i, j); /* typ == 3 */
      } else {
	if (typ == 4) place_gold(i, j); /* typ == 4 */
	else	      place_object(i, j); /* typ == 5 */
      }
    }
}


/* Creates objects nearby the coordinates given		-RAK-	*/
void random_object(y, x, num)
int y, x, num;
{
  register int i, j, k;
  register cave_type *cave_ptr;

  do
    {
      i = 0;
      do
	{
	  j = y - 3 + randint(5);
	  k = x - 4 + randint(7);
	  cave_ptr = &cave[j][k];
	  if ((cave_ptr->fval <= MAX_CAVE_FLOOR) && (cave_ptr->tptr == 0))
	    {
	      if (randint(100) < 75)
		place_object(j, k);
	      else
		place_gold(j, k);
	      i = 9;
	    }
	  i++;
	}
      while(i <= 10);
      num--;
    }
  while (num != 0);
}

/* Converts stat num into string			-RAK-	*/
void cnv_stat(stat, out_val)
int8u stat;
char *out_val;
{
  register int part1, part2;

  if (stat > 18)
    {
      part1 = 18;
      part2 = stat - 18;
      if (part2 == 100)
	(void) strcpy(out_val, "18/100");
      else
	(void) sprintf(out_val, " %2d/%02d", part1, part2);
    }
  else
    (void) sprintf(out_val, "%6d", stat);
}

/* Print character info in given row, column		-RAK-	*/
/* the longest title is 13 characters, so only pad to 13 */
void prt_field(win, info, row, column)
DWin win;
char *info;
int row, column;
{
  put_buffer (win, &blank_string[BLANK_LENGTH-13], row, column);
  put_buffer (win, info, row, column);
}

/* Print long number with header at given row, column */
static void prt_lnum(win, header, num, row, column)
DWin win;
char *header;
int32 num;
int row, column;
{
  vtype out_val;

  (void) sprintf(out_val, "%s: %6ld", header, num);
  put_buffer(win, out_val, row, column);
}

/* Print number with header at given row, column	-RAK-	*/
static void prt_num(win, header, num, row, column)
DWin win;
char *header;
int num, row, column;
{
  vtype out_val;

  (void) sprintf(out_val, "%s: %6d", header, num);
  put_buffer(win, out_val, row, column);
}

/* Adjustment for wisdom/intelligence				-JWT-	*/
int stat_adj(stat)
int stat;
{
  register int value;

  value = py.stats.use_stat[stat];
  if (value > 117)
    return(7);
  else if (value > 107)
    return(6);
  else if (value > 87)
    return(5);
  else if (value > 67)
    return(4);
  else if (value > 17)
    return(3);
  else if (value > 14)
    return(2);
  else if (value > 7)
    return(1);
  else
    return(0);
}


/* Adjustment for charisma				-RAK-	*/
/* Percent decrease or increase in price of goods		 */
int chr_adj()
{
  register int charisma;

  charisma = py.stats.use_stat[A_CHR];
  if (charisma > 117)
    return(90);
  else if (charisma > 107)
    return(92);
  else if (charisma > 87)
    return(94);
  else if (charisma > 67)
    return(96);
  else if (charisma > 18)
    return(98);
  else
    switch(charisma)
      {
      case 18:	return(100);
      case 17:	return(101);
      case 16:	return(102);
      case 15:	return(103);
      case 14:	return(104);
      case 13:	return(106);
      case 12:	return(108);
      case 11:	return(110);
      case 10:	return(112);
      case 9:  return(114);
      case 8:  return(116);
      case 7:  return(118);
      case 6:  return(120);
      case 5:  return(122);
      case 4:  return(125);
      case 3:  return(130);
      default: return(100);
      }
}


/* Returns a character's adjustment to hit points	 -JWT-	 */
int con_adj()
{
  register int con;

  con = py.stats.use_stat[A_CON];
  if (con < 7)
    return(con - 7);
  else if (con < 17)
    return(0);
  else if (con ==  17)
    return(1);
  else if (con <  94)
    return(2);
  else if (con < 117)
    return(3);
  else
    return(4);
}


char *title_string()
{
  register char *p;

  if (py.misc.lev < 1)
    p = "Babe in arms";
  else if (py.misc.lev <= MAX_PLAYER_LEVEL)
    p = player_title[py.misc.pclass][py.misc.lev-1];
  else if (py.misc.male)
    p = "**KING**";
  else
    p = "**QUEEN**";
  return p;
}

/* Prints title of character				-RAK-	*/
void prt_title()
{
  XFillRectangle(p_disp, Stats_window->win, clearGC, 5, 2*mfontheight, 100,
		 mfontheight+4);
  XFillRectangle(p_disp, Stats_window->map, clearGC, 5, 2*mfontheight, 100,
		 mfontheight+4);
  DisplayStat(5, 3, title_string());
}


/* Prints level						-RAK-	*/
void prt_level()
{
  char tmpstr[10];

  XFillRectangle(p_disp, Stats_window->win, clearGC, 300, 0, 20, mfontheight);
  XFillRectangle(p_disp, Stats_window->map, clearGC, 300, 0, 20, mfontheight);
  sprintf(tmpstr, "%u", py.misc.lev);
  DisplayStat(300, 1, tmpstr);
}


/* Prints players current mana points.		 -RAK-	*/
void prt_cmana()
{
  XFillRectangle(p_disp, Stats_window->win, clearGC, 301, 2*mfontheight + 3, 69,
		 mfontheight - 1);
  XFillRectangle(p_disp, Stats_window->map, clearGC, 301, 2*mfontheight + 3, 69,
		 mfontheight - 1);
  if (py.misc.mana > 0)
    {
      XSetFillStyle(p_disp, theGC, FillStippled);
      XFillRectangle(p_disp, Stats_window->win, theGC, 300, 2*mfontheight + 2,
		     (70 * py.misc.cmana) / py.misc.mana, mfontheight);
      XFillRectangle(p_disp, Stats_window->map, theGC, 300, 2*mfontheight + 2,
		     (70 * py.misc.cmana) / py.misc.mana, mfontheight);
      XSetFillStyle(p_disp, theGC, FillSolid);
    }
}

/* Prints Max hit points				-RAK-	*/
void prt_mhp()
{
  ClearArea(Stats_window, 281, 3*mfontheight+3, 89, mfontheight - 1, 0);
  if (py.misc.chp*10 < py.misc.mhp)
    {
      XSetFillStyle(p_disp, redGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
		     (90 * py.misc.chp) / py.misc.mhp, mfontheight, redGC);
      XSetFillStyle(p_disp, redGC, FillSolid);
    }
  else if (py.misc.chp*2 < py.misc.mhp)
    {
      XSetFillStyle(p_disp, goldGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
		     (90 * py.misc.chp) / py.misc.mhp, mfontheight, goldGC);
      XSetFillStyle(p_disp, goldGC, FillSolid);
    }
  else
    {
      XSetFillStyle(p_disp, theGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
		     (90 * py.misc.chp) / py.misc.mhp, mfontheight, theGC);
      XSetFillStyle(p_disp, theGC, FillSolid);
    }
}


/* Prints players current hit points			-RAK-	*/
void prt_chp()
{
  ClearArea(Stats_window, 281, 3*mfontheight + 2, 89, mfontheight - 1, 0);
  if (py.misc.chp*10 < py.misc.mhp)
    {
      XSetFillStyle(p_disp, redGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
	       (90 * py.misc.chp) / py.misc.mhp, mfontheight, redGC);
      XSetFillStyle(p_disp, redGC, FillSolid);
    }
  else if (py.misc.chp*2 < py.misc.mhp)
    {
      XSetFillStyle(p_disp, goldGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
	       (90 * py.misc.chp) / py.misc.mhp, mfontheight, goldGC);
      XSetFillStyle(p_disp, goldGC, FillSolid);
    }
  else
    {
      XSetFillStyle(p_disp, theGC, FillStippled);
      FillArea(Stats_window, 280, 3*mfontheight + 2,
	       (90 * py.misc.chp) / py.misc.mhp, mfontheight, theGC);
      XSetFillStyle(p_disp, theGC, FillSolid);
    }
}


/* prints current AC					-RAK-	*/
void prt_pac()
{
  char tmpstr[10];

  ClearArea(Stats_window, 300, 4*mfontheight+3, 150, mfontheight, 0);
  sprintf(tmpstr, "%u", py.misc.dis_ac);
  DisplayStat(300, 5, tmpstr);
}


/* Prints current gold					-RAK-	*/
void prt_gold()
{
  char tmpstr[20];

  ClearArea(Stats_window, 300, 5*mfontheight, 150, mfontheight, 0);
  sprintf(tmpstr, "%lu", py.misc.au);
  DisplayStat(300, 6, tmpstr);
}

/* Prints depth in stat area				-RAK-	*/
void prt_depth()
{
  vtype depths;
  register int depth;

  ClearArea(Flags_window, 7, 1, 80, mfontheight + 1, 0);
  depth = dun_level*50;
  if (depth == 0)
    (void) strcpy(depths, "Town level");
  else
    (void) sprintf(depths, "%d feet", depth);
  prt_flag(depths, 1);
}


/* Prints status of hunger				-RAK-	*/
void prt_hunger()
{
  ClearArea(Flags_window, 105, 0, 50, mfontheight + 2, 0);
  if (PY_WEAK & py.flags.status)
    prt_flag("Weak", 15);
  else if (PY_HUNGRY & py.flags.status)
    prt_flag("Hungry", 15);
}


/* Prints Blind status					-RAK-	*/
void prt_blind()
{
  if (PY_BLIND & py.flags.status)
    prt_flag("Blind", 30);
  else
    ClearArea(Flags_window, 210, 0, 40, mfontheight + 2, 0);
}


/* Prints Confusion status				-RAK-	*/
void prt_confused()
{
  if (PY_CONFUSED & py.flags.status)
    prt_flag("Confused", 45);
  else
    ClearArea(Flags_window, 315, 0, 65, mfontheight + 2, 0);
}


/* Prints Fear status					-RAK-	*/
void prt_afraid()
{
  if (PY_FEAR & py.flags.status)
    prt_flag("Afraid", 60);
  else
    ClearArea(Flags_window, 420, 0, 50, mfontheight + 2, 0);
}


/* Prints Poisoned status				-RAK-	*/
void prt_poisoned()
{
  if (PY_POISONED & py.flags.status)
    prt_flag("Poisoned", 85);
  else
    ClearArea(Flags_window, 105, mfontheight, 65, mfontheight + 2, 0);
}


/* Prints Searching, Resting, Paralysis, or 'count' status	-RAK-	*/
void prt_state()
{
  char tmp[16];

  py.flags.status &= ~PY_REPEAT;
  if (py.flags.paralysis > 1)
    prt_flag("Paralysed", 115);
  else if (PY_REST & py.flags.status)
    {
      (void) sprintf (tmp, "Rest %-5d", py.flags.rest);
      prt_flag(tmp, 115);
    }
  else if (command_count > 0)
    {
      if (PY_SEARCH & py.flags.status)
	prt_flag("Search", 115);
      else
	{
	  (void) sprintf (tmp, "Repeat %-3d", command_count);
	  py.flags.status |= PY_REPEAT;
	  prt_flag(tmp, 115);
	}
    }
  else if (PY_SEARCH & py.flags.status)
    prt_flag("Searching", 115);
  else		/* "repeat 999" is 10 characters */
    ClearArea(Flags_window, 315, mfontheight, 80, mfontheight + 2, 0);
}


/* Prints the speed of a character.			-CJS- */
void prt_speed ()
{
  register int i;
  char buf[20];

  ClearArea(Flags_window, 210, mfontheight, 70, mfontheight + 2, 0);
  i = py.flags.speed;
  if (PY_SEARCH & py.flags.status)   /* Search mode. */
    i--;
  if (i > 1)
    {
      sprintf(buf, "(%d) Slow", -i);
      prt_flag(buf, 100);
    }
  else if (i == 1)
    prt_flag("Slow", 100);
  else if (i == -1)
    prt_flag("Fast", 100);
  else if (i < -1)
    {
      sprintf(buf, "(+%d) Fast", -i);
      prt_flag(buf, 100);
    }
}


void prt_study()
{
  py.flags.status &= ~PY_STUDY;
  if (py.flags.new_spells == 0)
    ClearArea(Flags_window, 420, mfontheight, 40, mfontheight + 2, 0);
  else
    prt_flag("Study", 130);
}

/* Prints winner status on display			-RAK-	*/
void prt_winner()
{
puts("1");
  ClearArea(Stats_window, 5, 5*mfontheight, 100, mfontheight + 4, 0);
puts("2");
  if (noscore & 0x2)
    {
      if (wizard)
	DisplayStat(5, 6, "Is Wizard");
      else
	DisplayStat(5, 6, "Was Wizard");
    }
  else if (noscore & 0x1)
    DisplayStat(5, 6, "Resurrected");
  else if (total_winner)
    DisplayStat(5, 6, "*Winner*");
}

int8u modify_stat (stat, amount)
int stat;
int16 amount;
{
  register int loop, i;
  register int8u tmp_stat;

  tmp_stat = py.stats.cur_stat[stat];
  loop = (amount < 0 ? -amount : amount);
  for (i = 0; i < loop; i++)
    {
      if (amount > 0)
	{
	  if (tmp_stat < 18)
	    tmp_stat++;
	  else if (tmp_stat < 108)
	    tmp_stat += 10;
	  else
	    tmp_stat = 118;
	}
      else
	{
	  if (tmp_stat > 27)
	    tmp_stat -= 10;
	  else if (tmp_stat > 18)
	    tmp_stat = 18;
	  else if (tmp_stat > 3)
	    tmp_stat--;
	}
    }
  return tmp_stat;
}


/* Set the value of the stat which is actually used.	 -CJS- */
void set_use_stat(stat)
int stat;
{
  py.stats.use_stat[stat] = modify_stat (stat, py.stats.mod_stat[stat]);

  if (stat == A_STR)
    {
      py.flags.status |= PY_STR_WGT;
      calc_bonuses();
    }
  else if (stat == A_DEX)
    calc_bonuses();
  else if (stat == A_INT && class[py.misc.pclass].spell == MAGE)
    {
      calc_spells(A_INT);
      calc_mana(A_INT);
    }
  else if (stat == A_WIS && class[py.misc.pclass].spell == PRIEST)
    {
      calc_spells(A_WIS);
      calc_mana(A_WIS);
    }
  else if (stat == A_CON)
    calc_hitpoints();
}


/* Increases a stat by one randomized level		-RAK-	*/
int inc_stat(stat)
register int stat;
{
  register int tmp_stat, gain;

  tmp_stat = py.stats.cur_stat[stat];
  if (tmp_stat < 118)
    {
      if (tmp_stat < 18)
	tmp_stat++;
      else if (tmp_stat < 116)
	{
	  /* stat increases by 1/6 to 1/3 of difference from max */
	  gain = ((118 - tmp_stat)/3 + 1) >> 1;
	  tmp_stat += randint(gain) + gain;
	}
      else
	tmp_stat++;

      py.stats.cur_stat[stat] = tmp_stat;
      if (tmp_stat > py.stats.max_stat[stat])
	py.stats.max_stat[stat] = tmp_stat;
      set_use_stat (stat);
      prt_stat (stat);
      return TRUE;
    }
  else
    return FALSE;
}


/* Decreases a stat by one randomized level		-RAK-	*/
int dec_stat(stat)
register int stat;
{
  register int tmp_stat, loss;

  tmp_stat = py.stats.cur_stat[stat];
  if (tmp_stat > 3)
    {
      if (tmp_stat < 19)
	tmp_stat--;
      else if (tmp_stat < 117)
	{
	  loss = (((118 - tmp_stat) >> 1) + 1) >> 1;
	  tmp_stat += -randint(loss) - loss;
	  if (tmp_stat < 18)
	    tmp_stat = 18;
	}
      else
	tmp_stat--;

      py.stats.cur_stat[stat] = tmp_stat;
      set_use_stat (stat);
      prt_stat (stat);
      return TRUE;
    }
  else
    return FALSE;
}


/* Restore a stat.  Return TRUE only if this actually makes a difference. */
int res_stat (stat)
int stat;
{
  register int i;

  i = py.stats.max_stat[stat] - py.stats.cur_stat[stat];
  if (i)
    {
      py.stats.cur_stat[stat] += i;
      set_use_stat (stat);
      prt_stat (stat);
      return TRUE;
    }
  return FALSE;
}

/* Boost a stat artificially (by wearing something). If the display argument
   is TRUE, then increase is shown on the screen. */
void bst_stat (stat, amount)
int stat, amount;
{
  py.stats.mod_stat[stat] += amount;

  set_use_stat (stat);
  /* can not call prt_stat() here, may be in store, may be in inven_command */
  py.flags.status |= (PY_STR << stat);
}

/* Returns a character's adjustment to hit.		 -JWT-	 */
int tohit_adj()
{
  register int total, stat;

  stat = py.stats.use_stat[A_DEX];
  if	  (stat <   4)	total = -3;
  else if (stat <   6)	total = -2;
  else if (stat <   8)	total = -1;
  else if (stat <  16)	total =	 0;
  else if (stat <  17)	total =	 1;
  else if (stat <  18)	total =	 2;
  else if (stat <  69)	total =	 3;
  else if (stat < 118)	total =	 4;
  else			total =	 5;
  stat = py.stats.use_stat[A_STR];
  if	  (stat <   4)	total -= 3;
  else if (stat <   5)	total -= 2;
  else if (stat <   7)	total -= 1;
  else if (stat <  18)	total -= 0;
  else if (stat <  94)	total += 1;
  else if (stat < 109)	total += 2;
  else if (stat < 117)	total += 3;
  else			total += 4;
  return(total);
}


/* Returns a character's adjustment to armor class	 -JWT-	 */
int toac_adj()
{
  register int stat;

  stat = py.stats.use_stat[A_DEX];
  if	  (stat <   4)	return(-4);
  else if (stat ==  4)	return(-3);
  else if (stat ==  5)	return(-2);
  else if (stat ==  6)	return(-1);
  else if (stat <  15)	return( 0);
  else if (stat <  18)	return( 1);
  else if (stat <  59)	return( 2);
  else if (stat <  94)	return( 3);
  else if (stat < 117)	return( 4);
  else			return( 5);
}


/* Returns a character's adjustment to disarm		 -RAK-	 */
int todis_adj()
{
  register int stat;

  stat = py.stats.use_stat[A_DEX];
  if	  (stat <   3)	return(-8);
  else if (stat ==  4)	return(-6);
  else if (stat ==  5)	return(-4);
  else if (stat ==  6)	return(-2);
  else if (stat ==  7)	return(-1);
  else if (stat <  13)	return( 0);
  else if (stat <  16)	return( 1);
  else if (stat <  18)	return( 2);
  else if (stat <  59)	return( 4);
  else if (stat <  94)	return( 5);
  else if (stat < 117)	return( 6);
  else			return( 8);
}


/* Returns a character's adjustment to damage		 -JWT-	 */
int todam_adj()
{
  register int stat;

  stat = py.stats.use_stat[A_STR];
  if	  (stat <   4)	return(-2);
  else if (stat <   5)	return(-1);
  else if (stat <  16)	return( 0);
  else if (stat <  17)	return( 1);
  else if (stat <  18)	return( 2);
  else if (stat <  94)	return( 3);
  else if (stat < 109)	return( 4);
  else if (stat < 117)	return( 5);
  else			return( 6);
}

/* Draws entire screen					-RAK-	*/
void draw_cave()
{
  clear_screen (Map_window);
  prt_stat_block();
  prt_map();
  prt_depth();
}

/* Prints the following information on the screen.	-JWT-	*/
void put_character()
{
  register struct misc *m_ptr;

  m_ptr = &py.misc;
  put_buffer (Char_window, "Name        :", 2, 1);
  put_buffer (Char_window, "Race        :", 3, 1);
  put_buffer (Char_window, "Sex         :", 4, 1);
  put_buffer (Char_window, "Class       :", 5, 1);
  if (character_generated)
    {
      put_buffer (Char_window, m_ptr->name, 2, 15);
      put_buffer (Char_window, race[m_ptr->prace].trace, 3, 15);
      put_buffer (Char_window, (m_ptr->male ? "Male" : "Female"), 4, 15);
      put_buffer (Char_window, class[m_ptr->pclass].title, 5, 15);
    }
}


/* Prints the following information on the screen.	-JWT-	*/
void put_stats()
{
  register struct misc *m_ptr;
  register int i;
  vtype buf;

  ClearArea(Char_window, 55 * 7, mfontheight, 100, 7*mfontheight, 0);

  m_ptr = &py.misc;
  for (i = 0; i < 6; i++)
    {
      cnv_stat (py.stats.use_stat[i], buf);
      put_buffer (Char_window, stat_names[i], 2+i, 55);
      put_buffer (Char_window, buf, 2+i, 60);
      if (py.stats.max_stat[i] > py.stats.cur_stat[i])
	{
	  cnv_stat (py.stats.max_stat[i], buf);
	  put_buffer (Char_window, buf, 2+i, 68);
	}
    }
  ClearArea(Char_window, 13*7, 8*mfontheight, 100, 5*mfontheight, 0);
  prt_num(Char_window, "+ To Hit    ", m_ptr->dis_th,  9, 1);
  prt_num(Char_window, "+ To Damage ", m_ptr->dis_td, 10, 1);
  prt_num(Char_window, "+ To AC     ", m_ptr->dis_tac, 11, 1);
  prt_num(Char_window, "  Total AC  ", m_ptr->dis_ac, 12, 1);
}


/* Returns a rating of x depending on y			-JWT-	*/
char *likert(x, y)
int x, y;
{
  switch((x/y))
    {
      case -3: case -2: case -1: return("Very Bad");
      case 0: case 1:		 return("Bad");
      case 2:			 return("Poor");
      case 3: case 4:		 return("Fair");
      case  5:			 return("Good");
      case 6:			 return("Very Good");
      case 7: case 8:		 return("Excellent");
      default:			 return("Superb");
      }
}


/* Prints age, height, weight, and SC			-JWT-	*/
void put_misc1()
{
  register struct misc *m_ptr;

  ClearArea(Char_window, 45 * 7, mfontheight, 60, 5*mfontheight, 0);
  m_ptr = &py.misc;
  prt_num(Char_window, "Age          ", (int)m_ptr->age, 2, 32);
  prt_num(Char_window, "Height       ", (int)m_ptr->ht, 3, 32);
  prt_num(Char_window, "Weight       ", (int)m_ptr->wt, 4, 32);
  prt_num(Char_window, "Social Class ", (int)m_ptr->sc, 5, 32);
}


/* Prints the following information on the screen.	-JWT-	*/
void put_misc2()
{
  register struct misc *m_ptr;

  m_ptr = &py.misc;
  prt_num(Char_window, "Level      ", (int)m_ptr->lev, 9, 29);
  prt_lnum(Char_window, "Experience ", m_ptr->exp, 10, 29);
  prt_lnum(Char_window, "Max Exp    ", m_ptr->max_exp, 11, 29);
  prt_lnum(Char_window, "Exp to Adv.", (int32)(player_exp[m_ptr->lev-1] *
				  m_ptr->expfact/100), 12, 29);
  prt_lnum(Char_window, "Gold       ", m_ptr->au, 13, 29);
  prt_num(Char_window, "Max Hit Points ", m_ptr->mhp, 9, 52);
  prt_num(Char_window, "Cur Hit Points ", m_ptr->chp, 10, 52);
  prt_num(Char_window, "Max Mana       ", m_ptr->mana, 11, 52);
  prt_num(Char_window, "Cur Mana       ", m_ptr->cmana, 12, 52);
}


/* Prints ratings on certain abilities			-RAK-	*/
void put_misc3()
{
  int xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;
  vtype xinfra;
  register struct misc *p_ptr;

  p_ptr = &py.misc;
  xbth	= p_ptr->bth + p_ptr->ptohit*BTH_PLUS_ADJ
    + (class_level_adj[p_ptr->pclass][CLA_BTH] * p_ptr->lev);
  xbthb = p_ptr->bthb + p_ptr->ptohit*BTH_PLUS_ADJ
    + (class_level_adj[p_ptr->pclass][CLA_BTHB] * p_ptr->lev);
  /* this results in a range from 0 to 29 */
  xfos	= 40 - p_ptr->fos;
  if (xfos < 0)	 xfos = 0;
  xsrh	= p_ptr->srh;
  /* this results in a range from 0 to 9 */
  xstl	= p_ptr->stl + 1;
  xdis	= p_ptr->disarm + 2*todis_adj() + stat_adj(A_INT)
    + (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
  xsave = p_ptr->save + stat_adj(A_WIS)
    + (class_level_adj[p_ptr->pclass][CLA_SAVE] * p_ptr->lev / 3);
  xdev	= p_ptr->save + stat_adj(A_INT)
    + (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3);
    
  (void) sprintf(xinfra, "%d feet", py.flags.see_infra*10);

  ClearArea(Char_window, 0, 13*mfontheight, WIDTH - 20,
		 7 * mfontheight, 0);

  put_buffer (Char_window, "(Miscellaneous Abilities)", 15, 25);
  put_buffer (Char_window, "Fighting    :", 16, 1);
  put_buffer (Char_window, likert (xbth, 12), 16, 15);
  put_buffer (Char_window, "Bows/Throw  :", 17, 1);
  put_buffer (Char_window, likert (xbthb, 12), 17, 15);
  put_buffer (Char_window, "Saving Throw:", 18, 1);
  put_buffer (Char_window, likert (xsave, 6), 18, 15);

  put_buffer (Char_window, "Stealth     :", 16, 26);
  put_buffer (Char_window, likert (xstl, 1), 16, 40);
  put_buffer (Char_window, "Disarming   :", 17, 26);
  put_buffer (Char_window, likert (xdis, 8), 17, 40);
  put_buffer (Char_window, "Magic Device:", 18, 26);
  put_buffer (Char_window, likert (xdev, 6), 18, 40);

  put_buffer (Char_window, "Perception  :", 16, 51);
  put_buffer (Char_window, likert (xfos, 3), 16, 65);
  put_buffer (Char_window, "Searching   :", 17, 51);
  put_buffer (Char_window, likert (xsrh, 6), 17, 65);
  put_buffer (Char_window, "Infra-Vision:", 18, 51);
  put_buffer (Char_window, xinfra, 18, 69);
}


/* Used to display the character on the screen.		-RAK-	*/
void display_char()
{
  put_character();
  put_misc1();
  put_stats();
  put_misc2();
  put_misc3();
}

get_name_update()
{
  display_char();
  prt(Char_window, "Enter your player's name [press <RETURN> when finished]",
      21, 2);
}

/* Gets a name for the character			-JWT-	*/
void get_name()
{
  prt(Char_window, "Enter your player's name  [press <RETURN> when finished]",
      21, 2);

  if (!get_string(Char_window, py.misc.name, 2, 15, 23, get_name_update, "")
      || py.misc.name[0] == 0)
    {
      user_name (py.misc.name);
      put_buffer (Char_window, py.misc.name, 2, 15);
    }
}

/* Changes the name of the character			-JWT-	*/
void change_name()
{
  register char c;
  register int flag;
  vtype temp;

  flag = FALSE;
  if (!Char_window)
    {
      Char_window = CreateDWin(WIDTH, 25*mfontheight, 296, 500,
			     DefaultRootDWin, mfgpix, mbgpix);
      XSetTransientForHint(p_disp, Char_window->win,
			   DefaultRootWindow(p_disp));
      XSelectInput(p_disp, Char_window->win, ExposureMask | KeyPressMask |
		   ButtonPressMask);
      XMapWindow(p_disp, Char_window->win);
    }
  clear_screen(Char_window);
  XFlush(p_disp);
  display_char();
  prt(Char_window,
      "<f>ile character description. <c>hange character name.", 21, 2);
  do
    {
      c = inkeya(Char_window);

      switch(c)
	{
	case 'c':
	  get_name();
	  flag = TRUE;
	  break;
	case 'f':
	  prt (Char_window, "File name:", 0, 0);
	  if (get_string (Char_window, temp, 0, 10, 60) && temp[0])
	    if (file_character (temp))
	      flag = TRUE;
	  break;
	case ESCAPE: case ' ':
	case '\n': case '\r':
	  flag = TRUE;
	  break;
	default:
	  bell ();
	  break;
	}
    }
  while (!flag);
  DestroyDWin(Char_window);
  Char_window = NULL;
}

/* Destroy an item in the inventory			-RAK-	*/
void inven_destroy(item_val)
int item_val;
{
  register int j;
  register inven_type *i_ptr;

  i_ptr = &inventory[item_val];
  if ((i_ptr->number > 1) && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX))
    {
      i_ptr->number--;
      inven_weight -= i_ptr->weight;
    }
  else
    {
      inven_weight -= i_ptr->weight*i_ptr->number;
      for (j = item_val; j < inven_ctr-1; j++)
	inventory[j] = inventory[j+1];
      invcopy(&inventory[inven_ctr-1], OBJ_NOTHING);
      inven_ctr--;
    }
  py.flags.status |= PY_STR_WGT;
}


/* Copies the object in the second argument over the first argument.
   However, the second always gets a number of one except for ammo etc. */
void take_one_item (s_ptr, i_ptr)
register inven_type *s_ptr, *i_ptr;
{
  *s_ptr = *i_ptr;
  if ((s_ptr->number > 1) && (s_ptr->subval >= ITEM_SINGLE_STACK_MIN)
      && (s_ptr->subval <= ITEM_SINGLE_STACK_MAX))
    s_ptr->number = 1;
}


/* Drops an item from inventory to given location	-RAK-	*/
void inven_drop(item_val, drop_all)
register int item_val, drop_all;
{
  int i;
  register inven_type *i_ptr;
  vtype prt2;
  bigvtype prt1;

  if (cave[char_row][char_col].tptr != 0)
    (void) delete_object(char_row, char_col);
  i = popt ();
  i_ptr = &inventory[item_val];
  t_list[i] = *i_ptr;
  cave[char_row][char_col].tptr = i;

  if (item_val >= INVEN_WIELD)
    takeoff (item_val, -1);
  else
    {
      if (drop_all || i_ptr->number == 1)
	{
	  inven_weight -= i_ptr->weight*i_ptr->number;
	  inven_ctr--;
	  while (item_val < inven_ctr)
	    {
	      inventory[item_val] = inventory[item_val+1];
	      item_val++;
	    }
	  invcopy(&inventory[inven_ctr], OBJ_NOTHING);
	}
      else
	{
	  t_list[i].number = 1;
	  inven_weight -= i_ptr->weight;
	  i_ptr->number--;
	}
      objdes (prt1, &t_list[i], TRUE);
      (void) sprintf (prt2, "Dropped %s", prt1);
      msg_print (prt2);
    }
  py.flags.status |= PY_STR_WGT;
}


/* Destroys a type of item on a given percent chance	-RAK-	*/
int inven_damage(typ, perc)
int (*typ)();
register int perc;
{
  register int i, j;

  j = 0;
  for (i = 0; i < inven_ctr; i++)
    if ((*typ)(inventory[i].tval) && (randint(100) < perc))
      {
	inven_destroy(i);
	j++;
      }
  return(j);
}


/* Computes current weight limit			-RAK-	*/
int weight_limit()
{
  register int weight_cap;

  weight_cap = py.stats.use_stat[A_STR] * PLAYER_WEIGHT_CAP + py.misc.wt;
  if (weight_cap > 3000)  weight_cap = 3000;
  return(weight_cap);
}


/* this code must be identical to the inven_carry() code below */
int inven_check_num (t_ptr)
register inven_type *t_ptr;
{
  register int i;

  if (inven_ctr < INVEN_WIELD)
    return TRUE;
  else if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN)
    for (i = 0; i < inven_ctr; i++)
      if (inventory[i].tval == t_ptr->tval &&
	  inventory[i].subval == t_ptr->subval &&
	  /* make sure the number field doesn't overflow */
	  ((int)inventory[i].number + (int)t_ptr->number < 256) &&
	  /* they always stack (subval < 192), or else they have same p1 */
	  ((t_ptr->subval < ITEM_GROUP_MIN) || (inventory[i].p1 == t_ptr->p1))
	  /* only stack if both or neither are identified */
	  && (known1_p(&inventory[i]) == known1_p(t_ptr)))
	return TRUE;
  return FALSE;
}

/* return FALSE if picking up an object would change the players speed */ 
int inven_check_weight(i_ptr)
register inven_type *i_ptr;
{
  register int i, new_inven_weight;

  i = weight_limit();
  new_inven_weight = i_ptr->number*i_ptr->weight + inven_weight;
  if (i < new_inven_weight)
    i = new_inven_weight / (i + 1);
  else
    i = 0;

  if (pack_heavy != i)
    return FALSE;
  else
    return TRUE;
}


/* Are we strong enough for the current pack and weapon?  -CJS-	 */
void check_strength()
{
  register int i;
  register inven_type *i_ptr;

  i_ptr = &inventory[INVEN_WIELD];
  if (i_ptr->tval != TV_NOTHING
      && (py.stats.use_stat[A_STR]*15 < i_ptr->weight))
    {
      if (weapon_heavy == FALSE)
	{
	  msg_print("You have trouble wielding such a heavy weapon.");
	  weapon_heavy = TRUE;
	  calc_bonuses();
	}
    }
  else if (weapon_heavy == TRUE)
    {
      weapon_heavy = FALSE;
      if (i_ptr->tval != TV_NOTHING)
	msg_print("You are strong enough to wield your weapon.");
      calc_bonuses();
    }
  i = weight_limit();
  if (i < inven_weight)
    i = inven_weight / (i+1);
  else
    i = 0;
  if (pack_heavy != i)
    {
      if (pack_heavy < i)
	msg_print("Your pack is so heavy that it slows you down.");
      else
	msg_print("You move more easily under the weight of your pack.");
      change_speed(i - pack_heavy);
      pack_heavy = i;
    }
  py.flags.status &= ~PY_STR_WGT;
}


/* Add an item to players inventory.  Return the	*/
/* item position for a description if needed.	       -RAK-   */
/* this code must be identical to the inven_check_num() code above */
int inven_carry(i_ptr)
register inven_type *i_ptr;
{
  register int locn, i;
  register int typ, subt;
  register inven_type *t_ptr;

  typ = i_ptr->tval;
  subt = i_ptr->subval;
  /* Now, check to see if player can carry object  */
  for (locn = 0; ; locn++)
    {
      t_ptr = &inventory[locn];
      if ((typ == t_ptr->tval) && (subt == t_ptr->subval)
	  && (subt >= ITEM_SINGLE_STACK_MIN) &&
	  ((int)t_ptr->number + (int)i_ptr->number < 256) &&
	  ((subt < ITEM_GROUP_MIN) || (t_ptr->p1 == i_ptr->p1)) &&	
	  /* only stack if both or neither are identified */
	  (known1_p(i_ptr) == known1_p(t_ptr)))
	{
	  t_ptr->number += i_ptr->number;
	  break;
	}
      else if (typ > t_ptr->tval)
	{
	  for (i = inven_ctr - 1; i >= locn; i--)
	    inventory[i+1] = inventory[i];
	  inventory[locn] = *i_ptr;
	  inven_ctr++;
	  break;
	}
    }

  inven_weight += i_ptr->number*i_ptr->weight;
  py.flags.status |= PY_STR_WGT;
  return locn;
}


/* Returns spell chance of failure for spell		-RAK-	*/
int spell_chance(spell)
int spell;
{
  register spell_type *s_ptr;
  register int chance;
  register int stat;

  s_ptr = &magic_spell[py.misc.pclass-1][spell];
  chance = s_ptr->sfail - 3*(py.misc.lev-s_ptr->slevel);
  if (class[py.misc.pclass].spell == MAGE)
    stat = A_INT;
  else
    stat = A_WIS;
  chance -= 3 * (stat_adj(stat)-1);
  if (s_ptr->smana > py.misc.cmana)
    chance += 5 * (s_ptr->smana-py.misc.cmana);
  if (chance > 95)
    chance = 95;
  else if (chance < 5)
    chance = 5;
  return chance;
}

/* Print list of spells					-RAK-	*/
/* if nonconsec is -1: spells numbered consecutively from 'a' to 'a'+num
                  >=0: spells numbered by offset from nonconsec */
void print_spells(spell, num, comment, nonconsec)
int *spell;
register int num;
int comment, nonconsec;
{
  register int i, j;
  vtype out_val;
  register spell_type *s_ptr;
  int offset;
  char *p;
  char spell_char;

  offset = (class[py.misc.pclass].spell==MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
  erase_line(Spell_win, 1, 0);
  put_buffer(Spell_win, "Name", 1, 5);
  put_buffer(Spell_win, "Lv Mana Fail", 1, 35);
  /* only show the first 22 choices */
  if (num > 22)
    num = 22;
  for (i = 0; i < num; i++)
    {
      j = spell[i];
      s_ptr = &magic_spell[py.misc.pclass-1][j];
      if (comment == FALSE)
	p = "";
      else if ((spell_forgotten & (1L << j)) != 0)
	p = " forgotten";
      else if ((spell_learned & (1L << j)) == 0)
	p = " unknown";
      else if ((spell_worked & (1L << j)) == 0)
	p = " untried";
      else
	p = "";
      /* determine whether or not to leave holes in character choices,
	 nonconsec -1 when learning spells, consec offset>=0 when asking which
	 spell to cast */
      if (nonconsec == -1)
	spell_char = 'a' + i;
      else
	spell_char = 'a' + j - nonconsec;
      (void) sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s", spell_char,
		     spell_names[j+offset], s_ptr->slevel, s_ptr->smana,
		     spell_chance (j), p);
      prt(Spell_win, out_val, 2+i, 0);
    }
}

/* Returns spell pointer				-RAK-	*/
int get_spell(spell, num, sn, sc, prompt, first_spell)
int *spell;
register int num;
register int *sn, *sc;
char *prompt;
int first_spell;
{
  register spell_type *s_ptr;
  int flag, redraw, offset, i;
  char choice;
  vtype out_str, tmp_str;

  *sn = -1;
  flag = FALSE;
  (void) sprintf(out_str, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s",
		 spell[0]+'a'-first_spell, spell[num-1]+'a'-first_spell,
		 prompt);
  redraw = FALSE;
  offset = (class[py.misc.pclass].spell==MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
  while (flag == FALSE && get_com (Status_window, out_str, &choice))
    {
      if (isupper((int)choice))
	{
	  *sn = choice-'A'+first_spell;
	  /* verify that this is in spell[], at most 22 entries in spell[] */
	  for (i = 0; i < num; i++)
	    if (*sn == spell[i])
	      break;
	  if (i == num)
	    *sn = -2;
	  else
	    {
	      s_ptr = &magic_spell[py.misc.pclass-1][*sn];
	      (void) sprintf (tmp_str, "Cast %s (%d mana, %d%% fail)?",
			      spell_names[*sn+offset], s_ptr->smana,
			      spell_chance (*sn));
	      if (get_check (Status_window, tmp_str))
		flag = TRUE;
	      else
		*sn = -1;
	    }
	}
      else if (islower((int)choice))
	{
	  *sn = choice-'a'+first_spell;
	  /* verify that this is in spell[], at most 22 entries in spell[] */
	  for (i = 0; i < num; i++)
	    if (*sn == spell[i])
	      break;
	  if (i == num)
	    *sn = -2;
	  else
	    flag = TRUE;
	}
      else if (choice == '*')
	{
	  /* only do this drawing once */
	  if (!redraw)
	    {
	      Spell_win = CreateDWin(340, 23 * mfontheight, 110, 55,
				       Main, mfgpix, mbgpix);
	      XMapWindow(p_disp, Spell_win->win);
	      redraw = TRUE;
	      print_spells (spell, num, FALSE, first_spell);
	    }
	}
      else if (isalpha((int)choice))
	*sn = -2;
      else
	{
	  *sn = -1;
	  bell();
	}
      if (*sn == -2)
	msg_print("You don't know that spell.");
    }

  if (flag)
    *sc = spell_chance (*sn);

  if (redraw)
    DestroyDWin(Spell_win);

  return(flag);
}

/* calculate number of spells player should have, and learn forget spells
   until that number is met -JEW- */
calc_spells(stat)
int stat;
{
  register int i;
  register int32u mask;
  int32u spell_flag;
  int j, offset;
  int num_allowed = 0, new_spells, num_known, levels;
  vtype tmp_str;
  char *p;
  register struct misc *p_ptr;
  register spell_type *msp_ptr;

  p_ptr = &py.misc;
  msp_ptr = &magic_spell[p_ptr->pclass-1][0];
  if (stat == A_INT)
    {
      p = "spell";
      offset = SPELL_OFFSET;
    }
  else
    {
      p = "prayer";
      offset = PRAYER_OFFSET;
    }

  /* check to see if know any spells greater than level, eliminate them */
  for (i = 31, mask = 0x80000000; mask; mask >>= 1, i--)
    if (mask & spell_learned)
      {
	if (msp_ptr[i].slevel > p_ptr->lev)
	  {
	    spell_learned &= ~mask;
	    spell_forgotten |= mask;
	    (void) sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			   spell_names[i+offset]);
	    msg_print(tmp_str);
	  }
	else
	  break;
      }

  /* calc number of spells allowed */
  levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
  switch(stat_adj(stat))
    {
    case 0:		    num_allowed = 0; break;
    case 1: case 2: case 3: num_allowed = 1 * levels; break;
    case 4: case 5:	    num_allowed = 3 * levels / 2; break;
    case 6:		    num_allowed = 2 * levels; break;
    case 7:		    num_allowed = 5 * levels / 2; break;
    }

  num_known = 0;
  for (mask = 0x1; mask; mask <<= 1)
    if (mask & spell_learned)
      num_known++;
  new_spells = num_allowed - num_known;

  if (new_spells > 0)
    {
      /* remember forgotten spells while forgotten spells exist of new_spells
	 positive, remember the spells in the order that they were learned */
      for (i = 0; (spell_forgotten && new_spells
		   && (i < num_allowed) && (i < 32)); i++)
	{
	  /* j is (i+1)th spell learned */
	  j = spell_order[i];
	  /* shifting by amounts greater than number of bits in long gives
	     an undefined result, so don't shift for unknown spells */
	  if (j == 99)
	    mask = 0x0;
	  else
	    mask = 1L << j;
	  if (mask & spell_forgotten)
	    {
	      if (msp_ptr[j].slevel <= p_ptr->lev)
		{
		  new_spells--;
		  spell_forgotten &= ~mask;
		  spell_learned |= mask;
		  (void) sprintf(tmp_str, "You have remembered the %s of %s.",
				 p, spell_names[j+offset]);
		  msg_print(tmp_str);
		}
	      else
		num_allowed++;
	    }
	}

      if (new_spells > 0)
	{
	  /* determine which spells player can learn */
	  /* must check all spells here, in gain_spell() we actually check
	     if the books are present */
	  spell_flag = 0x7FFFFFFFL & ~spell_learned;

	  mask = 0x1;
	  i = 0;
	  for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++)
	    if (spell_flag & mask)
	      {
		spell_flag &= ~mask;
		if (msp_ptr[j].slevel <= p_ptr->lev)
		  i++;
	      }

	  if (new_spells > i)
	    new_spells = i;
	}
    }
  else if (new_spells < 0)
    {
      /* forget spells until new_spells zero or no more spells know, spells
	 are forgotten in the opposite order that they were learned */
      for (i = 31; new_spells && spell_learned; i--)
	{
	  /* j is the (i+1)th spell learned */
	  j = spell_order[i];
	  /* shifting by amounts greater than number of bits in long gives
	     an undefined result, so don't shift for unknown spells */
	  if (j == 99)
	    mask = 0x0;
	  else
	    mask = 1L << j;
	  if (mask & spell_learned)
	    {
	      spell_learned &= ~mask;
	      spell_forgotten |= mask;
	      new_spells++;
	      (void) sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			     spell_names[j+offset]);
	      msg_print(tmp_str);
	    }
	}

      new_spells = 0;
    }

  if (new_spells != py.flags.new_spells)
    {
      if (new_spells > 0 && py.flags.new_spells == 0)
	{
	  (void) sprintf(tmp_str, "You can learn some new %ss now.", p);
	  msg_print(tmp_str);
	}

      py.flags.new_spells = new_spells;
      py.flags.status |= PY_STUDY;
    }
}

/* gain spells when player wants to		- jw */
void gain_spells()
{
  char query;
  int stat, diff_spells, new_spells;
  int offset, last_known;
  register int i, j;
  register int32u spell_flag, mask;
  vtype tmp_str;
  struct misc *p_ptr;
  register spell_type *msp_ptr;
  int spells[31], num_spells;

  i = 0;
  if (py.flags.blind > 0)
    msg_print("You can't see to read your spell book!");
  else if (no_light())
    msg_print("You have no light to read by.");
  else if (py.flags.confused > 0)
    msg_print("You are too confused.");
  else
    i = 1;
  if (i == 0)
    return;

  new_spells = py.flags.new_spells;
  diff_spells = 0;
  p_ptr = &py.misc;
  msp_ptr = &magic_spell[p_ptr->pclass-1][0];
  if (class[p_ptr->pclass].spell == MAGE)
    {
      stat = A_INT;
      offset = SPELL_OFFSET;
    }
  else
    {
      stat = A_WIS;
      offset = PRAYER_OFFSET;
    }

  for (last_known = 0; last_known < 32; last_known++)
    if (spell_order[last_known] == 99)
      break;

  if (!new_spells)
    {
      (void) sprintf(tmp_str, "You can't learn any new %ss!",
		     (stat == A_INT ? "spell" : "prayer"));
      msg_print(tmp_str);
      free_turn_flag = TRUE;
    }
  else
    {
      /* determine which spells player can learn */
      /* mages need the book to learn a spell, priests do not need the book */
      if (stat == A_INT)
	{
	  spell_flag = 0;
	  for (i = 0; i < inven_ctr; i++)
	    if (((stat == A_INT) && (inventory[i].tval == TV_MAGIC_BOOK))
		|| ((stat == A_WIS) && (inventory[i].tval == TV_PRAYER_BOOK)))
	      spell_flag |= inventory[i].flags;
	}
      else
	spell_flag = 0x7FFFFFFF;

      /* clear bits for spells already learned */
      spell_flag &= ~spell_learned;
      
      mask = 0x1;
      i = 0;
      for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++)
	if (spell_flag & mask)
	  {
	    spell_flag &= ~mask;
	    if (msp_ptr[j].slevel <= p_ptr->lev)
	      {
		spells[i] = j;
		i++;
	      }
	  }
      num_spells = i;

      if (new_spells > i)
	{
	  msg_print("You seem to be missing a book.");
	  diff_spells = new_spells - i;
	  new_spells = i;
	}
      if (new_spells == 0)
	;
      else if (stat == A_INT)
	{
	  Spell_win = CreateDWin(340, (i + 1) * mfontheight, 110, 55,
				   Main, mfgpix, mbgpix);
	  XMapWindow(p_disp, Spell_win->win);

	  /* get to choose which mage spells will be learned */
	  print_spells (spells, i, FALSE, -1);
	  while (new_spells &&
		 (query = inkeyr(Status_window)))
	    {
	      j = query - 'a';
	      /* test j < 23 in case i is greater than 22, only 22 spells
		 are actually shown on the screen, so limit choice to those */
	      if (j >= 0 && j < i && j < 22)
		{
		  new_spells--;
		  spell_learned |= 1L << spells[j];
		  spell_order[last_known++] = spells[j];
		  for (; j <= i-1; j++)
		    spells[j] = spells[j+1];
		  i--;
		  erase_line (Spell_win, j, 31);
		  print_spells (spells, i, FALSE, -1);
		}
	      else
		bell();
	    }
	  DestroyDWin(Spell_win);
	}
      else
	{
	  /* pick a prayer at random */
	  while (new_spells)
	    {
	      j = randint(i) - 1;
	      spell_learned |= 1L << spells[j];
	      spell_order[last_known++] = spells[j];
	      (void) sprintf (tmp_str,
			      "You have learned the prayer of %s.",
			      spell_names[spells[j]+offset]);
	      msg_print(tmp_str);
	      for (; j <= i-1; j++)
		spells[j] = spells[j+1];
	      i--;
	      new_spells--;
	    }
	}
      py.flags.new_spells = new_spells + diff_spells;
      if (py.flags.new_spells == 0)
	py.flags.status |= PY_STUDY;
      /* set the mana for first level characters when they learn first spell */
      if (py.misc.mana == 0)
	calc_mana(stat);
    }
}


/* Gain some mana if you know at least one spell	-RAK-	*/
calc_mana(stat)
int stat;
{
  register int new_mana = 0, levels;
  register struct misc *p_ptr;
  register int32 value;

  p_ptr = &py.misc;
  if (spell_learned != 0)
    {
      levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
      switch(stat_adj(stat))
	{
	case 0: new_mana = 0; break;
	case 1: case 2: new_mana = 1 * levels; break;
	case 3: new_mana = 3 * levels / 2; break;
	case 4: new_mana = 2 * levels; break;
	case 5: new_mana = 5 * levels / 2; break;
	case 6: new_mana = 3 * levels; break;
	case 7: new_mana = 4 * levels; break;
	}
      /* increment mana by one, so that first level chars have 2 mana */
      if (new_mana > 0)
	new_mana++;

      /* mana can be zero when creating character */
      if (p_ptr->mana != new_mana)
	{
	  if (p_ptr->mana != 0)
	    {
	      /* change current mana proportionately to change of max mana,
		 divide first to avoid overflow, little loss of accuracy */
	      value = (((long)p_ptr->cmana << 16) + p_ptr->cmana_frac)
		/ p_ptr->mana * new_mana;
	      p_ptr->cmana = value >> 16;
	      p_ptr->cmana_frac = value & 0xFFFF;
	    }
	  else
	    {
	      p_ptr->cmana = new_mana;
	      p_ptr->cmana_frac = 0;
	    }
	  p_ptr->mana = new_mana;
	  /* can't print mana here, may be in store or inventory mode */
	  py.flags.status |= PY_MANA;
	}
    }
  else if (p_ptr->mana != 0)
    {
      p_ptr->mana = 0;
      p_ptr->cmana = 0;
      /* can't print mana here, may be in store or inventory mode */
      py.flags.status |= PY_MANA;
    }
}


/* Increases hit points and level			-RAK-	*/
static void gain_level()
{
  register int32 dif_exp, need_exp;
  vtype out_val;
  register struct misc *p_ptr;
  register class_type *c_ptr;

  p_ptr = &py.misc;
  p_ptr->lev++;
  (void) sprintf(out_val, "Welcome to level %d.", (int)p_ptr->lev);
  msg_print(out_val);
  calc_hitpoints();

  need_exp = player_exp[p_ptr->lev-1] * p_ptr->expfact / 100;
  if (p_ptr->exp > need_exp)
    {
      /* lose some of the 'extra' exp when gain a level */
      dif_exp = p_ptr->exp - need_exp;
      p_ptr->exp = need_exp + (dif_exp / 2);
    }
  prt_level();
  prt_title();
  c_ptr = &class[p_ptr->pclass];
  if (c_ptr->spell == MAGE)
    {
      calc_spells(A_INT);
      calc_mana(A_INT);
    }
  else if (c_ptr->spell == PRIEST)
    {
      calc_spells(A_WIS);
      calc_mana(A_WIS);
    }
}

/* Prints experience					-RAK-	*/
void prt_experience()
{
  register struct misc *p_ptr;
  char tmpstr[20];

  p_ptr = &py.misc;
  if (p_ptr->exp > MAX_EXP)
    p_ptr->exp = MAX_EXP;
  if (p_ptr->lev <= MAX_PLAYER_LEVEL)
    {
      while ((player_exp[p_ptr->lev-1] * p_ptr->expfact / 100) <= p_ptr->exp)
	gain_level();
      if (p_ptr->exp > p_ptr->max_exp)
	p_ptr->max_exp = p_ptr->exp;
    }
  ClearArea(Stats_window, 300, mfontheight, 100,
		 mfontheight, 0);
  sprintf(tmpstr, "%lu", p_ptr->exp);
  DisplayStat(300, 2, tmpstr);
}


/* Calculate the players hit points */
calc_hitpoints()
{
  register int hitpoints;
  register struct misc *p_ptr;
  register int32 value;

  p_ptr = &py.misc;
  hitpoints = player_hp[p_ptr->lev-1] + (con_adj() * p_ptr->lev);
  /* always give at least one point per level + 1 */
  if (hitpoints < (p_ptr->lev + 1))
    hitpoints = p_ptr->lev + 1;

  if (py.flags.status & PY_HERO)
    hitpoints += 10;
  if (py.flags.status & PY_SHERO)
    hitpoints += 20;

  /* mhp can equal zero while character is being created */
  if ((hitpoints != p_ptr->mhp) && (p_ptr->mhp != 0))
    {
      /* change current hit points proportionately to change of mhp,
	 divide first to avoid overflow, little loss of accuracy */
      value = (((long)p_ptr->chp << 16) + p_ptr->chp_frac) / p_ptr->mhp
	* hitpoints;
      p_ptr->chp = value >> 16;
      p_ptr->chp_frac = value & 0xFFFF;
      p_ptr->mhp = hitpoints;

      /* can't print hit points here, may be in store or inventory mode */
      py.flags.status |= PY_HP;
    }
}


/* Inserts a string into a string				*/
void insert_str(object_str, mtc_str, insert)
char *object_str, *mtc_str, *insert;
{
  int obj_len;
  char *bound, *pc;
  register int i, mtc_len;
  register char *temp_obj, *temp_mtc;
  char out_val[80];

  mtc_len = strlen(mtc_str);
  obj_len = strlen(object_str);
  bound = object_str + obj_len - mtc_len;
  for (pc = object_str; pc <= bound; pc++)
    {
      temp_obj = pc;
      temp_mtc = mtc_str;
      for (i = 0; i < mtc_len; i++)
	if (*temp_obj++ != *temp_mtc++)
	  break;
      if (i == mtc_len)
	break;
    }

  if (pc <= bound)
    {
      (void) strncpy(out_val, object_str, (pc-object_str));
      out_val[(pc-object_str)] = '\0';
      if (insert)
	(void) strcat(out_val, insert);
      (void) strcat(out_val, (char *)(pc+mtc_len));
      (void) strcpy(object_str, out_val);
    }
}



/* this is no longer used anywhere */
/* Inserts a number into a string				*/
void insert_num(object_str, mtc_str, number, show_sign)
char *object_str;
register char *mtc_str;
int number;
int show_sign;
{
  int mlen;
  vtype str1, str2;
  register char *string, *tmp_str;
  int flag;

  flag = 1;
  mlen = strlen(mtc_str);
  tmp_str = object_str;
  do
    {
      string = index(tmp_str, mtc_str[0]);
      if (string == NULL)
	flag = 0;
      else
	{
	  flag = strncmp(string, mtc_str, mlen);
	  if (flag)
	    tmp_str = string+1;
	}
    }
  while (flag);
  if (string)
    {
      (void) strncpy(str1, object_str, string - object_str);
      str1[string - object_str] = '\0';
      (void) strcpy(str2, string + mlen);
      if ((number >= 0) && (show_sign))
	(void) sprintf(object_str, "%s+%d%s", str1, number, str2);
      else
	(void) sprintf(object_str, "%s%d%s", str1, number, str2);
    }
}

void insert_lnum(object_str, mtc_str, number, show_sign)
char *object_str;
register char *mtc_str;
int32 number;
int show_sign;
{
  int mlen;
  vtype str1, str2;
  register char *string, *tmp_str;
  int flag;

  flag = 1;
  mlen = strlen(mtc_str);
  tmp_str = object_str;
  do
    {
      string = index(tmp_str, mtc_str[0]);
      if (string == 0)
	flag = 0;
      else
	{
	  flag = strncmp(string, mtc_str, mlen);
	  if (flag)
	    tmp_str = string+1;
	}
    }
  while (flag);
  if (string)
    {
      (void) strncpy(str1, object_str, string - object_str);
      str1[string - object_str] = '\0';
      (void) strcpy(str2, string + mlen);
      if ((number >= 0) && (show_sign))
	(void) sprintf(object_str, "%s+%ld%s", str1, number, str2);
      else
	(void) sprintf(object_str, "%s%ld%s", str1, number, str2);
    }
}


/* lets anyone enter wizard mode after a disclaimer...		- JEW - */
int enter_wiz_mode()
{
  register int answer = 0;

  if (!noscore)
    {
      msg_print("Wizard mode is for debugging and experimenting.");
      answer = get_check(Status_window, 
	"The game will not be scored if you enter wizard mode. Are you sure?");
    }
  if (noscore || answer)
    {
      noscore |= 0x2;
      wizard = TRUE;
      return(TRUE);
    }
  return(FALSE);
}


/* Weapon weight VS strength and dexterity		-RAK-	*/
int attack_blows(weight, wtohit)
int weight;
int *wtohit;
{
  register int adj_weight;
  register int str_index, dex_index, s, d;

  s = py.stats.use_stat[A_STR];
  d = py.stats.use_stat[A_DEX];
  if (s * 15 < weight)
    {
      *wtohit = s * 15 - weight;
      return 1;
    }
  else
    {
      *wtohit = 0;
      if      (d <  10)	 dex_index = 0;
      else if (d <  19)	 dex_index = 1;
      else if (d <  68)	 dex_index = 2;
      else if (d < 108)	 dex_index = 3;
      else if (d < 118)	 dex_index = 4;
      else		 dex_index = 5;
      adj_weight = (s * 10 / weight);
      if      (adj_weight < 2)	str_index = 0;
      else if (adj_weight < 3)	str_index = 1;
      else if (adj_weight < 4)	str_index = 2;
      else if (adj_weight < 5)	str_index = 3;
      else if (adj_weight < 7)	str_index = 4;
      else if (adj_weight < 9)	str_index = 5;
      else			str_index = 6;
      return (int)blows_table[str_index][dex_index];
    }
}


/* Special damage due to magical abilities of object	-RAK-	*/
int tot_dam(i_ptr, tdam, monster)
register inven_type *i_ptr;
register int tdam;
int monster;
{
  register creature_type *m_ptr;
  register recall_type *r_ptr;

  if ((i_ptr->flags & TR_EGO_WEAPON) &&
      (((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_ARROW)) ||
       ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_SWORD)) ||
       (i_ptr->tval == TV_FLASK)))
    {
      m_ptr = &c_list[monster];
      r_ptr = &c_recall[monster];
      /* Slay Dragon  */
      if ((m_ptr->cdefense & CD_DRAGON) && (i_ptr->flags & TR_SLAY_DRAGON))
	{
	  tdam = tdam * 4;
	  r_ptr->r_cdefense |= CD_DRAGON;
	}
      /* Slay Undead  */
      else if ((m_ptr->cdefense & CD_UNDEAD) &&(i_ptr->flags & TR_SLAY_UNDEAD))
	{
	  tdam = tdam * 3;
	  r_ptr->r_cdefense |= CD_UNDEAD;
	}
      /* Slay Animal  */
      else if ((m_ptr->cdefense & CD_ANIMAL) &&(i_ptr->flags & TR_SLAY_ANIMAL))
	{
	  tdam = tdam * 2;
	  r_ptr->r_cdefense |= CD_ANIMAL;
	}
      /* Slay Evil     */
      else if ((m_ptr->cdefense & CD_EVIL) && (i_ptr->flags & TR_SLAY_EVIL))
	{
	  tdam = tdam * 2;
	  r_ptr->r_cdefense |= CD_EVIL;
	}
      /* Frost	       */
      else if ((m_ptr->cdefense & CD_FROST) && (i_ptr->flags & TR_FROST_BRAND))
	{
	  tdam = tdam * 3 / 2;
	  r_ptr->r_cdefense |= CD_FROST;
	}
      /* Fire	      */
      else if ((m_ptr->cdefense & CD_FIRE) && (i_ptr->flags & TR_FLAME_TONGUE))
	{
	  tdam = tdam * 3 / 2;
	  r_ptr->r_cdefense |= CD_FIRE;
	}
    }
  return(tdam);
}


/* Critical hits, Nasty way to die.			-RAK-	*/
int critical_blow(weight, plus, dam, attack_type)
register int weight, plus, dam;
int attack_type;
{
  register int critical;

  critical = dam;
  /* Weight of weapon, plusses to hit, and character level all	    */
  /* contribute to the chance of a critical			   */
  if (randint(5000) <= (int)(weight + 5 * plus
			     + (class_level_adj[py.misc.pclass][attack_type]
				* py.misc.lev)))
    {
      weight += randint(650);
      if (weight < 400)
	{
	  critical = 2*dam + 5;
	  msg_print("It was a good hit! (x2 damage)");
	}
      else if (weight < 700)
	{
	  critical = 3*dam + 10;
	  msg_print("It was an excellent hit! (x3 damage)");
	}
      else if (weight < 900)
	{
	  critical = 4*dam + 15;
	  msg_print("It was a superb hit! (x4 damage)");
	}
      else
	{
	  critical = 5*dam + 20;
	  msg_print("It was a *GREAT* hit! (x5 damage)");
	}
    }
  return(critical);
}


/* Given direction "dir", returns new row, column location -RAK- */
int mmove(dir, y, x)
int dir;
register int *y, *x;
{
  register int new_row = *y, new_col = *x;
  int bool;

  switch(dir)
    {
    case 1:
      new_row = *y + 1;
      new_col = *x - 1;
      break;
    case 2:
      new_row = *y + 1;
      new_col = *x;
      break;
    case 3:
      new_row = *y + 1;
      new_col = *x + 1;
      break;
    case 4:
      new_row = *y;
      new_col = *x - 1;
      break;
    case 5:
      new_row = *y;
      new_col = *x;
      break;
    case 6:
      new_row = *y;
      new_col = *x + 1;
      break;
    case 7:
      new_row = *y - 1;
      new_col = *x - 1;
      break;
    case 8:
      new_row = *y - 1;
      new_col = *x;
      break;
    case 9:
      new_row = *y - 1;
      new_col = *x + 1;
      break;
    }
  bool = FALSE;
  if ((new_row >= 0) && (new_row < cur_height)
      && (new_col >= 0) && (new_col < cur_width))
    {
      *y = new_row;
      *x = new_col;
      bool = TRUE;
    }
  return(bool);
}

/* Saving throws for player character.		-RAK-	*/
int player_saves()
{
  if (randint(100) <= (py.misc.save + stat_adj(A_WIS)
		       + (class_level_adj[py.misc.pclass][CLA_SAVE]
			  * py.misc.lev / 3)))
    return(TRUE);
  else
    return(FALSE);
}


/* Finds range of item in inventory list		-RAK-	*/
int find_range(item1, item2, j, k)
int item1, item2;
register int *j, *k;
{
  register int i;
  register inven_type *i_ptr;
  int flag;

  i = 0;
  *j = -1;
  *k = -1;
  flag = FALSE;
  i_ptr = &inventory[0];
  while (i < inven_ctr)
    {
      if (!flag)
	{
	  if ((i_ptr->tval == item1) || (i_ptr->tval == item2))
	    {
	      flag = TRUE;
	      *j = i;
	    }
	}
      else
	{
	  if ((i_ptr->tval != item1) && (i_ptr->tval != item2))
	    {
	      *k = i - 1;
	      break;
	    }
	}
      i++;
      i_ptr++;
    }
  if (flag && (*k == -1))
    *k = inven_ctr - 1;
  return(flag);
}


/* Teleport the player to a new location		-RAK-	*/
void teleport(dis)
int dis;
{
  register int y, x, i, j;

  do
    {
      y = randint(cur_height) - 1;
      x = randint(cur_width) - 1;
      while (distance(y, x, char_row, char_col) > dis)
	{
	  y += ((char_row-y)/2);
	  x += ((char_col-x)/2);
	}
    }
  while ((cave[y][x].fval >= MIN_CLOSED_SPACE) || (cave[y][x].cptr >= 2));
  move_rec(char_row, char_col, y, x);
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++)
      {
	cave[i][j].tl = FALSE;
	lite_spot(i, j);
      }
  lite_spot(char_row, char_col);
  char_row = y;
  char_col = x;
  check_view();
  creatures(FALSE);
  teleport_flag = FALSE;
}


/* Add a comment to an object description.		-CJS- */
void scribe_object()
{
  int item_val, j;
  vtype out_val, tmp_str;

  if (inven_ctr > 0 || equip_ctr > 0)
    {
      if (get_item(&item_val, "Which one? ", 0, INVEN_ARRAY_SIZE))
	{
	  objdes(tmp_str, &inventory[item_val], TRUE);
	  (void) sprintf(out_val, "Inscribing %s", tmp_str);
	  msg_print(out_val);
	  if (inventory[item_val].inscrip[0] != '\0')
	    (void) sprintf(out_val, "Replace %s New inscription:",
			   inventory[item_val].inscrip);
	  else
	    (void) strcpy(out_val, "Inscription: ");
	  j = 78 - strlen(tmp_str);
	  if (j > 24)
	    j = 12;
	  msg_print(out_val);
	  if (get_string(Status_window, out_val, 0, strlen(out_val), j))
	    inscribe(&inventory[item_val], out_val);
	}
    }
  else
    msg_print("You are not carrying anything to inscribe.");
}

/* Append an additional comment to an object description.	-CJS- */
void add_inscribe(i_ptr, type)
inven_type *i_ptr;
int8u type;
{
  i_ptr->ident |= type;
}

/* Replace any existing comment in an object description with a new one. CJS*/
inscribe(i_ptr, str)
inven_type *i_ptr;
char *str;
{
  (void) strcpy(i_ptr->inscrip, str);
}


/* We need to reset the view of things.			-CJS- */
check_view()
{
  register int i, j;
  register cave_type *c_ptr, *d_ptr;

  c_ptr = &cave[char_row][char_col];
  /* Check for new panel		   */
  if (get_panel(char_row, char_col, FALSE))
    prt_map();
  /* Move the light source		   */
  move_light(char_row, char_col, char_row, char_col);
  /* A room of light should be lit.	 */
  if (c_ptr->fval == LIGHT_FLOOR)
    {
      if ((py.flags.blind < 1) && !c_ptr->pl)
	light_room(char_row, char_col);
    }
  /* In doorway of light-room?		   */
  else if (c_ptr->lr && (py.flags.blind < 1))
    {
      for (i = (char_row - 1); i <= (char_row + 1); i++)
	for (j = (char_col - 1); j <= (char_col + 1); j++)
	  {
	    d_ptr = &cave[i][j];
	    if ((d_ptr->fval == LIGHT_FLOOR) && !d_ptr->pl)
	      light_room(i, j);
	  }
    }
}

