#ifndef UTILS_H
#define UTILS_H

#include <X11/Xlib.h>
#include <glib.h>

#define LENGTH(array) (sizeof(array)/sizeof(array[0]))

typedef struct
{
    Window window;
    int tag;

} Client;

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

gint compareWindows(gconstpointer a, gconstpointer b);

void changeTag(int *currentTag, int targetTag, GSList *windows, Display *display);


#endif /* UTILS_H */
