// ===============================================================================
// netscape junk -- borrowed from http://home.netscape.com/newsref/std/remote.c
//
#ifndef NO_HELP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>	/* for XmuClientWindow() */
#include "mozilla.h"
#include "CUtils.h"

static const char *expected_mozilla_version = "1.1";

#define MOZILLA_VERSION_PROP   "_MOZILLA_VERSION"
#define MOZILLA_LOCK_PROP      "_MOZILLA_LOCK"
#define MOZILLA_COMMAND_PROP   "_MOZILLA_COMMAND"
#define MOZILLA_RESPONSE_PROP  "_MOZILLA_RESPONSE"
static Atom XA_MOZILLA_VERSION  = 0;
static Atom XA_MOZILLA_LOCK     = 0;
static Atom XA_MOZILLA_COMMAND  = 0;
static Atom XA_MOZILLA_RESPONSE = 0;

void mozilla_remote_init_atoms (Display *dpy)
{
  if (! XA_MOZILLA_VERSION)
    XA_MOZILLA_VERSION = XInternAtom (dpy, MOZILLA_VERSION_PROP, False);
  if (! XA_MOZILLA_LOCK)
    XA_MOZILLA_LOCK = XInternAtom (dpy, MOZILLA_LOCK_PROP, False);
  if (! XA_MOZILLA_COMMAND)
    XA_MOZILLA_COMMAND = XInternAtom (dpy, MOZILLA_COMMAND_PROP, False);
  if (! XA_MOZILLA_RESPONSE)
    XA_MOZILLA_RESPONSE = XInternAtom (dpy, MOZILLA_RESPONSE_PROP, False);
}

Window mozilla_remote_find_window (Display *dpy)
{
  int i;
  Window root = RootWindowOfScreen (DefaultScreenOfDisplay (dpy));
  Window root2, parent, *kids;
  unsigned int nkids;
  Window result = 0;
  Window tenative = 0;
  unsigned char *tenative_version = 0;

  if (! XQueryTree (dpy, root, &root2, &parent, &kids, &nkids))
    {
      return((Window)NULL);
    }

  /* root != root2 is possible with virtual root WMs. */

  if (! (kids && nkids))
    {
      return((Window)NULL);
    }

  for (i = nkids-1; i >= 0; i--)
    {
      Atom type;
      int format;
      unsigned long nitems, bytesafter;
      unsigned char *version = 0;
      Window w = XmuClientWindow (dpy, kids[i]);
      int status = XGetWindowProperty (dpy, w, XA_MOZILLA_VERSION,
				       0, (65536 / sizeof (long)),
				       False, XA_STRING,
				       &type, &format, &nitems, &bytesafter,
				       &version);
      if (! version)
	continue;
      if (strcmp ((char *) version, expected_mozilla_version) &&
	  !tenative)
	{
	  tenative = w;
	  tenative_version = version;
	  continue;
	}
      XFree (version);
      if (status == Success && type != None)
	{
	  result = w;
	  break;
	}
    }

  // don't really care about version, just return what you found - PH
  if (tenative) {
  	if (!result) result = tenative;
  	XFree(tenative_version);
  }
  return(result);
}

static char *lock_data = 0;

int mozilla_remote_obtain_lock(Display *dpy, Window window)
{
  Bool locked = False;
  Bool waited = False;

  if (! lock_data)
    {
      lock_data = (char *) malloc (255);
      sprintf (lock_data, "pid%d@", getpid ());
      if (gethostname (lock_data + strlen (lock_data), 100))
	{
	  return(-1);
	}
    }

  do
    {
      int result;
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytes_after;
      unsigned char *data = 0;

      XGrabServer (dpy);   /* ################################# DANGER! */

      result = XGetWindowProperty (dpy, window, XA_MOZILLA_LOCK,
				   0, (65536 / sizeof (long)),
				   False, /* don't delete */
				   XA_STRING,
				   &actual_type, &actual_format,
				   &nitems, &bytes_after,
				   &data);
      if (result != Success || actual_type == None)
	{
	  /* It's not now locked - lock it. */
#ifdef DEBUG_PROPS
	  Printf ("(writing " MOZILLA_LOCK_PROP
		   " \"%s\" to 0x%x)\n",
		   lock_data, (unsigned int) window);
#endif
	  XChangeProperty (dpy, window, XA_MOZILLA_LOCK, XA_STRING, 8,
			   PropModeReplace, (unsigned char *) lock_data,
			   strlen (lock_data));
	  locked = True;
	}

      XUngrabServer (dpy); /* ################################# danger over */
      XSync (dpy, False);

      if (! locked)
	{
	  /* We tried to grab the lock this time, and failed because someone
	     else is holding it already.  So, wait for a PropertyDelete event
	     to come in, and try again. */

	  Printf("Netscape window is locked by %s; waiting...\n",data);
	  waited = True;

	  while (1)
	    {
	      XEvent event;
	      XNextEvent (dpy, &event);
	      if (event.xany.type == DestroyNotify &&
		  event.xdestroywindow.window == window)
		{
		  return(-1);
		}
	      else if (event.xany.type == PropertyNotify &&
		       event.xproperty.state == PropertyDelete &&
		       event.xproperty.window == window &&
		       event.xproperty.atom == XA_MOZILLA_LOCK)
		{
		  /* Ok!  Someone deleted their lock, so now we can try
		     again. */
#ifdef DEBUG_PROPS
		  Printf("(0x%x unlocked, trying again...)\n", (unsigned int) window);
#endif
		  break;
		}
	    }
	}
      if (data)
	XFree (data);
    }
  while (! locked);

  if (waited)
    Printf ("obtained lock.\n");
  return(0);
}


