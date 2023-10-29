#include "xmoria.h"
/* moria1.c: misc code, mainly to handle player movement, inventory, etc.

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include <stdio.h>
#include <ctype.h>

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#else
char *strcat();
int strlen();
#endif
#else
#include <strings.h>
#endif

#if defined(LINT_ARGS)
static void inven_screen(int);
static char map_roguedir(char);
static void sub1_move_light(int, int, int, int);
static void sub3_move_light(int, int, int, int);
static int see_wall(int, int, int);
static int see_nothing(int, int, int);
#else
static int see_wall();
#endif

/* Changes speed of monsters relative to player		-RAK-	*/
/* Note: When the player is sped up or slowed down, I simply	 */
/*	 change the speed of all the monsters.	This greatly	 */
/*	 simplified the logic.				       */
void change_speed(num)
register int num;
{
  register int i;

  py.flags.speed += num;
  py.flags.status |= PY_SPEED;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
      m_list[i].cspeed += num;
}


/* Player bonuses					-RAK-	*/
/* When an item is worn or taken off, this re-adjusts the player */
/* bonuses.  Factor=1 : wear; Factor=-1 : removed		 */
/* Only calculates properties with cumulative effect.  Properties that
   depend on everything being worn are recalculated by calc_bonuses() -CJS - */
void py_bonuses(t_ptr, factor)
register inven_type *t_ptr;
register int factor;
{
  register int i, amount;

  amount = t_ptr->p1 * factor;
  if (t_ptr->flags & TR_STATS)
    {
      for(i = 0; i < 6; i++)
	if ((1 << i) & t_ptr->flags)
	  bst_stat(i, amount);
    }
  if (TR_SEARCH & t_ptr->flags)
    {
      py.misc.srh += amount;
      py.misc.fos -= amount;
    }
  if (TR_STEALTH & t_ptr->flags)
    py.misc.stl += amount;
  if (TR_SPEED & t_ptr->flags)
    change_speed(-amount);
  if ((TR_BLIND & t_ptr->flags) && (factor > 0))
    py.flags.blind += 1000;
  if ((TR_TIMID & t_ptr->flags) && (factor > 0))
    py.flags.afraid += 50;
  if (TR_INFRA & t_ptr->flags)
    py.flags.see_infra += amount;
}

/* Recalculate the effect of all the stuff we use.		  -CJS- */
void calc_bonuses()
{
  register int32u item_flags;
#if defined(ATARIST_MWC)
  int32u holder;		/* to avoid a compiler bug */
#endif
  int old_dis_ac;
  register struct flags *p_ptr;
  register struct misc *m_ptr;
  register inven_type *i_ptr;
  register int i;

  p_ptr = &py.flags;
  m_ptr = &py.misc;
  if (p_ptr->slow_digest)
    p_ptr->food_digested++;
  if (p_ptr->regenerate)
    p_ptr->food_digested -= 3;
  p_ptr->see_inv     = FALSE;
  p_ptr->teleport    = FALSE;
  p_ptr->free_act    = FALSE;
  p_ptr->slow_digest = FALSE;
  p_ptr->aggravate   = FALSE;
  p_ptr->sustain_str = FALSE;
  p_ptr->sustain_int = FALSE;
  p_ptr->sustain_wis = FALSE;
  p_ptr->sustain_con = FALSE;
  p_ptr->sustain_dex = FALSE;
  p_ptr->sustain_chr = FALSE;
  p_ptr->fire_resist = FALSE;
  p_ptr->acid_resist = FALSE;
  p_ptr->cold_resist = FALSE;
  p_ptr->regenerate  = FALSE;
  p_ptr->lght_resist = FALSE;
  p_ptr->ffall	     = FALSE;

  old_dis_ac = m_ptr->dis_ac;
  m_ptr->ptohit	 = tohit_adj();	      /* Real To Hit   */
  m_ptr->ptodam	 = todam_adj();	      /* Real To Dam   */
  m_ptr->ptoac	 = toac_adj();	      /* Real To AC    */
  m_ptr->pac	 = 0;		    /* Real AC	     */
  m_ptr->dis_th	 = m_ptr->ptohit;  /* Display To Hit	    */
  m_ptr->dis_td	 = m_ptr->ptodam;  /* Display To Dam	    */
  m_ptr->dis_ac	 = 0;		/* Display AC		 */
  m_ptr->dis_tac = m_ptr->ptoac;   /* Display To AC	    */
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++)
    {
      i_ptr = &inventory[i];
      if (i_ptr->tval != TV_NOTHING)
	{
	  if ((TR_CURSED & i_ptr->flags) == 0)
	    {
	      m_ptr->pac += i_ptr->ac;
	      m_ptr->dis_ac += i_ptr->ac;
	    }
	  m_ptr->ptohit += i_ptr->tohit;
	  if (i_ptr->tval != TV_BOW)		/* Bows can't damage. -CJS- */
	    m_ptr->ptodam += i_ptr->todam;
	  m_ptr->ptoac	+= i_ptr->toac;
	  if (known2_p(i_ptr))
	    {
	      m_ptr->dis_th  += i_ptr->tohit;
	      if (i_ptr->tval != TV_BOW)
		m_ptr->dis_td  += i_ptr->todam;	/* Bows can't damage. -CJS- */
	      m_ptr->dis_tac += i_ptr->toac;
	    }
	}
    }
  m_ptr->dis_ac += m_ptr->dis_tac;

  if (weapon_heavy)
    m_ptr->dis_th += (py.stats.use_stat[A_STR] * 15 -
		      inventory[INVEN_WIELD].weight);

  /* Add in temporary spell increases	*/
  if (p_ptr->invuln > 0)
    {
      m_ptr->pac += 100;
      m_ptr->dis_ac += 100;
    }
  if (p_ptr->blessed > 0)
    {
      m_ptr->pac    += 2;
      m_ptr->dis_ac += 2;
    }
  if (p_ptr->detect_inv > 0)
    p_ptr->see_inv = TRUE;

  /* can't print AC here because might be in a store */
  if (old_dis_ac != m_ptr->dis_ac)
    p_ptr->status |= PY_ARMOR;

  item_flags = 0;
  i_ptr = &inventory[INVEN_WIELD];
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++)
    {
      item_flags |= i_ptr->flags;
      i_ptr++;
    }
#if !defined(ATARIST_MWC)
  if (TR_SLOW_DIGEST & item_flags)
    p_ptr->slow_digest = TRUE;
  if (TR_AGGRAVATE & item_flags)
    p_ptr->aggravate = TRUE;
  if (TR_TELEPORT & item_flags)
    p_ptr->teleport = TRUE;
  if (TR_REGEN & item_flags)
    p_ptr->regenerate = TRUE;
  if (TR_RES_FIRE & item_flags)
    p_ptr->fire_resist = TRUE;
  if (TR_RES_ACID & item_flags)
    p_ptr->acid_resist = TRUE;
  if (TR_RES_COLD & item_flags)
    p_ptr->cold_resist = TRUE;
  if (TR_FREE_ACT & item_flags)
    p_ptr->free_act = TRUE;
  if (TR_SEE_INVIS & item_flags)
    p_ptr->see_inv = TRUE;
  if (TR_RES_LIGHT & item_flags)
    p_ptr->lght_resist = TRUE;
  if (TR_FFALL & item_flags)
    p_ptr->ffall = TRUE;
#else
  /* this avoids a bug in the Mark Williams C compiler for the Atari ST */
  holder = TR_SLOW_DIGEST;
  if (holder & item_flags)
    p_ptr->slow_digest = TRUE;
  holder = TR_AGGRAVATE;
  if (holder & item_flags)
    p_ptr->aggravate = TRUE;
  holder = TR_TELEPORT;
  if (holder & item_flags)
    p_ptr->teleport = TRUE;
  holder = TR_REGEN;
  if (holder & item_flags)
    p_ptr->regenerate = TRUE;
  holder = TR_RES_FIRE;
  if (holder & item_flags)
    p_ptr->fire_resist = TRUE;
  holder = TR_RES_ACID;
  if (holder & item_flags)
    p_ptr->acid_resist = TRUE;
  holder = TR_RES_COLD;
  if (holder & item_flags)
    p_ptr->cold_resist = TRUE;
  holder = TR_FREE_ACT;
  if (holder & item_flags)
    p_ptr->free_act = TRUE;
  holder = TR_SEE_INVIS;
  if (holder & item_flags)
    p_ptr->see_inv = TRUE;
  holder = TR_RES_LIGHT;
  if (holder & item_flags)
    p_ptr->lght_resist = TRUE;
  holder = TR_FFALL;
  if (holder & item_flags)
    p_ptr->ffall = TRUE;
#endif

  i_ptr = &inventory[INVEN_WIELD];
  for (i = INVEN_WIELD; i < INVEN_LIGHT; i++)
    {
      if (TR_SUST_STAT & i_ptr->flags)
	switch(i_ptr->p1)
	  {
	  case 1: p_ptr->sustain_str = TRUE; break;
	  case 2: p_ptr->sustain_int = TRUE; break;
	  case 3: p_ptr->sustain_wis = TRUE; break;
	  case 4: p_ptr->sustain_con = TRUE; break;
	  case 5: p_ptr->sustain_dex = TRUE; break;
	  case 6: p_ptr->sustain_chr = TRUE; break;
	  default: break;
	  }
      i_ptr++;
    }

  if (p_ptr->slow_digest)
    p_ptr->food_digested--;
  if (p_ptr->regenerate)
    p_ptr->food_digested += 3;
}


/* Displays inventory items from r1 to r2	-RAK-	*/
/* Designed to keep the display as far to the right as possible.  The  -CJS-
   parameter col gives a column at which to start, but if the display does
   not fit, it may be moved left.  The return value is the left edge used. */
