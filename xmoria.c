#define MAIN
#include "xmoria.h"
#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"
#include <stdio.h>

player_type py;
int generate;
int result;
int new_game = FALSE;
int force_rogue_like = FALSE;
int force_keys_to;
char string[80];

main(argc, argv)
int argc;
char *argv[];
{
  char *tmpstr;
  Colormap default_cmap;
  int32u seed;
  char *p;

  /* default command set defined in config.h file */
  rogue_like_commands = ROGUE_LIKE;

/* Call this routine to grab a file pointer to the log files and
   start the backup process before relinquishing setuid privileges */
  init_files();

  tmpstr = (char *)malloc(100);

  if (0 != setuid(getuid()))
    {
      perror("Can't set permissions correctly!  Setuid call failed.\n");
      exit(0);
    }
  if (0 != setgid(getgid()))
    {
      perror("Can't set permissions correctly!  Setgid call failed.\n");
      exit(0);
    }

  seed = 0; /* let wizard specify rng seed */
  /* check for user interface option */
  for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv)
    switch (argv[0][1])
      {
      case 'N':
      case 'n': new_game = TRUE; break;
      case 'O':
      case 'o':
	/* rogue_like_commands may be set in get_char(), so delay this
	   until after read savefile if any */
	force_rogue_like = TRUE;
	force_keys_to = FALSE;
	break;
      case 'R':
      case 'r':
	force_rogue_like = TRUE;
	force_keys_to = TRUE;
	break;
      case 'S':
      case 's':
	display_scores(0, TRUE);
	exit();
	break;
      case 'W':
      case 'w':
	to_be_wizard = TRUE;

	if (isdigit((int)argv[0][2]))
	  seed = atoi(&argv[0][2]);
	break;
      default: (void) printf("Usage: xmoria [-norsw] [savefile]\n");
	exit_game();
      }

  p_disp = OpenDefaultDisplay();

#if (COST_ADJ != 100)
  price_adjust();
#endif

  /* Grab a random seed from the clock		*/
  init_seeds(seed);

  /* Init monster and treasure levels for allocate */
  init_m_level();
  init_t_level();

  /* Init the store inventories			*/
  store_init();

  /* Auto-restart of saved file */
  if (argv[0] != NULL)
    (void) strcpy (savefile, argv[0]);
  else if (p = (char *)getenv("MORIA_SAV"))
    (void) strcpy(savefile, p);
  else if (p = (char *)getenv("HOME"))
    (void) sprintf(savefile, "%s/%s", p, MORIA_SAV);
  else
    (void) strcpy(savefile, MORIA_SAV);

  CreateDefaultRootDWin();

  Spell_win = NULL;
  Char_window = NULL;
  Inven_window = NULL;
  Memory_window = NULL;

  mfontstruct = GetFont(COURIER_MEDIUM_12);
  mfontheight = mfontstruct->max_bounds.ascent +
    mfontstruct->max_bounds.descent;
  BigFont = GetFont(TIMES_BOLD_24);
  BigFontheight = BigFont->max_bounds.ascent + BigFont->max_bounds.descent;
  Height = HEIGHT;
  Width = WIDTH;
  default_cmap = DefaultColormap(p_disp, DefaultScreen(p_disp));
  mbgpix = GetColors("NavyBlue", default_cmap, BP);
  mfgpix = GetColors("white", default_cmap, WP);

  Main = CreateDWin(Width, Height, 0, 280, DefaultRootDWin,
		      mfgpix, mbgpix);
  WindowProperties(Main->win, Width, Height, 100, 100, "Xmoria", argc, argv,
		   mfontstruct);

  SetWMHints_Main(Main->win);

  clearGC = CreateGC(Main->win, mfontstruct, mbgpix, mbgpix);
  theGC = CreateGC(Main->win, mfontstruct, mfgpix, mbgpix);
  BFGC = CreateGC(Main->win, BigFont, mfgpix, mbgpix);
  newcolor = GetColors("red", default_cmap, WP);
  redGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  newcolor = GetColors("gold", default_cmap, WP);
  goldGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  newcolor = GetColors("orange", default_cmap, WP);
  doorGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  newcolor = GetColors("slategrey", default_cmap, WP);
  wallGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  silvercolor = GetColors("grey", default_cmap, WP);
  silverGC = CreateGC(Main->win, mfontstruct, silvercolor, mbgpix);
  newcolor = GetColors("skyblue", default_cmap, WP);
  electricblueGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  newcolor = GetColors("burlywood3", default_cmap, WP);
  woodGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  tancolor = GetColors("tan", default_cmap, WP);
  tanGC = CreateGC(Main->win, mfontstruct, tancolor, mbgpix);
  newcolor = GetColors("springgreen", default_cmap, WP);
  greenGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);
  newcolor = GetColors("grey65", default_cmap, WP);
  floorGC = CreateGC(Main->win, mfontstruct, newcolor, mbgpix);

  XSelectInput(p_disp, Main->win, ExposureMask | KeyPressMask | ButtonPressMask);

  ClearArea(Main, 0, 0, Width, Height, 0);

  GetMaps();
  XSetStipple(p_disp, theGC, stipple.image);
  XSetStipple(p_disp, redGC, stipple.image);
  XSetStipple(p_disp, goldGC, stipple.image);

  play_game(argv);

  XFreeGC(p_disp, theGC);
  XFreeGC(p_disp, redGC);
  XFreeGC(p_disp, BFGC);
  DestroyDWin(Main);
  XCloseDisplay(p_disp);
}

