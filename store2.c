/* store2.c: store code, entering, command interpreter, buying, selling

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
#ifndef VMS
#include <strings.h>
#endif
#endif

#if defined(LINT_ARGS)
static void put_buffer_comment1(void);
static void put_buffer_comment2(int32, int32, int);
static void put_buffer_comment3(int32, int32, int);
static void put_buffer_comment4(void);
static void put_buffer_comment5(void);
static void put_buffer_comment6(void);
static void display_commands(void);
static void haggle_commands(int);
static void display_inventory(int, int);
static void display_cost(int, int);
static void store_put_buffer_gold(void);
static void display_store(int, int);
static int get_store_item(int *, char *, int, int);
static int increase_insults(int);
static void decrease_insults(int);
static int haggle_insults(int);
static int get_haggle(char *, int32 *, int);
static int receive_offer(int, char *, int32 *, int32, int, int);
static int purchase_haggle(int, int32 *, struct inven_type *);
static int sell_haggle(int, int32 *, struct inven_type *);
static int store_purchase(int, int *);
static int store_sell(int, int *);
#endif

long atol();

static char *comment1[14] = {
  "Done!",  "Accepted!",  "Fine.",  "Agreed!",  "Ok.",  "Taken!",
  "You drive a hard bargain, but taken.",
  "You'll force me bankrupt, but it's a deal.",  "Sigh.  I'll take it.",
  "My poor sick children may starve, but done!",  "Finally!  I accept.",
  "Robbed again.",  "A pleasure to do business with you!",
  "My spouse will skin me, but accepted."
  };

static char *comment2a[3] = {
  "%A2 is my final offer; take it or leave it.",
  "I'll give you no more than %A2.",
  "My patience grows thin.  %A2 is final."
  };

static char *comment2b[16] = {
  "%A1 for such a fine item?  HA!  No less than %A2.",
  "%A1 is an insult!  Try %A2 gold pieces.",
  "%A1?!?  You would rob my poor starving children?",
  "Why, I'll take no less than %A2 gold pieces.",
  "Ha!  No less than %A2 gold pieces.",
  "Thou knave!  No less than %A2 gold pieces.",
  "%A1 is far too little, how about %A2?",
  "I paid more than %A1 for it myself, try %A2.",
  "%A1?  Are you mad?!?  How about %A2 gold pieces?",
  "As scrap this would bring %A1.  Try %A2 in gold.",
  "May the fleas of 1000 orcs molest you.  I want %A2.",
  "My mother you can get for %A1, this costs %A2.",
  "May your chickens grow lips.  I want %A2 in gold!",
  "Sell this for such a pittance?  Give me %A2 gold.",
  "May the Balrog find you tasty!  %A2 gold pieces?",
  "Your mother was a Troll!  %A2 or I'll tell."
  };

static char *comment3a[3] = {
  "I'll pay no more than %A1; take it or leave it.",
  "You'll get no more than %A1 from me.",
  "%A1 and that's final."
  };

static char *comment3b[15] = {
  "%A2 for that piece of junk?  No more than %A1.",
  "For %A2 I could own ten of those.  Try %A1.",
  "%A2?  NEVER!  %A1 is more like it.",
  "Let's be reasonable. How about %A1 gold pieces?",
  "%A1 gold for that junk, no more.",
  "%A1 gold pieces and be thankful for it!",
  "%A1 gold pieces and not a copper more.",
  "%A2 gold?  HA!  %A1 is more like it.",  "Try about %A1 gold.",
  "I wouldn't pay %A2 for your children, try %A1.",
  "*CHOKE* For that!?  Let's say %A1.",  "How about %A1?",
  "That looks war surplus!  Say %A1 gold.",
  "I'll buy it as scrap for %A1.",
  "%A2 is too much, let us say %A1 gold."
  };

static char *comment4a[5] = {
  "ENOUGH!  You have abused me once too often!",
  "THAT DOES IT!  You shall waste my time no more!",
  "This is getting nowhere.  I'm going home!",
  "BAH!  No more shall you insult me!",
  "Begone!  I have had enough abuse for one day."
  };

static char *comment4b[5] = {
  "Out of my place!",  "out... Out... OUT!!!",  "Come back tomorrow.",
  "Leave my place.  Begone!",  "Come back when thou art richer."
  };

static char *comment5[10] = {
  "You will have to do better than that!",  "That's an insult!",
  "Do you wish to do business or not?",  "Hah!  Try again.",
  "Ridiculous!",  "You've got to be kidding!",  "You'd better be kidding!",
  "You try my patience.",  "I don't hear you.",
  "Hmmm, nice weather we're having."
  };

static char *comment6[5] = {
  "I must have heard you wrong.",  "What was that?",
  "I'm sorry, say that again.",  "What did you say?",
  "Sorry, what was that again?"
  };

/* Comments vary.					-RAK-	*/
/* Comment one : Finished haggling				*/
static void put_buffer_comment1()
{
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment1[randint(14)-1], 1, 0);
}