int show_inven(r1, r2, weight, col)
register int r1, r2;
int weight, col;
{
  register int i;
  int total_weight, len, l, lim;
  bigvtype tmp_val;
  vtype out_val[23];

  len = 42 - col;
  if (weight)
    lim = 68;
  else
    lim = 76;

  for (i = r1; i <= r2; i++)		 /* Print the items	  */
    {
      objdes(tmp_val, &inventory[i], TRUE);
      tmp_val[lim] = 0;	 /* Truncate if too long. */
      (void) sprintf(out_val[i], "  %c) %s", 'a'+i, tmp_val);
      l = strlen(out_val[i]);
      if (weight)
	l += 9;
      if (l > len)
	len = l;
    }

  col = 42 - len;
  if (col < 0)
    col = 0;

  for (i = r1; i <= r2; i++)
    {
      /* don't need first two spaces if in first column */
      if (col == 0)
	prt(Inven_window, &out_val[i][2], 2+i-r1, col + 5);
      else
	prt(Inven_window, out_val[i], 2+i-r1, col);
      if (weight)
	{
	  total_weight = inventory[i].weight*inventory[i].number;
	  (void) sprintf (tmp_val, "%3d.%d lb",
			  (total_weight) / 10, (total_weight) % 10);
	  prt(Inven_window, tmp_val, 2+i-r1, 50);
	}
    }
  return col;
}


/* Return a string describing how a given equipment item is carried. -CJS- */
char *describe_use(i)
register int i;
{
  register char *p;

  switch(i)
    {
    case INVEN_WIELD:
      p = "wielding"; break;
    case INVEN_HEAD:
      p = "wearing on your head"; break;
    case INVEN_NECK:
      p = "wearing around your neck"; break;
    case INVEN_BODY:
      p = "wearing on your body"; break;
    case INVEN_ARM:
      p = "wearing on your arm"; break;
    case INVEN_HANDS:
      p = "wearing on your hands"; break;
    case INVEN_RIGHT:
      p = "wearing on your right hand"; break;
    case INVEN_LEFT:
      p = "wearing on your left hand"; break;
    case INVEN_FEET:
      p = "wearing on your feet"; break;
    case INVEN_OUTER:
      p = "wearing about your body"; break;
    case INVEN_LIGHT:
      p = "using to light the way"; break;
    case INVEN_AUX:
      p = "holding ready by your side"; break;
    default:
      p = "carrying in your pack"; break;
    }
  return p;
}


/* Displays equipment items from r1 to end	-RAK-	*/
/* Keep display as far right as possible. -CJS- */
int show_equip(weight, col)
int weight, col;
{
  register int i, line;
  int total_weight, l, len, lim;
  register char *prt1;
  bigvtype prt2;
  vtype out_val[INVEN_ARRAY_SIZE-INVEN_WIELD];
  register inven_type *i_ptr;

  line = 0;
  len = 79 - col;
  if (weight)
    lim = 52;
  else
    lim = 60;
  for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) /* Range of equipment */
    {
      i_ptr = &inventory[i];
      if (i_ptr->tval != TV_NOTHING)
	{
	  switch(i)	     /* Get position	      */
	    {
	    case INVEN_WIELD:
	      if (py.stats.use_stat[A_STR]*15 < i_ptr->weight)
		prt1 = "Just lifting";
	      else
		prt1 = "Wielding";
	      break;
	    case INVEN_HEAD:
	      prt1 = "On head"; break;
	    case INVEN_NECK:
	      prt1 = "Around neck"; break;
	    case INVEN_BODY:
	      prt1 = "On body"; break;
	    case INVEN_ARM:
	      prt1 = "On arm"; break;
	    case INVEN_HANDS:
	      prt1 = "On hands"; break;
	    case INVEN_RIGHT:
	      prt1 = "On right hand"; break;
	    case INVEN_LEFT:
	      prt1 = "On left hand"; break;
	    case INVEN_FEET:
	      prt1 = "On feet"; break;
	    case INVEN_OUTER:
	      prt1 = "About body"; break;
	    case INVEN_LIGHT:
	      prt1 = "Light source"; break;
	    case INVEN_AUX:
	      prt1 = "Spare weapon"; break;
	    default:
	      prt1 = "Unknown value"; break;
	    }
	  objdes(prt2, &inventory[i], TRUE);
	  prt2[lim] = 0; /* Truncate if necessary */
	  (void) sprintf(out_val[line], "  %c) %-14s: %s", line+'a',
			 prt1, prt2);
	  l = strlen(out_val[line]);
	  if (weight)
	    l += 9;
	  if (l > len)
	    len = l;
	  line++;
	}
    }
  col = 79 - len;
  if (col < 0)
    col = 0;
    
  line = 0;
  for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) /* Range of equipment */
    {
      i_ptr = &inventory[i];
      if (i_ptr->tval != TV_NOTHING)
	{
	  /* don't need first two spaces when using whole screen */
	  if (col == 0)
	    prt(Inven_window, &out_val[line][2], line+2, col);
	  else
	    prt(Inven_window, out_val[line], line+2, col);
	  if (weight)
	    {
	      total_weight = i_ptr->weight*i_ptr->number;
	      (void) sprintf(prt2, "%3d.%d lb",
			     (total_weight) / 10, (total_weight) % 10);
	      prt(Inven_window, prt2, line+2, 71);
	    }
	  line++;
	}
    }
  return col;
}

/* Remove item from equipment list		-RAK-	*/
void takeoff(item_val, posn)
int item_val, posn;
{
  register char *p;
  bigvtype out_val, prt2;
  register inven_type *t_ptr;

  equip_ctr--;
  t_ptr = &inventory[item_val];
  inven_weight -= t_ptr->weight*t_ptr->number;
  py.flags.status |= PY_STR_WGT;

  if (item_val == INVEN_WIELD || item_val == INVEN_AUX)
    p = "Was wielding ";
  else if (item_val == INVEN_LIGHT)
    p = "Light source was ";
  else
    p = "Was wearing ";

  objdes(prt2, t_ptr, TRUE);
  if (posn >= 0)
    (void) sprintf(out_val, "%s%s (%c)", p, prt2, 'a'+posn);
  else
    (void) sprintf(out_val, "%s%s", p, prt2);
  msg_print(out_val);
  if (item_val != INVEN_AUX)	  /* For secondary weapon  */
    py_bonuses(t_ptr, -1);
  invcopy(t_ptr, OBJ_NOTHING);
}


/* Used to verify if this really is the item we wish to	 -CJS-
   wear or read. */
int verify(prompt, item)
char *prompt;
int item;
{
  bigvtype out_str, object;

  objdes(object, &inventory[item], TRUE);
  object[strlen(object)-1] = '?'; /* change the period to a question mark */
  (void) sprintf(out_str, "%s %s", prompt, object);
  return get_check(Inven_window, out_str);
}


/* All inventory commands (wear, exchange, take off, drop, inventory and
   equipment) are handled in an alternative command input mode, which accepts
   any of the inventory commands.

   It is intended that this function be called several times in succession,
   as some commands take up a turn, and the rest of moria must proceed in the
   interim. A global variable is provided, doing_inven, which is normally
   zero; however if on return from inven_command it is expected that
   inven_command should be called *again*, (being still in inventory command
   input mode), then doing_inven is set to the inventory command character
   which should be used in the next call to inven_command.

   On return, the screen is restored, but not flushed. Provided no flush of
   the screen takes place before the next call to inven_command, the inventory
   command screen is silently redisplayed, and no actual output takes place at
   all. If the screen is flushed before a subsequent call, then the player is
   prompted to see if we should continue. This allows the player to see any
   changes that take place on the screen during inventory command input.

  The global variable, screen_change, is cleared by inven_command, and set
  when the screen is flushed. This is the means by which inven_command tell
  if the screen has been flushed.

  The display of inventory items is kept to the right of the screen to
  minimize the work done to restore the screen afterwards.		-CJS-*/

/* Inventory command screen states. */
#define BLANK_SCR	0
#define EQUIP_SCR	1
#define INVEN_SCR	2
#define WEAR_SCR	3
#define HELP_SCR	4
#define WRONG_SCR	5

/* Keep track of the state of the inventory screen. */
static int scr_state, scr_left, scr_base;
static int wear_low, wear_high;

/* Draw the inventory screen. */
static void inven_screen(new_scr)
int new_scr;
{
  register int line;

  if (new_scr != scr_state)
    {
      scr_state = new_scr;
      clear_screen(Inven_window); XFlush(p_disp);
      switch(new_scr)
	{
	case BLANK_SCR:
	  line = 0;
	  break;
	case HELP_SCR:
	  if (scr_left > 52)
	    scr_left = 52;
	  prt(Inven_window, "  ESC: exit", 1, scr_left);
	  prt(Inven_window, "  w  : wear or wield object", 2, scr_left);
	  prt(Inven_window, "  t  : take off item", 3, scr_left);
	  prt(Inven_window, "  d  : drop object", 4, scr_left);
	  prt(Inven_window, "  x  : exchange weapons", 5, scr_left);
	  prt(Inven_window, "  i  : inventory of pack", 6, scr_left);
	  prt(Inven_window, "  e  : list used equipment", 7, scr_left);
	  line = 7;
	  break;
	case INVEN_SCR:
	  scr_left = show_inven(0, inven_ctr - 1, show_weight_flag, 1);
	  line = inven_ctr;
	  break;
	case WEAR_SCR:
	  scr_left = show_inven(wear_low, wear_high,show_weight_flag, 1);
	  line = wear_high - wear_low + 1;
	  break;
	case EQUIP_SCR:
	  scr_left = show_equip(show_weight_flag, 10);
	  line = equip_ctr;
	  break;
	}
    }
   XFlush(p_disp);
}

char comm;

