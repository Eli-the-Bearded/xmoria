/* moria2.c: misc code, mainly to handle player commands

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include <ctype.h>

#include "xmoria.h"

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#include <strings.h>
#endif


#if defined(LINT_ARGS)
static int look_ray(int, int, int);
static int look_see(int, int, int *);
static void hit_trap(int, int);
static void carry(int, int, int);
static int summon_object(int, int, int, int);
static void py_attack(int, int);
static void chest_trap(int, int);
static void inven_throw(int, struct inven_type *);
static void facts(struct inven_type *, int *, int *, int *, int *);
static void drop_throw(int, int, struct inven_type *);
static void py_bash(int, int);
#else
static int look_ray();
static int look_see();
#endif

/* Player hit a trap.	(Chuckle)			-RAK-	*/
static void hit_trap(y, x)
int y, x;
{
  int i, ty, tx, num, dam;
  register cave_type *c_ptr;
  register struct misc *p_ptr;
  register inven_type *t_ptr;
  bigvtype tmp;

  end_find();
  change_trap(y, x);
  c_ptr = &cave[y][x];
  p_ptr = &py.misc;
  t_ptr = &t_list[c_ptr->tptr];
  dam = pdamroll(t_ptr->damage);
  switch(t_ptr->subval)
    {
    case 1:  /* Open pit*/
      msg_print("You fell into a pit!");
      if (py.flags.ffall)
	msg_print("You gently float down.");
      else
      {
	objdes(tmp, t_ptr, TRUE);
	take_hit(dam, tmp);
      }
      break;
    case 2: /* Arrow trap*/
      if (test_hit(125, 0, 0, p_ptr->pac+p_ptr->ptoac, CLA_MISC_HIT))
	{
	  objdes(tmp, t_ptr, TRUE);
	  take_hit(dam, tmp);
	  msg_print("An arrow hits you.");
	}
      else
	msg_print("An arrow barely misses you.");
      break;
    case 3: /* Covered pit*/
      msg_print("You fell into a covered pit.");
      if (py.flags.ffall)
	msg_print("You gently float down.");
      else
      {
	objdes(tmp, t_ptr, TRUE);
	take_hit(dam, tmp);
      }
      place_trap(y, x, 0);
      break;
    case 4: /* Trap door*/
      msg_print("You fell through a trap door!");
      new_level_flag = TRUE;
      dun_level++;
      if (py.flags.ffall)
	msg_print("You gently float down.");
      else
      {
	objdes(tmp, t_ptr, TRUE);
	take_hit(dam, tmp);
      }
      break;
    case 5: /* Sleep gas*/
      if (py.flags.paralysis == 0)
	{
	  msg_print("A strange white mist surrounds you!");
	  if (py.flags.free_act)
	    msg_print("You are unaffected.");
	  else
	    {
	      msg_print("You fall asleep.");
	      py.flags.paralysis += randint(10) + 4;
	    }
	}
      break;
    case 6: /* Hid Obj*/
      (void) delete_object(y, x);
      place_object(y, x);
      msg_print("Hmmm, there was something under this rock.");
      break;
    case 7:  /* STR Dart*/
      if (test_hit(125, 0, 0, p_ptr->pac+p_ptr->ptoac, CLA_MISC_HIT))
	{
	  if (!py.flags.sustain_str)
	    {
	      (void) dec_stat(A_STR);
	      objdes(tmp, t_ptr, TRUE);
	      take_hit(dam, tmp);
	      msg_print("A small dart weakens you!");
	    }
	  else
	    msg_print("A small dart hits you.");
	}
      else
	msg_print("A small dart barely misses you.");
      break;
    case 8: /* Teleport*/
      teleport_flag = TRUE;
      msg_print("You hit a teleport trap!");
      break;
    case 9: /* Rockfall*/
      take_hit(dam, "a falling rock");
      (void) delete_object(y, x);
      place_rubble(y, x);
      msg_print("You are hit by falling rock.");
      break;
    case 10: /* Corrode gas*/
      corrode_gas("corrosion gas");
      msg_print("A strange red gas surrounds you.");
      break;
    case 11: /* Summon mon*/
      (void) delete_object(y, x);	/* Rune disappears.    */
      num = 2 + randint (3);
      for (i = 0; i < num; i++)
	{
	  ty = y;
	  tx = x;
	  (void) summon_monster(&ty, &tx, FALSE);
	}
      break;
    case 12: /* Fire trap*/
      fire_dam(dam, "a fire trap");
      msg_print("You are enveloped in flames!");
      break;
    case 13: /* Acid trap*/
      acid_dam(dam, "an acid trap");
      msg_print("You are splashed with acid!");
      break;
    case 14: /* Poison gas*/
      poison_gas(dam, "a poison gas trap");
      msg_print("A pungent green gas surrounds you!");
      break;
    case 15: /* Blind Gas */
      msg_print("A black gas surrounds you!");
      py.flags.blind += randint(50) + 50;
      break;
    case 16: /* Confuse Gas*/
      msg_print("A gas of scintillating colors surrounds you!");
      py.flags.confused += randint(15) + 15;
      break;
    case 17: /* Slow Dart*/
      if (test_hit(125, 0, 0, p_ptr->pac+p_ptr->ptoac, CLA_MISC_HIT))
	{
	  objdes(tmp, t_ptr, TRUE);
	  take_hit(dam, tmp);
	  msg_print("A small dart hits you!");
	  if (py.flags.free_act)
	    msg_print("You are unaffected.");
	  else
	    py.flags.slow += randint(20) + 10;
	}
      else
	msg_print("A small dart barely misses you.");
      break;
    case 18: /* CON Dart*/
      if (test_hit(125, 0, 0, p_ptr->pac+p_ptr->ptoac, CLA_MISC_HIT))
	{
	  if (!py.flags.sustain_con)
	    {
	      (void) dec_stat(A_CON);
	      objdes(tmp, t_ptr, TRUE);
	      take_hit(dam, tmp);
	      msg_print("A small dart saps your health!");
	    }
	  else
	    msg_print("A small dart hits you.");
	}
      else
	msg_print("A small dart barely misses you.");
      break;
    case 19: /*Secret Door*/
      break;
    case 99: /* Scare Mon*/
      break;

      /* Town level traps are special,	the stores.	*/
    case 101: /* General    */
      enter_store(0);
      break;
    case 102: /* Armory	    */
      enter_store(1);
      break;
    case 103: /* Weaponsmith*/
      enter_store(2);
      break;
    case 104: /* Temple	    */
      enter_store(3);
      break;
    case 105: /* Alchemy    */
      enter_store(4);
      break;
    case 106: /* Magic-User */
      enter_store(5);
      break;

    default:
      msg_print("Unknown trap value.");
      break;
    }
}


/* Return spell number and failure chance		-RAK-	*/
/* returns -1 if no spells in book
   returns 1 if choose a spell in book to cast
   returns 0 if don't choose a spell, i.e. exit with an escape */
int cast_spell(prompt, item_val, sn, sc)
char *prompt;
int item_val;
int *sn, *sc;
{
  int32u j;
  register int i, k;
  int spell[31], result, first_spell;
  register spell_type *s_ptr;

  result = -1;
  i = 0;
  j = inventory[item_val].flags;
  first_spell = bit_pos(&j);
  /* set j again, since bit_pos modified it */
  j = inventory[item_val].flags & spell_learned;
  s_ptr = magic_spell[py.misc.pclass-1];
  while (j)
    {
      k = bit_pos(&j);
      if (s_ptr[k].slevel <= py.misc.lev)
	{
	  spell[i] = k;
	  i++;
	}
    }
  if (i > 0)
    {
      result = get_spell(spell, i, sn, sc, prompt, first_spell);
      if (result && magic_spell[py.misc.pclass-1][*sn].smana > py.misc.cmana)
	{
	  if (py.misc.pclass)
	    result = get_check(Status_window,
	        "You summon your limited strength to cast this one! Confirm?");
	  else
	    result = get_check(Status_window,
		     "The gods may think you presumptuous for this! Confirm?");
	}
    }
  return(result);
}


