#ifndef UTILS_H
#define UTILS_H

#include <X11/Xlib.h>

void Spawn(char* command[]);

void CatchExitStatus(int signum);

int SendEvent(Display *display, Window window, Atom protocol);

#endif /* UTILS_H */