refresh_inven(c)
char *c;
{
  char prt1[80];

  switch(*c)
    {
    case 'i':
      show_inven(0, inven_ctr - 1, show_weight_flag, 10);
      break;
    case 'e':
    case 't':
      show_equip(show_weight_flag, 10);
      break;
    case 'd':
      if (scr_state == EQUIP_SCR)
	show_equip(show_weight_flag, 10);
      else
	show_inven(0, inven_ctr - 1, show_weight_flag, 10);
      break;
    case 'w':
      show_inven(wear_low, wear_high, show_weight_flag, 10);
      break;
    case '?':
      prt(Inven_window, "ESC: exit", 1, 10);
      prt(Inven_window, "w  : wear or wield object", 2, 10);
      prt(Inven_window, "t  : take off item", 3, 10);
      prt(Inven_window, "d  : drop object", 4, 10);
      prt(Inven_window, "x  : exchange weapons", 5, 10);
      prt(Inven_window, "i  : inventory of pack", 6, 10);
      prt(Inven_window, "e  : list used equipment", 7, 10);
      break;
    default:
      bell();
    }
  if (scr_state == INVEN_SCR)
    {
      (void) sprintf(prt1,
		     "You are carrying %d.%d pounds. In your pack there is %s",
		     inven_weight / 10, inven_weight % 10,
		     (inven_ctr == 0 ? "nothing." : "-"));
      prt(Inven_window, prt1, 1, 0);
    }
  else if (scr_state == WEAR_SCR)
    {
      if (wear_high < wear_low)
	prt(Inven_window, "You have nothing you could wield.", 1, 0);
      else
	prt(Inven_window, "You could wield -", 1, 0);
    }
  else if (scr_state == EQUIP_SCR)
    {
      if (equip_ctr == 0)
	prt(Inven_window, "You are not using anything.", 1, 0);
      else
	prt(Inven_window, "You are using -", 1, 0);
    }
  else
    prt(Inven_window, "Allowed commands:", 23, 0);
  prt(Inven_window, "e/i/t/w/x/d/?/ESC:", 24, 0);
}

update_select(prompt)
char *prompt;
{
  refresh_inven(&comm);
  clear_from(Inven_window, 22);
  msg_print(prompt);
}