/* Player is on an object.  Many things can happen based -RAK-	*/
/* on the TVAL of the object.  Traps are set off, money and most */
/* objects are picked up.  Some objects, such as open doors, just*/
/* sit there.						       */
static void carry(y, x, pickup)
int y, x;
int pickup;
{
  register int locn, i;
  bigvtype out_val, tmp_str;
  register cave_type *c_ptr;
  register inven_type *i_ptr;

  c_ptr = &cave[y][x];
  i_ptr = &t_list[c_ptr->tptr];
  i = t_list[c_ptr->tptr].tval;
  if (i <= TV_MAX_PICK_UP)
    {
      end_find();
      /* There's GOLD in them thar hills!      */
      if (i == TV_GOLD)
	{
	  py.misc.au += i_ptr->cost;
	  objdes(tmp_str, i_ptr, TRUE);
	  (void) sprintf(out_val,
			 "You have found %ld gold pieces worth of %s",
			 i_ptr->cost, tmp_str);
	  prt_gold();
	  (void) delete_object(y, x);
	  msg_print(out_val);
	}
      else
	{
	  if (inven_check_num(i_ptr))	   /* Too many objects?	    */
	    {			    /* Okay,  pick it up      */
	      if (pickup && prompt_carry_flag)
		{
		  objdes(tmp_str, i_ptr, TRUE);
		  /* change the period to a question mark */
		  tmp_str[strlen(tmp_str)-1] = '?';
		  (void) sprintf(out_val, "Pick up %s", tmp_str);
		  pickup = get_check(Status_window, out_val);
		}
	      /* Check to see if it will change the players speed. */
	      if (pickup && !inven_check_weight(i_ptr))
		{
		  objdes(tmp_str, i_ptr, TRUE);
		  /* change the period to a question mark */
		  tmp_str[strlen(tmp_str)-1] = '?';
		  (void) sprintf(out_val,
				 "Exceed your weight limit to pick up %s",
				 tmp_str);
		  pickup = get_check(Status_window, out_val);
		}
	      /* Attempt to pick up an object.	       */
	      if (pickup)
		{
		  locn = inven_carry(i_ptr);
		  objdes(tmp_str, &inventory[locn], TRUE);
		  (void) sprintf(out_val, "You have %s (%c)",tmp_str,locn+'a');
		  msg_print(out_val);
		  (void) delete_object(y, x);
		}
	    }
	  else
	    {
	      objdes(tmp_str, i_ptr, TRUE);
	      (void) sprintf(out_val, "You can't carry %s", tmp_str);
	      msg_print(out_val);
	    }
	}
    }
  /* OPPS!				   */
  else if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP || i == TV_STORE_DOOR)
    hit_trap(y, x);
}


/* Deletes a monster entry from the level		-RAK-	*/
delete_monster(j)
int j;
{
  register monster_type *m_ptr;

  m_ptr = &m_list[j];
  cave[m_ptr->fy][m_ptr->fx].cptr = 0;
  if (m_ptr->ml)
    lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
  if (j != mfptr - 1)
    {
      m_ptr = &m_list[mfptr - 1];
      cave[m_ptr->fy][m_ptr->fx].cptr = j;
      m_list[j] = m_list[mfptr - 1];
    }
  mfptr--;
  m_list[mfptr] = blank_monster;
  if (mon_tot_mult > 0)
    mon_tot_mult--;
}

/* The following two procedures implement the same function as delete monster.
   However, they are used within creatures(), because deleting a monster
   while scanning the m_list causes two problems, monsters might get two
   turns, and m_ptr/monptr might be invalid after the delete_monster.
   Hence the delete is done in two steps. */
/* fix1_delete_monster does everything delete_monster does except delete
   the monster record and reduce mfptr, this is called in breathe, and
   a couple of places in creatures.c */
fix1_delete_monster(j)
int j;
{
  register monster_type *m_ptr;

  m_ptr = &m_list[j];
  /* force the hp negative to ensure that the monster is dead, for example,
     if the monster was just eaten by another, it will still have positive
     hit points */
  m_ptr->hp = -1;
  cave[m_ptr->fy][m_ptr->fx].cptr = 0;
  if (m_ptr->ml)
    lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
  if (mon_tot_mult > 0)
    mon_tot_mult--;
}

/* fix2_delete_monster does everything in delete_monster that wasn't done
   by fix1_monster_delete above, this is only called in creatures() */
void fix2_delete_monster(j)
int j;
{
  register monster_type *m_ptr;

  if (j != mfptr - 1)
    {
      m_ptr = &m_list[mfptr - 1];
      cave[m_ptr->fy][m_ptr->fx].cptr = j;
      m_list[j] = m_list[mfptr - 1];
    }
  m_list[mfptr - 1] = blank_monster;
  mfptr--;
}


/* Creates objects nearby the coordinates given		-RAK-	  */
static int summon_object(y, x, num, typ)
int y, x, num, typ;
{
  register int i, j, k;
  register cave_type *c_ptr;
  int real_typ, res;

  if (typ == 1)
    real_typ = 1; /* typ == 1 -> objects */
  else
    real_typ = 256; /* typ == 2 -> gold */
  res = 0;
  do
    {
      i = 0;
      do
	{
	  j = y - 3 + randint(5);
	  k = x - 3 + randint(5);
	  if (in_bounds(j, k) && los(y, x, j, k))
	    {
	      c_ptr = &cave[j][k];
	      if (c_ptr->fval <= MAX_OPEN_SPACE && (c_ptr->tptr == 0))
		{
		  if (typ == 3) /* typ == 3 -> 50% objects, 50% gold */
		    {
		      if (randint(100) < 50)
			real_typ = 1;
		      else
			real_typ = 256;
		    }
		  if (real_typ == 1)
		    place_object(j, k);
		  else
		    place_gold(j, k);
		  lite_spot(j, k);
		  if (test_light(j, k))
		    res += real_typ;
		  i = 20;
		}
	    }
	  i++;
	}
      while (i <= 20);
      num--;
    }
  while (num != 0);
  return res;
}


/* Deletes object from given location			-RAK-	*/
delete_object(y, x)
int y, x;
{
  register int delete;
  register cave_type *c_ptr;

  c_ptr = &cave[y][x];
  if (c_ptr->fval == BLOCKED_FLOOR)
    c_ptr->fval = CORR_FLOOR;
  pusht(c_ptr->tptr);
  c_ptr->tptr = 0;
  c_ptr->fm = FALSE;
  lite_spot(y, x);
  if (test_light(y, x))
    delete = TRUE;
  else
    delete = FALSE;
  return(delete);
}


/* Allocates objects upon a creatures death		-RAK-	*/
/* Oh well,  another creature bites the dust.  Reward the victor*/
/* based on flags set in the main creature record		 */
/* Returns a mask of bits from the given flags which indicates what the
   monster is seen to have dropped.  This may be added to monster memory. */
int32u monster_death(y, x, flags)
int y, x;
register int32u flags;
{
  register int i, number;
  int32u dump, res;
#if defined(ATARIST_MWC)
  int32u holder;	/* avoid a compiler bug */
#endif

#if !defined(ATARIST_MWC)
  if (flags & CM_CARRY_OBJ)
    i = 1;
  else
    i = 0;
  if (flags & CM_CARRY_GOLD)
    i += 2;

  number = 0;
  if ((flags & CM_60_RANDOM) && (randint(100) < 60))
    number++;
  if ((flags & CM_90_RANDOM) && (randint(100) < 90))
    number++;
  if (flags & CM_1D2_OBJ)
    number += randint(2);
  if (flags & CM_2D2_OBJ)
    number += damroll(2, 2);
  if (flags & CM_4D2_OBJ)
    number += damroll(4, 2);
  if (number > 0)
    dump = summon_object(y, x, number, i);
  else
    dump = 0;
#else
  holder = CM_CARRY_OBJ;
  if (flags & holder)
    i = 1;
  else
    i = 0;
  holder = CM_CARRY_GOLD;
  if (flags & holder)
    i += 2;

  number = 0;
  holder = CM_60_RANDOM;
  if ((flags & holder) && (randint(100) < 60))
    number++;
  holder = CM_90_RANDOM;
  if ((flags & holder) && (randint(100) < 90))
    number++;
  holder = CM_1D2_OBJ;
  if (flags & holder)
    number += randint(2);
  holder = CM_2D2_OBJ;
  if (flags & holder)
    number += damroll(2, 2);
  holder = CM_4D2_OBJ;
  if (flags & holder)
    number += damroll(4, 2);
  if (number > 0)
    dump = summon_object(y, x, number, i);
  else
    dump = 0;


#endif

#if defined(ATARIST_MWC)
  holder = CM_WIN;
  if (flags & holder)
#else
  if (flags & CM_WIN)
#endif
    {
      total_winner = TRUE;
      prt_winner();
      msg_print("*** CONGRATULATIONS *** You have won the game.");
      msg_print("You cannot save this game, but you may retire when ready.");
    }

  if (dump)
    {
      res = 0;
      if (dump & 255)
#ifdef ATARIST_MWC
	{
	  holder = CM_CARRY_OBJ;
	  res |= holder;
	}
#else
        res |= CM_CARRY_OBJ;
#endif
      if (dump >= 256)
#ifdef ATARIST_MWC
	{
	  holder = CM_CARRY_GOLD;
	  res |= holder;
	}
#else
	res |= CM_CARRY_GOLD;
#endif
      dump = (dump % 256) + (dump / 256);  /* number of items */
      res |= dump << CM_TR_SHIFT;
    }
  else
    res = 0;

  return res;
}


