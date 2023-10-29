#define XPROGS

#include "Xprogs.h"

unsigned long GetColors(name, cmap, def)
char *name;
Colormap cmap;
unsigned long def;
{
  XColor color;
  unsigned long retval;
 
  if (XParseColor(p_disp, cmap, name, &color) == 0 || 
      XAllocColor(p_disp, cmap, &color) == 0)
    retval = def;
  else
    retval = color.pixel;
  return retval;
}
 
Display *OpenDefaultDisplay()
{
  Display *displ;
  if((displ = XOpenDisplay(getenv("DISPLAY"))) == NULL)
    {
      fputs("Can't open standard display",stderr);
      exit(1);
    }
  return displ;
}
 
XFontStruct *GetFont(fname)
char *fname;
{
  XFontStruct *tempfont;
 
  if ((tempfont = XLoadQueryFont(p_disp,fname)) == NULL)
    {
      fputs("Can't open font",stderr);
      fputs(fname,stderr);
      exit(1);
    }
  return tempfont;
}
 
Window CreateWindow(width, height, x, y, parent, fgcolor, bgcolor)
unsigned width, height, x, y;
Window parent;
unsigned long fgcolor, bgcolor;
{
  return (XCreateSimpleWindow(p_disp, parent, x, y, width, height, 1,
			      fgcolor, bgcolor));
}

DWin CreateDefaultRootDWin()
{
  DefaultRootDWin = (DWin)malloc(sizeof(struct DWIN));

  DefaultRootDWin->win = DefaultRootWindow(p_disp);
}
 
DWin CreateDWin(width, height, x, y, parent, fgcolor, bgcolor)
unsigned width, height, x, y;
DWin parent;
unsigned long fgcolor, bgcolor;
{
  DWin rval;
 
  rval = (DWin)malloc(sizeof(struct DWIN));
  rval->win = CreateWindow(width, height, x, y, parent->win,
    fgcolor, bgcolor);
  rval->map = XCreatePixmap(p_disp, parent->win, width, height,
			    DefaultDepth(p_disp, DefaultScreen(p_disp)));
  rval->height = height;
  rval->width = width;
  return rval;
}
 
Refresh_DWin(win)
DWin win;
{
  XCopyArea(p_disp, win->map, win->win, theGC, 0, 0, win->width,
	    win->height, 0, 0);
}
 
DestroyDWin(win)
DWin win;
{
  XDestroyWindow(p_disp, win->win);
  XFreePixmap(p_disp, win->map);
  free(win);
}
 
Window CreateDefaultRootWindow(width, height, x, y, fgcname, bgcname, fname)
unsigned width, height, x, y;
char *fgcname, *bgcname, *fname;
{
  Colormap default_cmap;
 
  mfontstruct = GetFont(fname);
  default_cmap = DefaultColormap(p_disp, DefaultScreen(p_disp));
  mbgpix = GetColors(bgcname, default_cmap, BP);
  mfgpix = GetColors(fgcname, default_cmap, WP);
  mfontheight = mfontstruct->max_bounds.ascent +
    mfontstruct->max_bounds.descent;
 
  return(CreateWindow(width, height, x, y, DefaultRootWindow(p_disp),
		      mfgpix, mbgpix));
}
 
WindowProperties(wind, width, height, x, y, pname, argc, argv, font)
Window wind;
unsigned width, height, x, y;
char *pname;
int argc;
char *argv[];
XFontStruct *font;
{
  char geom[80];
  XSizeHints xsh;
  int bitmask;
 
  xsh.flags = (PPosition | PSize | PMinSize);
  xsh.height = height;
  xsh.min_height = height;
  xsh.width = width;
  xsh.min_width = width;
  xsh.x = x;
  xsh.y = y;
 
  sprintf(geom, "%dx%d+%d+%d", width, height, x, y);
 
  bitmask = XGeometry(p_disp, DefaultScreen(p_disp), geom, geom, 1,
		      font->max_bounds.width, font->max_bounds.ascent+
		      font->max_bounds.descent,1, 1, &(xsh.x), &(xsh.y),
		      &(xsh.width), &(xsh.height));
 
  if(bitmask & (XValue | YValue))
    xsh.flags |= USPosition;
  if(bitmask & (WidthValue | HeightValue))
    xsh.flags |= USSize;
 
  XSetStandardProperties(p_disp, wind, pname, pname, None, argv, argc, &xsh);
}
 
SetWMHints_Main(wind)
Window wind;
{
  XWMHints xwmh;
 
  xwmh.flags = (InputHint | StateHint);
  xwmh.input = True;
  xwmh.initial_state = NormalState;
  XSetWMHints(p_disp, wind, &xwmh);
}

SetWMHints(wind)
Window wind;
{
  XWMHints xwmh;

  xwmh.flags = (InputHint | StateHint);
  xwmh.input = False;
  xwmh.initial_state = NormalState;
  XSetWMHints(p_disp, wind, &xwmh);
}

 
GC CreateGC(wind, fntstruct, fgpix, bgpix)
Window wind;
XFontStruct *fntstruct;
unsigned long fgpix, bgpix;
{
  XGCValues gcv;
 
  gcv.font = fntstruct->fid;
  gcv.foreground = fgpix;
  gcv.background = bgpix;
 
  return(XCreateGC(p_disp, wind, (GCFont | GCForeground | GCBackground), &gcv));
}
 
