#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xinerama.h>
#include <X11/cursorfont.h>

#include "debug.h"
#include "utils.h"
#include "grab.h"

void MappingNotifyHandler(Display *display, XEvent *event) {
    XMappingEvent *mappingEvent = &event->xmapping;

    XRefreshKeyboardMapping(mappingEvent);
    if(mappingEvent->request == MappingKeyboard)
        GrabKeys(display);
}


int KeyPressHandler(Display *display, XEvent *xEvent)
{
    XKeyEvent *xKeyEvent = (XKeyEvent *)xEvent;

    if((xKeyEvent->state & Mod4Mask) //XXX bug inviting code
                                     //no difference between (Mod4Mask | ShiftMask) + r
                                     //and Mod4Mask + r
        && (XkbKeycodeToKeysym(display, xKeyEvent->keycode, 0, 0) == XK_r))
    {
        char* command[] = {"dmenu_run", NULL};
        Spawn(command);
    }
    else if((xKeyEvent->state & Mod4Mask) ///XXX like above
            && (XkbKeycodeToKeysym(display, xKeyEvent->keycode, 0, 0) == XK_c))
    {
        if(xKeyEvent->subwindow)
        {
            if(SendEvent(display, xKeyEvent->subwindow, XInternAtom(display, "WM_DELETE_WINDOW", True)))
            {
                XKillClient(display, xKeyEvent->subwindow);
            }
        }
    }
    else if((xKeyEvent->state & Mod4Mask) ///XXX like above
            && (XkbKeycodeToKeysym(display, xKeyEvent->keycode, 0, 0) == XK_q))
    {
        return 1;
    }

    return 0;
}

int Setup(Display *display, Cursor *cursor)
{
    signal(SIGCHLD, CatchExitStatus);

    if(GrabKeys(display))
    {
        DLOG("failed to grabKeys.");
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