GetBitmap(buf, filename)
Pix *buf;
char *filename;
{
  Pix temp;
  int itemp;

  temp.image = XCreatePixmap(p_disp, Main->win, 100, 100, 1);
  if (XReadBitmapFile(p_disp, Main->win, filename, &temp.width, &temp.height,
		  &temp.image, &itemp, &itemp) != BitmapSuccess)
    buf->width = 0;
  else
    {
      buf->image = XCreatePixmap(p_disp, Main->win, temp.width, temp.height, 1);
      XReadBitmapFile(p_disp, Main->win, filename, &(buf->width),
		      &(buf->height), &(buf->image), &itemp, &itemp);
      buf->height = temp.height;
      buf->width = temp.width;
    }
  XFreePixmap(p_disp, temp.image);
}

testbuttons()
{
  read_scroll();
}

testbuttonsw()
{
  inven_command('e');
}

testbuttonb()
{
  if (class[py.misc.pclass].spell == MAGE)
    cast();
  if (class[py.misc.pclass].spell == PRIEST)
    pray();
  prt_cmana();
}

get_event()
{
  XNextEvent(p_disp, &theEvent);
  return(theEvent.type);
}

charinfo_button()
{
  int event_type;

  if (!Char_window)
    {
      Char_window = CreateDWin(WIDTH, 22 * mfontheight, 296, 500,
			       DefaultRootDWin, mfgpix, mbgpix);
      XSetTransientForHint(p_disp, Char_window->win,
			   DefaultRootWindow(p_disp));
      XSelectInput(p_disp, Char_window->win, ExposureMask | ButtonPressMask);
      XMapWindow(p_disp, Char_window->win);
      ClearArea(Char_window, 0, 0, WIDTH, 22*mfontheight, 0);
    }
  CenterDrawString(Char_window, theGC, 20 * mfontheight, "Click to continue");
  display_char();

  while ((event_type = get_event()) != ButtonPress)
    {
      if (event_type == Expose)
	{
	  CenterDrawString(Char_window, theGC, 20 * mfontheight, "Click to continue");
	  display_char();
	}
    }
  DestroyDWin(Char_window);
  Char_window = NULL;
}

inventory_button()
{
  inven_command('i');
}

potion_button()
{
  quaff();
}

staff_button()
{
  use();
}

wand_button()
{
  aim();
}

exit_button()
{
  if (save_char())
    exit_game();
}