/* Decreases monsters hit points and deletes monster if needed.	*/
/* (Picking on my babies.)			       -RAK-   */
int mon_take_hit(monptr, dam)
int monptr, dam;
{
  register int32u i;
  int32 new_exp, new_exp_frac;
  register monster_type *m_ptr;
  register struct misc *p_ptr;
  register creature_type *c_ptr;
  int m_take_hit;
  int32u tmp;
#ifdef ATARIST_MWC
  int32u holder;
#endif

  m_ptr = &m_list[monptr];
  m_ptr->hp -= dam;
  m_ptr->csleep = 0;
  if (m_ptr->hp < 0)
    {
      i = monster_death((int)m_ptr->fy, (int)m_ptr->fx,
			c_list[m_ptr->mptr].cmove);
      if ((py.flags.blind < 1 && m_ptr->ml)
	  || (c_list[m_ptr->mptr].cmove & CM_WIN))
	{
#ifdef ATARIST_MWC
	  holder = CM_TREASURE;
	  tmp = (c_recall[m_ptr->mptr].r_cmove & holder) >> CM_TR_SHIFT;
	  if (tmp > ((i & holder) >> CM_TR_SHIFT))
	    i = (i & ~holder) | (tmp << CM_TR_SHIFT);
	  c_recall[m_ptr->mptr].r_cmove =
	    (c_recall[m_ptr->mptr].r_cmove & ~holder) | i;
#else
	  tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE) >> CM_TR_SHIFT;
	  if (tmp > ((i & CM_TREASURE) >> CM_TR_SHIFT))
	    i = (i & ~CM_TREASURE) | (tmp << CM_TR_SHIFT);
	  c_recall[m_ptr->mptr].r_cmove =
	    (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE) | i;
#endif
	  if (c_recall[m_ptr->mptr].r_kills < MAX_SHORT)
	    c_recall[m_ptr->mptr].r_kills++;
	}
      c_ptr = &c_list[m_ptr->mptr];
      p_ptr = &py.misc;

      new_exp = ((long)c_ptr->mexp * c_ptr->level) / p_ptr->lev;
      new_exp_frac = ((((long)c_ptr->mexp * c_ptr->level) % p_ptr->lev)
	* 0x10000L / p_ptr->lev) + p_ptr->exp_frac;
      if (new_exp_frac >= 0x10000L)
	{
	  new_exp++;
	  p_ptr->exp_frac = new_exp_frac - 0x10000L;
	}
      else
	p_ptr->exp_frac = new_exp_frac;

      p_ptr->exp += new_exp;
      /* can't call prt_experience() here, as that would result in "new level"
	 message appearing before "monster dies" message */
      m_take_hit = m_ptr->mptr;
      /* in case this is called from within creatures(), this is a
	 horrible hack, the m_list/creatures() code needs to be
	 rewritten */
      if (hack_monptr < monptr)
	delete_monster(monptr);
      else
	fix1_delete_monster(monptr);
    }
  else
    m_take_hit = -1;
  return(m_take_hit);
}


/* Player attacks a (poor, defenseless) creature	-RAK-	*/
static void py_attack(y, x)
int y, x;
{
  register int k, blows;
  int crptr, monptr, tot_tohit, base_tohit;
  vtype m_name, out_val;
  register inven_type *i_ptr;
  register struct misc *p_ptr;

  crptr = cave[y][x].cptr;
  monptr = m_list[crptr].mptr;
  m_list[crptr].csleep = 0;
  i_ptr = &inventory[INVEN_WIELD];
  /* Does the player know what he's fighting?	   */
  if (!m_list[crptr].ml)
    (void) strcpy(m_name, "it");
  else
    (void) sprintf(m_name, "the %s", c_list[monptr].name);
  if (i_ptr->tval != TV_NOTHING)		/* Proper weapon */
    blows = attack_blows((int)i_ptr->weight, &tot_tohit);
  else					  /* Bare hands?   */
    {
      blows = 2;
      tot_tohit = -3;
    }
  if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_ARROW))
    /* Fix for arrows */
    blows = 1;
  p_ptr = &py.misc;
  tot_tohit += p_ptr->ptohit;
  /* if creature not lit, make it more difficult to hit */
  if (m_list[crptr].ml)
    base_tohit = p_ptr->bth;
  else
    base_tohit = (p_ptr->bth / 2) - (tot_tohit * (BTH_PLUS_ADJ-1))
      - (p_ptr->lev * class_level_adj[p_ptr->pclass][CLA_BTH] / 2);

  /* Loop for number of blows,	trying to hit the critter.	  */
  do
    {
      if (test_hit(base_tohit, (int)p_ptr->lev, tot_tohit,
		   (int)c_list[monptr].ac, CLA_BTH))
	{
	  (void) sprintf(out_val, "You hit %s.", m_name);
	  msg_print(out_val);
	  if (i_ptr->tval != TV_NOTHING)
	    {
	      k = pdamroll(i_ptr->damage);
	      k = tot_dam(i_ptr, k, monptr);
	      k = critical_blow((int)i_ptr->weight, tot_tohit, k, CLA_BTH);
	    }
	  else			      /* Bare hands!?  */
	    {
	      k = damroll(1, 1);
	      k = critical_blow(1, 0, k, CLA_BTH);
	    }
	  k += p_ptr->ptodam;
	  if (k < 0)  k = 0;

	  if (py.flags.confuse_monster)
	    {
	      py.flags.confuse_monster = FALSE;
	      msg_print("Your hands stop glowing.");
	      if ((c_list[monptr].cdefense & CD_NO_SLEEP)
		  || (randint(MAX_MONS_LEVEL) < c_list[monptr].level))
		(void) sprintf(out_val, "%s is unaffected.", m_name);
	      else
		{
		  (void) sprintf(out_val, "%s appears confused.", m_name);
		  m_list[crptr].confused = TRUE;
		}
	      msg_print(out_val);
	      if (m_list[crptr].ml && randint(4) == 1)
		c_recall[monptr].r_cdefense |=
		  c_list[monptr].cdefense & CD_NO_SLEEP;
	    }

	  /* See if we done it in.				 */
	  if (mon_take_hit(crptr, k) >= 0)
	    {
	      (void) sprintf(out_val, "You have slain %s.", m_name);
	      msg_print(out_val);
	      prt_experience();
	      blows = 0;
	    }

	  if ((i_ptr->tval >= TV_SLING_AMMO)
	      && (i_ptr->tval <= TV_ARROW)) /* Use missiles up*/
	    {
	      i_ptr->number--;
	      inven_weight -= i_ptr->weight;
	      py.flags.status |= PY_STR_WGT;
	      if (i_ptr->number == 0)
		{
		  equip_ctr--;
		  py_bonuses(i_ptr, -1);
		  invcopy(i_ptr, OBJ_NOTHING);
		  calc_bonuses();
		}
	    }
	}
      else
	{
	  (void) sprintf(out_val, "You miss %s.", m_name);
	  msg_print(out_val);
	}
      blows--;
    }
  while (blows >= 1);
}


