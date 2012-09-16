#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/extensions/Xinerama.h>
#include <X11/cursorfont.h>
#include <glib.h>

#include "debug.h"
#include "grab.h"
#include "utils.h"
#include "handlers.h"


int Setup(Display *display, Cursor *cursor, ScreenInfo **screenInfo, int *screenCount, int *currentScreenNumber)
{
    signal(SIGCHLD, CatchExitStatus);

    if(XineramaIsActive(display))
    {
        DLOG("xinerma is on");
        XineramaScreenInfo *xineramaScreenInfo;
        xineramaScreenInfo = XineramaQueryScreens(display, screenCount);
        *screenInfo = malloc(sizeof(ScreenInfo) * (*screenCount));
        for(int i = 0; i < *screenCount; ++i)
        {
            (*screenInfo)[i].screenNumber = xineramaScreenInfo[i].screen_number;
            (*screenInfo)[i].x = xineramaScreenInfo[i].x_org;
            (*screenInfo)[i].y = xineramaScreenInfo[i].y_org;
            (*screenInfo)[i].width = xineramaScreenInfo[i].width;
            (*screenInfo)[i].height = xineramaScreenInfo[i].height;
            DLOG("screen number %d; x %d;  y %d; w %d; h %d;", xineramaScreenInfo[i].screen_number,
                 xineramaScreenInfo[i].x_org, xineramaScreenInfo[i].y_org, xineramaScreenInfo[i].width, xineramaScreenInfo[i].height);
        }
        XFree(xineramaScreenInfo);
    }
    else
    {
        DLOG("xinerma is off");
        *screenCount = 1;
        *screenInfo = malloc(sizeof(ScreenInfo));
        (*screenInfo)->screenNumber = XDefaultScreen(display);
        Screen *screen = XScreenOfDisplay(display, (*screenInfo)->screenNumber);
        (*screenInfo)->x = 0;
        (*screenInfo)->y = 0;
        (*screenInfo)->width = XWidthOfScreen(screen);
        (*screenInfo)->height = XHeightOfScreen(screen);
    }

    *currentScreenNumber = (*screenInfo)->screenNumber;

    for(int i = 0; i < *screenCount; ++i)
    {
        (*screenInfo)[i].currentTag = 1;
    }

    if(GrabKeys(display))
    {
        DLOG("failed to grabKeys.");
        return 1;
    }

    if(GrabButtons(display))
    {
        DLOG("failed to grabButtons.");
        return 1;
    }

    Window root = XDefaultRootWindow(display);
    cursor[NormalCursor] = XCreateFontCursor(display, XC_left_ptr);
    cursor[ResizeCursor] = XCreateFontCursor(display, XC_sizing);
    cursor[MoveCursor] = XCreateFontCursor(display, XC_fleur);
    XDefineCursor(display, root, cursor[NormalCursor]);

    XSetWindowAttributes rootAttributes;
    rootAttributes.event_mask = SubstructureNotifyMask | SubstructureRedirectMask | PointerMotionMask;
    XChangeWindowAttributes(display, root, CWEventMask, &rootAttributes);

    XSelectInput(display, root, rootAttributes.event_mask);

    return 0;
}

void Run(Display *display, Cursor *cursors, GSList **windows, ScreenInfo **screenInfo, int *screenCount, int *currentScreenNumber)
{
    XEvent xEvent;
    MouseGrabInfo mouseGrabInfo = {.isActive = 0};

    XSync(display, False);

    Bool quit = False;
    while(!quit && !XNextEvent(display, &xEvent))
    {
        switch(xEvent.type)
        {
            case MappingNotify:
                DLOG("mappingnotify event");
                MappingNotifyHandler(display, &xEvent);
                break;
            case KeyPress:
                DLOG("keypress event");
                if(KeyPressHandler(display, &xEvent, *windows, *screenInfo, screenCount))
                {
                    quit = True;
                }
                break;
            case KeyRelease:
                DLOG("keyrelease event");
                break;
            case ButtonPress:
                DLOG("pointer button press");
                MousePressHandler(display, &xEvent, &mouseGrabInfo, cursors);
                break;
            case ButtonRelease:
                DLOG("pointer button release");
                MouseReleaseHandler(display, &xEvent, &mouseGrabInfo, cursors);
                break;
            case MotionNotify:
                DLOG("motion notify");
                MouseMotionHandler(display, &xEvent, &mouseGrabInfo, *screenInfo, *screenCount, *windows, currentScreenNumber);
                break;
            case MapNotify:
                DLOG("map notify");
                break;
            case UnmapNotify:
                DLOG("unmap notify");
                break;
            case CirculateNotify:
                DLOG("circulate notify");
                break;
            case ConfigureNotify:
                DLOG("configure notify");
                break;
            case CreateNotify:
                DLOG("create notify");
                break;
            case DestroyNotify:
                DLOG("destroy notify");
                DestroyNotifyHandler(display, &xEvent, windows, *screenInfo, *screenCount);
                break;
            case GravityNotify:
                DLOG("gravity notify");
                break;
            case ReparentNotify:
                DLOG("reparent notify");
                break;
            case CirculateRequest:
                DLOG("circulate request");
                break;
            case ConfigureRequest:
                DLOG("configure request");
                ConfigureRequestHandler(display, &xEvent);
                break;
            case MapRequest:
                DLOG("map request");
                MapRequestHandler(display, &xEvent, windows, *screenInfo, *screenCount, *currentScreenNumber);
                break;
            default:
                DLOG("event type %d not handled", xEvent.type);
        }

        GSList *windowIterator = *windows;
        while(windowIterator)
        {
            Client *client = (Client *) windowIterator->data;
            DLOG("window screen: %d; tag: %d", client->screenNumber, client->tag);

            windowIterator = windowIterator->next;
        }

        DLOG("windows list size %d", g_slist_length(*windows));
    }
}

void Cleanup(Display *display, Cursor *cursor)
{
    XFreeCursor(display, cursor[NormalCursor]);
    XFreeCursor(display, cursor[ResizeCursor]);
    XFreeCursor(display, cursor[MoveCursor]);
}

int main()
{
    Display *display = XOpenDisplay(NULL);
    Cursor cursors[CursorsCount];

    GSList *windows = NULL;
    (void)windows;

    if(!display)
    {
        DLOG("failed to open display.");

        return EXIT_FAILURE;
    }

    ScreenInfo *screenInfo = NULL;
    int screenCount = 0;
    int currentScreenNumber = 0;
    if(Setup(display, cursors, &screenInfo, &screenCount, &currentScreenNumber))
    {
        DLOG("setup failed.");
        return EXIT_FAILURE;
    }

    Run(display, cursors, &windows, &screenInfo, &screenCount, &currentScreenNumber);

    Cleanup(display, cursors);

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
