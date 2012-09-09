#ifndef GRAB_H
#define GRAB_H

#include <X11/Xlib.h>

unsigned int NumlockMask(Display *display);
Bool GrabKeys(Display *display);
Bool GrabButtons(Display *display);

#endif /* GRAB_H */
