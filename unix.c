 /* unix.c: UNIX dependent code.					-CJS-

   Copyright (c) 1989 James E. Wilson, Christopher J. Stuart

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#if defined(unix) || defined(AIXV3)

/* defines NULL */
#include <stdio.h>
/* defines CTRL */
#include <sys/ioctl.h>
/* defines TRUE and FALSE */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#if defined(SYS_V) && defined(lint)
/* for AIX, prevent hundreds of unnecessary lint errors, must define before
   signal.h is included */
#define _h_IEEETRAP
typedef struct { int stuff; } fpvmach;
#endif

#include <signal.h>

#ifdef M_XENIX
#include <sys/select.h>
#endif

#ifndef USG
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>
#endif

#ifdef USG
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#else
#include <strings.h>
#if defined(atarist) && defined(__GNUC__)
/* doesn't have <sys/wait.h> */
#else
#include <sys/wait.h>
#endif
#endif

#include <pwd.h>
#include <sys/errno.h>

#ifdef USG
struct passwd *getpwuid();
struct passwd *getpwnam();
#endif

#if defined(SYS_V) && defined(lint)
struct screen { int dumb; };
#endif

/* Fooling lint. Unfortunately, c defines all the TIO constants to be long,
   and lint expects them to be int. Also, ioctl is sometimes called with just
   two arguments. The following definition keeps lint happy. It may need to be
   reset for different systems. */
#ifdef lint
#ifdef Pyramid
/* Pyramid makes constants greater than 65535 into long! Gakk! -CJS- */
/*ARGSUSED*/
/*VARARGS2*/
static Ioctl(i, l, p) long l; char *p; { return 0; }
#else
/*ARGSUSED*/
/*VARARGS2*/
static Ioctl(i, l, p) char *p; { return 0; }
#endif
#define ioctl	    Ioctl
#endif

#if 0
/* This is not used, however, this should be compared against shell_out
   in io.c */

/* A command for the operating system. Standard library function
   'system' is unsafe, as it leaves various file descriptors
   open. This also is very careful with signals and interrupts,
   and does rudimentary job control, and puts the terminal back
   in a standard mode. */
int system_cmd(p)
char *p;
{
  int pgrp, pid, i, mask;
  union wait w;
  extern char *getenv();

  mask = sigsetmask(~0);	/* No interrupts. */
  /* Are we in the control terminal group? */
  if (ioctl(0, TIOCGPGRP, (char *)&pgrp) < 0 || pgrp != getpgrp(0))
    pgrp = -1;
  pid = fork();
  if (pid < 0)
    {
      (void) sigsetmask(mask);
      return -1;
    }
  if (pid == 0)
    {
      (void) sigsetmask(0);	/* Interrupts on. */
      /* Transfer control terminal. */
      if (pgrp >= 0)
	{
	  i = getpid();
	  (void) ioctl(0, TIOCSPGRP, (char *)&i);
	  (void) setpgrp(i, i);
	}
      for(i = 2; i < 30; i++)
	(void) close(i);	/* Close all but standard in and out.*/
      (void) dup2(1, 2);	/* Make standard error as standard out. */
      if (p == 0 || *p == 0)
	{
	  p = getenv("SHELL");
	  if (p)
	    execl(p, p, 0);
	  execl("/bin/sh", "sh", 0);
	}
      else
	execl("/bin/sh", "sh", "-c", p, 0);
      _exit(1);
    }
  /* Wait for child termination. */
  for(;;)
    {
      i = wait3(&w, WUNTRACED, (struct rusage *)0);
      if (i == pid)
	{
	  if (WIFSTOPPED(w))
	    {
	      /* Stop outselves, if child stops. */
	      (void) kill(getpid(), SIGSTOP);
	      /* Restore the control terminal, and restart subprocess. */
	      if (pgrp >= 0)
		(void) ioctl(0, TIOCSPGRP, (char *)&pid);
	      (void) killpg(pid, SIGCONT);
	    }
	  else
	    break;
	}
    }
  /* Get the control terminal back. */
  if (pgrp >= 0)
    (void) ioctl(0, TIOCSPGRP, (char *)&pgrp);
  (void) sigsetmask(mask);	/* Interrupts on. */
  return 0;
}
#endif

#ifdef USG
unsigned short getuid();
#else
#ifndef SECURE
#ifdef BSD4_3
uid_t getuid();
#else  /* other BSD versions */
int getuid();
#endif
#endif
#endif

/* Find a default user name from the system. */
void user_name(buf)
char *buf;
{
  extern char *getlogin();
  char pwline[256];
  register char *p;

#ifdef AIXV3
  char *index(char *, char);
#else
  char *index();
#endif

  p = getlogin();
  if (p && p[0])
    (void) strcpy(buf, p);
  else
    {
      (void) getpw((int)getuid(), pwline);
      p = index(pwline, ':');
      if (p)
	*p = 0;
      (void) strcpy(buf, pwline);
    }
  if (!buf[0])
    (void) strcpy(buf, "X");	/* Gotta have some name */
}

/* expands a tilde at the beginning of a file name to a users home directory */
int tilde(file, exp)
char *file, *exp;
{
  *exp = '\0';
  if (file)
    {
      if (*file == '~')
	{
	  char user[128];
	  struct passwd *pw = NULL;
	  int i = 0;

	  user[0] = '\0';
	  file++;
	  while (*file != '/' && i < sizeof(user))
	    user[i++] = *file++;
	  user[i] = '\0';
	  if (i == 0)
	    {
	      char *login = (char *) getlogin();

	      if (login != NULL)
		(void) strcpy (user, login);
	      else if ((pw = getpwuid(getuid())) == NULL)
		return 0;
	    }
	  if (pw == NULL && (pw = getpwnam(user)) == NULL)
	    return 0;
	  (void) strcpy (exp, pw->pw_dir);
	}
      (void) strcat(exp, file);
      return 1;
    }
  return 0;
}

/* undefine these so that tfopen and topen will work */
#undef fopen
#undef open

/* open a file just as does fopen, but allow a leading ~ to specify a home
   directory */
FILE *tfopen(file, mode)
char *file;
char *mode;
{
  char buf[1024];
  extern int errno;

  if (tilde(file, buf))
    return (fopen(buf, mode));
  errno = ENOENT;
  return NULL;
}

/* open a file just as does open, but expand a leading ~ into a home directory
   name */
int topen(file, flags, mode)
char *file;
int flags, mode;
{
  char buf[1024];
  extern int errno;

  if (tilde(file, buf))
    return (open(buf, flags, mode));
  errno = ENOENT;
  return -1;
}

#endif