void mozilla_remote_free_lock (Display *dpy, Window window)
{
  int result;
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *data = 0;

#ifdef DEBUG_PROPS
	  Printf ("(deleting " MOZILLA_LOCK_PROP
		   " \"%s\" from 0x%x)\n",
		   lock_data, (unsigned int) window);
#endif

  result = XGetWindowProperty (dpy, window, XA_MOZILLA_LOCK,
			       0, (65536 / sizeof (long)),
			       True, /* atomic delete after */
			       XA_STRING,
			       &actual_type, &actual_format,
			       &nitems, &bytes_after,
			       &data);
  if (result != Success)
    {
      Printf ("unable to read and delete " MOZILLA_LOCK_PROP
	       " property\n");
      return;
    }
  else if (!data || !*data)
    {
      Printf ("invalid data on " MOZILLA_LOCK_PROP
	       " of window 0x%x.\n",
	       (unsigned int) window);
      return;
    }
  else if (strcmp ((char *) data, lock_data))
    {
      Printf (MOZILLA_LOCK_PROP
	       " was stolen!  Expected \"%s\", saw \"%s\"!\n",
	       lock_data, data);
      return;
    }

  if (data)
    XFree (data);
}


int mozilla_remote_command (Display *dpy, Window window, const char *command,
			Bool raise_p)
{
  int result=0;
  Bool done = False;
  char *new_command = 0;

  /* The -noraise option is implemented by passing a "noraise" argument
     to each command to which it should apply.
   */
  if (! raise_p)
    {
      char *close;
      new_command = (char *) malloc (strlen (command) + 20);
      strcpy (new_command, command);
      close = strrchr (new_command, ')');
      if (close)
	strcpy (close, ", noraise)");
      else
	strcat (new_command, "(noraise)");
      command = new_command;
    }

#ifdef DEBUG_PROPS
  Printf ("(writing " MOZILLA_COMMAND_PROP " \"%s\" to 0x%x)\n",
	   command, (unsigned int) window);
#endif

  XChangeProperty (dpy, window, XA_MOZILLA_COMMAND, XA_STRING, 8,
		   PropModeReplace, (unsigned char *) command,
		   strlen (command));

  while (!done)
    {
      XEvent event;
      XNextEvent (dpy, &event);
      if (event.xany.type == DestroyNotify &&
	  event.xdestroywindow.window == window)
	{
	  /* Print to warn user...*/
	  Printf ("window 0x%x was destroyed.\n", (unsigned int) window);
	  result = 6;
	  goto DONE;
	}
      else if (event.xany.type == PropertyNotify &&
	       event.xproperty.state == PropertyNewValue &&
	       event.xproperty.window == window &&
	       event.xproperty.atom == XA_MOZILLA_RESPONSE)
	{
	  Atom actual_type;
	  int actual_format;
	  unsigned long nitems, bytes_after;
	  unsigned char *data = 0;

	  result = XGetWindowProperty (dpy, window, XA_MOZILLA_RESPONSE,
				       0, (65536 / sizeof (long)),
				       True, /* atomic delete after */
				       XA_STRING,
				       &actual_type, &actual_format,
				       &nitems, &bytes_after,
				       &data);
#ifdef DEBUG_PROPS
	  if (result == Success && data && *data)
	    {
	      Printf("(server sent " MOZILLA_RESPONSE_PROP
		       " \"%s\" to 0x%x.)\n",
		       data, (unsigned int) window);
	    }
#endif

	  if (result != Success)
	    {
	      Printf("failed reading " MOZILLA_RESPONSE_PROP
		       " from window 0x%0x.\n",
		       (unsigned int) window);
	      result = 6;
	      done = True;
	    }
	  else if (!data || strlen((char *) data) < 5)
	    {
	      Printf("invalid data on " MOZILLA_RESPONSE_PROP
		       " property of window 0x%0x.\n",
		       (unsigned int) window);
	      result = 6;
	      done = True;
	    }
	  else if (*data == '1')	/* positive preliminary reply */
	    {
	      Printf("%s\n", data + 4);
	      /* keep going */
	      done = False;
	    }
#if 1
	  else if (!strncmp ((char *)data, "200", 3)) /* positive completion */
	    {
	      result = 0;
	      done = True;
	    }
#endif
	  else if (*data == '2')		/* positive completion */
	    {
	      Printf("%s\n", data + 4);
	      result = 0;
	      done = True;
	    }
	  else if (*data == '3')	/* positive intermediate reply */
	    {
	      Printf("internal error: "
		       "server wants more information?  (%s)\n", data);
	      result = 3;
	      done = True;
	    }
	  else if (*data == '4' ||	/* transient negative completion */
		   *data == '5')	/* permanent negative completion */
	    {
	      Printf("%s\n", data + 4);
	      result = (*data - '0');
	      done = True;
	    }
	  else
	    {
	      Printf("unrecognised " MOZILLA_RESPONSE_PROP
		       " from window 0x%x: %s\n",
		       (unsigned int) window, data);
	      result = 6;
	      done = True;
	    }

	  if (data)
	    XFree (data);
	}
#ifdef DEBUG_PROPS
      else if (event.xany.type == PropertyNotify &&
	       event.xproperty.window == window &&
	       event.xproperty.state == PropertyDelete &&
	       event.xproperty.atom == XA_MOZILLA_COMMAND)
	{
	  Printf("(server 0x%x has accepted "
		   MOZILLA_COMMAND_PROP ".)\n",
		   (unsigned int) window);
	}
#endif /* DEBUG_PROPS */
    }

 DONE:

  if (new_command)
    free (new_command);

  return result;
}



#endif // NO_HELP
