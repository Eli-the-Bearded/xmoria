/* config.h: configuration definitions

   Copyright (c) 1989 James E. Wilson

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* Person to bother if something goes wrong. */
/* Recompile files.c and misc2.c if these change. */
#define WIZARD	"Joel Lord"
/* wizard password and wizard uid no longer used */


/* files used by moria, set these to valid pathnames */

/* probably unix */
#define MORIA_SAV	"moria.save"
#define MORIA_HOU	"/home/33/lordj/SparcWorks/xmoria/files/hours"
#define MORIA_TOP	"/home/33/lordj/SparcWorks/xmoria/files/scores"
#define MORIA_HELP	"/home/33/lordj/SparcWorks/xmoria/files/roglcmds.hlp"
#define MORIA_ORIG_HELP	"/home/33/lordj/SparcWorks/xmoria/files/origcmds.hlp"
#define MORIA_WIZ_HELP	"/home/33/lordj/SparcWorks/xmoria/files/rwizcmds.hlp"
#define MORIA_OWIZ_HELP	"/home/33/lordj/SparcWorks/xmoria/files/owizcmds.hlp"
#define MORIA_WELCOME	"/home/33/lordj/SparcWorks/xmoria/files/welcome.hlp"
#define MORIA_LOG	"/home/33/lordj/SparcWorks/xmoria/files/moria.log"
#define MORIA_VER	"/home/33/lordj/SparcWorks/xmoria/files/version.hlp"


/* this sets the default user interface */
/* to use the original key bindings (keypad for movement) set ROGUE_LIKE
   to FALSE, to use the rogue-like key bindings (vi style movement)
   set ROGUE_LIKE to TRUE */
/* if you change this, you only need to recompile main.c */
#define ROGUE_LIKE TRUE


/* for the ANDREW distributed file system, define this to ensure that
   the program is secure with respect to the setuid code, this prohibits
   inferior shells, also does not relinquish setuid priviledges at the start,
   but instead calls the ANDREW library routines bePlayer(), beGames(),
   and Authenticate() */
/* #define SECURE */


/* no system definitions are needed for 4.3BSD, SUN OS, DG/UX */

/* system dependent defines follow, you should not need to change anything
   below */

#ifdef SYS_III
char *index();
#endif

#if defined(SYS_III) || defined(SYS_V) || defined(MSDOS) || defined(MAC)
#ifndef USG
#define USG
#endif
#endif

#ifdef ATARIST_MWC
#define USG
#endif

/* Pyramid runs 4.2BSD-like UNIX version */
#if defined(Pyramid)
#define ultrix
#endif

#if defined(SYS_V) && defined(lint)
/* to prevent <string.h> from including <NLchar.h>, this prevents a bunch
   of lint errors. */
#define RTPC_NO_NLS
#endif

#ifdef SECURE
extern int PlayerUID;
#define getuid() PlayerUID
#define geteuid() PlayerUID
#endif