GC AlternateColor(wind, cname, cmap, font, def)
Window wind;
char *cname;
Colormap cmap;
XFontStruct *font;
unsigned long def;
{
  XGCValues gcv;
 
  gcv.foreground = GetColors(cname, cmap, def);
  gcv.font = font->fid;
 
  return(XCreateGC(p_disp, wind, GCForeground | GCFont, &gcv));
}
 
SetMap(wind, cmap)
Window wind;
Colormap cmap;
{
  XSetWindowAttributes xswa;
 
  xswa.colormap = cmap;
  xswa.bit_gravity = CenterGravity;
  XChangeWindowAttributes(p_disp, wind, (CWColormap | CWBitGravity), &xswa);
}
 
XWIN *MakeXButton(width, height, x, y, bdwidth, bdcolor, bgcolor, parent,
		  label, button_action, action_data)
unsigned width, height;
int x, y;
unsigned bdwidth;
unsigned long bdcolor, bgcolor;
Window parent;
char *label;
int (*button_action)();
caddr_t action_data;
{
  XWIN	*new_button;
  D_BUTTON *p_data;
  int button_handler();
 
  if ((p_data = (D_BUTTON *)calloc(1, sizeof(D_BUTTON))) == NULL)
    {
      fputs("No memory for button's data", stderr);
      exit(1);
    }
 
  if ((new_button = (XWIN *)calloc(1, sizeof(XWIN))) == NULL)
    {
      fputs("No memory for button", stderr);
      exit(1);
    }
 
  p_data->action = button_action;
  p_data->action_args = action_data;
  p_data->label = label;
 
  new_button->data = p_data;
  new_button->event_handler = button_handler;
  new_button->parent = parent;
  new_button->xid = XCreateSimpleWindow(p_disp, parent, x, y, width, height,
					bdwidth, bdcolor, bgcolor);
 
  new_button->button_gc = CreateGC(new_button->xid, mfontstruct, bdcolor,
				   bgcolor);
 
  if (XSaveContext(p_disp, new_button->xid, xwin_context, (caddr_t) new_button)
      != 0)
    {
      fputs("Error saving xwin_context data", stderr);
      exit(1);
    }
 
  XSelectInput(p_disp, new_button->xid, ExposureMask | ButtonPressMask);
 
  XMapWindow(p_disp, new_button->xid);
 
  return new_button;
}
 
XWIN *MakeXBitmapButton(x, y, bdwidth, bdcolor, bgcolor, parent,
		  bpix, button_action, action_data)
int x, y;
unsigned bdwidth;
unsigned long bdcolor, bgcolor;
Window parent;
Pix *bpix;
int (*button_action)();
caddr_t action_data;
{
  XWIN	*new_button;
  D_BUTTON *p_data;
  int button_handler();
 
  if ((p_data = (D_BUTTON *)calloc(1, sizeof(D_BUTTON))) == NULL)
    {
      fputs("No memory for button's data", stderr);
      exit(1);
    }
 
  if ((new_button = (XWIN *)calloc(1, sizeof(XWIN))) == NULL)
    {
      fputs("No memory for button", stderr);
      exit(1);
    }
 
  p_data->action = button_action;
  p_data->action_args = action_data;
  p_data->pix = bpix;
  p_data->label = NULL;
 
  new_button->data = p_data;
  new_button->event_handler = button_handler;
  new_button->parent = parent;
  new_button->xid = XCreateSimpleWindow(p_disp, parent, x, y, bpix->width + 2,
				  bpix->height + 2, bdwidth, bdcolor, bgcolor);
 
  new_button->button_gc = CreateGC(new_button->xid, mfontstruct, bdcolor,
				   bgcolor);
 
  if (XSaveContext(p_disp, new_button->xid, xwin_context, (caddr_t) new_button)
      != 0)
    {
      fputs("Error saving xwin_context data", stderr);
      exit(1);
    }
 
  XSelectInput(p_disp, new_button->xid, ExposureMask | ButtonPressMask);
 
  XMapWindow(p_disp, new_button->xid);
 
  return new_button;
}
 
button_handler(p_xwin)
XWIN *p_xwin;
{
  D_BUTTON *p_data = (D_BUTTON *) p_xwin->data;
 
  if (theEvent.xany.window == p_xwin->xid)
    {
      switch(theEvent.type)
	{
	case Expose:
	  if (theEvent.xexpose.count == 0)
	    {
	      if (p_data->label != NULL)
		XDrawString(p_disp, p_xwin->xid, p_xwin->button_gc, 1,
			  mfontheight, p_data -> label, strlen(p_data->label));
	      if (p_data->label == NULL)
		XCopyPlane(p_disp, (p_data->pix)->image, p_xwin->xid,
			   p_xwin->button_gc, 0, 0, (p_data->pix)->width,
			   (p_data->pix)->height, 1, 1, 1);
	    }
	  break;
	case ButtonPress:
	  if (p_data->action != NULL)
	    (*p_data->action)(p_data->action_args);
	}
    }
}
 
DisplayPix(win, x, y, dpix, gc)
Window win;
int x, y;
Pix dpix;
GC gc;
{
  XCopyPlane(p_disp, dpix.image, win, gc, 0, 0, dpix.width, dpix.height,
	     x, y, 1);
}
 
DisplayPixD(dwin, x, y, dpix, gc)
DWin dwin;
int x, y;
Pix dpix;
GC gc;
{
  XCopyPlane(p_disp, dpix.image, dwin->win, gc, 0, 0, dpix.width, dpix.height,
	     x, y, 1);
  XCopyPlane(p_disp, dpix.image, dwin->map, gc, 0, 0, dpix.width, dpix.height,
	     x, y, 1);
}
