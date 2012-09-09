#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/extensions/Xinerama.h>
#include <X11/cursorfont.h>

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
    *cursor = XCreateFontCursor(display, XC_left_ptr);
    XDefineCursor(display, root, *cursor);

    return 0;
}

void Run(Display *display)
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
                if(KeyPressHandler(display, &xEvent))
                {
                    quit = True;
                }
                break;
            case KeyRelease:
                DLOG("keyrelease event");
                break;
            case ButtonPress:
                DLOG("pointer button press");
                MousePressHandler(display, &xEvent, &mouseGrabInfo);
                break;
            case ButtonRelease:
                DLOG("pointer button release");
                MouseReleaseHandler(display, &xEvent, &mouseGrabInfo);
                break;
            case MotionNotify:
                DLOG("motion notify");
                MouseMotionHandler(display, &xEvent, &mouseGrabInfo);
                break;
            default:
                DLOG("event type %d not handled", xEvent.type);
        }
    }
}

int main()
{
    Display *display = XOpenDisplay(NULL);
    Cursor cursor;

    if(!display)
    {
        DLOG("failed to open display.");

        return EXIT_FAILURE;
    }

    if(Setup(display, &cursor))
    {
        DLOG("setup failed.");
        return EXIT_FAILURE;
    }

    Run(display);

    XFreeCursor(display, cursor);
    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