/* Moves player from one space to another.		-RAK-	*/
/* Note: This routine has been pre-declared; see that for argument*/
move_char(dir, do_pickup)
int dir, do_pickup;
{
  int old_row, old_col, old_find_flag;
  int y, x;
  register int i, j;
  register cave_type *c_ptr, *d_ptr;

  if ((py.flags.confused > 0) &&    /* Confused?	     */
      (randint(4) > 1) &&	    /* 75% random movement   */
      (dir != 5))		    /* Never random if sitting*/
    {
      dir = randint(9);
      end_find();
    }
  y = char_row;
  x = char_col;
  if (mmove(dir, &y, &x))    /* Legal move?	      */
    {
      c_ptr = &cave[y][x];
      /* if there is no creature, or an unlit creature in the walls then... */
      /* disallow attacks against unlit creatures in walls because moving into
	 a wall is a free turn normally, hence don't give player free turns
	 attacking each wall in an attempt to locate the invisible creature,
	 instead force player to tunnel into walls which always takes a turn */
      if ((c_ptr->cptr < 2)
	  || (!m_list[c_ptr->cptr].ml && c_ptr->fval >= MIN_CLOSED_SPACE))
	{
	  if (c_ptr->fval <= MAX_OPEN_SPACE) /* Open floor spot	*/
	    {
	      /* Make final assignments of char co-ords */
	      old_row = char_row;
	      old_col = char_col;
	      char_row = y;
	      char_col = x;
	      /* Move character record (-1)	       */
	      move_rec(old_row, old_col, char_row, char_col);
	      /* Check for new panel		       */
	      if (get_panel(char_row, char_col, FALSE))
		prt_map();
	      /* Check to see if he should stop	       */
	      if (find_flag)
		area_affect(dir, char_row, char_col);
	      /* Check to see if he notices something  */
	      /* fos may be negative if have good rings of searching */
	      if ((py.misc.fos <= 1) || (randint(py.misc.fos) == 1) ||
		  (search_flag))
		search(char_row, char_col, py.misc.srh);
	      /* A room of light should be lit.	     */
	      if (c_ptr->fval == LIGHT_FLOOR)
		{
		  if (!c_ptr->pl && !py.flags.blind)
		    light_room(char_row, char_col);
		}
	      /* in doorway of light-room?	       */
	      else if (c_ptr->lr && (py.flags.blind < 1))
		for (i = (char_row - 1); i <= (char_row + 1); i++)
		  for (j = (char_col - 1); j <= (char_col + 1); j++)
		    {
		      d_ptr = &cave[i][j];
		      if ((d_ptr->fval == LIGHT_FLOOR) && (!d_ptr->pl))
			light_room(i, j);
		    }
	      /* Move the light source		       */
	      move_light(old_row, old_col, char_row, char_col);
	      /* An object is beneath him.	     */
	      if (c_ptr->tptr != 0)
		{
		  carry(char_row, char_col, do_pickup);
		  /* if stepped on falling rock trap, and space contains
		     rubble, then step back into a clear area */
		  if (t_list[c_ptr->tptr].tval == TV_RUBBLE)
		    {
		      move_rec(char_row, char_col, old_row, old_col);
		      move_light(char_row, char_col, old_row, old_col);
		      char_row = old_row;
		      char_col = old_col;
		      /* check to see if we have stepped back onto another
			 trap, if so, set it off */
		      c_ptr = &cave[char_row][char_col];
		      if (c_ptr->tptr != 0)
			{
			  i = t_list[c_ptr->tptr].tval;
			  if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP
			      || i == TV_STORE_DOOR)
			    hit_trap(char_row, char_col);
			}
		    }
		}
	    }
	  else	  /*Can't move onto floor space*/
	    {
	      if (!find_flag && (c_ptr->tptr != 0))
		{
		  if (t_list[c_ptr->tptr].tval == TV_RUBBLE)
		    msg_print("There is rubble blocking your way.");
		  else if (t_list[c_ptr->tptr].tval == TV_CLOSED_DOOR)
		    msg_print("There is a closed door blocking your way.");
		}
	      else
		end_find();
	      free_turn_flag = TRUE;
	    }
	}
      else	  /* Attacking a creature! */
	{
	  old_find_flag = find_flag;
	  end_find();
	  /* if player can see monster, and was in find mode, then nothing */
	  if (m_list[c_ptr->cptr].ml && old_find_flag)
	    {
	      /* did not do anything this turn */
	      free_turn_flag = TRUE;
	    }
	  else
	    {
	      if (py.flags.afraid < 1)		/* Coward?	*/
		py_attack(y, x);
	      else				/* Coward!	*/
		msg_print("You are too afraid!");
	    }
	}
    }
}


/* Chests have traps too.				-RAK-	*/
/* Note: Chest traps are based on the FLAGS value		 */
static void chest_trap(y, x)
int y, x;
{
  register int i;
  int j, k;
  register inven_type *t_ptr;

  t_ptr = &t_list[cave[y][x].tptr];
  if (CH_LOSE_STR & t_ptr->flags)
    {
      msg_print("A small needle has pricked you!");
      if (!py.flags.sustain_str)
	{
	  (void) dec_stat(A_STR);
	  take_hit(damroll(1, 4), "a poison needle");
	  msg_print("You feel weakened!");
	}
      else
	msg_print("You are unaffected.");
    }
  if (CH_POISON & t_ptr->flags)
    {
      msg_print("A small needle has pricked you!");
      take_hit(damroll(1, 6), "a poison needle");
      py.flags.poisoned += 10 + randint(20);
    }
  if (CH_PARALYSED & t_ptr->flags)
    {
      msg_print("A puff of yellow gas surrounds you!");
      if (py.flags.free_act)
	msg_print("You are unaffected.");
      else
	{
	  msg_print("You choke and pass out.");
	  py.flags.paralysis = 10 + randint(20);
	}
    }
  if (CH_SUMMON & t_ptr->flags)
    {
      for (i = 0; i < 3; i++)
	{
	  j = y;
	  k = x;
	  (void) summon_monster(&j, &k, FALSE);
	}
    }
  if (CH_EXPLODE & t_ptr->flags)
    {
      msg_print("There is a sudden explosion!");
      (void) delete_object(y, x);
      take_hit(damroll(5, 8), "an exploding chest");
    }
}


/* Opens a closed door or closed chest.		-RAK-	*/
void openobject()
{
  int y, x, i, dir;
  int flag, no_object;
  register cave_type *c_ptr;
  register inven_type *t_ptr;
  register struct misc *p_ptr;
  register monster_type *m_ptr;
  vtype m_name, out_val;

  y = char_row;
  x = char_col;
  if (get_dir(NULL, &dir))
    {
      (void) mmove(dir, &y, &x);
      c_ptr = &cave[y][x];
      no_object = FALSE;
      if (c_ptr->cptr > 1 && c_ptr->tptr != 0 &&
	  (t_list[c_ptr->tptr].tval == TV_CLOSED_DOOR
	   || t_list[c_ptr->tptr].tval == TV_CHEST))
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  if (m_ptr->ml)
	    (void) sprintf (m_name, "The %s", c_list[m_ptr->mptr].name);
	  else
	    (void) strcpy (m_name, "Something");
	  (void) sprintf(out_val, "%s is in your way!", m_name);
	  msg_print(out_val);
	}
      else if (c_ptr->tptr != 0)
	/* Closed door		 */
	if (t_list[c_ptr->tptr].tval == TV_CLOSED_DOOR)
	  {
	    t_ptr = &t_list[c_ptr->tptr];
	    if (t_ptr->p1 > 0)	 /* It's locked.	*/
	      {
		p_ptr = &py.misc;
		i = p_ptr->disarm + 2*todis_adj() + stat_adj(A_INT)
		  + (class_level_adj[p_ptr->pclass][CLA_DISARM]
		     * p_ptr->lev / 3);
		if (py.flags.confused > 0)
		  msg_print("You are too confused to pick the lock.");
		else if ((i-t_ptr->p1) > randint(100))
		  {
		    msg_print("You have picked the lock.");
		    py.misc.exp++;
		    prt_experience();
		    t_ptr->p1 = 0;
		  }
		else
		  count_msg_print("You failed to pick the lock.");
	      }
	    else if (t_ptr->p1 < 0)	 /* It's stuck	  */
	      msg_print("It appears to be stuck.");
	    if (t_ptr->p1 == 0)
	      {
		invcopy(&t_list[c_ptr->tptr], OBJ_OPEN_DOOR);
		c_ptr->fval = CORR_FLOOR;
		lite_spot(y, x);
		command_count = 0;
	      }
	  }
      /* Open a closed chest.		     */
	else if (t_list[c_ptr->tptr].tval == TV_CHEST)
	  {
	    p_ptr = &py.misc;
	    i = p_ptr->disarm + 2*todis_adj() + stat_adj(A_INT)
	      + (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
	    t_ptr = &t_list[c_ptr->tptr];
	    flag = FALSE;
	    if (CH_LOCKED & t_ptr->flags)
	      if (py.flags.confused > 0)
		msg_print("You are too confused to pick the lock.");
	      else if ((i-(int)t_ptr->level) > randint(100))
		{
		  msg_print("You have picked the lock.");
		  flag = TRUE;
		  py.misc.exp += t_ptr->level;
		  prt_experience();
		}
	      else
		count_msg_print("You failed to pick the lock.");
	    else
	      flag = TRUE;
	    if (flag)
	      {
		t_ptr->flags &= ~CH_LOCKED;
		t_ptr->name2 = SN_EMPTY;
		known2(t_ptr);
		t_ptr->cost = 0;
	      }
	    flag = FALSE;
	    /* Was chest still trapped?	 (Snicker)   */
	    if ((CH_LOCKED & t_ptr->flags) == 0)
	      {
		chest_trap(y, x);
		if (c_ptr->tptr != 0)
		  flag = TRUE;
	      }
	    /* Chest treasure is allocated as if a creature   */
	    /* had been killed.				   */
	    if (flag)
	      {
		/* clear the cursed chest/monster win flag, so that people
		   can not win by opening a cursed chest */
		t_list[c_ptr->tptr].flags &= ~TR_CURSED;
		(void) monster_death(y, x, t_list[c_ptr->tptr].flags);
		t_list[c_ptr->tptr].flags = 0;
	      }
	  }
	else
	  no_object = TRUE;
      else
	no_object = TRUE;

      if (no_object)
	{
	  msg_print("I do not see anything you can open there.");
	  free_turn_flag = TRUE;
	}
    }
}