/* %A1 is offer, %A2 is asking.		*/
static void put_buffer_comment2(offer, asking, final)
int32 offer, asking; int final;
{
  vtype comment;

  if (final > 0)
    (void) strcpy(comment, comment2a[randint(3)-1]);
  else
    (void) strcpy(comment, comment2b[randint(16)-1]);

  insert_lnum(comment, "%A1", offer, FALSE);
  insert_lnum(comment, "%A2", asking, FALSE);
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment, 1, 0);
}


static void put_buffer_comment3(offer, asking, final)
int32 offer, asking; int final;
{
  vtype comment;

  if (final > 0)
    (void) strcpy (comment, comment3a[randint(3)-1]);
  else
    (void) strcpy (comment, comment3b[randint(15)-1]);

  insert_lnum(comment, "%A1", offer, FALSE);
  insert_lnum(comment, "%A2", asking, FALSE);
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment, 1, 0);
}


/* Kick 'da bum out.					-RAK-	*/
static void put_buffer_comment4()
{
  register int tmp;

  tmp = randint(5) - 1;
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment4a[tmp], 1, 0);
  put_buffer(Store_window, comment4b[tmp], 1, strlen(comment4a[tmp]) + 2);
}


static void put_buffer_comment5()
{
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment5[randint(10)-1], 1, 0);
}


static void put_buffer_comment6()
{
  erase_line(Store_window, 1, 0);
  put_buffer(Store_window, comment6[randint(5)-1], 1, 0);
}


/* Displays the set of commands				-RAK-	*/
static void display_commands()
{
  erase_line(Store_window, 21, 0);
  erase_line(Store_window, 22, 0);
  erase_line(Store_window, 23, 0);
  erase_line(Store_window, 24, 0);
  put_buffer(Store_window, "You may:", 21, 0);
  put_buffer(Store_window, " p) Purchase an item.           b) Browse store's inventory.", 22, 0);
  put_buffer(Store_window, " s) Sell an item.               i/e/t/w/x) Inventory/Equipment Lists.",
      23, 0);
  put_buffer(Store_window, "ESC) Exit from Building.        ^R) Redraw the screen.", 24, 0);
}


/* Displays the set of commands				-RAK-	*/
static void haggle_commands(typ)
int typ;
{
  erase_line(Store_window, 21, 0);
  erase_line(Store_window, 22, 0);
  erase_line(Store_window, 23, 0);
  if (typ == -1)
    put_buffer(Store_window, "Specify an asking-price in gold pieces.", 22, 0);
  else
    put_buffer(Store_window, "Specify an offer in gold pieces.", 22, 0);
  put_buffer(Store_window, "ESC) Quit Haggling.", 23, 0);
  erase_line (Store_window, 24, 0);	/* clear last line */
}