/* This does all the work. */
void inven_command(command)
char command;
{
  register int slot, item;
  int tmp, tmp2, selecting, from, to;
  char *prompt, *swap, *disp, *string;
  char which, query;
  bigvtype prt1, prt2;
  register inven_type *i_ptr;
  inven_type tmp_obj;

  comm = command;
  slot = 0;

  if (!doing_inven)
    {
      Inven_window = CreateDWin(WIDTH, 25 * mfontheight, 296, 500,
				  DefaultRootDWin, mfgpix, mbgpix);
      XSetTransientForHint(p_disp, Inven_window->win, DefaultRootWindow(p_disp));
      XSelectInput(p_disp, Inven_window->win, ExposureMask | KeyPressMask |
		   ButtonPressMask);
      XMapWindow(p_disp, Inven_window->win);
    }
  else
    {
      XClearArea(p_disp, Inven_window->win, 0, 0, 0, 0, 1);
      XFillRectangle(p_disp, Inven_window->map, clearGC, 0, 0,
		     Inven_window->width, Inven_window->height);
    }

  free_turn_flag = TRUE;
  /* Take up where we left off after a previous inventory command. -CJS- */
  if (doing_inven)
    {
      /* If the screen has been flushed, we need to redraw. If the command is
	 a simple ' ' to recover the screen, just quit. Otherwise, check and
	 see what the user wants. */
      if (screen_change)
	{
	  if (command == ' ' || !get_check(Status_window,
					 "Continuing with inventory command?"))
	    {
	      doing_inven = FALSE;
	      DestroyDWin(Inven_window);
	      return;
	    }
	  scr_left = 0;
	  scr_base = 0;
	}
      tmp = scr_state;
      scr_state = WRONG_SCR;
      inven_screen(tmp);
    }
  else
    {
      scr_left = 50;
      scr_base = 0;
      /* this forces exit of inven_command() if selecting is not set true */
      scr_state = BLANK_SCR;
    }
  do
    {
      if (isupper((int)command))
	command = tolower((int)command);
      /* Simple command getting and screen selection. */
      selecting = FALSE;
      switch(command)
	{
	case 'i':	   /* Inventory	    */
	  if (inven_ctr == 0)
	    prt(Inven_window, "You are not carrying anything.", 1, 0);
	  else
	    inven_screen(INVEN_SCR);
	  break;
	case 'e':	  /* Equipment	   */
	  if (equip_ctr == 0)
	    prt(Inven_window, "You are not using any equipment.", 1, 0);
	  else
	    inven_screen(EQUIP_SCR);
	  break;
	case 't':	  /* Take off	   */
	  if (equip_ctr == 0)
	    prt(Inven_window, "You are not using any equipment.", 1, 0);
	  /* don't print message restarting inven command after taking off
	     something, it is confusing */
	  else if (inven_ctr >= INVEN_WIELD && !doing_inven)
	    prt(Inven_window, "You will have to drop something first.", 1, 0);
	  else
	    {
	      inven_screen(EQUIP_SCR);
	      selecting = TRUE;
	    }
	  break;
	case 'd':		/* Drop */
	  if (inven_ctr == 0 && equip_ctr == 0)
	    prt(Inven_window, "But you're not carrying anything.", 1, 0);
	  else if (cave[char_row][char_col].tptr != 0)
	    prt(Inven_window, "There's no room to drop anything here.", 1, 0);
	  else
	    {
	      selecting = TRUE;
	      if ((scr_state == EQUIP_SCR && equip_ctr > 0) || inven_ctr == 0)
		{
		  inven_screen(EQUIP_SCR);
		  command = 'r';	/* Remove - or take off and drop. */
		}
	      else
		inven_screen(INVEN_SCR);
	    }
	  break;
	case 'w':	  /* Wear/wield	   */
	  for (wear_low = 0;
	       wear_low < inven_ctr && inventory[wear_low].tval > TV_MAX_WEAR;
	       wear_low++)
	    ;
	  for(wear_high = wear_low;
	      wear_high < inven_ctr && inventory[wear_high].tval >=TV_MIN_WEAR;
	      wear_high++)
	    ;
	  wear_high--;
	  if (wear_low > wear_high)
	    prt(Inven_window, "You have nothing to wear or wield.", 1, 0);
	  else
	    {
	      inven_screen(WEAR_SCR);
	      selecting = TRUE;
	    }
	  break;
	case 'x':
	  if (inventory[INVEN_WIELD].tval == TV_NOTHING &&
	      inventory[INVEN_AUX].tval == TV_NOTHING)
	    prt(Inven_window, "But you are wielding no weapons.", 1, 0);
	  else if (TR_CURSED & inventory[INVEN_WIELD].flags)
	    {
	      objdes(prt1, &inventory[INVEN_WIELD], FALSE);
	      (void) sprintf(prt2,
		     "The %s you are wielding appears to be cursed.", prt1);
	      prt(Inven_window, prt2, 1, 0);
	    }
	  else
	    {
	      free_turn_flag = FALSE;
	      tmp_obj = inventory[INVEN_AUX];
	      inventory[INVEN_AUX] = inventory[INVEN_WIELD];
	      inventory[INVEN_WIELD] = tmp_obj;
	      if (scr_state == EQUIP_SCR)
		scr_left = show_equip(show_weight_flag, scr_left);
	      py_bonuses(&inventory[INVEN_AUX], -1);	 /* Subtract bonuses */
	      py_bonuses(&inventory[INVEN_WIELD], 1);	   /* Add bonuses    */
	      check_strength();
	      if (inventory[INVEN_WIELD].tval != TV_NOTHING)
		{
		  (void) strcpy(prt1, "Primary weapon   : ");
		  objdes(prt2, &inventory[INVEN_WIELD], TRUE);
		  prt(Inven_window, strcat(prt1, prt2), 1, 0);
		}
	      else
		prt(Inven_window, "No primary weapon.", 1, 0);
	    }
	  break;
	case ' ':	/* Dummy command to return again to main prompt. */
	  break;
	case '?':
	  inven_screen(HELP_SCR);
	  break;
	default:
	  /* Nonsense command					   */
	  bell();
	  break;
	}

      /* Clear the doing_inven flag here, instead of at beginning, so that
	 can use it to control when messages above appear. */
      doing_inven = 0;

      /* Keep looking for objects to drop/wear/take off/throw off */
      which = 'z';
      while (selecting && free_turn_flag)
	{
	  swap = "";
	  if (command == 'w')
	    {
	      from = wear_low;
	      to = wear_high;
	      prompt = "Wear/Wield";
	    }
	  else
	    {
	      from = 0;
	      if (command == 'd')
		{
		  to = inven_ctr - 1;
		  prompt = "Drop";
		  if (equip_ctr > 0)
		    swap = ", / for Equip";
		}
	      else
		{
		  to = equip_ctr - 1;
		  if (command == 't')
		    prompt = "Take off";
		  else	/* command == 'r' */
		    {
		      prompt = "Throw off";
		      if (inven_ctr > 0)
			swap = ", / for Inven";
		    }
		}
	    }
	  if (from > to)
	    selecting = FALSE;
	  else
	    {
	      if (scr_state == BLANK_SCR)
		disp = ", * to list";
	      else
		disp = "";
	      (void) sprintf(prt1,
		      "(%c-%c%s%s, space to break, ESC to exit) %s which one?",
		      from+'a', to+'a', disp, swap, prompt);

	      /* Abort everything. */
	      msg_print(prt1);
	      which = inkeyr(Inven_window);
	      if (which == '\0' || which == '\033')
		{
		  selecting = FALSE;
		  which = ESCAPE;
		}
	      /* Draw the screen and maybe exit to main prompt. */
	      else if (which == ' ' || which == '*')
		{
		  if (command == 't' || command == 'r')
		    inven_screen(EQUIP_SCR);
		  else if (command == 'w' && scr_state != INVEN_SCR)
		    inven_screen(WEAR_SCR);
		  else
		    inven_screen(INVEN_SCR);
		  if (which == ' ')
		    selecting = FALSE;
		}
	      /* Swap screens (for drop) */
	      else if (which == '/' && swap[0])
		{
		  if (command == 'd')
		    command = 'r';
		  else
		    command = 'd';
		  if (scr_state == EQUIP_SCR)
		    inven_screen(INVEN_SCR);
		  else if (scr_state == INVEN_SCR)
		    inven_screen(EQUIP_SCR);
		}
	      else if ((which < from + 'a' || which > to + 'a')
		       && (which < from + 'A' || which > to + 'A'))
		bell();
	      else  /* Found an item! */
		{
		  if (isupper((int)which))
		    item = which - 'A';
		  else
		    item = which - 'a';
		  if (command == 'r' || command == 't')
		    {
		      /* Get its place in the equipment list. */
		      tmp = item;
		      item = 21;
		      do
			{
			  item++;
			  if (inventory[item].tval != TV_NOTHING)
			    tmp--;
			}
		      while (tmp >= 0);
		      if (isupper((int)which) && !verify(prompt, item))
			item = -1;
		      else if (TR_CURSED & inventory[item].flags)
			{
			  prt(Inven_window, "Hmmm, it seems to be cursed.",
			      1, 0);
			  item = -1;
			}
		      else if (command == 't' &&
			       !inven_check_num(&inventory[item]))
			{
			  if (cave[char_row][char_col].tptr != 0)
			    {
			      prt(Inven_window, "You can't carry it.", 1, 0);
			      item = -1;
			    }
			  else if (get_check(Status_window, "You can't carry it.  Drop it?"))
			    command = 'r';
			  else
			    item = -1;
			}
		      if (item >= 0)
			{
			  if (command == 'r')
			    inven_drop(item, TRUE);
			  else
			    {
			      slot = inven_carry(&inventory[item]);
			      takeoff(item, slot);
			    }
			  check_strength();
			  free_turn_flag = FALSE;
			  if (command == 'r')
			    selecting = FALSE;
			}
		    }
		  else if (command == 'w')
		    {
		      /* Wearing. Go to a bit of trouble over replacing
			 existing equipment. */
		      if (isupper((int)which) && !verify(prompt, item))
			item = -1;
		      else switch(inventory[item].tval)
			{ /* Slot for equipment	   */
			case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW:
			case TV_BOW: case TV_HAFTED: case TV_POLEARM:
			case TV_SWORD: case TV_DIGGING:
			  slot = INVEN_WIELD; break;
			case TV_LIGHT: slot = INVEN_LIGHT; break;
			case TV_BOOTS: slot = INVEN_FEET; break;
			case TV_GLOVES: slot = INVEN_HANDS; break;
			case TV_CLOAK: slot = INVEN_OUTER; break;
			case TV_HELM: slot = INVEN_HEAD; break;
			case TV_SHIELD: slot = INVEN_ARM; break;
			case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
			  slot = INVEN_BODY; break;
			case TV_AMULET: slot = INVEN_NECK; break;
			case TV_RING:
			  if (inventory[INVEN_RIGHT].tval == TV_NOTHING)
			    slot = INVEN_RIGHT;
			  else if (inventory[INVEN_LEFT].tval == TV_NOTHING)
			    slot = INVEN_LEFT;
			  else
			    {
			      slot = 0;
			      /* Rings. Give some choice over where they go. */
			      do
				{
				  if (!get_com(Inven_window, 
			       "Put ring on which hand (l/r/L/R)?", &query))
				    {
				      item = -1;
				      slot = -1;
				    }
				  else if (query == 'l')
				    slot = INVEN_LEFT;
				  else if (query == 'r')
				    slot = INVEN_RIGHT;
				  else
				    {
				      if (query == 'L')
					slot = INVEN_LEFT;
				      else if (query == 'R')
					slot = INVEN_RIGHT;
				      else
					bell();
				      if (slot && !verify("Replace", slot))
					slot = 0;
				    }
				}
			      while(slot == 0);
			    }
			  break;
			default:
		  prt(Inven_window,
		      "IMPOSSIBLE: I don't see how you can use that.", 1, 0);
			  item = -1;
			  break;
			}
		      if (item >= 0 && inventory[slot].tval != TV_NOTHING)
			{
			  if (TR_CURSED & inventory[slot].flags)
			    {
			      objdes(prt1, &inventory[slot], FALSE);
			      (void) sprintf(prt2, "The %s you are ", prt1);
			      if (slot == INVEN_HEAD)
				(void) strcat(prt2, "wielding ");
			      else
				(void) strcat(prt2, "wearing ");
			      prt(Inven_window,
				  strcat(prt2, "appears to be cursed."), 1, 0);
			      item = -1;
			    }
			  else if (inventory[item].subval == ITEM_GROUP_MIN &&
				   inventory[item].number > 1 &&
				   !inven_check_num(&inventory[slot]))
			    {
			      /* this can happen if try to wield a torch, and
				 have more than one in your inventory */
			   prt(Inven_window, "You will have to drop something first.", 1, 0);
			      item = -1;
			    }
			}
		      if (item >= 0)
			{
			  /* OK. Wear it. */
			  free_turn_flag = FALSE;

			  /* first remove new item from inventory */
			  tmp_obj = inventory[item];
			  i_ptr = &tmp_obj;

			  wear_high--;
			  /* Fix for torches	   */
			  if (i_ptr->number > 1
			      && i_ptr->subval <= ITEM_SINGLE_STACK_MAX)
			    {
			      i_ptr->number = 1;
			      wear_high++;
			    }
			  inven_weight += i_ptr->weight*i_ptr->number;
			  inven_destroy(item);	/* Subtracts weight */

			  /* second, add old item to inv and remove from
			     equipment list, if necessary */
			  i_ptr = &inventory[slot];
			  if (i_ptr->tval != TV_NOTHING)
			    {
			      tmp2 = inven_ctr;
			      tmp = inven_carry(i_ptr);
			      /* if item removed did not stack with anything in
				 inventory, then increment wear_high */
			      if (inven_ctr != tmp2)
				wear_high++;
			      takeoff(slot, tmp);
			    }

			  /* third, wear new item */			
			  *i_ptr = tmp_obj;
			  equip_ctr++;
			  py_bonuses(i_ptr, 1);
			  if (slot == INVEN_WIELD)
			    string = "You are wielding";
			  else if (slot == INVEN_LIGHT)
			    string = "Your light source is";
			  else
			    string = "You are wearing";
			  objdes(prt2, i_ptr, TRUE);
			  /* Get the right equipment letter. */
			  tmp = INVEN_WIELD;
			  item = 0;
			  while (tmp != slot)
			    if (inventory[tmp++].tval != TV_NOTHING)
			      item++;

			  (void) sprintf(prt1, "%s %s (%c)", string, prt2,
					 'a'+item);
			  prt(Inven_window, prt1, 1, 0);
			  check_strength();
			  if (i_ptr->flags & TR_CURSED)
			    {
			      prt(Inven_window, "Oops! It feels deathly cold!", 1, 0);
			      add_inscribe(i_ptr, ID_DAMD);
			      /* To force a cost of 0, even if unidentified. */
			      i_ptr->cost = -1;
			    }
			}
		    }
		  else /* command == 'd' */
		    {
		      if (inventory[item].number > 1)
			{
			  objdes(prt1, &inventory[item], TRUE);
			  prt1[strlen(prt1)-1] = '?';
			  (void) sprintf(prt2, "Drop all %s [y/n]", prt1);
			  prt1[strlen(prt1)-1] = '.';
			  prt(Inven_window, prt2, 1, 0);
			  query = inkey(TRUE);
			  if (query != 'y' && query != 'n')
			    {
			      if (query != ESCAPE)
				bell();
			      erase_line(Inven_window, 0, 1);
			      item = -1;
			    }
			}
		      else if (isupper((int)which) && !verify(prompt, item))
			item = -1;
		      else
			query = 'y';
		      if (item >= 0)
			{
			  free_turn_flag = FALSE;    /* Player turn   */
			  inven_drop(item, query == 'y');
			  check_strength();
			}
		      selecting = FALSE;
		    }
		  if (free_turn_flag == FALSE && scr_state == BLANK_SCR)
		    selecting = FALSE;
		}
	    }
	}
      if (which == ESCAPE || scr_state == BLANK_SCR)
	command = ESCAPE;
      else if (!free_turn_flag)
	{
	  /* Save state for recovery if they want to call us again next turn.*/
	  if (selecting)
	    doing_inven = command;
	  else
	    doing_inven = ' ';	/* A dummy command to recover screen. */
	  screen_change = FALSE;/* This lets us know if the world changes */
	  command = ESCAPE;
	}
      else
	{
	  /* Put an appropriate header. */
	  if (scr_state == INVEN_SCR)
	    {
	      (void) sprintf(prt1,
		  "You are carrying %d.%d pounds. In your pack there is %s",
		  inven_weight / 10, inven_weight % 10,
		  (inven_ctr == 0 ? "nothing." : "-"));
	      prt(Inven_window, prt1, 1, 0);
	    }
	  else if (scr_state == WEAR_SCR)
	    {
	      if (wear_high < wear_low)
		prt(Inven_window, "You have nothing you could wield.", 1, 0);
	      else
		prt(Inven_window, "You could wield -", 1, 0);
	    }
	  else if (scr_state == EQUIP_SCR)
	    {
	      if (equip_ctr == 0)
		prt(Inven_window, "You are not using anything.", 1, 0);
	      else
		prt(Inven_window, "You are using -", 1, 0);
	    }
	  else
	    prt(Inven_window, "Allowed commands:", 23, 0);
	  erase_line(Inven_window, 24, 0);
	  prt(Inven_window, "e/i/t/w/x/d/?/ESC:", 24, 0);
	  command = inkeyr(Inven_window);
	  erase_line(Inven_window, 24, 0);
	}
    }
  while (command != ESCAPE);
  calc_bonuses();
  if (!doing_inven)
    DestroyDWin(Inven_window);
}