play_game(argv)
char *argv[];
{
  int itemp;
  DWin starting_window;
  char tmpstr[100];

  Char_window = CreateDWin(WIDTH, 25*mfontheight, 296, 500,
				DefaultRootDWin, mfgpix, mbgpix);
  ClearArea(Char_window, 0, 0, WIDTH, 25*mfontheight, 0);
  XSetTransientForHint(p_disp, Char_window->win, DefaultRootWindow(p_disp));
  XSelectInput(p_disp, Char_window->win, ExposureMask | ButtonPressMask);
  XMapWindow(p_disp, Char_window->win);

  do
    {
      XNextEvent(p_disp, &theEvent);
      if (theEvent.type == Expose)
	opening_window(Char_window);
    } while (theEvent.type != ButtonPress);

  DestroyDWin(Char_window);
  Char_window = 0;

  Stats_window = CreateDWin(WIDTH - 20, 7 * mfontheight, 10, 60, Main,
			      mfgpix, mbgpix);
  Map_window = CreateDWin(660, 440, 492, 0, DefaultRootDWin,
			    mfgpix, mbgpix);
  WindowProperties(Map_window->win, 660, 440, 492, 0, "XMoria Adventure Map",
		   1, argv, mfontstruct);
  SetWMHints(Map_window->win);
  XSelectInput(p_disp, Map_window->win, ExposureMask | KeyPressMask |
	       ButtonPressMask);
  Status_window = CreateDWin(WIDTH - 50, mfontheight + 2, 40,
			       30 + (30 - mfontheight) / 2, Main, mfgpix,
			       mbgpix);
  Flags_window = CreateDWin(WIDTH - 20, 2 * mfontheight+2, 10,
			      HEIGHT - 2 * mfontheight - 12, Main,
			      mfgpix, mbgpix);
  mapscreen = CreateDWin(MAX_WIDTH * 4, MAX_HEIGHT * 4,
			   360, 500, DefaultRootDWin, mfgpix,
			   mbgpix);
  WindowProperties(mapscreen->win, MAX_WIDTH * 4, MAX_HEIGHT * 4,
		   300, 100, "XMoria Dungeon Map", 1, argv,
		   mfontstruct);
  SetWMHints(mapscreen->win);
  XSelectInput(p_disp, mapscreen->win, ExposureMask | ButtonPressMask |
	       KeyPressMask);

  result = get_char(&itemp);

  if (to_be_wizard)
    if (!enter_wiz_mode())
      exit_game(); 

  if ((new_game == FALSE) && !access(savefile, 0) && result)
    {

      change_name();
      /* could be restoring a dead character after a signal or HANGUP */
      if (py.misc.chp < 0)
	death = TRUE;
    }
  else
    {	  /* Create character	   */
      create_character();

      char_inven_init();
      py.flags.food = 7500;
      py.flags.food_digested = 2;
      if (class[py.misc.pclass].spell == MAGE)
	{	  /* Magic realm   */
	  calc_spells(A_INT);
	  calc_mana(A_INT);
	}
      else if (class[py.misc.pclass].spell == PRIEST)
	{	  /* Clerical realm*/
	  calc_spells(A_WIS);
	  calc_mana(A_WIS);
	}
      if (!_new_log())
	{
	  (void) sprintf(string, "Can't get at log file \"%s\".", MORIA_LOG);
	  msg_print(string);
	  exit_game();
	}
      /* prevent ^c quit from entering score into scoreboard,
	 and prevent signal from creating panic save until this point,
	 all info needed for save file is now valid */
      character_generated = 1;
      itemp = TRUE;
    }

  XMapWindow(p_disp, Main->win);

  XMapWindow(p_disp, Stats_window->win);
  XMapWindow(p_disp, Status_window->win);
  XMapWindow(p_disp, Flags_window->win);
  XMapWindow(p_disp, mapscreen->win);
  XMapWindow(p_disp, Map_window->win);
  buttons[0] = MakeXBitmapButton(40, 10, 1, tancolor, mbgpix, Main->win,
				 &bigscroll, testbuttons, "");
  buttons[1] = MakeXBitmapButton(90, 10, 1, silvercolor, mbgpix, Main->win,
				 &bigsword, testbuttonsw, "");
  buttons[2] = MakeXBitmapButton(140, 10, 1, tancolor, mbgpix, Main->win,
				 &bigbook, testbuttonb, "");
  buttons[3] = MakeXBitmapButton(10, 10, 1, silvercolor, mbgpix, Main->win,
				 &bigperson, charinfo_button, "");
  buttons[4] = MakeXBitmapButton(190, 10, 1, tancolor, mbgpix, Main->win,
				 &backpack, inventory_button, "");
  buttons[5] = MakeXBitmapButton(370, 10, 1, mfgpix, mbgpix, Main->win,
				 &exit_pix, exit_button, "");
  buttons[6] = MakeXBitmapButton(240, 10, 1, silvercolor, mbgpix, Main->win,
				 &bigpotion, potion_button, "");
  buttons[7] = MakeXBitmapButton(290, 10, 1, silvercolor, mbgpix, Main->win,
				 &staff, staff_button, "");
  buttons[8] = MakeXBitmapButton(320, 10, 1, silvercolor, mbgpix, Main->win,
				 &wand, wand_button, "");

  if (force_rogue_like)
    rogue_like_commands = force_keys_to;

  magic_init();

  if (itemp)
    generate_cave();

  while(!death)
    {
      screen_map();
      prt_stat_block();
      dungeon();				  /* Dungeon logic */

      /* check for eof here, see inkey() in io.c */
      /* eof can occur if the process gets a HANGUP signal */
      if (eof_flag)
	{
	  (void) strcpy(died_from, "(end of input: saved)");
	  if (!save_char())
	    {
	      (void) strcpy(died_from, "unexpected eof");
	    }
	  /* should not reach here, by if we do, this guarantees exit */
	  death = TRUE;
	}

      if (!death) generate_cave();	       /* New level	*/
    }

  exit_game();		/* Character gets buried. */
}

