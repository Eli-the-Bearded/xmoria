/* wizard.c: Version history and info, and wizard mode debugging aids.

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

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

long atol();

/* Light up the dungeon					-RAK-	*/
void wizard_light()
{
  register cave_type *c_ptr;
  register int k, l, i, j;
  int flag;

  if (cave[char_row][char_col].pl)
    flag = FALSE;
  else
    flag = TRUE;
  for (i = 0; i < cur_height; i++)
    for (j = 0; j < cur_width; j++)
      if (cave[i][j].fval <= MAX_CAVE_FLOOR)
	for (k = i-1; k <= i+1; k++)
	  for (l = j-1; l <= j+1; l++)
	    {
	      c_ptr = &cave[k][l];
	      c_ptr->pl = flag;
	      if (!flag)
		c_ptr->fm = FALSE;
	    }
  prt_map();
}


/* Wizard routine for gaining on stats			-RAK-	*/
void change_character()
{
  register int tmp_val;
  register int32 tmp_lval;
  int8u *a_ptr;
  vtype tmp_str;
  register struct misc *m_ptr;

  a_ptr = py.stats.max_stat;
  msg_print("(3 - 118) Strength     = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_STR] = tmp_val;
	  (void) res_stat(A_STR);
	}
    }
  else
    return;

  msg_print("(3 - 118) Intelligence = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_INT] = tmp_val;
	  (void) res_stat(A_INT);
	}
    }
  else
    return;

  msg_print("(3 - 118) Wisdom       = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_WIS] = tmp_val;
	  (void) res_stat(A_WIS);
	}
    }
  else
    return;

  msg_print("(3 - 118) Dexterity    = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_DEX] = tmp_val;
	  (void) res_stat(A_DEX);
	}
    }
  else
    return;

  msg_print("(3 - 118) Constitution = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_CON] = tmp_val;
	  (void) res_stat(A_CON);
	}
    }
  else
    return;

  msg_print("(3 - 118) Charisma     = ");
  if (get_string(Status_window, tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_CHR] = tmp_val;
	  (void) res_stat(A_CHR);
	}
    }
  else
    return;

  m_ptr = &py.misc;
  msg_print("(1 - 32767) Hit points = ");
  if (get_string(Status_window, tmp_str, 0, 25, 5))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 0) && (tmp_val <= MAX_SHORT))
	{
	  m_ptr->mhp  = tmp_val;
	  m_ptr->chp  = tmp_val;
	  m_ptr->chp_frac = 0;
	  prt_mhp();
	  prt_chp();
	}
    }
  else
    return;

  msg_print("(0 - 32767) Mana       = ");
  if (get_string(Status_window, tmp_str, 0, 25, 5))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val <= MAX_SHORT) && (*tmp_str != '\0'))
	{
	  m_ptr->mana  = tmp_val;
	  m_ptr->cmana = tmp_val;
	  m_ptr->cmana_frac = 0;
	  prt_cmana();
	}
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%ld  Gold = ", m_ptr->au);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 7))
    {
      tmp_lval = atol(tmp_str);
      if (tmp_lval > -1 && (*tmp_str != '\0'))
	{
	  m_ptr->au = tmp_lval;
	  prt_gold();
	}
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Searching = ", m_ptr->srh);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->srh  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", m_ptr->stl);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -2) && (tmp_val < 19) && (*tmp_str != '\0'))
	m_ptr->stl  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", m_ptr->disarm);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->disarm = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-100) Save = ", m_ptr->save);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->save = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", m_ptr->bth);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->bth  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", m_ptr->bthb);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->bthb = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  Weight = ", m_ptr->wt);
  tmp_val = strlen(tmp_str);
  msg_print(tmp_str);
  if (get_string(Status_window, tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if (tmp_val > -1 && (*tmp_str != '\0'))
	m_ptr->wt = tmp_val;
    }
  else
    return;

  while(get_com(Status_window, "Alter speed? (+/-)", tmp_str))
    {
      if (*tmp_str == '+')
	change_speed(-1);
      else if (*tmp_str == '-')
	change_speed(1);
      else
	break;
      prt_speed();
    }
}


/* Wizard routine for creating objects			-RAK-	*/
void wizard_create()
{
  register int tmp_val;
  int32 tmp_lval;
  vtype tmp_str;
  register inven_type *i_ptr;
  inven_type forge;
  register cave_type *c_ptr;
  char pattern[4];

  msg_print("Warning: This routine can cause a fatal error.");
  i_ptr = &forge;
  i_ptr->index = OBJ_WIZARD;
  i_ptr->name2 = 0;
  inscribe(i_ptr, "wizard item");
  i_ptr->ident = ID_KNOWN2|ID_STOREBOUGHT;

  msg_print("Tval   : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->tval = tmp_val;

  msg_print("Tchar  : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 1))
    return;
  i_ptr->tchar = tmp_str[0];

  msg_print("Subval : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 5))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->subval = tmp_val;

  msg_print("Weight : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 5))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->weight = tmp_val;

  msg_print("Number : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 5))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->number = tmp_val;

  msg_print("Damage (dice): ");
  if (!get_string(Status_window, tmp_str, 0, 15, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->damage[0] = tmp_val;

  msg_print("Damage (sides): ");
  if (!get_string(Status_window, tmp_str, 0, 16, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->damage[1] = tmp_val;

  msg_print("+To hit: ");
  if (!get_string(Status_window, tmp_str, 0, 9, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->tohit = tmp_val;

  msg_print("+To dam: ");
  if (!get_string(Status_window, tmp_str, 0, 9, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->todam = tmp_val;

  msg_print("AC     : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->ac = tmp_val;

  msg_print("+To AC : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->toac = tmp_val;

  msg_print("P1     : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 5))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->p1 = tmp_val;

  msg_print("Flags (In HEX): ");
  if (!get_string(Status_window, tmp_str, 0, 16, 8))
    return;
  /* can't be constant string, this causes problems with the GCC compiler
     and some scanf routines */
  (void) strcpy (pattern, "%lx");
  (void) sscanf(tmp_str, pattern, &tmp_lval);
  i_ptr->flags = tmp_lval;

  msg_print("Cost : ");
  if (!get_string(Status_window, tmp_str, 0, 9, 8))
    return;
  tmp_lval = atol(tmp_str);
  i_ptr->cost = tmp_lval;

  msg_print("Level : ");
  if (!get_string(Status_window, tmp_str, 0, 10, 3))
    return;
  tmp_val = atoi(tmp_str);
  i_ptr->level = tmp_val;

  if (get_check(Status_window, "Allocate?"))
    {
      /* delete object first if any, before call popt */
      c_ptr = &cave[char_row][char_col];
      if (c_ptr->tptr != 0)
	(void) delete_object(char_row, char_col);

      tmp_val = popt();
      t_list[tmp_val] = forge;
      c_ptr->tptr = tmp_val;
      msg_print("Allocated.");
    }
  else
    msg_print("Aborted.");
}