int i_scr, first, last;

update_item()
{
  if (i_scr > 0)
    (void) show_inven (first, last, FALSE, 1);
  else
    (void) show_equip (FALSE, 1);
}

/* Get the ID of an item and return the CTR value of it	-RAK-	*/
int get_item(com_val, pmt, i, j)
int *com_val;
char *pmt;
int i, j;
{
  vtype out_val;
  char which;
  register int test_flag, item;
  int full, redraw, i_flag = FALSE;

  item = FALSE;
  redraw = FALSE;
  *com_val = 0;
  i_scr = 1;
  first = i;
  last = j;
  if (j > INVEN_WIELD)
    {
      full = TRUE;
      if (inven_ctr == 0)
	{
	  i_scr = 0;
	  j = equip_ctr - 1;
	}
      else
	j = inven_ctr - 1;
    }
  else
    full = FALSE;

  if (inven_ctr > 0 || (full && equip_ctr > 0))
    {
      do
	{
	  if (redraw && i_flag)
	    {
	      update_item();
            }
	  if (full)
	    (void) sprintf(out_val,
			   "(%s: %c-%c,%s / for %s, or ESC) %s",
			   (i_scr > 0 ? "Inven" : "Equip"), i+'a', j+'a',
			   (redraw ? "" : " * to see,"),
			   (i_scr > 0 ? "Equip" : "Inven"), pmt);
	  else
	    (void) sprintf(out_val,
			   "(Items %c-%c,%s ESC to exit) %s", i+'a', j+'a',
			   (redraw ? "" : " * for inventory list,"), pmt);
	  test_flag = FALSE;
	  msg_print(out_val);
	  do
	    {
	      if (i_flag)
		which = inkeyr(Inven_window);
	      else
		which = inkey(TRUE);
	      switch(which)
		{
		case ESCAPE:
		  test_flag = TRUE;
		  free_turn_flag = TRUE;
		  i_scr = -1;
		  break;
		case '/':
		  if (full)
		    {
		      if (i_scr > 0)
			{
			  if (equip_ctr == 0)
			    {
			     msg_print("But you're not using anything -more-");
			      (void) inkey(TRUE);
			    }
			  else
			    {
			      i_scr = 0;
			      test_flag = TRUE;
			      if (redraw)
				{
				  j = equip_ctr;
				  while (j < inven_ctr)
				    {
				      j++;
				      erase_line(Inven_window, j, 0);
				    }
				}
			      j = equip_ctr - 1;
			    }
			  msg_print(out_val);
			}
		      else
			{
			  if (inven_ctr == 0)
			    {
			  msg_print("But you're not carrying anything -more-");
			      (void) inkey(TRUE);
			    }
			  else
			    {
			      i_scr = 1;
			      test_flag = TRUE;
			      if (redraw)
				{
				  j = inven_ctr;
				  while (j < equip_ctr)
				    {
				      j++;
				      erase_line (Inven_window, j, 0);
				    }
				}
			      j = inven_ctr - 1;
			    }
			}
		    }
		  break;
		case '*':
		  Inven_window = CreateDWin(340, (j - i + 3) * mfontheight,
					      50, 0, DefaultRootDWin,
					      mfgpix, mbgpix);
		  XSetTransientForHint(p_disp, Inven_window->win,
				       DefaultRootWindow(p_disp));
		  XSelectInput(p_disp, Inven_window->win, ExposureMask |
			       KeyPressMask | ButtonPressMask);
		  XMapWindow(p_disp, Inven_window->win);
		  clear_screen(Inven_window);
		  i_flag = TRUE;
		  if (!redraw)
		    {
		      test_flag = TRUE;
		      redraw = TRUE;
		    }
		  break;
		default:
		  if (isupper((int)which))
		    *com_val = which - 'A';
		  else
		    *com_val = which - 'a';
		  if ((*com_val >= i) && (*com_val <= j))
		    {
		      if (i_scr == 0)
			{
			  i = 21;
			  j = *com_val;
			  do
			    {
			      while (inventory[++i].tval == TV_NOTHING);
			      j--;
			    }
			  while (j >= 0);
			  *com_val = i;
			}
		      if (isupper((int)which) && !verify("Try", *com_val))
			{
			  test_flag = TRUE;
			  free_turn_flag = TRUE;
			  i_scr = -1;
			  break;
			}
		      test_flag = TRUE;
		      item = TRUE;
		      i_scr = -1;
		    }
		  else
		    bell();
		  break;
		}
	    }
	  while (!test_flag);
	}
      while (i_scr >= 0);
      erase_line(Status_window, 0, 0);
    }
  else
    msg_print("You are not carrying anything.");
  if (i_flag)
    DestroyDWin(Inven_window);
  return(item);
}

int get_item_for_sale(com_val, pmt, i, j)
int *com_val;
char *pmt;
int i, j;
{
  vtype out_val;
  char which;
  register int test_flag, item;
  int full, redraw, i_flag = FALSE;

  item = FALSE;
  redraw = FALSE;
  *com_val = 0;
  i_scr = 1;
  first = i;
  last = j;
  if (j > INVEN_WIELD)
    {
      full = TRUE;
      if (inven_ctr == 0)
	{
	  i_scr = 0;
	  j = equip_ctr - 1;
	}
      else
	j = inven_ctr - 1;
    }
  else
    full = FALSE;

  if (inven_ctr > 0 || (full && equip_ctr > 0))
    {
      do
	{
	  if (redraw && i_flag)
	    {
	      update_item();
            }
	  if (full)
	    (void) sprintf(out_val,
			   "(%s: %c-%c,%s / for %s, or ESC) %s",
			   (i_scr > 0 ? "Inven" : "Equip"), i+'a', j+'a',
			   (redraw ? "" : " * to see,"),
			   (i_scr > 0 ? "Equip" : "Inven"), pmt);
	  else
	    (void) sprintf(out_val,
			   "(Items %c-%c,%s ESC to exit) %s", i+'a', j+'a',
			   (redraw ? "" : " * for inventory list,"), pmt);
	  test_flag = FALSE;
	  prt(Store_window, out_val, 1, 0);
	  do
	    {
	      if (i_flag)
		which = inkeyr(Inven_window);
	      else
		which = inkey(TRUE);
	      switch(which)
		{
		case ESCAPE:
		  test_flag = TRUE;
		  free_turn_flag = TRUE;
		  i_scr = -1;
		  break;
		case '/':
		  if (full)
		    {
		      if (i_scr > 0)
			{
			  if (equip_ctr == 0)
			    {
			     msg_print("But you're not using anything -more-");
			      (void) inkey(TRUE);
			    }
			  else
			    {
			      i_scr = 0;
			      test_flag = TRUE;
			      if (redraw)
				{
				  j = equip_ctr;
				  while (j < inven_ctr)
				    {
				      j++;
				      erase_line(Inven_window, j, 0);
				    }
				}
			      j = equip_ctr - 1;
			    }
			  msg_print(out_val);
			}
		      else
			{
			  if (inven_ctr == 0)
			    {
			  msg_print("But you're not carrying anything -more-");
			      (void) inkey(TRUE);
			    }
			  else
			    {
			      i_scr = 1;
			      test_flag = TRUE;
			      if (redraw)
				{
				  j = inven_ctr;
				  while (j < equip_ctr)
				    {
				      j++;
				      erase_line (Inven_window, j, 0);
				    }
				}
			      j = inven_ctr - 1;
			    }
			}
		    }
		  break;
		case '*':
		  Inven_window = CreateDWin(340, (j - i + 3) * mfontheight,
					      500, 0,
					      DefaultRootDWin,
					      mfgpix, mbgpix);
		  XSetTransientForHint(p_disp, Inven_window->win,
				       DefaultRootWindow(p_disp));
		  XSelectInput(p_disp, Inven_window->win, ExposureMask |
			       KeyPressMask | ButtonPressMask);
		  XMapWindow(p_disp, Inven_window->win);
		  clear_screen(Inven_window);
		  i_flag = TRUE;
		  if (!redraw)
		    {
		      test_flag = TRUE;
		      redraw = TRUE;
		    }
		  break;
		default:
		  if (isupper((int)which))
		    *com_val = which - 'A';
		  else
		    *com_val = which - 'a';
		  if ((*com_val >= i) && (*com_val <= j))
		    {
		      if (i_scr == 0)
			{
			  i = 21;
			  j = *com_val;
			  do
			    {
			      while (inventory[++i].tval == TV_NOTHING);
			      j--;
			    }
			  while (j >= 0);
			  *com_val = i;
			}
		      if (isupper((int)which) && !verify("Try", *com_val))
			{
			  test_flag = TRUE;
			  free_turn_flag = TRUE;
			  i_scr = -1;
			  break;
			}
		      test_flag = TRUE;
		      item = TRUE;
		      i_scr = -1;
		    }
		  else
		    bell();
		  break;
		}
	    }
	  while (!test_flag);
	}
      while (i_scr >= 0);
      erase_line(Status_window, 0, 0);
    }
  else
    msg_print("You are not carrying anything.");
  if (i_flag)
    DestroyDWin(Inven_window);
  return(item);
}

/* I may have written the town level code, but I'm not exactly	 */
/* proud of it.	 Adding the stores required some real slucky	 */
/* hooks which I have not had time to re-think.		 -RAK-	 */