process_events()
{
  int itemp, i = 0;

  XNextEvent(p_disp, &theEvent);
  
  for (itemp = 0; itemp < 9; itemp++)
    button_handler(buttons[itemp]);
  if (theEvent.xany.window == mapscreen->win && theEvent.type == ButtonPress)
    screen_map();
  if (theEvent.type == Expose)
    {
      if (theEvent.xany.window == mapscreen->win)
	  Refresh_DWin(mapscreen);
      else if (theEvent.xany.window == Map_window->win)
	  Refresh_DWin(Map_window);
      else if (theEvent.xany.window == Stats_window->win)
	  Refresh_DWin(Stats_window);
      else if (theEvent.xany.window == Status_window->win)
	  Refresh_DWin(Status_window);
      else if (theEvent.xany.window == Flags_window->win)
	  Refresh_DWin(Flags_window);
      else
	{
	  for (itemp = 0; itemp < 9; itemp++)
	    if (theEvent.xany.window == buttons[itemp]->xid)
	      i = 1;
	  if (i == 0)
	      new_char_window();
	}
    }
}

MapChar(col, row, obj, gc)
unsigned col, row;
Pix obj;
GC gc;
{
  if (obj.width != 0)
    DisplayPixD(Map_window, col*20, row*20, obj, gc);
  else
    DisplayPixD(Map_window, col*20, row*20, question, gc);
}

new_char_window()
{
  unsigned i;

  prt_stat_block();
  for (i = 0; i < 6; i++)
    prt_stat(i);
}