/* Closes an open door.				-RAK-	*/
void closeobject()
{
  int y, x, dir, no_object;
  vtype out_val, m_name;
  register cave_type *c_ptr;
  register monster_type *m_ptr;

  y = char_row;
  x = char_col;
  if (get_dir(NULL, &dir))
    {
      (void) mmove(dir, &y, &x);
      c_ptr = &cave[y][x];
      no_object = FALSE;
      if (c_ptr->tptr != 0)
	if (t_list[c_ptr->tptr].tval == TV_OPEN_DOOR)
	  if (c_ptr->cptr == 0)
	    if (t_list[c_ptr->tptr].p1 == 0)
	      {
		invcopy(&t_list[c_ptr->tptr], OBJ_CLOSED_DOOR);
		c_ptr->fval = BLOCKED_FLOOR;
		lite_spot(y, x);
	      }
	    else
	      msg_print("The door appears to be broken.");
	  else
	    {
	      m_ptr = &m_list[c_ptr->cptr];
	      if (m_ptr->ml)
		(void) sprintf (m_name, "The %s", c_list[m_ptr->mptr].name);
	      else
		(void) strcpy (m_name, "Something");
	      (void) sprintf(out_val, "%s is in your way!", m_name);
	      msg_print(out_val);
	    }
	else
	  no_object = TRUE;
      else
	no_object = TRUE;

      if (no_object)
	{
	  msg_print("I do not see anything you can close there.");
	  free_turn_flag = TRUE;
	}
    }
}


/* Tunneling through real wall: 10, 11, 12		-RAK-	*/
/* Used by TUNNEL and WALL_TO_MUD				 */
int twall(y, x, t1, t2)
int y, x, t1, t2;
{
  register int i, j;
  register cave_type *c_ptr;
  int res, found;

  res = FALSE;
  if (t1 > t2)
    {
      c_ptr = &cave[y][x];
      if (c_ptr->lr)
	{
	  /* should become a room space, check to see whether it should be
	     LIGHT_FLOOR or DARK_FLOOR */
	  found = FALSE;
	  for (i = y-1; i <= y+1; i++)
	    for (j = x-1; j <= x+1; j++)
	      if (cave[i][j].fval <= MAX_CAVE_ROOM)
		{
		  c_ptr->fval = cave[i][j].fval;
		  c_ptr->pl = cave[i][j].pl;
		  found = TRUE;
		  break;
		}
	  if (!found)
	    {
	      c_ptr->fval = CORR_FLOOR;
	      c_ptr->pl = FALSE;
	    }
	}
      else
	{
	  /* should become a corridor space */
	  c_ptr->fval  = CORR_FLOOR;
	  c_ptr->pl = FALSE;
	}
      c_ptr->fm = FALSE;
      if (panel_contains(y, x))
	if ((c_ptr->tl || c_ptr->pl) && c_ptr->tptr != 0)
	  msg_print("You have found something!");
      lite_spot(y, x);
      res = TRUE;
    }
  return(res);
}


/* Tunnels through rubble and walls			-RAK-	*/
/* Must take into account: secret doors,  special tools		  */
void tunnel(dir)
int dir;
{
  register int i, tabil;
  register cave_type *c_ptr;
  register inven_type *i_ptr;
  int y, x;
  monster_type *m_ptr;
  vtype out_val, m_name;

  if ((py.flags.confused > 0) &&    /* Confused?	     */
      (randint(4) > 1))		    /* 75% random movement   */
    dir = randint(9);
  y = char_row;
  x = char_col;
  (void) mmove(dir, &y, &x);

  c_ptr = &cave[y][x];
  /* Compute the digging ability of player; based on	   */
  /* strength, and type of tool used			   */
  tabil = py.stats.use_stat[A_STR];
  i_ptr = &inventory[INVEN_WIELD];
  if (c_ptr->cptr > 1)
    {
      m_ptr = &m_list[c_ptr->cptr];
      if (m_ptr->ml)
	(void) sprintf (m_name, "The %s", c_list[m_ptr->mptr].name);
      else
	(void) strcpy (m_name, "Something");
      (void) sprintf(out_val, "%s is in your way!", m_name);
      msg_print(out_val);

      /* let the player attack the creature */
      if (py.flags.afraid < 1)
	py_attack(y, x);
      else
	msg_print("You are too afraid!");
    }
  else if (i_ptr->tval != TV_NOTHING)
    {
      if (TR_TUNNEL & i_ptr->flags)
	tabil += 25 + i_ptr->p1*50;
      else
	{
	  tabil += (i_ptr->damage[0]*i_ptr->damage[1]) + i_ptr->tohit
	    + i_ptr->todam;
	  /* divide by two so that digging without shovel isn't too easy */
	  tabil >>= 1;
	}

      /* Regular walls; Granite, magma intrusion, quartz vein  */
      /* Don't forget the boundary walls, made of titanium (255)*/
      switch(c_ptr->fval)
	{
	case GRANITE_WALL:
	  i = randint(1200) + 80;
	  if (twall(y, x, tabil, i))
	    msg_print("You have finished the tunnel.");
	  else
	    count_msg_print("You tunnel into the granite wall.");
	  break;
	case MAGMA_WALL:
	  i = randint(600) + 10;
	  if (twall(y, x, tabil, i))
	    msg_print("You have finished the tunnel.");
	  else
	    count_msg_print("You tunnel into the magma intrusion.");
	  break;
	case QUARTZ_WALL:
	  i = randint(400) + 10;
	  if (twall(y, x, tabil, i))
	    msg_print("You have finished the tunnel.");
	  else
	    count_msg_print("You tunnel into the quartz vein.");
	  break;
	case BOUNDARY_WALL:
	  msg_print("This seems to be permanent rock.");
	  break;
	default:
	  /* Is there an object in the way?  (Rubble and secret doors)*/
	  if (c_ptr->tptr != 0)
	    {
	      /* Rubble.     */
	      if (t_list[c_ptr->tptr].tval == TV_RUBBLE)
		{
		  if (tabil > randint(180))
		    {
		      (void) delete_object(y, x);
		      msg_print("You have removed the rubble.");
		      if (randint(10) == 1)
			{
			  place_object(y, x);
			  if (test_light(y, x))
			    msg_print("You have found something!");
			}
		      lite_spot(y, x);
		    }
		  else
		    count_msg_print("You dig in the rubble.");
		}
	      /* Secret doors.*/
	      else if (t_list[c_ptr->tptr].tval == TV_SECRET_DOOR)
		{
		  count_msg_print("You tunnel into the granite wall.");
		  search(char_row, char_col, py.misc.srh);
		}
	      else
		{
		  msg_print("You can't tunnel through that.");
		  free_turn_flag = TRUE;
		}
	    }
	  else
	    {
	      msg_print("Tunnel through what?  Empty air?!?");
	      free_turn_flag = TRUE;
	    }
	  break;
	}
    }
  else
    msg_print("You dig with your hands, making no progress.");
}


/* Disarms a trap					-RAK-	*/
void disarm_trap()
{
  int y, x, level, tmp, dir, no_disarm;
  register int tot, i;
  register cave_type *c_ptr;
  register inven_type *i_ptr;
  monster_type *m_ptr;
  vtype m_name, out_val;

  y = char_row;
  x = char_col;
  if (get_dir(NULL, &dir))
    {
      (void) mmove(dir, &y, &x);
      c_ptr = &cave[y][x];
      no_disarm = FALSE;
      if (c_ptr->cptr > 1 && c_ptr->tptr != 0 &&
	  (t_list[c_ptr->tptr].tval == TV_VIS_TRAP
	   || t_list[c_ptr->tptr].tval == TV_CHEST))
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  if (m_ptr->ml)
	    (void) sprintf (m_name, "The %s", c_list[m_ptr->mptr].name);
	  else
	    (void) strcpy (m_name, "Something");
	  (void) sprintf(out_val, "%s is in your way!", m_name);
	  msg_print(out_val);
	}
      else if (c_ptr->tptr != 0)
	{
	  tot = py.misc.disarm + 2*todis_adj() + stat_adj(A_INT)
	    + (class_level_adj[py.misc.pclass][CLA_DISARM] * py.misc.lev / 3);
	  if ((py.flags.blind > 0) || (no_light()))
	    tot = tot / 10;
	  if (py.flags.confused > 0)
	    tot = tot / 10;
	  if (py.flags.image > 0)
	    tot = tot / 10;
	  i_ptr = &t_list[c_ptr->tptr];
	  i = i_ptr->tval;
	  level = i_ptr->level;
	  if (i == TV_VIS_TRAP)		    /* Floor trap    */
	    {
	      if ((tot + 100 - level) > randint(100))
		{
		  msg_print("You have disarmed the trap.");
		  py.misc.exp += i_ptr->p1;
		  (void) delete_object(y, x);
		  /* make sure we move onto the trap even if confused */
		  tmp = py.flags.confused;
		  py.flags.confused = 0;
		  move_char(dir, FALSE);
		  py.flags.confused = tmp;
		  prt_experience();
		}
	      /* avoid randint(0) call */
	      else if ((tot > 5) && (randint(tot) > 5))
		count_msg_print("You failed to disarm the trap.");
	      else
		{
		  msg_print("You set the trap off!");
		  /* make sure we move onto the trap even if confused */
		  tmp = py.flags.confused;
		  py.flags.confused = 0;
		  move_char(dir, FALSE);
		  py.flags.confused += tmp;
		}
	    }
	  else if (i == TV_CHEST)
	    {
	      if (!known2_p(i_ptr))
		{
		  msg_print("I don't see a trap.");
		  free_turn_flag = TRUE;
		}
	      else if (CH_TRAPPED & i_ptr->flags)
		{
		  if ((tot - level) > randint(100))
		    {
		      i_ptr->flags &= ~CH_TRAPPED;
		      if (CH_LOCKED & i_ptr->flags)
			i_ptr->name2 = SN_LOCKED;
		      else
			i_ptr->name2 = SN_DISARMED;
		      msg_print("You have disarmed the chest.");
		      known2(i_ptr);
		      py.misc.exp += level;
		      prt_experience();
		    }
		  else if ((tot > 5) && (randint(tot) > 5))
		    count_msg_print("You failed to disarm the chest.");
		  else
		    {
		      msg_print("You set a trap off!");
		      known2(i_ptr);
		      chest_trap(y, x);
		    }
		}
	      else
		{
		  msg_print("The chest was not trapped.");
		  free_turn_flag = TRUE;
		}
	    }
	  else
	    no_disarm = TRUE;
	}
      else
	no_disarm = TRUE;

      if (no_disarm)
	{
	  msg_print("I do not see anything to disarm there.");
	  free_turn_flag = TRUE;
	}
    }
}