/* Displays a store's inventory				-RAK-	*/
static void display_inventory(store_num, start)
int store_num, start;
{
  register store_type *s_ptr;
  register inven_type *i_ptr;
  register int i, j, stop;
  bigvtype out_val1, out_val2;
  int32 x;

  s_ptr = &store[store_num];
  i = (start % 12);
  stop = ((start / 12) + 1) * 12;
  if (stop > s_ptr->store_ctr)	stop = s_ptr->store_ctr;
  while (start < stop)
    {
      i_ptr = &s_ptr->store_inven[start].sitem;
      x = i_ptr->number;
      if ((i_ptr->subval >= ITEM_SINGLE_STACK_MIN)
	  && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX))
	i_ptr->number = 1;
      objdes(out_val1, i_ptr, TRUE);
      i_ptr->number = x;
      (void) sprintf(out_val2, "%c) %s", 'a'+i, out_val1);
      erase_line(Store_window, i+6, 0);
      put_buffer(Store_window, out_val2, i+6, 0);
      x = s_ptr->store_inven[start].scost;
      if (x <= 0)
	{
	  int32 value = -x;
	  value = value * chr_adj() / 100;
	  if (value <= 0)
	    value = 1;
	  (void) sprintf(out_val2, "%9ld", value);
	}
      else
	(void) sprintf(out_val2,"%9ld [Fixed]", x);
      put_buffer(Store_window, out_val2, i+6, 59);
      i++;
      start++;
    }
  if (i < 12)
    for (j = 0; j < (11 - i + 1); j++)
      erase_line (Store_window, j+i+6, 0); /* clear remaining lines */
  if (s_ptr->store_ctr > 12)
    put_buffer(Store_window, "- cont. -", 18, 60);
  else
    erase_line (Store_window, 18, 60);
}


/* Re-displays only a single cost			-RAK-	*/
static void display_cost(store_num, pos)
int store_num, pos;
{
  register int i;
  register int32 j;
  vtype out_val;
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  i = (pos % 12);
  if (s_ptr->store_inven[pos].scost < 0)
    {
      j = - s_ptr->store_inven[pos].scost;
      j = j * chr_adj() / 100;
      (void) sprintf(out_val, "%ld", j);
    }
  else
    (void) sprintf(out_val, "%9ld [Fixed]", s_ptr->store_inven[pos].scost);
  put_buffer(Store_window, out_val, i+6, 59);
}


/* Displays players gold					-RAK-	*/
static void store_put_buffer_gold()
{
  vtype out_val;

  (void) sprintf(out_val, "Gold Remaining : %ld", py.misc.au);
  put_buffer(Store_window, out_val, 19, 17);
}


/* Displays store					-RAK-	*/
static display_store(store_num, cur_top)
int store_num, cur_top;
{
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  put_buffer(Store_window, owners[s_ptr->owner].owner_name, 4, 9);
  put_buffer(Store_window, "Item", 5, 3);
  put_buffer(Store_window, "Asking Price", 5, 60);
  store_put_buffer_gold();
  display_commands();
  display_inventory(store_num, cur_top);
}


/* Get the ID of a store item and return it's value	-RAK-	*/
static int get_store_item(com_val, pmt, i, j)
int *com_val;
char *pmt;
register int i, j;
{
  char command;
  vtype out_val;
  register int flag;

  *com_val = -1;
  flag = FALSE;
  (void) sprintf(out_val, "(Items %c-%c, ESC to exit) %s", i+'a', j+'a', pmt);
  while (get_com(Store_window, out_val, &command))
    {
      command -= 'a';
      if (command >= i && command <= j)
	{
	  flag = TRUE;
	  *com_val = command;
	  break;
	}
      bell();
    }
  erase_line(Store_window, 1, 0);
  return(flag);
}


/* Increase the insult counter and get angry if too many -RAK-	*/
static int increase_insults(store_num)
int store_num;
{
  register int increase;
  register store_type *s_ptr;

  increase = FALSE;
  s_ptr = &store[store_num];
  s_ptr->insult_cur++;
  if (s_ptr->insult_cur > owners[s_ptr->owner].insult_max)
    {
      put_buffer_comment4();
      s_ptr->insult_cur = 0;
      s_ptr->good_buy = 0;
      s_ptr->bad_buy = 0;
      s_ptr->store_open = turn + 2500 + randint(2500);
      increase = TRUE;
    }
  return(increase);
}