PrintMapChar(c, i, j)
char c;
unsigned i, j;
{
  if (islower(c))
    MapChar(i, j, monsters[(int)(c - 'a')], redGC);
  else if (isupper(c))
    MapChar(i, j, monsters[(int)(c - 'A' + 'z' - 'a') + 1], redGC);
  else
    {
      switch(c)
	{
	case '#':
	  MapChar(i, j, stone, wallGC);
	  break;
	case '+':
	  MapChar(i, j, closed_door, doorGC);
	  break;
	case '?':
	  MapChar(i, j, scroll, tanGC);
	  break;
	case '/':
	  MapChar(i, j, polearm, woodGC);
	  break;
	case '<':
	  MapChar(i, j, up_stairs, wallGC);
	  break;
	case '>':
	  MapChar(i, j, down_stairs, wallGC);
	  break;
	case '\"':
	  MapChar(i, j, amulet, goldGC);
	  break;
	case '\'':
	  MapChar(i, j, open_door, doorGC);
	  break;
	case '{':
	  MapChar(i, j, arrow, woodGC);
	  break;
	case '}':
	  MapChar(i, j, bow, woodGC);
	  break;
	case ')':
	  MapChar(i, j, shield, silverGC);
	  break;
	case '=':
	  MapChar(i, j, ring, goldGC);
	  break;
	case '-':
	  MapChar(i, j, wand, greenGC);
	  break;
	case '_':
	  MapChar(i, j, staff, theGC);
	  break;
	case '*':
	  MapChar(i, j, gem, theGC);
	  break;
	case '&':
	  MapChar(i, j, chest, woodGC);
	  break;
	case '$':
	  MapChar(i, j, coins, goldGC);
	  break;
	case '!':
	  MapChar(i, j, potion, electricblueGC);
	  break;
	case '|':
	  MapChar(i, j, sword, silverGC);
	  break;
	case '.':
	  MapChar(i, j, floor, floorGC);
	  break;
	case '@':
	  if (py.misc.pclass == 1)
	    MapChar(i, j, mage, theGC);
	  else if (py.misc.pclass == 0)
	    MapChar(i, j, warrior, theGC);
	  else if (py.misc.pclass == 5)
	    MapChar(i, j, paladin, theGC);
	  else if (py.misc.pclass == 2)
	    MapChar(i, j, priest, theGC);
	  else if (py.misc.pclass == 4)
	    MapChar(i, j, ranger, theGC);
	  else if (py.misc.pclass == 3)
	    MapChar(i, j, rogue, theGC);
	  break;
	case ':':
	  MapChar(i, j, rubble, wallGC);
	  break;
	case ';':
	  MapChar(i, j, loose_rock, wallGC);
	  break;
	case '[':
	  MapChar(i, j, marmor, silverGC);
	  break;
	case ']':
	  MapChar(i, j, miscarmor, silverGC);
	  break;
	case '1':
	  MapChar(i, j, generalstore, doorGC);
	  break;
	case '6':
	  MapChar(i, j, magicstore, doorGC);
	  break;
	case '3':
	  MapChar(i, j, weaponstore, doorGC);
	  break;
	case '5':
	  MapChar(i, j, alchemistshop, doorGC);
	  break;
	case '2':
	  MapChar(i, j, armorer, doorGC);
	  break;
	case '4':
	  MapChar(i, j, templeshop, doorGC);
	  break;
	case '^':
	  MapChar(i, j, trap, theGC);
	  break;
	case ' ':
	  MapChar(i, j, solid, clearGC);
	  break;
	case ',':
	  MapChar(i, j, food, theGC);
	  break;
	case '%':
	  MapChar(i, j, quartz, wallGC);
	  break;
	case '(':
	  MapChar(i, j, softarmor, silverGC);
	  break;
	case '\\':
	  MapChar(i, j, hafted_weapon, woodGC);
	  break;
	case '~':
	  MapChar(i, j, misc_item, woodGC);
	  break;
	default:
	  MapChar(i, j, mage, theGC);
	}
    }
  if (isalpha(c))
    Draw4Point(mapscreen, redGC, i + panel_col_min, j + panel_row_min);
  else
    {
      switch(c)
	{
	case '@':
	  Draw4Point(mapscreen, theGC, i+panel_col_min, j+panel_row_min);
	  break;
	case '#':
	  Draw4Point(mapscreen, wallGC, i + panel_col_min, j + panel_row_min);
	  break;
	case '<':
	  Draw4Point(mapscreen, electricblueGC, i + panel_col_min, j + panel_row_min);
	  break;
	case '>':
	  Draw4Point(mapscreen, woodGC, i + panel_col_min, j + panel_row_min);
	  break;
	case '*':
	case '$':
	  Draw4Point(mapscreen, goldGC, i + panel_col_min, j + panel_row_min);
	  break;
	case ' ':
	case '.':
	  Draw4Point(mapscreen, clearGC, i + panel_col_min, j + panel_row_min);
	  break;
	case '+':
	  Draw4Point(mapscreen, doorGC, i + panel_col_min, j + panel_row_min);
	  break;
	default:
	  Draw4Point(mapscreen, silverGC, i + panel_col_min, j + panel_row_min);
	}
    }
}
	
