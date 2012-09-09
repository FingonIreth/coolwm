#ifndef UTILS_H
#define UTILS_H

#include <X11/Xlib.h>

enum
{
    NormalCursor,
    ResizeCursor,
    MoveCursor,
    CursorsCount
};

void Spawn(char* command[]);

void CatchExitStatus(int signum);

int SendEvent(Display *display, Window window, Atom protocol);

#endif /* UTILS_H */