/* An enhanced look, with peripheral vision. Looking all 8	-CJS-
   directions will see everything which ought to be visible. Can
   specify direction 5, which looks in all directions.

   For the purpose of hindering vision, each place is regarded as
   a diamond just touching its four immediate neighbours. A
   diamond is opaque if it is a wall, or shut door, or something
   like that. A place is visible if any part of its diamond is
   visible: i.e. there is a line from the view point to part of
   the diamond which does not pass through any opaque diamonds.

   Consider the following situation:

     @....			    X	X   X	X   X
     .##..			   / \ / \ / \ / \ / \
     .....			  X @ X . X . X 1 X . X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	   Expanded view, with	   / \ / \ / \ / \ / \
	   diamonds inscribed	  X . X # X # X 2 X . X
	   about each point,	   \ / \ / \ / \ / \ /
	   and some locations	    X	X   X	X   X
	   numbered.		   / \ / \ / \ / \ / \
				  X . X . X . X 3 X 4 X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	- Location 1 is fully visible.
	- Location 2 is visible, even though partially obscured.
	- Location 3 is invisible, but if either # were
	  transparent, it would be visible.
	- Location 4 is completely obscured by a single #.

   The function which does the work is look_ray. It sets up its
   own co-ordinate frame (global variables map back to the
   dungeon frame) and looks for everything between two angles
   specified from a central line. It is recursive, and each call
   looks at stuff visible along a line parallel to the center
   line, and a set distance away from it. A diagonal look uses
   more extreme peripheral vision from the closest horizontal and
   vertical directions; horizontal or vertical looks take a call
   for each side of the central line. */

/* Globally accessed variables: gl_nseen counts the number of places where
   something is seen. gl_rock indicates a look for rock or objects.

   The others map co-ords in the ray frame to dungeon co-ords.

   dungeon y = char_row	 + gl_fyx * (ray x)  + gl_fyy * (ray y)
   dungeon x = char_col	 + gl_fxx * (ray x)  + gl_fxy * (ray y) */
static int gl_fxx, gl_fxy, gl_fyx, gl_fyy;
static int gl_nseen, gl_noquery;
static int gl_rock;
/* Intended to be indexed by dir/2, since is only relevant to horizontal or
   vertical directions. */
static int set_fxy[] = { 0,  1,	 0,  0, -1 };
static int set_fxx[] = { 0,  0, -1,  1,	 0 };
static int set_fyy[] = { 0,  0,	 1, -1,	 0 };
static int set_fyx[] = { 0,  1,	 0,  0, -1 };
/* Map diagonal-dir/2 to a normal-dir/2. */
static int map_diag1[] = { 1, 3, 0, 2, 4 };
static int map_diag2[] = { 2, 1, 0, 4, 3 };

#define GRADF	10000	/* Any sufficiently big number will do */

/* Look at what we can see. This is a free move.

   Prompts for a direction, and then looks at every object in
   turn within a cone of vision in that direction. For each
   object, the cursor is moved over the object, a description is
   given, and we wait for the user to type something. Typing
   ESCAPE will abort the entire look.

   Looks first at real objects and monsters, and looks at rock
   types only after all other things have been seen.  Only looks at rock
   types if the highlight_seams option is set. */

void look()
{
  register int i, abort;
  int dir, dummy;

  if (py.flags.blind > 0)
    msg_print("You can't see a damn thing!");
  else if (py.flags.image > 0)
    msg_print("You can't believe what you are seeing! It's like a dream!");
  else if (get_alldir("Look which direction?", &dir))
    {
      abort = FALSE;
      gl_nseen = 0;
      gl_rock = 0;
      gl_noquery = FALSE;	/* Have to set this up for the look_see */
      if (look_see(0, 0, &dummy))
	abort = TRUE;
      else
	{
	  do
	    {
	      abort = FALSE;
	      if (dir == 5)
		{
		  for (i = 1; i <= 4; i++)
		    {
		      gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
		      gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
		      if (look_ray(0, 2*GRADF-1, 1))
			{
			  abort = TRUE;
			  break;
			}
		      gl_fxy = -gl_fxy;
		      gl_fyy = -gl_fyy;
		      if (look_ray(0, 2*GRADF, 2))
			{
			  abort = TRUE;
			  break;
			}
		    }
		}
	      else if ((dir & 1) == 0)	/* Straight directions */
		{
		  i = dir >> 1;
		  gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
		  gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
		  if (look_ray(0, GRADF, 1))
		    abort = TRUE;
		  else
		    {
		      gl_fxy = -gl_fxy;
		      gl_fyy = -gl_fyy;
		      abort = look_ray(0, GRADF, 2);
		    }
		}
	      else
		{
		  i = map_diag1[dir >> 1];
		  gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
		  gl_fxy = -set_fxy[i]; gl_fyy = -set_fyy[i];
		  if (look_ray(1, 2*GRADF, GRADF))
		    abort = TRUE;
		  else
		    {
		      i = map_diag2[dir >> 1];
		      gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
		      gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
		      abort = look_ray(1, 2*GRADF-1, GRADF);
		    }
		}
	    }
	  while (abort == FALSE && highlight_seams && (++gl_rock < 2));
	  if (abort)
	    msg_print("--Aborting look--");
	  else
	    {
	      if (gl_nseen)
		{
		  if (dir == 5)
		    msg_print("That's all you see.");
		  else
		    msg_print("That's all you see in that direction.");
		}
	      else if (dir == 5)
		msg_print("You see nothing of interest.");
	      else
		msg_print("You see nothing of interest in that direction.");
	    }
	}
    }
}

/* Look at everything within a cone of vision between two ray
   lines emanating from the player, and y or more places away
   from the direct line of view. This is recursive.

   Rays are specified by gradients, y over x, multiplied by
   2*GRADF. This is ONLY called with gradients between 2*GRADF
   (45 degrees) and 1 (almost horizontal).

   (y axis)/ angle from
     ^	  /	    ___ angle to
     |	 /	 ___
  ...|../.....___.................... parameter y (look at things in the
     | /   ___			      cone, and on or above this line)
     |/ ___
     @-------------------->   direction in which you are looking. (x axis)
     |
     | */
static int look_ray(y, from, to)
int y, from, to;
{
  register int max_x, x;
  int transparent;

  /* from is the larger angle of the ray, since we scan towards the
     center line. If from is smaller, then the ray does not exist. */
  if (from <= to || y > MAX_SIGHT)
    return FALSE;
  /* Find first visible location along this line. Minimum x such
     that (2x-1)/x < from/GRADF <=> x > GRADF(2x-1)/from. This may
     be called with y=0 whence x will be set to 0. Thus we need a
     special fix. */
  x = (long)GRADF * (2 * y - 1) / from + 1;
  if (x <= 0)
    x = 1;

  /* Find last visible location along this line.
     Maximum x such that (2x+1)/x > to/GRADF <=> x < GRADF(2x+1)/to */
  max_x = ((long)GRADF * (2 * y + 1) - 1) / to;
  if (max_x > MAX_SIGHT)
    max_x = MAX_SIGHT;
  if (max_x < x)
    return FALSE;

  /* gl_noquery is a HACK to prevent doubling up on direct lines of
     sight. If 'to' is	greater than 1, we do not really look at
     stuff along the direct line of sight, but we do have to see
     what is opaque for the purposes of obscuring other objects. */
  if (y == 0 && to > 1 || y == x && from < GRADF*2)
    gl_noquery = TRUE;
  else
    gl_noquery = FALSE;
  if (look_see(x, y, &transparent))
    return TRUE;
  if (y == x)
    gl_noquery = FALSE;
  if (transparent)
    goto init_transparent;

  for (;;)
    {
      /* Look down the window we've found. */
      if (look_ray(y+1, from, (int)((2 * y + 1) * (long)GRADF / x)))
	return TRUE;
      /* Find the start of next window. */
      do
	{
	  if (x == max_x)
	    return FALSE;
	  /* See if this seals off the scan. (If y is zero, then it will.) */
	  from = (2 * y - 1) * (long)GRADF / x;
	  if (from <= to)
	    return FALSE;
	  x++;

	  if (look_see(x, y, &transparent))
	    return TRUE;
	}
      while(!transparent);

    init_transparent:

      /* Find the end of this window of visibility. */
      do
	{
	  if (x == max_x)
	    /* The window is trimmed by an earlier limit. */
	    return look_ray(y+1, from, to);
	  x++;

	  if (look_see(x, y, &transparent))
	    return TRUE;
	}
      while(transparent);
    }
}