opening_window(starting_window)
DWin starting_window;
{
  clear_screen(starting_window);
  XDrawString(p_disp, starting_window->win, BFGC, (Width - XTextWidth(BigFont, 
	          OPENING_HEADER_STRING, strlen(OPENING_HEADER_STRING))) / 2,
          BigFontheight, OPENING_HEADER_STRING, strlen(OPENING_HEADER_STRING));
  XDrawString(p_disp, starting_window->map, BFGC, (Width - XTextWidth(BigFont, 
	          OPENING_HEADER_STRING, strlen(OPENING_HEADER_STRING))) / 2,
          BigFontheight, OPENING_HEADER_STRING, strlen(OPENING_HEADER_STRING));
  DrawString(starting_window, theGC, 10, 5*mfontheight,
	     "XProgrammer: Joel Lord (lordj@rpi.edu)");
  DrawString(starting_window, theGC, 10, 7*mfontheight,
	     "BitMap Artists: Jim Driscoll");
  DrawString(starting_window, theGC, 10, 8*mfontheight,
	     "                Elizabeth Reeser (reesee@rpi.edu)");
  DrawString(starting_window, theGC, 10, 9*mfontheight,
	     "                Jeff Rogers (rogerj@rpi.edu)");
  DrawString(starting_window, theGC, 10, 10*mfontheight,
	     "                Jeff Young (youngj4@rpi.edu)");
  DrawString(starting_window, theGC, 10, 11*mfontheight,
	     "                Mike Zeylikman (zeylim@rpi.edu)");
  DrawString(starting_window, theGC, 10, 13*mfontheight,
	     "Logic Programmers: Robert Alan Koeneke");
  DrawString(starting_window, theGC, 10, 14*mfontheight,
	     "                   Jimmey Wayne Todd");
  DrawString(starting_window, theGC, 10, 15*mfontheight,
	     "UNIX Port: James E. Wilson");
  DrawString(starting_window, theGC, 10, 17*mfontheight,
	     "NOTE:  This version is thought to be buggy.  Anyone finding any");
  DrawString(starting_window, theGC, 10, 18*mfontheight,
	     "       problems should mail a complete description of the");
  DrawString(starting_window, theGC, 10, 19*mfontheight,
	     "       problem and what led up to it to lordj@rpi.edu");
  DrawString(starting_window, theGC, 10, 20*mfontheight,
	     "       Thank you for your help.");
  CenterDrawString(starting_window, redGC, 22*mfontheight,
		   "<Click To Continue>");
}

DisplayStatus(str)
char *str;
{
  clear_screen(Status_window);
  DrawString(Status_window, theGC, 5, mfontheight - 3, str);
}

DrawString(win, gc, col, row, str)
DWin win;
GC gc;
int row, col;
char *str;
{
  XDrawString(p_disp, win->win, gc, col, row, str, strlen(str));
  XDrawString(p_disp, win->map, gc, col, row, str, strlen(str));
}

CenterDrawString(win, gc, row, str)
DWin win;
GC gc;
int row;
char *str;
{
  unsigned width, height;

  width = win->width;
  height = win->height;
  
  XDrawString(p_disp, win->win, gc, (width - XTextWidth(mfontstruct, str,
				    strlen(str))) / 2, row, str, strlen(str));
  XDrawString(p_disp, win->map, gc, (width - XTextWidth(mfontstruct, str,
				    strlen(str))) / 2, row, str, strlen(str));
}