/* Returns true if player has no light			-RAK-	*/
int no_light()
{
  register cave_type *c_ptr;

  c_ptr = &cave[char_row][char_col];
  if (!c_ptr->tl && !c_ptr->pl)
    return TRUE;
  return FALSE;
}


/* map rogue_like direction commands into numbers */
static char map_roguedir(comval)
register char comval;
{
  switch(comval)
    {
    case 'h':
      comval = '4';
      break;
    case 'y':
      comval = '7';
      break;
    case 'k':
      comval = '8';
      break;
    case 'u':
      comval = '9';
      break;
    case 'l':
      comval = '6';
      break;
    case 'n':
      comval = '3';
      break;
    case 'j':
      comval = '2';
      break;
    case 'b':
      comval = '1';
      break;
    case '.':
      comval = '5';
      break;
    }
  return(comval);
}


/* Prompts for a direction				-RAK-	*/
/* Direction memory added, for repeated commands.  -CJS */
int get_dir(prompt, dir)
char *prompt;
int *dir;
{
  char command;
  int save;
  static char prev_dir;		/* Direction memory. -CJS- */

  if (default_dir)	/* used in counted commands. -CJS- */
    {
      *dir = prev_dir;
      return TRUE;
    }
  if (prompt == NULL)
    prompt = "Which direction?";
  for (;;)
    {
      save = command_count;	/* Don't end a counted command. -CJS- */
      if (!get_com(Status_window, prompt, &command))
	{
	  free_turn_flag = TRUE;
	  return FALSE;
	}
      command_count = save;
      if (rogue_like_commands)
	command = map_roguedir(command);
      if (command >= '1' && command <= '9' && command != '5')
	{
	  prev_dir = command - '0';
	  *dir = prev_dir;
	  return TRUE;
	}
      bell();
    }
}



/* Similar to get_dir, except that no memory exists, and it is		-CJS-
   allowed to enter the null direction. */
int get_alldir(prompt, dir)
char *prompt;
int *dir;
{
  char command;

  for(;;)
    {
      if (!get_com(Status_window, prompt, &command))
	{
	  free_turn_flag = TRUE;
	  return FALSE;
	}
      if (rogue_like_commands)
	command = map_roguedir(command);
      if (command >= '1' && command <= '9')
	{
	  *dir = command - '0';
	  return TRUE;
	}
      bell();
    }
}


/* Moves creature record from one space to another	-RAK-	*/
void move_rec(y1, x1, y2, x2)
register int y1, x1, y2, x2;
{
  int tmp;

  /* this always works correctly, even if y1==y2 and x1==x2 */
  tmp = cave[y1][x1].cptr;
  cave[y1][x1].cptr = 0;
  cave[y2][x2].cptr = tmp;
}


/* Room is lit, make it appear				-RAK-	*/
void light_room(y, x)
int y, x;
{
  register int i, j, start_col, end_col;
  int tmp1, tmp2, start_row, end_row;
  register cave_type *c_ptr;

  tmp1 = (SCREEN_HEIGHT/2);
  tmp2 = (SCREEN_WIDTH /2);
  start_row = (y/tmp1)*tmp1;
  start_col = (x/tmp2)*tmp2;
  end_row = start_row + tmp1 - 1;
  end_col = start_col + tmp2 - 1;
  for (i = start_row; i <= end_row; i++)
    for (j = start_col; j <= end_col; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->lr & !c_ptr->pl)
	  {
	    if (c_ptr->fval == DARK_FLOOR)
	      c_ptr->fval = LIGHT_FLOOR;
	    c_ptr->pl = TRUE;
	    PrintMapChar(loc_symbol(i, j), j - panel_col_min,
			 i - panel_row_min);
	  }
      }
}


/* Lights up given location				-RAK-	*/
void lite_spot(y, x)
register int y, x;
{
  if (panel_contains(y, x))
    PrintMapChar(loc_symbol(y, x), x - panel_col_min, y - panel_row_min);
}


/* Normal movement					*/
/* When FIND_FLAG,  light only permanent features	*/
static void sub1_move_light(y1, x1, y2, x2)
register int x1, x2;
int y1, y2;
{
  register int i, j;
  register cave_type *c_ptr;
  int tval, top, left, bottom, right;

  if (light_flag)
    {
      for (i = y1-1; i <= y1+1; i++)	   /* Turn off lamp light	*/
	for (j = x1-1; j <= x1+1; j++)
	  cave[i][j].tl = FALSE;
      if (find_flag && !find_prself)
	light_flag = FALSE;
    }
  else if (!find_flag || find_prself)
    light_flag = TRUE;

  for (i = y2-1; i <= y2+1; i++)
    for (j = x2-1; j <= x2+1; j++)
      {
	c_ptr = &cave[i][j];
	/* only light up if normal movement */
	if (light_flag)
	  c_ptr->tl = TRUE;
	if (c_ptr->fval >= MIN_CAVE_WALL)
	  c_ptr->pl = TRUE;
	else if (!c_ptr->fm && c_ptr->tptr != 0)
	  {
	    tval = t_list[c_ptr->tptr].tval;
	    if ((tval >= TV_MIN_VISIBLE) && (tval <= TV_MAX_VISIBLE))
	      c_ptr->fm = TRUE;
	  }
      }

  /* From uppermost to bottom most lines player was on.	 */
  if (y1 < y2)
    {
      top = y1 - 1;
      bottom = y2 + 1;
    }
  else
    {
      top = y2 - 1;
      bottom = y1 + 1;
    }
  if (x1 < x2)
    {
      left = x1 - 1;
      right = x2 + 1;
    }
  else
    {
      left = x2 - 1;
      right = x1 + 1;
    }
  for (i = top; i <= bottom; i++)
    for (j = left; j <= right; j++)   /* Leftmost to rightmost do*/
      PrintMapChar(loc_symbol(i, j), j - panel_col_min, i - panel_row_min);
}


/* When blinded,  move only the player symbol.		*/
/* With no light,  movement becomes involved.		*/
static void sub3_move_light(y1, x1, y2, x2)
register int y1, x1;
int y2, x2;
{
  register int i, j;

  if (light_flag)
    {
      for (i = y1-1; i <= y1+1; i++)
	for (j = x1-1; j <= x1+1; j++)
	  {
	    cave[i][j].tl = FALSE;
	    PrintMapChar(loc_symbol(i, j), j - panel_col_min,
			 i - panel_row_min);
	  }
      light_flag = FALSE;
    }
  else
    PrintMapChar(loc_symbol(y1, x1), x1 - panel_col_min, y1 - panel_row_min);

  if (!find_flag || find_prself)
    PrintMapChar('@', x2 - panel_col_min, y2 - panel_row_min);
}


/* Package for moving the character's light about the screen	 */
/* Four cases : Normal, Finding, Blind, and Nolight	 -RAK-	 */
void move_light(y1, x1, y2, x2)
int y1, x1, y2, x2;
{
  if (py.flags.blind > 0 || !player_light)
    sub3_move_light(y1, x1, y2, x2);
  else
    sub1_move_light(y1, x1, y2, x2);
}


/* Something happens to disturb the player.		-CJS-
   The first arg indicates a major disturbance, which affects search.
   The second arg indicates a light change. */
void disturb(s, l)
int s, l;
{
  command_count = 0;
  if (s && search_flag)
    search_off();
  if (py.flags.rest > 0)
    rest_off();
  if (l || find_flag)
    {
      find_flag = FALSE;
      check_view();
    }
}


/* Search Mode enhancement				-RAK-	*/
void search_on()
{
  search_flag = TRUE;
  change_speed(1);
  py.flags.status |= PY_SEARCH;
  prt_state();
  prt_speed();
  py.flags.food_digested++;
}

search_off()
{
  search_flag = FALSE;
  check_view();
  change_speed(-1);
  py.flags.status &= ~PY_SEARCH;
  prt_state();
  prt_speed();
  py.flags.food_digested--;
}


/* Resting allows a player to safely restore his hp	-RAK-	*/
void rest()
{
  int rest_num;
  vtype rest_str;

  if (command_count > 0)
    {
      rest_num = command_count;
      command_count = 0;
    }
  else
    {
      msg_print("Rest for how long? ");
      rest_num = 0;
      if (get_string(Status_window, rest_str, 0, 19, 5))
	rest_num = atoi(rest_str);
    }
  if (rest_num > 0)
    {
      if (search_flag)
	search_off();
      py.flags.rest = rest_num;
      py.flags.status |= PY_REST;
      prt_state();
      py.flags.food_digested--;
      msg_print ("Press any key to stop resting...");
      put_qio();
    }
  else
    {
      erase_line(Status_window, 0, 0);
      free_turn_flag = TRUE;
    }
}

rest_off()
{
  py.flags.rest = 0;
  py.flags.status &= ~PY_REST;
  prt_state();
  msg_print(NULL); /* flush last message, or delete "press any key" message */
  py.flags.food_digested++;
}


/* Attacker's level and plusses,  defender's AC		-RAK-	*/
int test_hit(bth, level, pth, ac, attack_type)
int bth, level, pth, ac, attack_type;
{
  register int i, die;

  disturb (1, 0);
  i = bth + pth * BTH_PLUS_ADJ
    + (level * class_level_adj[py.misc.pclass][attack_type]);
  /* pth could be less than 0 if player wielding weapon too heavy for him */
  /* always miss 1 out of 20, always hit 1 out of 20 */
  die = randint (20);
  if ((die != 1) && ((die == 20)
		     || ((i > 0) && (randint (i) > ac))))  /* normal hit */
    return TRUE;
  else
    return FALSE;
}


