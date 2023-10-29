#include "xmoria.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"
#include <ctype.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/wait.h>
#include <X11/keysym.h>
 
char *getenv();
 
char inkeyr(wind)
DWin wind;
{
  int i = 0;
  char c[2];
  KeySym key;
  XComposeStatus cs;
 
  command_count = 0;
  while (1)
    {
      i = 0;
      while (!i)
	{
	  process_events();
	  if (theEvent.type == Expose && theEvent.xany.window == wind->win)
	    Refresh_DWin(wind);
	  if (theEvent.type == KeyPress)
	    {
	      XLookupString(&theEvent, c, 2, &key, &cs);
	      if (key < 0xFFE1 || key > 0xFFEE)
		{
		  switch(key)
		    {
		    case XK_R7:
		      i = 'y';
		      break;
		    case XK_Left:
		      i = 'h';
		      break;
		    case XK_Up:
		      i = 'k';
		      break;
		    case XK_Right:
		      i = 'l';
		      break;
		    case XK_Down:
		      i = 'j';
		      break;
		    case XK_R9:
		      i = 'u';
		      break;
		    case XK_R13:
		      i = 'b';
		      break;
		    case XK_R15:
		      i = 'n';
		      break;
		    case XK_BackSpace:
		    case XK_Delete:
		      i = 0x08;
		      break;
		    default:
		      i = (int)(c[0]);
		    }
		}
	    }
	}
      if (i != ('R' & 0x1F))
	return (char)i;
    }
}
 
char inkeya(wind)
DWin wind;
{
  int i = 0;
  char c[2];
  KeySym key;
  XComposeStatus cs;
 
  command_count = 0;
  while (1)
    {
      i = 0;
      while (!i)
	{
	  XNextEvent(p_disp, &theEvent);
	  if (theEvent.type == Expose && theEvent.xany.window == wind->win)
		  Refresh_DWin(wind);
	  if (theEvent.type == KeyPress)
	    {
	      XLookupString(&theEvent, c, 2, &key, &cs);
	      if (key < 0xFFE1 || key > 0xFFEE)
		{
		  switch(key)
		    {
		    case XK_R7:
		      i = 'y';
		      break;
		    case XK_Left:
		      i = 'h';
		      break;
		    case XK_Up:
		      i = 'k';
		      break;
		    case XK_Right:
		      i = 'l';
		      break;
		    case XK_Down:
		      i = 'j';
		      break;
		    case XK_R9:
		      i = 'u';
		      break;
		    case XK_R13:
		      i = 'b';
		      break;
		    case XK_R15:
		      i = 'n';
		      break;
		    case XK_BackSpace:
		    case XK_Delete:
		      i = 0x08;
		      break;
		    default:
		      i = (int)(c[0]);
		    }
		}
	    }
	}
 
      if (i != ('R' & 0x1F))
	return (char)i;
    }
}
 
inkey(event_flag)
unsigned event_flag;
{
  int i = 0;
  char c[2];
  KeySym key;
  XComposeStatus cs;
 
  command_count = 0;
  while (1)
    {
      while (!i)
	{
	  if (event_flag)
	    process_events();
	  else
	    XNextEvent(p_disp, &theEvent);
	  if (theEvent.type == KeyPress)
	    {
	      XLookupString(&theEvent, c, 2, &key, &cs);
	      i = (int)(c[0]);
	    }
	}
 
      if (i != ('R' & 0x1F))
	return (char)i;
    }
}
 
erase_line(win, row, col)
DWin win;
int row, col;
{
  unsigned width, height;
 
  width = win->width;
  height = win->height;

  ClearArea(win, col * 7, (row - 1) * mfontheight + 2, width - col * 7,
	    mfontheight, 0);
}
 
clear_screen(win)
DWin win;
{
  XClearArea(p_disp, win->win, 0, 0, 0, 0, 0);
  XFillRectangle(p_disp, win->map, clearGC, 0, 0, win->width, win->height);
}
 
clear_from(win, row)
DWin win;
int row;
{
  unsigned width, height;
 
  width = win->width;
  height = win->height;
  
  XClearArea(p_disp, win->win, 0, row * mfontheight, width, 
	     height - row * mfontheight, 0);
  XFillRectangle(p_disp, win->map, clearGC, 0, row * mfontheight, width,
		 height - row * mfontheight);
}
 
print(win, ch, row, col)
DWin win;
char ch;
int row, col;
{
  char tmp[2];
 
  tmp[0] = ch;
  tmp[1] = 0;
 
  put_buffer(win, tmp, row, col);
}
 
count_msg_print(p)
char *p;
{
  int i;
 
  i = command_count;
  msg_print(p);
  command_count = i;
}
 
prt(win, str_buff, row, col)
DWin win;
char *str_buff;
int row, col;
{
/*  if (row == 0 && msg_flag)
    msg_print(0);  */
  erase_line(win, row, col);
  put_buffer(win, str_buff, row, col);
}
 
int get_check(win, prompt)
DWin win;
char *prompt;
{
  int res;
 
  msg_print(prompt);
  
  do
    {
      res = inkeyr(win);
    } while (res == ' ');
  if (res == 'Y' || res == 'y')
    return 1;
  return 0;
}
 