GetMaps()
{
  unsigned i;

  GetPathBitmap(&stipple, "stipple");
  GetPathBitmap(&bigscroll, "bigscroll");
  GetPathBitmap(&amulet, "amulet");
  GetPathBitmap(&bow, "bow");
  GetPathBitmap(&scroll, "scroll");
  GetPathBitmap(&coins, "coin");
  GetPathBitmap(&closed_door, "doorclose");
  GetPathBitmap(&open_door, "dooropen");
  GetPathBitmap(&gem, "gem");
  GetPathBitmap(&potion, "potion");
  GetPathBitmap(&shield, "shield");
  GetPathBitmap(&down_stairs, "stairdown");
  GetPathBitmap(&sword, "sword");
  GetPathBitmap(&wand, "wand");
  GetPathBitmap(&bigsword, "wear");
  GetPathBitmap(&arrow, "arrow");
  GetPathBitmap(&chest, "chest");
  GetPathBitmap(&magicbook, "magicbook");
  GetPathBitmap(&polearm, "polearm");
  GetPathBitmap(&prayerbook, "prayerbook");
  GetPathBitmap(&staff, "staff");
  GetPathBitmap(&up_stairs, "stairup");
  GetPathBitmap(&bigbook, "bigbook");
  GetPathBitmap(&stone, "stone");
  GetPathBitmap(&bigperson, "bigperson");
  GetPathBitmap(&backpack, "backpack");
  GetPathBitmap(&worldmap, "worldmap");
  GetPathBitmap(&exit_pix, "exit");
  GetPathBitmap(&bigpotion, "bigpotion");
  GetPathBitmap(&mage, "wizard");
  GetPathBitmap(&warrior, "warrior");
  GetPathBitmap(&ring, "ring");
  GetPathBitmap(&floor, "floor");
  GetPathBitmap(&rubble, "rubble");
  GetPathBitmap(&solid, "solid");
  GetPathBitmap(&question, "question");
  GetPathBitmap(&magicstore, "magicstore");
  GetPathBitmap(&weaponstore, "weaponstore");
  GetPathBitmap(&trap, "trap");
  GetPathBitmap(&alchemistshop, "alchemist");
  GetPathBitmap(&armorer, "armorer");
  GetPathBitmap(&marmor, "marmor");
  GetPathBitmap(&paladin, "paladin");
  GetPathBitmap(&priest, "priest");
  GetPathBitmap(&ranger, "ranger");
  GetPathBitmap(&rogue, "rogue");
  GetPathBitmap(&templeshop, "temple");
  GetPathBitmap(&loose_rock, "rocks");
  GetPathBitmap(&food, "food");
  GetPathBitmap(&generalstore, "generalstore");
  GetPathBitmap(&miscarmor, "miscarmor");
  GetPathBitmap(&quartz, "quartz");
  GetPathBitmap(&softarmor, "softarmor");
  GetPathBitmap(&hafted_weapon, "ballnchain");
  GetPathBitmap(&misc_item, "misc");
  for (i = 0; i < 51; i++)
    {
      if (strcmp(monster_files[i], " "))
	{
	  GetPathBitmap(&monsters[i], monster_files[i]);
	}
    }
}

GetPathBitmap(map, name)
Pix *map;
char *name;
{
  char file[100];

  strcpy(file, BITMAP_PATH);
  strcat(file, name);
  strcat(file, BITMAP_TYPE);

  GetBitmap(map, file);
}

DisplayStat(x, y, str)
unsigned x, y;
char *str;
{
  DrawString(Stats_window, theGC, x, mfontheight * y, str);
}

ClearArea(win, x, y, width, height, exp)
DWin win;
unsigned x, y, width, height, exp;
{
  XClearArea(p_disp, win->win, x, y, width, height, exp);
  XFillRectangle(p_disp, win->map, clearGC, x, y, width, height);
}

FillArea(win, x, y, width, height, gc)
DWin win;
unsigned x, y, width, height;
GC gc;
{
  XFillRectangle(p_disp, win->win, gc, x, y, width, height);
  XFillRectangle(p_disp, win->map, gc, x, y, width, height);
}

prt_stat_block()
{
  unsigned i;
  char tmpstr[100];
  unsigned long status;

  DisplayStat(5, 1, race[py.misc.prace].trace);
  DisplayStat(5, 2, class[py.misc.pclass].title);
  DisplayStat(5, 3, title_string());
  for (i = 0; i < 6; i++)
    prt_stat (i);
  DisplayStat(260, 1, "LEV");
  DisplayStat(260, 2, "EXP");
  DisplayStat(260, 3, "MANA");
  DisplayStat(260, 4, "HP");
  DisplayStat(260, 5, "AC");
  DisplayStat(260, 6, "GOLD");
  sprintf(tmpstr, "%u", py.misc.lev);
  DisplayStat(300, 1, tmpstr);
  sprintf(tmpstr, "%lu", py.misc.exp);
  DisplayStat(300, 2, tmpstr);

  XDrawRectangle(p_disp, Stats_window->win, theGC, 280, 3*mfontheight + 2, 90,
		 mfontheight);
  XDrawRectangle(p_disp, Stats_window->map, theGC, 280, 3*mfontheight + 2, 90,
		 mfontheight);

  prt_mhp();

  XDrawRectangle(p_disp, Stats_window->win, theGC, 300, 2*mfontheight + 2, 70,
		 mfontheight);
  XDrawRectangle(p_disp, Stats_window->map, theGC, 300, 2*mfontheight + 2, 70,
		 mfontheight);

  prt_cmana();

  sprintf(tmpstr, "%d", py.misc.dis_ac);
  DisplayStat(300, 5, tmpstr);
  sprintf(tmpstr, "%lu", py.misc.au);
  DisplayStat(300, 6, tmpstr);

  prt_depth();
  status = py.flags.status;
  if ((PY_HUNGRY|PY_WEAK) & status)
    prt_hunger();
  if (PY_BLIND & status)
    prt_blind();
  if (PY_CONFUSED & status)
    prt_confused();
  if (PY_FEAR & status)
    prt_afraid();
  if (PY_POISONED & status)
    prt_poisoned();
  if ((PY_SEARCH|PY_REST) & status)
    prt_state ();
  if (py.flags.speed - ((PY_SEARCH & status) >> 8) != 0)
    prt_speed ();
}