/* Decreases players hit points and sets death flag if necessary*/
/*							 -RAK-	 */
void take_hit(damage, hit_from)
int damage;
char *hit_from;
{
  if (py.flags.invuln > 0)  damage = 0;
  py.misc.chp -= damage;
  if (py.misc.chp < 0)
    {
      if (!death)
	{
	  death = TRUE;
	  (void) strcpy(died_from, hit_from);
	  total_winner = FALSE;
	}
      new_level_flag = TRUE;
    }
  else
    prt_chp();
}


/* Change a trap from invisible to visible		-RAK-	*/
/* Note: Secret doors are handled here				 */
void change_trap(y, x)
register int y, x;
{
  register cave_type *c_ptr;
  register inven_type *t_ptr;

  c_ptr = &cave[y][x];
  t_ptr = &t_list[c_ptr->tptr];
  if (t_ptr->tval == TV_INVIS_TRAP)
    {
      t_ptr->tval = TV_VIS_TRAP;
      lite_spot(y, x);
    }
  else if (t_ptr->tval == TV_SECRET_DOOR)
    {
      /* change secret door to closed door */
      t_ptr->index = OBJ_CLOSED_DOOR;
      t_ptr->tval = object_list[OBJ_CLOSED_DOOR].tval;
      t_ptr->tchar = object_list[OBJ_CLOSED_DOOR].tchar;
      lite_spot(y, x);
    }
}


/* Searches for hidden things.			-RAK-	*/
void search(y, x, chance)
int y, x, chance;
{
  register int i, j;
  register cave_type *c_ptr;
  register inven_type *t_ptr;
  register struct flags *p_ptr;
  bigvtype tmp_str, tmp_str2;

  p_ptr = &py.flags;
  if (p_ptr->confused > 0)
    chance = chance / 10;
  if ((p_ptr->blind > 0) || no_light())
    chance = chance / 10;
  if (p_ptr->image > 0)
    chance = chance / 10;
  for (i = (y - 1); i <= (y + 1); i++)
    for (j = (x - 1); j <= (x + 1); j++)
      if (randint(100) < chance)	/* always in_bounds here */
	{
	  c_ptr = &cave[i][j];
	  /* Search for hidden objects		   */
	  if (c_ptr->tptr != 0)
	    {
	      t_ptr = &t_list[c_ptr->tptr];
	      /* Trap on floor?		       */
	      if (t_ptr->tval == TV_INVIS_TRAP)
		{
		  objdes(tmp_str2, t_ptr, TRUE);
		  (void) sprintf(tmp_str,"You have found %s",tmp_str2);
		  msg_print(tmp_str);
		  change_trap(i, j);
		  end_find();
		}
	      /* Secret door?		       */
	      else if (t_ptr->tval == TV_SECRET_DOOR)
		{
		  msg_print("You have found a secret door.");
		  change_trap(i, j);
		  end_find();
		}
	      /* Chest is trapped?	       */
	      else if (t_ptr->tval == TV_CHEST)
		{
		  /* mask out the treasure bits */
		  if ((t_ptr->flags & CH_TRAPPED) > 1)
		    if (!known2_p(t_ptr))
		      {
			known2(t_ptr);
			msg_print("You have discovered a trap on the chest!");
		      }
		    else
		      msg_print("The chest is trapped!");
		}
	    }
	}
}


/* The running algorithm:			-CJS-

   Overview: You keep moving until something interesting happens.
   If you are in an enclosed space, you follow corners. This is
   the usual corridor scheme. If you are in an open space, you go
   straight, but stop before entering enclosed space. This is
   analogous to reaching doorways. If you have enclosed space on
   one side only (that is, running along side a wall) stop if
   your wall opens out, or your open space closes in. Either case
   corresponds to a doorway.

   What happens depends on what you can really SEE. (i.e. if you
   have no light, then running along a dark corridor is JUST like
   running in a dark room.) The algorithm works equally well in
   corridors, rooms, mine tailings, earthquake rubble, etc, etc.

   These conditions are kept in static memory:
	find_openarea	 You are in the open on at least one
			 side.
	find_breakleft	 You have a wall on the left, and will
			 stop if it opens
	find_breakright	 You have a wall on the right, and will
			 stop if it opens

   To initialize these conditions is the task of find_init. If
   moving from the square marked @ to the square marked . (in the
   two diagrams below), then two adjacent sqares on the left and
   the right (L and R) are considered. If either one is seen to
   be closed, then that side is considered to be closed. If both
   sides are closed, then it is an enclosed (corridor) run.

	 LL		L
	@.	       L.R
	 RR	       @R

   Looking at more than just the immediate squares is
   significant. Consider the following case. A run along the
   corridor will stop just before entering the center point,
   because a choice is clearly established. Running in any of
   three available directions will be defined as a corridor run.
   Note that a minor hack is inserted to make the angled corridor
   entry (with one side blocked near and the other side blocked
   further away from the runner) work correctly. The runner moves
   diagonally, but then saves the previous direction as being
   straight into the gap. Otherwise, the tail end of the other
   entry would be perceived as an alternative on the next move.

	   #.#
	  ##.##
	  .@...
	  ##.##
	   #.#

   Likewise, a run along a wall, and then into a doorway (two
   runs) will work correctly. A single run rightwards from @ will
   stop at 1. Another run right and down will enter the corridor
   and make the corner, stopping at the 2.

	#@	  1
	########### ######
	2	    #
	#############
	#

   After any move, the function area_affect is called to
   determine the new surroundings, and the direction of
   subsequent moves. It takes a location (at which the runner has
   just arrived) and the previous direction (from which the
   runner is considered to have come). Moving one square in some
   direction places you adjacent to three or five new squares
   (for straight and diagonal moves) to which you were not
   previously adjacent.

       ...!	  ...	       EG Moving from 1 to 2.
       .12!	  .1.!		  . means previously adjacent
       ...!	  ..2!		  ! means newly adjacent
		   !!!

   You STOP if you can't even make the move in the chosen
   direction. You STOP if any of the new squares are interesting
   in any way: usually containing monsters or treasure. You STOP
   if any of the newly adjacent squares seem to be open, and you
   are also looking for a break on that side. (i.e. find_openarea
   AND find_break) You STOP if any of the newly adjacent squares
   do NOT seem to be open and you are in an open area, and that
   side was previously entirely open.

   Corners: If you are not in the open (i.e. you are in a
   corridor) and there is only one way to go in the new squares,
   then turn in that direction. If there are more than two new
   ways to go, STOP. If there are two ways to go, and those ways
   are separated by a square which does not seem to be open, then
   STOP.

   Otherwise, we have a potential corner. There are two new open
   squares, which are also adjacent. One of the new squares is
   diagonally located, the other is straight on (as in the
   diagram). We consider two more squares further out (marked
   below as ?).
	  .X
	 @.?
	  #?
   If they are both seen to be closed, then it is seen that no
   benefit is gained from moving straight. It is a known corner.
   To cut the corner, go diagonally, otherwise go straight, but
   pretend you stepped diagonally into that next location for a
   full view next time. Conversely, if one of the ? squares is
   not seen to be closed, then there is a potential choice. We check
   to see whether it is a potential corner or an intersection/room entrance.
   If the square two spaces straight ahead, and the space marked with 'X'
   are both blank, then it is a potential corner and enter if find_examine
   is set, otherwise must stop because it is not a corner. */

/* The cycle lists the directions in anticlockwise order, for	-CJS-
   over two complete cycles. The chome array maps a direction on
   to its position in the cycle.
*/
static int cycle[] = { 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };
static int chome[] = { -1, 8, 9, 10, 7, -1, 11, 6, 5, 4 };
static int find_openarea, find_breakright, find_breakleft, find_prevdir;
static int find_direction; /* Keep a record of which way we are going. */

void find_init(dir)
int dir;
{
  int row, col, deepleft, deepright;
  register int i, shortleft, shortright;

  row = char_row;
  col = char_col;
  if (!mmove(dir, &row, &col))
    find_flag = FALSE;
  else
    {
      find_direction = dir;
      find_flag = 1;
      find_breakright = find_breakleft = FALSE;
      find_prevdir = dir;
      if (py.flags.blind < 1)
	{
	  i = chome[dir];
	  deepleft = deepright = FALSE;
	  shortright = shortleft = FALSE;
	  if (see_wall(cycle[i+1], char_row, char_col))
	    {
	      find_breakleft = TRUE;
	      shortleft = TRUE;
	    }
	  else if (see_wall(cycle[i+1], row, col))
	    {
	      find_breakleft = TRUE;
	      deepleft = TRUE;
	    }
	  if (see_wall(cycle[i-1], char_row, char_col))
	    {
	      find_breakright = TRUE;
	      shortright = TRUE;
	    }
	  else if (see_wall(cycle[i-1], row, col))
	    {
	      find_breakright = TRUE;
	      deepright = TRUE;
	    }
	  if (find_breakleft && find_breakright)
	    {
	      find_openarea = FALSE;
	      if (dir & 1)
		{		/* a hack to allow angled corridor entry */
		  if (deepleft && !deepright)
		    find_prevdir = cycle[i-1];
		  else if (deepright && !deepleft)
		    find_prevdir = cycle[i+1];
		}
	      /* else if there is a wall two spaces ahead and seem to be in a
		 corridor, then force a turn into the side corridor, must
		 be moving straight into a corridor here */
	      else if (see_wall(cycle[i], row, col))
		{
		  if (shortleft && !shortright)
		    find_prevdir = cycle[i-2];
		  else if (shortright && !shortleft)
		    find_prevdir = cycle[i+2];
		}
	    }
	  else
	    find_openarea = TRUE;
	}
    }
  move_char(dir, TRUE);
  if (find_flag == FALSE)
    command_count = 0;
}

void find_run()
{
  /* prevent infinite loops in find mode, will stop after moving 100 times */
  if (find_flag++ > 100)
    {
      msg_print("You stop running to catch your breath.");
      end_find();
    }
  else
    move_char(find_direction, TRUE);
}

