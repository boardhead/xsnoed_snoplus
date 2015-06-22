#include <X11/Xlib.h>

void mozilla_remote_init_atoms (Display *dpy);
Window mozilla_remote_find_window (Display *dpy);
int mozilla_remote_obtain_lock(Display *dpy, Window window);
void mozilla_remote_free_lock (Display *dpy, Window window);
int mozilla_remote_command (Display *dpy, Window window, const char *command, Bool raise_p);