prt_stat(stat)
int stat;
{
  stat_type out_val1;

  XFillRectangle(p_disp, Stats_window->win, clearGC, 180, stat * mfontheight,
		 50, mfontheight + 2);
  XFillRectangle(p_disp, Stats_window->map, clearGC, 180, stat * mfontheight,
		 50, mfontheight + 2);

  cnv_stat(py.stats.use_stat[stat], out_val1);
  DisplayStat(140, stat + 1, stat_names[stat]);
  DisplayStat(180, stat + 1, out_val1);
}

prt_flag(str, col)
char *str;
unsigned col;
{
  if (col < 70)
    put_buffer(Flags_window, str, 1, col);
  else
    put_buffer(Flags_window, str, 2, col - 70);
}

put_qio()
{
}

/* Init players with some belongings			-RAK-	*/
static char_inven_init()
{
  register int i, j;
  inven_type inven_init;

  /* this is needed for bash to work right, it can't hurt anyway */
  for (i = 0; i < INVEN_ARRAY_SIZE; i++)
    invcopy(&inventory[i], OBJ_NOTHING);

  for (i = 0; i < 5; i++)
    {
      j = player_init[py.misc.pclass][i];
      invcopy(&inven_init, j);
      /* this makes it known2 and known1 */
      store_bought(&inven_init);
      /* must set this bit to display tohit/todam for stiletto */
      if (inven_init.tval == TV_SWORD)
	inven_init.ident |= ID_SHOW_HITDAM;
      (void) inven_carry(&inven_init);
    }

  /* wierd place for it, but why not? */
  for (i = 0; i < 32; i++)
    spell_order[i] = 99;
}

/* Initializes M_LEVEL array for use with PLACE_MONSTER	-RAK-	*/
static init_m_level()
{
  register int i, k;

  for (i = 0; i <= MAX_MONS_LEVEL; i++)
    m_level[i] = 0;

  k = MAX_CREATURES - WIN_MON_TOT;
  for (i = 0; i < k; i++)
    m_level[c_list[i].level]++;

  for (i = 1; i <= MAX_MONS_LEVEL; i++)
    m_level[i] += m_level[i-1];
}


/* Initializes T_LEVEL array for use with PLACE_OBJECT	-RAK-	*/
static init_t_level()
{
  register int i, l;
  int tmp[MAX_OBJ_LEVEL+1];

  for (i = 0; i <= MAX_OBJ_LEVEL; i++)
    t_level[i] = 0;
  for (i = 0; i < MAX_DUNGEON_OBJ; i++)
    t_level[object_list[i].level]++;
  for (i = 1; i <= MAX_OBJ_LEVEL; i++)
    t_level[i] += t_level[i-1];

  /* now produce an array with object indexes sorted by level, by using
     the info in t_level, this is an O(n) sort! */
  /* this is not a stable sort, but that does not matter */
  for (i = 0; i <= MAX_OBJ_LEVEL; i++)
    tmp[i] = 1;
  for (i = 0; i < MAX_DUNGEON_OBJ; i++)
    {
      l = object_list[i].level;
      sorted_objects[t_level[l] - tmp[l]] = i;
      tmp[l]++;
    }
}


#if (COST_ADJ != 100)
/* Adjust prices of objects				-RAK-	*/
static void price_adjust()
{
  register int i;

  /* round half-way cases up */
  for (i = 0; i < MAX_OBJECTS; i++)
    object_list[i].cost = ((object_list[i].cost * COST_ADJ) + 50) / 100;
}
#endif