/* Decrease insults					-RAK-	*/
static void decrease_insults(store_num)
int store_num;
{
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  if (s_ptr->insult_cur != 0)
    s_ptr->insult_cur--;
}


/* Have insulted while haggling				-RAK-	*/
static int haggle_insults(store_num)
int store_num;
{
  register int haggle;

  haggle = FALSE;
  if (increase_insults(store_num))
    haggle = TRUE;
  else
    put_buffer_comment5();
  return(haggle);
}


static int get_haggle(comment, new_offer, num_offer)
char *comment;
int32 *new_offer;
int num_offer;
{
  register int32 i;
  vtype out_val;
  register int flag, clen;
  register char *p;

  flag = TRUE;
  clen = strlen(comment);
  i = 0;
  do
    {
      put_buffer(Store_window, comment, 2, 15);
      if (!get_string(Store_window, out_val, 2, clen + 15, 40))
	flag = FALSE;
      i = atol(out_val);
      /* don't allow incremental haggling, if player has not made an offer
	 yet */
      for (p = out_val; *p == ' '; p++)
	;
      if (flag && num_offer == 0 && (*p == '+' || *p == '-'))
	{
	  put_buffer(Store_window, "You haven't even made your first offer yet!", 1, 0);
	  i = 0;
	}
    }
  while (flag && (i == 0));
  if (flag)
    {
      for (p = out_val; *p == ' '; p++)
	;
      if (*p == '+' || *p == '-')
	*new_offer += i;
      else
	*new_offer = i;
    }
  else
    erase_line (Store_window, 1, 0);
  return(flag);
}


static int receive_offer(store_num, comment, new_offer, last_offer,
			 num_offer, factor)
int store_num;
char *comment;
register int32 *new_offer, last_offer;
int num_offer, factor;
{
  register int flag;
  register int receive;

  receive = 0;
  flag = FALSE;
  do
    {
      if (get_haggle(comment, new_offer, num_offer))
	{
	  if (*new_offer*factor >= last_offer*factor)
	    flag = TRUE;
	  else if (haggle_insults(store_num))
	    {
	      receive = 2;
	      flag = TRUE;
	    }
	  else
	    /* new_offer rejected, reset new_offer so that incremental
	       haggling works correctly */
	    *new_offer = last_offer;
	}
      else
	{
	  receive = 1;
	  flag = TRUE;
	}
    }
  while (!flag);
  return(receive);
}


