/**
 *  Modularized X Windows functions
 *
 *   Meant to simplify basic X Windows programming
 *
 *
 * GetColors(name, cmap, def)
 *  Allocates color "name", or uses color "def" if "name" is unavailable
 *  char *name		name of color to be allocated
 *  Colormap cmap	colormap to get "name" from
 *  unsigned long def	default color to use if "name" is unavailable
 *		  	 probably the output from a function like "BlackPixel"
 *
 * Display *OpenDefaultDisplay()
 *  Opens the default display ("NULL")
 *
 * XFontStruct *GetFont(fontname)
 *  Load a font for use in X Windows
 *  char *fontname	name of font to load
 *
 * Window CreateWindow(width, height, x, y, parent, fgcolor, bgcolor)
 *  Create a window
 *  unsigned width, height, x, y;
 *  Window parent;
 *  unsigned long fgcolor, bgcolor;
 *
 * DWin DefaultRootDWin()
 *  Returns the DWin equivalent to DefaultRootWindow()
 *
 * DWin CreateDWin(width, height, x, y, parent, fgcolor, bgcolor)
 *  Create a "double-window", with a pixmap backup of the actual window
 *  unsigned width, height, x, y;
 *  DWin parent;
 *  unsigned long fgcolor, bgcolor;
 *
 * Window CreateDefaultRootWindow(width, height, x, y,
 *						fgcname, bgcname, fname)
 *  Create a root window and allocate colors fgcname and bgcname
 *    and allocate font fname
 *  unsigned width, height, x, y;
 *  char *fgcname, *bgcname, *fname;
 *
 * WindowProperties(window, width, height, x, y, pname, argc, argv, font)
 *  Set the properties of a root window -- icon name, window name, etc.
 *  Window window;			the window to set the properties of
 *  unsigned width, height, x, y;
 *  char *pname;			the name on the icon, and the window
 *  int argc;
 *  char *argv[];
 *  XFontStruct font;			The default font for use in the window
 *
 * SetWMHints(window)
 *  Set the window manager Input and Initial_State hints
 *  Window window;		The window to set the hints for
 *
 * GC CreateGC(window, font, fgcolor, bgcolor)
 *  Create a Graphics Context for a window
 *  Window window;		the window to create the graphics context for
 *  XFontStruct font;		The default font for the window
 *  unsigned long fgcolor, bgcolor;
 *
 * GC AlternateColor(wind, cname, cmap, font, def)
 *  Create an altername GC with a different foreground color
 *  Window wind;
 *  char *cname;
 *  Colormap cmap;
 *  XFontStruct *font;
 *  unsigned long def;
 *
 * SetMap(window, cmap)
 *  Set the default colormap and bit gravity for a window
 *  Window window;		The window to set for
 *  Colormap cmap;		The default colormap for the window
 *
 * XWIN *MakeXButton(width, height, x, y, bdwidth, bdcolor, bgcolor, parent,
 *					label, button_action, action_data)
 *  Create a mouse-click button
 *  unsigned width, height, x, y, bdwidth;
 *  unsigned long bdcolor, bgcolor;
 *  Window parent;
 *  char *label;
 *  int (*button_action)();
 *  caddr_t action_data;
 *
 * XWIN *MakeXBitmapButton(x, y, bdwidth, bdcolor, bgcolor, parent,
 *					bpix, button_action, action_data)
 *  Create a mouse-click button with a bitmap instead of text
 *  unsigned x, y, bdwidth;
 *  unsigned long bdcolor, bgcolor;
 *  Window parent;
 *  Pix *bpix;
 *  int (*button_action)();
 *  caddr_t action_data;
 *
 * int button_handler(p_xwin)
 *  Handles events for the buttons
 *  XWIN *p_xwin;
 *
 * DisplayPix(win, x, y, dpix, gc)
 *  Display a Pix image
 *  unsigned x, y;
 *  Pix dpix;
 *  GC gc;
 *
 *
 *  Joel P. Lord 10-91
 *
**/
 
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include "fontlist.h"
 
#define WP	WhitePixel(p_disp, DefaultScreen(p_disp))
#define BP	BlackPixel(p_disp, DefaultScreen(p_disp))
 
typedef struct XWIN
{
  Window	xid;
  Window	parent;
  void		*data;
  int		(*event_handler)();
  GC		button_gc;
} XWIN;
 
typedef struct Pix
{
  Pixmap	image;
  unsigned	width;
  unsigned	height;
} Pix;
 
typedef struct D_BUTTON
{
  char		*label;
  int		(*action)();
  caddr_t	action_args;
  Pix		*pix;
} D_BUTTON;
 
typedef struct DWIN
{
  Window	win;
  Pixmap	map;
  unsigned	width, height;
} *DWin;
 
#ifdef MAIN
XFontStruct *mfontstruct;
unsigned mfontheight;
Display *p_disp;
DWin Main;
DWin DefaultRootDWin;
GC theGC;
XEvent theEvent;
char default_geometry[80];
unsigned long mbgpix, mfgpix;
XContext xwin_context;
 
#else
 
extern XFontStruct *mfontstruct;
extern unsigned mfontheight;
extern Display *p_disp;
extern DWin Main, DefaultRootDWin;
extern GC theGC;
extern XEvent theEvent;
extern char default_geometry[80];
extern unsigned long mbgpix, mfgpix;
extern XContext xwin_context;

#endif

#ifndef XPROGS

unsigned long GetColors();
Display *OpenDefaultDisplay();
XFontStruct *GetFont();
Window CreateWindow();
DWin CreateDefaultRootDWin();
DWin CreateDWin();
void Refresh_DWin();
void DestroyDWin();
Window CreateDefaultRootWindow();
void WindowProperties();
void SetWMHints();
GC CreateGC();
GC AlternateColor();
void SetMap();
XWIN *MakeXButton();
XWIN *MakeXBitmapButton();
int button_handler();
void DisplayPix();
void DisplayPixD();

#endif
