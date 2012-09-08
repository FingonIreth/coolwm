#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/XKBlib.h>

#include "debug.h"
#include "utils.h"
#include "grab.h"

void MappingNotifyHandler(Display *display, XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if(ev->request == MappingKeyboard)
        GrabKeys(display);
}

void KeyPressHandler(Display *display, XEvent *xevent)
{
    XKeyEvent *xKeyEvent = (XKeyEvent *)xevent;

    if((xKeyEvent->state & Mod4Mask)
        && (XkbKeycodeToKeysym(display, xKeyEvent->keycode, 0, 0) == XK_r))
    {
        char* cmd[] = {"dmenu_run", NULL};
        Spawn(cmd);
    }
}

int Setup(Display *display)
{
    signal(SIGCHLD, CatchExitStatus);

    if(GrabKeys(display))
    {
        DLOG("failed to grabKeys.");
        return 1;
    }

    return 0;
}

void Run(Display *display)
{
    XEvent xevent;

    XSync(display, False);
    while(!XNextEvent(display, &xevent))
    {
        switch(xevent.type)
        {
            case MappingNotify:
                DLOG("mappingnotify event");
                MappingNotifyHandler(display, &xevent);
                break;
            case KeyPress:
                DLOG("keypress event");
                KeyPressHandler(display, &xevent);
                break;
            case KeyRelease:
                DLOG("keyrelease event");
                break;
            default:
                DLOG("event type %d not handled", xevent.type);
        }
    }
}

int main()
{
    Display *display = XOpenDisplay(NULL);

    if(!display)
    {
        DLOG("failed to open display.");

        return EXIT_FAILURE;
    }

    if(Setup(display))
    {
        DLOG("setup failed.");
        return EXIT_FAILURE;
    }

    Run(display);

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