/* Haggling routine					-RAK-	*/
static int purchase_haggle(store_num, price, item)
int store_num;
int32 *price;
inven_type *item;
{
  int32 max_sell, min_sell, max_buy;
  int32 cost, cur_ask, final_ask, min_offer;
  int32 last_offer, new_offer;
  int32 x1, x2, x3;
  int32 min_per, max_per;
  register int flag, loop_flag;
  char *comment;
  vtype out_val;
  int purchase, num_offer, final_flag;
  register store_type *s_ptr;
  register owner_type *o_ptr;

  flag = FALSE;
  purchase = 0;
  *price = 0;
  final_flag = 0;
  s_ptr = &store[store_num];
  o_ptr = &owners[s_ptr->owner];
  cost = sell_price(store_num, &max_sell, &min_sell, item);
  max_sell = max_sell * chr_adj() / 100;
  if (max_sell <= 0)  max_sell = 1;
  min_sell = min_sell * chr_adj() / 100;
  if (min_sell <= 0)  min_sell = 1;
  /* cast max_inflate to signed so that subtraction works correctly */
  max_buy  = cost * (200 - (int)o_ptr->max_inflate) / 100;
  if (max_buy <= 0) max_buy = 1;
  min_per  = o_ptr->haggle_per;
  max_per  = min_per * 3;
  haggle_commands(1);
  cur_ask   = max_sell;
  final_ask = min_sell;
  min_offer = max_buy;
  last_offer = min_offer;
  new_offer = 0;
  num_offer = 0; /* this prevents incremental haggling on first try */
  comment = "Asking";

  /* go right to final price if player has bargained well */
  if (noneedtobargain(store_num, final_ask))
    {
      put_buffer(Store_window, "After a long bargaining session, you agree upon the price.", 1, 0);
      cur_ask = min_sell;
    }

  do
    {
      do
	{
	  loop_flag = TRUE;
	  (void) sprintf(out_val, "%s :  %ld", comment, cur_ask);
	  put_buffer(Store_window, out_val, 2, 0);
	  purchase = receive_offer(store_num, "What do you offer? ",
				   &new_offer, last_offer, num_offer, 1);
	  if (purchase != 0)
	    flag = TRUE;
	  else
	    {
	      if (new_offer > cur_ask)
		{
		  put_buffer_comment6();
		  /* rejected, reset new_offer for incremental haggling */
		  new_offer = last_offer;
		}
	      else if (new_offer == cur_ask)
		{
		  flag = TRUE;
		  *price = new_offer;
		}
	      else
		loop_flag = FALSE;
	    }
	}
      while (!flag && loop_flag);
      if (!flag)
	{
	  x1 = (new_offer - last_offer) * 100 / (cur_ask - last_offer);
	  if (x1 < min_per)
	    {
	      flag = haggle_insults(store_num);
	      if (flag)	 purchase = 2;
	    }
	  else if (x1 > max_per)
	    {
	      x1 = x1 * 75 / 100;
	      if (x1 < max_per)	 x1 = max_per;
	    }
	  x2 = x1 + randint(5) - 3;
	  x3 = ((cur_ask - new_offer) * x2 / 100) + 1;
	  /* don't let the price go up */
	  if (x3 < 0)
	    x3 = 0;
	  cur_ask -= x3;
	  if (cur_ask < final_ask)
	    {
	      cur_ask = final_ask;
	      comment = "Final Offer";
	      final_flag++;
	      if (final_flag > 3)
		{
		  if (increase_insults(store_num))
		    purchase = 2;
		  else
		    purchase = 1;
		  flag = TRUE;
		}
	    }
	  else if (new_offer >= cur_ask)
	    {
	      flag = TRUE;
	      *price = new_offer;
	    }
	  if (!flag)
	    {
	      last_offer = new_offer;
	      num_offer++; /* enable incremental haggling */
	      erase_line (Store_window, 2, 0);
	      (void) sprintf(out_val, "Your last offer : %ld", last_offer);
	      put_buffer(Store_window, out_val, 2, 39);
	      put_buffer_comment2(last_offer, cur_ask, final_flag);
	    }
	}
    }
  while (!flag);

  /* update bargaining info */
  if (purchase == 0)
    updatebargain(store_num, *price, final_ask);

  return(purchase);
}