/* Switch off the run flag - and get the light correct. -CJS- */
end_find()
{
  if (find_flag)
    {
      find_flag = FALSE;
      move_light(char_row, char_col, char_row, char_col);
    }
}

/* Do we see a wall? Used in running.		-CJS- */
static int see_wall(dir, y, x)
int dir, y, x;
{
  char c;

  if (!mmove(dir, &y, &x))	/* check to see if movement there possible */
    return TRUE;
#ifdef MSDOS
  else if ((c = loc_symbol(y, x)) == wallsym || c == '%')
#else
#ifdef ATARIST_MWC
  else if ((c = loc_symbol(y, x)) == (unsigned char)240 || c == '%')
#else
  else if ((c = loc_symbol(y, x)) == '#' || c == '%')
#endif
#endif
    return TRUE;
  else
    return FALSE;
}

/* Do we see anything? Used in running.		-CJS- */
static int see_nothing(dir, y, x)
int dir, y, x;
{
  if (!mmove(dir, &y, &x))	/* check to see if movement there possible */
    return FALSE;
  else if (loc_symbol(y, x) == ' ')
    return TRUE;
  else
    return FALSE;
}


/* Determine the next direction for a run, or if we should stop.  -CJS- */
void area_affect(dir, y, x)
int dir, y, x;
{
  int newdir, t, inv, check_dir, row, col;
  register int i, max, option, option2;
  register cave_type *c_ptr;

  check_dir = 0;

  if (py.flags.blind < 1)
    {
      option = 0;
      option2 = 0;
      dir = find_prevdir;
      max = (dir & 1) + 1;
      /* Look at every newly adjacent square. */
      for(i = -max; i <= max; i++)
	{
	  newdir = cycle[chome[dir]+i];
	  row = y;
	  col = x;
	  if (mmove(newdir, &row, &col))
	    {
	      /* Objects player can see (Including doors?) cause a stop. */
	      c_ptr = &cave[row][col];
	      if (player_light || c_ptr->tl || c_ptr->pl || c_ptr->fm)
		{
		  if (c_ptr->tptr != 0)
		    {
		      t = t_list[c_ptr->tptr].tval;
		      if (t != TV_INVIS_TRAP && t != TV_SECRET_DOOR
			  && (t != TV_OPEN_DOOR || !find_ignore_doors))
			{
			  end_find();
			  return;
			}
		    }
		  /* Also Creatures		*/
		  /* the monster should be visible since update_mon() checks
		     for the special case of being in find mode */
		  if (c_ptr->cptr > 1 && m_list[c_ptr->cptr].ml)
		    {
		      end_find();
		      return;
		    }
		  inv = FALSE;
		}
	      else
		inv = TRUE;	/* Square unseen. Treat as open. */

	      if (c_ptr->fval <= MAX_OPEN_SPACE || inv)
		{
		  if (find_openarea)
		    {
		      /* Have we found a break? */
		      if (i < 0)
			{
			  if (find_breakright)
			    {
			      end_find();
			      return;
			    }
			}
		      else if (i > 0)
			{
			  if (find_breakleft)
			    {
			      end_find();
			      return;
			    }
			}
		    }
		  else if (option == 0)
		    option = newdir;	/* The first new direction. */
		  else if (option2 != 0)
		    {
		      end_find();	/* Three new directions. STOP. */
		      return;
		    }
		  else if (option != cycle[chome[dir]+i-1])
		    {
		      end_find();	/* If not adjacent to prev, STOP */
		      return;
		    }
		  else
		    {
		      /* Two adjacent choices. Make option2 the diagonal,
			 and remember the other diagonal adjacent to the first
			 option. */
		      if ((newdir & 1) == 1)
			{
			  check_dir = cycle[chome[dir]+i-2];
			  option2 = newdir;
			}
		      else
			{
			  check_dir = cycle[chome[dir]+i+1];
			  option2 = option;
			  option = newdir;
			}
		    }
		}
	      else if (find_openarea)
		{
		  /* We see an obstacle. In open area, STOP if on a side
		     previously open. */
		  if (i < 0)
		    {
		      if (find_breakleft)
			{
			  end_find();
			  return;
			}
		      find_breakright = TRUE;
		    }
		  else if (i > 0)
		    {
		      if (find_breakright)
			{
			  end_find();
			  return;
			}
		      find_breakleft = TRUE;
		    }
		}
	    }
	}

      if (find_openarea == FALSE)
	{	/* choose a direction. */
	  if (option2 == 0 || (find_examine && !find_cut))
	    {
	      /* There is only one option, or if two, then we always examine
		 potential corners and never cur known corners, so you step
		 into the straight option. */
	      if (option != 0)
		find_direction = option;
	      if (option2 == 0)
		find_prevdir = option;
	      else
		find_prevdir = option2;
	    }
	  else
	    {
	      /* Two options! */
	      row = y;
	      col = x;
	      (void) mmove(option, &row, &col);
	      if (!see_wall(option, row, col)
		  || !see_wall(check_dir, row, col))
		{
		  /* Don't see that it is closed off.  This could be a
		     potential corner or an intersection. */
		  if (find_examine && see_nothing(option, row, col)
		      && see_nothing(option2, row, col))
		    /* Can not see anything ahead and in the direction we are
		       turning, assume that it is a potential corner. */
		    {
		      find_direction = option;
		      find_prevdir = option2;
		    }
		  else
		    /* STOP: we are next to an intersection or a room */
		    end_find();
		}
	      else if (find_cut)
		{
		  /* This corner is seen to be enclosed; we cut the corner. */
		  find_direction = option2;
		  find_prevdir = option2;
		}
	      else
		{
		  /* This corner is seen to be enclosed, and we deliberately
		     go the long way. */
		  find_direction = option;
		  find_prevdir = option2;
		}
	    }
	}
    }
}


/* AC gets worse					-RAK-	*/
/* Note: This routine affects magical AC bonuses so that stores	  */
/*	 can detect the damage.					 */
int minus_ac(typ_dam)
int32u typ_dam;
{
  register int i, j;
  int tmp[6], minus;
  register inven_type *i_ptr;
  bigvtype out_val, tmp_str;

  i = 0;
  if (inventory[INVEN_BODY].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_BODY;
      i++;
    }
  if (inventory[INVEN_ARM].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_ARM;
      i++;
    }
  if (inventory[INVEN_OUTER].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_OUTER;
      i++;
    }
  if (inventory[INVEN_HANDS].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_HANDS;
      i++;
    }
  if (inventory[INVEN_HEAD].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_HEAD;
      i++;
    }
  /* also affect boots */
  if (inventory[INVEN_FEET].tval != TV_NOTHING)
    {
      tmp[i] = INVEN_FEET;
      i++;
    }
  minus = FALSE;
  if (i > 0)
    {
      j = tmp[randint(i) - 1];
      i_ptr = &inventory[j];
      if (i_ptr->flags & typ_dam)
	{
	  objdes(tmp_str, &inventory[j], FALSE);
	  (void) sprintf(out_val, "Your %s resists damage!", tmp_str);
	  msg_print(out_val);
	  minus = TRUE;
	}
      else if ((i_ptr->ac+i_ptr->toac) > 0)
	{
	  objdes(tmp_str, &inventory[j], FALSE);
	  (void) sprintf(out_val, "Your %s is damaged!", tmp_str);
	  msg_print(out_val);
	  i_ptr->toac--;
	  calc_bonuses();
	  minus = TRUE;
	}
    }
  return(minus);
}

extern int set_corrodes();
extern int set_flammable();
extern int set_frost_destroy();
extern int set_acid_affect();

/* Corrode the unsuspecting person's armor		 -RAK-	 */
void corrode_gas(kb_str)
char *kb_str;
{
  if (!minus_ac((int32u) TR_RES_ACID))
    take_hit(randint(8), kb_str);
  if (inven_damage(set_corrodes, 5) > 0)
    msg_print("There is an acrid smell coming from your pack.");
}


/* Poison gas the idiot.				-RAK-	*/
void poison_gas(dam, kb_str)
int dam;
char *kb_str;
{
  take_hit(dam, kb_str);
  py.flags.poisoned += 12 + randint(dam);

}


/* Burn the fool up.					-RAK-	*/
void fire_dam(dam, kb_str)
int dam;
char *kb_str;
{
  if (py.flags.fire_resist)
    dam = dam / 3;
  if (py.flags.resist_heat > 0)
    dam = dam / 3;
  take_hit(dam, kb_str);
  if (inven_damage(set_flammable, 3) > 0)
    msg_print("There is smoke coming from your pack!");
}


/* Freeze him to death.				-RAK-	*/
void cold_dam(dam, kb_str)
int dam;
char *kb_str;
{
  if (py.flags.cold_resist)
    dam = dam / 3;
  if (py.flags.resist_cold > 0)
    dam = dam / 3;
  take_hit(dam, kb_str);
  if (inven_damage(set_frost_destroy, 5) > 0)
    msg_print("Something shatters inside your pack!");
}


/* Lightning bolt the sucker away.			-RAK-	*/
void light_dam(dam, kb_str)
int dam;
char *kb_str;
{
  if (py.flags.lght_resist)
    take_hit((dam / 3), kb_str);
  else
    take_hit(dam, kb_str);
}


/* Throw acid on the hapless victim			-RAK-	*/
void acid_dam(dam, kb_str)
int dam;
char *kb_str;
{
  register int flag;

  flag = 0;
  if (minus_ac((int32u) TR_RES_ACID))
    flag = 1;
  if (py.flags.acid_resist)
    flag += 2;
  take_hit (dam / (flag + 1), kb_str);
  if (inven_damage(set_acid_affect, 3) > 0)
    msg_print("There is an acrid smell coming from your pack!");
}
