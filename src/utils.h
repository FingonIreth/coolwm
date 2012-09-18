#ifndef UTILS_H
#define UTILS_H

#include <X11/Xlib.h>
#include <glib.h>

#define LENGTH(array) (sizeof(array)/sizeof(array[0]))

typedef struct
{
    Window window;
    int tag;
    int screenNumber;
} Client;

typedef struct
{
    int screenNumber;
    int x;
    int y;
    int width;
    int height;
    int currentTag;
} ScreenInfo;

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

void changeTag(ScreenInfo *screen, int targetTag, GSList *windows,
               Display *display);

int PointToScreenNumber(ScreenInfo *screenInfo, int *screenCount, int x, int y);

ScreenInfo *ScreenNumberToScreen(ScreenInfo *screenInfo, int screenCount,
                                 int screenNumber);

void TileScreen(Display *display, GSList *windows, ScreenInfo *screen, int tag);

#endif /* UTILS_H */