/* Haggling routine					-RAK-	*/
static int sell_haggle(store_num, price, item)
int store_num;
int32 *price;
inven_type *item;
{
  int32 max_sell, max_buy, min_buy;
  int32 cost, cur_ask, final_ask, min_offer;
  int32 last_offer, new_offer;
  int32 max_gold;
  int32 x1, x2, x3;
  int32 min_per, max_per;
  register int flag, loop_flag;
  char *comment;
  vtype out_val;
  register store_type *s_ptr;
  register owner_type *o_ptr;
  int sell, num_offer, final_flag;

  flag = FALSE;
  sell = 0;
  *price = 0;
  final_flag = 0;
  s_ptr = &store[store_num];
  cost = item_value(item);
  if (cost < 1)
    {
      sell = 3;
      flag = TRUE;
    }
  else
    {
      o_ptr = &owners[s_ptr->owner];
      cost = cost * (200 - chr_adj()) / 100;
      cost = cost * (200 - rgold_adj[o_ptr->owner_race][py.misc.prace]) / 100;
      if (cost < 1)  cost = 1;
      max_sell = cost * o_ptr->max_inflate / 100;
      /* cast max_inflate to signed so that subtraction works correctly */
      max_buy  = cost * (200 - (int)o_ptr->max_inflate) / 100;
      min_buy  = cost * (200 - o_ptr->min_inflate) / 100;
      if (min_buy < 1) min_buy = 1;
      if (max_buy < 1) max_buy = 1;
      if (min_buy < max_buy)  min_buy = max_buy;
      min_per  = o_ptr->haggle_per;
      max_per  = min_per * 3;
      max_gold = o_ptr->max_cost;
    }
  if (!flag)
    {
      haggle_commands(-1);
      if (max_buy > max_gold)
	{
	  final_flag= 1;
	  comment = "Final Offer";
	  cur_ask   = max_gold;
	  final_ask = max_gold;
 put_buffer(Store_window,
     "I am sorry, but I have not the money to afford such a fine item.", 1, 0);
	}
      else
	{
	  cur_ask   = max_buy;
	  final_ask = min_buy;
	  if (final_ask > max_gold)
	    final_ask = max_gold;
	  comment = "Offer";

	  /* go right to final price if player has bargained well */
          if (noneedtobargain(store_num, final_ask))
	    {
       put_buffer(Store_window,
	   "After a long bargaining session, you agree upon the price.", 1, 0);
              cur_ask = final_ask;
            }
	}
      min_offer = max_sell;
      last_offer = min_offer;
      new_offer = 0;
      num_offer = 0; /* this prevents incremental haggling on first try */
      if (cur_ask < 1)	cur_ask = 1;
      do
	{
	  do
	    {
	      loop_flag = TRUE;
	      (void) sprintf(out_val, "%s :  %ld", comment, cur_ask);
	      put_buffer(Store_window, out_val, 2, 0);
	      sell = receive_offer(store_num, "What price do you ask? ",
				 &new_offer, last_offer, num_offer, -1);
	      if (sell != 0)
		flag   = TRUE;
	      else
		{
		  if (new_offer < cur_ask)
		    {
		      put_buffer_comment6();
		      /* rejected, reset new_offer for incremental haggling */
		      new_offer = last_offer;
		    }
		  else if (new_offer == cur_ask)
		    {
		      flag = TRUE;
		      *price = new_offer;
		    }
		  else
		    loop_flag = FALSE;
		}
	    }
	  while (!flag && loop_flag);
	  if (!flag)
	    {
	      x1 = (last_offer - new_offer) * 100 / (last_offer - cur_ask);
	      if (x1 < min_per)
		{
		  flag = haggle_insults(store_num);
		  if (flag)  sell = 2;
		}
	      else if (x1 > max_per)
		{
		  x1 = x1 * 75 / 100;
		  if (x1 < max_per)  x1 = max_per;
		}
	      x2 = x1 + randint(5) - 3;
	      x3 = ((new_offer - cur_ask) * x2 / 100) + 1;
	      /* don't let the price go down */
	      if (x3 < 0)
		x3 = 0;
	      cur_ask += x3;
	      if (cur_ask > final_ask)
		{
		  cur_ask = final_ask;
		  comment = "Final Offer";
		  final_flag++;
		  if (final_flag > 3)
		    {
		      if (increase_insults(store_num))
			sell = 2;
		      else
			sell = 1;
		      flag = TRUE;
		    }
		}
	      else if (new_offer <= cur_ask)
		{
		  flag = TRUE;
		  *price = new_offer;
		}
	      if (!flag)
		{
		  last_offer = new_offer;
		  num_offer++; /* enable incremental haggling */
		  erase_line (Store_window, 2, 0);
		  (void) sprintf(out_val, "Your last bid %ld", last_offer);
		  put_buffer(Store_window, out_val, 2, 39);
		  put_buffer_comment3(cur_ask, last_offer, final_flag);
		}
	    }
	}
      while (!flag);
    }

  /* update bargaining info */
  if (sell == 0)
    updatebargain(store_num, *price, final_ask);

  return(sell);
}