static int look_see(x, y, transparent)
register int x, y;
int *transparent;
{
  char *dstring, *string, query;
  register cave_type *c_ptr;
  register int j;
  bigvtype out_val, tmp_str;

  if (x < 0 || y < 0 || y > x)
    {
      (void) sprintf(tmp_str, "Illegal call to look_see(%d, %d)", x, y);
      msg_print(tmp_str);
    }
  if (x == 0 && y == 0)
    dstring = "You are on";
  else
    dstring = "You see";
  j = char_col + gl_fxx * x + gl_fxy * y;
  y = char_row + gl_fyx * x + gl_fyy * y;
  x = j;
  if (!panel_contains(y, x))
    {
      *transparent = FALSE;
      return FALSE;
    }
  c_ptr = &cave[y][x];
  *transparent = c_ptr->fval <= MAX_OPEN_SPACE;
  if (gl_noquery)
    return FALSE; /* Don't look at a direct line of sight. A hack. */
  out_val[0] = 0;
  if (gl_rock == 0 && c_ptr->cptr > 1 && m_list[c_ptr->cptr].ml)
    {
      j = m_list[c_ptr->cptr].mptr;
      (void) sprintf(out_val, "%s %s %s. [(r)ecall]",
		     dstring,
		     is_a_vowel( c_list[j].name[0] ) ? "an" : "a",
		     c_list[j].name);
      dstring = "It is on";
      msg_print(out_val);
      query = inkey(TRUE);
      if (query == 'r' || query == 'R')
	{
	  query = roff_recall(j);
	}
    }
  if (c_ptr->tl || c_ptr->pl || c_ptr->fm)
    {
      if (c_ptr->tptr != 0)
	{
	  if (t_list[c_ptr->tptr].tval == TV_SECRET_DOOR)
	    goto granite;
	  if (gl_rock ==0 && t_list[c_ptr->tptr].tval != TV_INVIS_TRAP)
	    {
	      objdes(tmp_str, &t_list[c_ptr->tptr], TRUE);
	      (void) sprintf(out_val, "%s %s ---pause---", dstring, tmp_str);
	      dstring = "It is in";
	      msg_print(out_val);
	      query = inkey(TRUE);
	    }
	}
      if ((gl_rock || out_val[0]) && c_ptr->fval >= MIN_CLOSED_SPACE)
	{
	  switch(c_ptr->fval)
	    {
	    case BOUNDARY_WALL:
	    case GRANITE_WALL:
	    granite:
	      /* Granite is only interesting if it contains something. */
	      if(out_val[0])
		string = "a granite wall";
	      else
		string = NULL;	/* In case we jump here */
	      break;
	    case MAGMA_WALL: string = "some dark rock";
	      break;
	    case QUARTZ_WALL: string = "a quartz vein";
	      break;
	    default: string = NULL;
	      break;
	    }
	  if (string)
	    {
	      (void) sprintf(out_val, "%s %s ---pause---", dstring, string);
	      msg_print(out_val);
	      query = inkey(TRUE);
	    }
	}
    }
  if (out_val[0])
    {
      gl_nseen++;
      if (query == ESCAPE)
	return TRUE;
    }

  return FALSE;
}


static void inven_throw(item_val, t_ptr)
int item_val;
inven_type *t_ptr;
{
  register inven_type *i_ptr;

  i_ptr = &inventory[item_val];
  *t_ptr = *i_ptr;
  if (i_ptr->number > 1)
    {
      t_ptr->number = 1;
      i_ptr->number--;
      inven_weight -= i_ptr->weight;
      py.flags.status |= PY_STR_WGT;
    }
  else
    inven_destroy(item_val);
}


/* Obtain the hit and damage bonuses and the maximum distance for a
   thrown missile. */
static void facts(i_ptr, tbth, tpth, tdam, tdis)
register inven_type *i_ptr;
int *tbth, *tpth, *tdam, *tdis;
{
  register int tmp_weight;

  if (i_ptr->weight < 1)
    tmp_weight = 1;
  else
    tmp_weight = i_ptr->weight;

  /* Throwing objects			*/
  *tdam = pdamroll(i_ptr->damage) + i_ptr->todam;
  *tbth = py.misc.bthb * 75 / 100;
  *tpth = py.misc.ptohit + i_ptr->tohit;

  /* Add this back later if the correct throwing device. -CJS- */
  if (inventory[INVEN_WIELD].tval != TV_NOTHING)
    *tpth -= inventory[INVEN_WIELD].tohit;

  *tdis = (((py.stats.use_stat[A_STR]+20)*10)/tmp_weight);
  if (*tdis > 10)  *tdis = 10;

  /* multiply damage bonuses instead of adding, when have proper
     missile/weapon combo, this makes them much more useful */

  /* Using Bows,  slings,  or crossbows	*/
  if (inventory[INVEN_WIELD].tval == TV_BOW)
    switch(inventory[INVEN_WIELD].p1)
      {
      case 1:
	if (i_ptr->tval == TV_SLING_AMMO) /* Sling and ammo */
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 2;
	    *tdis = 20;
	  }
	break;
      case 2:
	if (i_ptr->tval == TV_ARROW)      /* Short Bow and Arrow	*/
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 2;
	    *tdis = 25;
	  }
	break;
      case 3:
	if (i_ptr->tval == TV_ARROW)      /* Long Bow and Arrow	*/
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 3;
	    *tdis = 30;
	  }
	break;
      case 4:
	if (i_ptr->tval == TV_ARROW)      /* Composite Bow and Arrow*/
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 4;
	    *tdis = 35;
	  }
	break;
      case 5:
	if (i_ptr->tval == TV_BOLT)      /* Light Crossbow and Bolt*/
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 3;
	    *tdis = 25;
	  }
	break;
      case 6:
	if (i_ptr->tval == TV_BOLT)      /* Heavy Crossbow and Bolt*/
	  {
	    *tbth = py.misc.bthb;
	    *tpth += 2 * inventory[INVEN_WIELD].tohit;
	    *tdam += inventory[INVEN_WIELD].todam;
	    *tdam = *tdam * 4;
	    *tdis = 35;
	  }
	break;
      }
}


static void drop_throw(y, x, t_ptr)
int y, x;
inven_type *t_ptr;
{
  register int i, j, k;
  int flag, cur_pos;
  bigvtype out_val, tmp_str;
  register cave_type *c_ptr;

  flag = FALSE;
  i = y;
  j = x;
  k = 0;
  if (randint(10) > 1)
    {
      do
	{
	  if (in_bounds(i, j))
	    {
	      c_ptr = &cave[i][j];
	      if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->tptr == 0)
		flag = TRUE;
	    }
	  if (!flag)
	    {
	      i = y + randint(3) - 2;
	      j = x + randint(3) - 2;
	      k++;
	    }
	}
      while ((!flag) && (k <= 9));
    }
  if (flag)
    {
      cur_pos = popt();
      cave[i][j].tptr = cur_pos;
      t_list[cur_pos] = *t_ptr;
      lite_spot(i, j);
    }
  else
    {
      objdes(tmp_str, t_ptr, FALSE);
      (void) sprintf(out_val, "The %s disappears.", tmp_str);
      msg_print(out_val);
    }
}

