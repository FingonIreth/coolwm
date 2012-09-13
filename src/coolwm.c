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


int Setup(Display *display, Cursor *cursor)
{
    signal(SIGCHLD, CatchExitStatus);

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
    rootAttributes.event_mask = SubstructureNotifyMask | SubstructureRedirectMask;
    XChangeWindowAttributes(display, root, CWEventMask, &rootAttributes);

    XSelectInput(display, root, rootAttributes.event_mask);

    return 0;
}

void Run(Display *display, Cursor *cursors, GSList **windows, int *currentTag)
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
                if(KeyPressHandler(display, &xEvent, currentTag, *windows))
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
                MouseMotionHandler(display, &xEvent, &mouseGrabInfo);
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
                DestroyNotifyHandler(display, &xEvent, windows, currentTag);
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
                MapRequestHandler(display, &xEvent, windows, currentTag);
                break;
            default:
                DLOG("event type %d not handled", xEvent.type);
        }

        DLOG("current tag %d", *currentTag);
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
    int currentTag = 1;

    GSList *windows = NULL;
    (void)windows;

    if(!display)
    {
        DLOG("failed to open display.");

        return EXIT_FAILURE;
    }

    if(Setup(display, cursors))
    {
        DLOG("setup failed.");
        return EXIT_FAILURE;
    }

    Run(display, cursors, &windows, &currentTag);

    Cleanup(display, cursors);

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