/* Buy an item from a store				-RAK-	*/
static int store_purchase(store_num, cur_top)
int store_num;
int *cur_top;
{
  int32 price;
  register int i, choice;
  bigvtype out_val, tmp_str;
  register store_type *s_ptr;
  inven_type sell_obj;
  register inven_record *r_ptr;
  int item_val, item_new, purchase;

  purchase = FALSE;
  s_ptr = &store[store_num];
  /* i == number of objects shown on screen	*/
  if (*cur_top == 12)
    i = s_ptr->store_ctr - 1 - 12;
  else if (s_ptr->store_ctr > 11)
    i = 11;
  else
    i = s_ptr->store_ctr - 1;
  if (s_ptr->store_ctr < 1)
    put_buffer(Store_window, "I am currently out of stock.", 1, 0);
      /* Get the item number to be bought		*/
  else if (get_store_item(&item_val,
			  "Which item are you interested in? ", 0, i))
    {
      item_val = item_val + *cur_top;	/* TRUE item_val	*/
      take_one_item(&sell_obj, &s_ptr->store_inven[item_val].sitem);
      if (inven_check_num(&sell_obj))
	{
	  if (s_ptr->store_inven[item_val].scost > 0)
	    {
	      price = s_ptr->store_inven[item_val].scost;
	      choice = 0;
	    }
	  else
	    choice = purchase_haggle(store_num, &price, &sell_obj);
	  if (choice == 0)
	    {
	      if (py.misc.au >= price)
		{
		  put_buffer_comment1();
		  decrease_insults(store_num);
		  py.misc.au -= price;
		  item_new = inven_carry(&sell_obj);
		  i = s_ptr->store_ctr;
		  store_destroy(store_num, item_val, TRUE);
		  objdes(tmp_str, &inventory[item_new], TRUE);
		  (void) sprintf(out_val, "You have %s (%c)",
				   tmp_str, item_new+'a');
		  put_buffer(Store_window, out_val, 1, 0);
		  check_strength();
		  if (*cur_top >= s_ptr->store_ctr)
		    {
		      *cur_top = 0;
		      display_inventory(store_num, *cur_top);
		    }
		  else
		    {
		      r_ptr = &s_ptr->store_inven[item_val];
		      if (i == s_ptr->store_ctr)
			{
			  if (r_ptr->scost < 0)
			    {
			      r_ptr->scost = price;
			      display_cost(store_num, item_val);
			    }
			}
		      else
			display_inventory(store_num, item_val);
		      store_put_buffer_gold();
		    }
		}
	      else
		{
		  if (increase_insults(store_num))
		    purchase = TRUE;
		  else
		    {
		      put_buffer_comment1();
		      put_buffer(Store_window,
				 "Liar!  You have not the gold!", 1, 0);
		    }
		}
	    }
	  else if (choice == 2)
	    purchase = TRUE;

	  /* Less intuitive, but looks better here than in purchase_haggle. */
	  display_commands();
	  erase_line (Store_window, 2, 0);
	}
      else
	put_buffer(Store_window, "You cannot carry that many different items.", 1, 0);
    }
  return(purchase);
}


/* Sell an item to the store				-RAK-	*/
static int store_sell(store_num, cur_top)
int store_num, *cur_top;
{
  int item_val, item_pos;
  int32 price;
  bigvtype out_val, tmp_str;
  inven_type sold_obj;
  register int sell, choice;

  sell = FALSE;
  if (get_item_for_sale(&item_val, "Which one? ", 0, inven_ctr-1))
    {
      take_one_item(&sold_obj, &inventory[item_val]);
      objdes(tmp_str, &sold_obj, TRUE);
      (void) sprintf(out_val, "Selling %s (%c)", tmp_str, item_val+'a');
      put_buffer(Store_window, out_val, 1, 0);
#ifdef MAC
      /* MPW limitations force a subroutine call, see sets.c */
      if (store_buy(store_num, sold_obj.tval))
#else
      if ((*store_buy[store_num])(sold_obj.tval))
#endif
	if (store_check_num(&sold_obj, store_num))
	  {
	    choice = sell_haggle(store_num, &price, &sold_obj);
	    if (choice == 0)
	      {
		put_buffer_comment1();
		decrease_insults(store_num);
		py.misc.au += price;
		/* identify object in inventory to set object_ident */
		identify(&item_val);
		/* retake sold_obj so that it will be identified */
		take_one_item(&sold_obj, &inventory[item_val]);
		/* call known2 for store item, so charges/pluses are known */
		known2(&sold_obj);
		inven_destroy(item_val);
		objdes(tmp_str, &sold_obj, TRUE);
		(void) sprintf(out_val, "You've sold %s", tmp_str);
		put_buffer(Store_window, out_val, 1, 0);
		store_carry(store_num, &item_pos, &sold_obj);
		check_strength();
		if (item_pos >= 0)
		  {
		    if (item_pos < 12)
		      if (*cur_top < 12)
			display_inventory(store_num, item_pos);
		      else
			{
			  *cur_top = 0;
			  display_inventory(store_num, *cur_top);
			}
		    else if (*cur_top > 11)
		      display_inventory(store_num, item_pos);
		    else
		      {
			*cur_top = 12;
			display_inventory(store_num, *cur_top);
		      }
		  }
		store_put_buffer_gold();
	      }
	    else if (choice == 2)
	      sell = TRUE;
	    else if (choice == 3)
	      {
		put_buffer(Store_window,
			   "How dare you!  I will not buy that!", 1, 0);
		sell = increase_insults(store_num);
	      }
	    /* Less intuitive, but looks better here than in sell_haggle. */
	    erase_line (Store_window, 2, 0);
	    display_commands();
	  }
	else
	  put_buffer(Store_window,
		     "I have not the room in my store to keep it.", 1, 0);
      else
	put_buffer(Store_window, "I do not buy such items.", 1, 0);
    }
  return(sell);
}