int get_com(win, prompt, command)
DWin win;
char *prompt;
char *command;
{
  int res;
 
  if (prompt)
    prt(win, prompt, 1, 0);
  *command = inkeyr(win);
  if (*command == '\0' || *command == '\033')
    res = 0;
  else
    res = 1;
  return res;
}
 
int get_string(win, in_str, row, col, slen)
DWin win;
char *in_str;
int row, col, slen;
{
  int start_col, end_col, i;
  char *p;
  int flag, aborted;
 
  aborted = 0;
  flag = 0;
 
  start_col = col;
  end_col = col + slen - 1;
  if (end_col > 79)
    {
      slen = 80 - col;
      end_col = 79;
    }
  p = in_str;
  for (i = 0; i < slen; i++)
    *p++ = '\0';
  p = in_str;
  do
    {
      i = inkeya(win);
      switch(i)
	{
	case '\033':
	  aborted = 1;
	  break;
	case ('J' & 0x1F):
	case ('M' & 0x1F):
	  flag = 1;
	  break;
	case 0x7f:
	case 0x08:
	  if (col > start_col)
	    {
	      col--;
	      *--p = '\0';
	      erase_line(win, row, start_col);
	      put_buffer(win, in_str, row, start_col);
	    }
	  break;
	default:
	  if (!isascii(i) || col > end_col)
	    bell();
	  else
	    {
	      *p++ = i;
	      *(p+1) = '\0';
	      col++;
	      erase_line(win, row, start_col);
	      put_buffer(win, in_str, row, start_col);
	    }
	  break;
	}
    }
  while ((!flag) && (!aborted));
  if (aborted)
    return 0;
 
  while (p > in_str && p[-1] == ' ')
    p--;
  *p = '\0';
  return 1;
}
 
pause_line()
{
  msg_print("Press any key to continue");
  inkeyr(Main);
}
 
pause_exit(delay)
int delay;
{
  char dummy;
 
  msg_print("Press any key to continue, or Q to exit.");
  dummy = inkeyr(Main);
  if (dummy == 'Q')
    {
      if (delay > 0)
	sleep(delay);
      exit_game();
    }
}
 
/* MAX_HEIGHT MAX_WIDTH are height and width of the map */
 
screen_map()
{
  XClearWindow(p_disp, mapscreen->win);
  XFillRectangle(p_disp, mapscreen->map, clearGC, 0, 0, mapscreen->width,
		 mapscreen->height);
  draw_map(mapscreen);
}
 
draw_map(win)
DWin win;
{
  int i, j;
  char c;
 
  for (i = 0; i < MAX_HEIGHT; i++)
    {
      for (j = 0; j < MAX_WIDTH; j++)
	{
	  c = loc_symbol(i, j);
	  
	  if (isalpha(c))
	    Draw4Point(win, redGC, j, i);
	  else
	    switch(c)
	      {
	      case '@':
		Draw4Point(win, theGC, j, i);
		break;
	      case '#':
		Draw4Point(win, wallGC, j, i);
		break;
	      case '<':
		Draw4Point(win, electricblueGC, j, i);
		break;
	      case '>':
		Draw4Point(win, woodGC, j, i);
		break;
	      case '*':
	      case '$':
		Draw4Point(win, goldGC, j, i);
		break;
	      case ' ':
		break;
	      case '.':
		break;
	      case '+':
		Draw4Point(win, doorGC, j, i);
		break;
	      default:
		Draw4Point(win, silverGC, j, i);
	      }
	}
    }
}
 
Draw4Point(win, gc, col, row)
DWin win;
GC gc;
unsigned row, col;
{
  unsigned abs_row = row * 4;
  unsigned abs_col = col * 4;
  unsigned i, j;
 
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
    {
      XDrawPoint(p_disp, win->win, gc, abs_col + j, abs_row + i);
      XDrawPoint(p_disp, win->map, gc, abs_col + j, abs_row + i);
    }
}
 
bell()
{
  put_qio();
  write(1, "\007", 1);
}
 
msg_print(str)
char *str;
{
  char temp[80];
 
  if (str)
    {
      last_msg++;
      if (last_msg >= 22)
	last_msg = 0;
      strncpy(old_msg[last_msg], str, 80);
      old_msg[last_msg][79] = '\0';
      msg_flag = 1;
      DisplayStatus(str);
    }
  else
    {
      strcpy(temp, old_msg[last_msg]);
      strcat(temp, " -pause-");
      DisplayStatus(temp);
      inkeyr(Status_window);
      msg_flag = 0;
    }
}
 
put_buffer(win, str, row, col)
DWin win;
char *str;
unsigned row, col;
{
  XFillRectangle(p_disp, win->win, clearGC, col * 7, (row - 1) * mfontheight,
		 XTextWidth(mfontstruct, str, strlen(str)), mfontheight + 2);
  XFillRectangle(p_disp, win->map, clearGC, col * 7, (row - 1) * mfontheight,
	         XTextWidth(mfontstruct, str, strlen(str)), mfontheight + 2);
  DrawString(win, theGC, col * 7, row * mfontheight - 3, str);
}