/* Throw an object across the dungeon.		-RAK-	*/
/* Note: Flasks of oil do fire damage				 */
/* Note: Extra damage and chance of hitting when missiles are used*/
/*	 with correct weapon.  I.E.  wield bow and throw arrow.	 */
void throw_object()
{
  int item_val, tbth, tpth, tdam, tdis;
  int y, x, oldy, oldx, cur_dis, dir;
  int flag, visible;
  bigvtype out_val, tmp_str;
  inven_type throw_obj;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register int i;
  char tchar;

  if (inven_ctr == 0)
    {
      msg_print("But you are not carrying anything.");
      free_turn_flag = TRUE;
    }
  else if (get_item(&item_val, "Fire/Throw which one?", 0, inven_ctr-1))
    {
      if (get_dir(NULL, &dir))
	{
	  desc_remain(item_val);
	  if (py.flags.confused > 0)
	    {
	      msg_print("You are confused.");
	      do
		{
		  dir = randint(9);
		}
	      while (dir == 5);
	    }
	  inven_throw(item_val, &throw_obj);
	  facts(&throw_obj, &tbth, &tpth, &tdam, &tdis);
	  tchar = throw_obj.tchar;
	  flag = FALSE;
	  y = char_row;
	  x = char_col;
	  oldy = char_row;
	  oldx = char_col;
	  cur_dis = 0;
	  do
	    {
	      (void) mmove(dir, &y, &x);
	      cur_dis++;
	      lite_spot(oldy, oldx);
	      if (cur_dis > tdis)  flag = TRUE;
	      c_ptr = &cave[y][x];
	      if ((c_ptr->fval <= MAX_OPEN_SPACE) && (!flag))
		{
		  if (c_ptr->cptr > 1)
		    {
		      flag = TRUE;
		      m_ptr = &m_list[c_ptr->cptr];
		      tbth = tbth - cur_dis;
		      /* if monster not lit, make it much more difficult to
			 hit, subtract off most bonuses, and reduce bthb
			 depending on distance */
		      if (!m_ptr->ml)
			tbth = (tbth / (cur_dis+2))
			  - (py.misc.lev *
			     class_level_adj[py.misc.pclass][CLA_BTHB] / 2)
			    - (tpth * (BTH_PLUS_ADJ-1));
		      if (test_hit(tbth, (int)py.misc.lev, tpth,
				   (int)c_list[m_ptr->mptr].ac, CLA_BTHB))
			{
			  i = m_ptr->mptr;
			  objdes(tmp_str, &throw_obj, FALSE);
			  /* Does the player know what he's fighting?	   */
			  if (!m_ptr->ml)
			    {
			      (void) sprintf(out_val,
			    "You hear a cry as the %s finds a mark.", tmp_str);
			      visible = FALSE;
			    }
			  else
			    {
			      (void) sprintf(out_val, "The %s hits the %s.",
					     tmp_str, c_list[i].name);
			      visible = TRUE;
			    }
			  msg_print(out_val);
			  tdam = tot_dam(&throw_obj, tdam, i);
			  tdam = critical_blow((int)throw_obj.weight,
					       tpth, tdam, CLA_BTHB);
			  i = mon_take_hit((int)c_ptr->cptr, tdam);
			  if (i >= 0)
			    {
			      if (!visible)
				msg_print("You have killed something!");
			      else
				{
			      (void) sprintf(out_val,"You have killed the %s.",
						 c_list[i].name);
				  msg_print(out_val);
				}
			      prt_experience();
			    }
			}
		      else
			drop_throw(oldy, oldx, &throw_obj);
		    }
		}
	      else
		{
		  flag = TRUE;
		  drop_throw(oldy, oldx, &throw_obj);
		}
	      oldy = y;
	      oldx = x;
	    }
	  while (!flag);
	}
    }
}


/* Make a bash attack on someone.				-CJS-
   Used to be part of bash above. */
static void py_bash(y, x)
int y, x;
{
  int monster, k, avg_max_hp, base_tohit;
  register creature_type *c_ptr;
  register monster_type *m_ptr;
  vtype m_name, out_val;

  monster = cave[y][x].cptr;
  m_ptr = &m_list[monster];
  c_ptr = &c_list[m_ptr->mptr];
  m_ptr->csleep = 0;
  /* Does the player know what he's fighting?	   */
  if (!m_ptr->ml)
    (void) strcpy(m_name, "it");
  else
    (void) sprintf(m_name, "the %s", c_ptr->name);
  base_tohit = py.stats.use_stat[A_STR] + inventory[INVEN_ARM].weight/2
    + py.misc.wt/10;
  if (!m_ptr->ml)
    base_tohit = (base_tohit / 2) - (py.stats.use_stat[A_DEX]*(BTH_PLUS_ADJ-1))
      - (py.misc.lev * class_level_adj[py.misc.pclass][CLA_BTH] / 2);

  if (test_hit(base_tohit, (int)py.misc.lev,
	       (int)py.stats.use_stat[A_DEX], (int)c_ptr->ac, CLA_BTH))
    {
      (void) sprintf(out_val, "You hit %s.", m_name);
      msg_print(out_val);
      k = pdamroll(inventory[INVEN_ARM].damage);
      k = critical_blow((int)(inventory[INVEN_ARM].weight / 4
			      + py.stats.use_stat[A_STR]), 0, k, CLA_BTH);
      k += py.misc.wt/60 + 3;

      /* See if we done it in.				     */
      if (mon_take_hit(monster, k) >= 0)
	{
	  (void) sprintf(out_val, "You have slain %s.", m_name);
	  msg_print(out_val);
	  prt_experience();
	}
      else
	{
	  m_name[0] = toupper((int)m_name[0]); /* Capitalize */
	  /* Can not stun Balrog */
	  avg_max_hp = (c_ptr->cdefense & CD_MAX_HP ? 
			c_ptr->hd[0] * c_ptr->hd[1] :
			(c_ptr->hd[0] * (c_ptr->hd[1] + 1)) >> 1);
	  if ((100 + randint(400) + randint(400))
	      > (m_ptr->hp + avg_max_hp))
	    {
	      m_ptr->stunned += randint(3) + 1;
	      if (m_ptr->stunned > 24)	m_ptr->stunned = 24;
	      (void) sprintf(out_val, "%s appears stunned!", m_name);
	    }
	  else
	    (void) sprintf(out_val, "%s ignores your bash!", m_name);
	  msg_print(out_val);
	}
    }
  else
    {
      (void) sprintf(out_val, "You miss %s.", m_name);
      msg_print(out_val);
    }
  if (randint(150) > py.stats.use_stat[A_DEX])
    {
      msg_print("You are off balance.");
      py.flags.paralysis = 1 + randint(2);
    }
}


/* Bash open a door or chest				-RAK-	*/
/* Note: Affected by strength and weight of character

   For a closed door, p1 is positive if locked; negative if
   stuck. A disarm spell unlocks and unjams doors!

   For an open door, p1 is positive for a broken door.

   A closed door can be opened - harder if locked. Any door might
   be bashed open (and thereby broken). Bashing a door is
   (potentially) faster! You move into the door way. To open a
   stuck door, it must be bashed. A closed door can be jammed
   (which makes it stuck if previously locked).

   Creatures can also open doors. A creature with open door
   ability will (if not in the line of sight) move though a
   closed or secret door with no changes. If in the line of
   sight, closed door are openned, & secret door revealed.
   Whether in the line of sight or not, such a creature may
   unlock or unstick a door.

   A creature with no such ability will attempt to bash a
   non-secret door. */
void bash()
{
  int y, x, dir, tmp, no_bash;
  register cave_type *c_ptr;
  register inven_type *t_ptr;

  y = char_row;
  x = char_col;
  if (get_dir(NULL, &dir))
    {
      no_bash = FALSE;
      (void) mmove(dir, &y, &x);
      c_ptr = &cave[y][x];
      if (c_ptr->cptr > 1)
	{
	  if (py.flags.afraid > 0)
	    msg_print("You are afraid!");
	  else
	    py_bash(y, x);
	}
      else if (c_ptr->tptr != 0)
	{
	  t_ptr = &t_list[c_ptr->tptr];
	  if (t_ptr->tval == TV_CLOSED_DOOR)
	    {
	      count_msg_print("You smash into the door!");
	      tmp = py.stats.use_stat[A_STR] + py.misc.wt / 2;
	      /* Use (roughly) similar method as for monsters. */
	      if (randint(tmp*(20+abs(t_ptr->p1))) < 10*(tmp-abs(t_ptr->p1)))
		{
		  msg_print("The door crashes open!");
		  invcopy(&t_list[c_ptr->tptr], OBJ_OPEN_DOOR);
		  t_ptr->p1 = randint(2) - 1; /* 50% chance of breaking door */
		  c_ptr->fval = CORR_FLOOR;
		  if (py.flags.confused == 0)
		    move_char(dir, FALSE);
		  else
		    lite_spot(y, x);
		}
	      else if (randint(150) > py.stats.use_stat[A_DEX])
		{
		  msg_print("You are off-balance.");
		  py.flags.paralysis = 1 + randint(2);
		}
	      else if (command_count == 0)
		msg_print("The door holds firm.");
	    }
	  else if (t_ptr->tval == TV_CHEST)
	    {
	      if (randint(10) == 1)
		{
		  msg_print("You have destroyed the chest.");
		  msg_print("and its contents!");
		  t_ptr->index = OBJ_RUINED_CHEST;
		  t_ptr->flags = 0;
		}
	      else if ((CH_LOCKED & t_ptr->flags) && (randint(10) == 1))
		{
		  msg_print("The lock breaks open!");
		  t_ptr->flags &= ~CH_LOCKED;
		}
	      else
		count_msg_print("The chest holds firm.");
	    }
	  else
	    no_bash = TRUE;
	}
      else
	no_bash = TRUE;

      if (no_bash)
	{
	  msg_print("I do not see anything you can bash there.");
	  free_turn_flag = TRUE;
	}
    }
}