/* Entering a store					-RAK-	*/
void enter_store(store_num)
int store_num;
{
  int tmp_chr;
  char command;
  register int exit_flag;
  register store_type *s_ptr;
  int cur_top, storen;

  storen = store_num;

  Store_window = CreateDWin(WIDTH, 25 * mfontheight, 296, 550,
			      DefaultRootDWin, mfgpix, mbgpix);
  XSetTransientForHint(p_disp, Store_window->win, DefaultRootWindow(p_disp));
  SetWMHints(Store_window->win);
  XSelectInput(p_disp, Store_window->win, ExposureMask | ButtonPressMask |
	       KeyPressMask);
  XMapWindow(p_disp, Store_window->win);
  clear_screen(Store_window);
  s_ptr = &store[store_num];
  if (s_ptr->store_open < turn)
    {
      exit_flag = FALSE;
      cur_top = 0;
      display_store(store_num, cur_top);
      do
	{
	  /* clear the msg flag just like we do in dungeon.c */
	  msg_flag = FALSE;
	  command = inkeyr(Store_window);
	  if (command != '\0' && command != '\033')
	    {
	      switch(command)
		{
		case 'b':
		  if (cur_top == 0)
		    if (s_ptr->store_ctr > 12)
		      {
			cur_top = 12;
			display_inventory(store_num, cur_top);
		      }
		    else
		      put_buffer(Store_window,
				 "Entire inventory is shown.", 1, 0);
		  else
		    {
		      cur_top = 0;
		      display_inventory(store_num, cur_top);
		    }
		  break;
		case 'E': case 'e':	         /* Equipment List	*/
		case 'I': case 'i':	         /* Inventory		*/
		case 'T': case 't':	         /* Take off		*/
		case 'W': case 'w':	         /* Wear		*/
		case 'X': case 'x':	         /* Switch weapon	*/
		  tmp_chr = py.stats.use_stat[A_CHR];
		  do
		    {
		      inven_command(command);
		      command = doing_inven;
		    }
		  while (command);
		  /* redisplay store prices if charisma changes */
		  if (tmp_chr != py.stats.use_stat[A_CHR])
		    display_inventory(store_num, cur_top);
		  free_turn_flag = FALSE;	/* No free moves here. -CJS- */
		  break;
		case 'p':
		  exit_flag = store_purchase(store_num, &cur_top);
		  break;
		case 's':
		  exit_flag = store_sell(store_num, &cur_top);
		  break;
		default:
		  bell();
		  break;
		}
	    }
	  else
	    exit_flag = TRUE;
	}
      while (!exit_flag);
      /* Can't save and restore the screen, because inven_command does that. */
      draw_cave();
    }
  else
    put_buffer(Store_window, "The doors are locked.", 1, 0);
  DestroyDWin(Store_window);
  prt_gold();
}
