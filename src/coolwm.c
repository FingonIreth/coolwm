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

typedef struct {
    int isActive;
    int mouseX;
    int mouseY;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    unsigned int dragButton;
    Window draggedWindow;
} MouseGrabInfo;

void MappingNotifyHandler(Display *display, XEvent *event) {
    XMappingEvent *mappingEvent = &event->xmapping;

    XRefreshKeyboardMapping(mappingEvent);
    if(mappingEvent->request == MappingKeyboard)
        GrabKeys(display);
}


void MouseMotionHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo)
{
    XMotionEvent *xMotionEvent = (XMotionEvent *)xEvent;
    if(!mouseGrabInfo->isActive)
    {
        return;
    }

    if(mouseGrabInfo->dragButton == Button1)
    {
        int xDifference = xMotionEvent->x_root - mouseGrabInfo->mouseX;
        int yDifference = xMotionEvent->y_root - mouseGrabInfo->mouseY;

        int newWindowX = mouseGrabInfo->windowX + xDifference;
        int newWindowY = mouseGrabInfo->windowY + yDifference;

        XMoveWindow(display, mouseGrabInfo->draggedWindow, newWindowX, newWindowY);
    }
    else if(mouseGrabInfo->dragButton == Button3)
    {
        int xDifference = xMotionEvent->x_root - mouseGrabInfo->mouseX;
        int yDifference = xMotionEvent->y_root - mouseGrabInfo->mouseY;

        int newWindowWidth = mouseGrabInfo->windowWidth + xDifference;

        const int minWidth = 10;
        const int minHeight = 10;
        if(newWindowWidth < minWidth)
        {
            newWindowWidth = minWidth;
        }


        int newWindowHeight = mouseGrabInfo->windowHeight + yDifference;
        if(newWindowHeight < minHeight)
        {
            newWindowHeight = minHeight;
        }

        XResizeWindow(display, mouseGrabInfo->draggedWindow, newWindowWidth, newWindowHeight);
    }
}

void MouseReleaseHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo)
{
    XButtonEvent *xButtonEvent = (XButtonEvent *)xEvent;

    if(!mouseGrabInfo->isActive)
    {
        return;
    }

    if(xButtonEvent->button == mouseGrabInfo->dragButton)
    {
        XUngrabPointer(display, CurrentTime);
        mouseGrabInfo->isActive = 0;
    }
}

void MousePressHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo)
{
    if(mouseGrabInfo->isActive)
    {
        return;
    }

    XButtonEvent *xButtonEvent = (XButtonEvent *)xEvent;

    if(!xButtonEvent->subwindow)
    {
        return;
    }

    if((xButtonEvent->state & Mod4Mask) && xButtonEvent->button == Button1)
    {

        XWindowAttributes xWindowAttributes;
        XGetWindowAttributes(display, xButtonEvent->subwindow, &xWindowAttributes);
        if(XGrabPointer(display, xWindowAttributes.root, False,
                        ButtonPressMask | PointerMotionMask | ButtonReleaseMask,
                        GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
        {
            return;
        }

        mouseGrabInfo->isActive = 1;
        mouseGrabInfo->dragButton = xButtonEvent->button;
        mouseGrabInfo->mouseX = xButtonEvent->x_root;
        mouseGrabInfo->mouseY = xButtonEvent->y_root;
        mouseGrabInfo->windowX = xWindowAttributes.x;
        mouseGrabInfo->windowY = xWindowAttributes.y;
        mouseGrabInfo->draggedWindow = xButtonEvent->subwindow;
    }
    else if((xButtonEvent->state & Mod4Mask) && xButtonEvent->button == Button3)
    {
        XWindowAttributes xWindowAttributes;
        XGetWindowAttributes(display, xButtonEvent->subwindow, &xWindowAttributes);

        if(XGrabPointer(display, xWindowAttributes.root, False,
                        ButtonPressMask | PointerMotionMask | ButtonReleaseMask,
                        GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
        {
            return;
        }

        mouseGrabInfo->isActive = 1;
        mouseGrabInfo->dragButton = xButtonEvent->button;
        mouseGrabInfo->mouseX = xWindowAttributes.x + xWindowAttributes.width;
        mouseGrabInfo->mouseY = xWindowAttributes.y + xWindowAttributes.height;
        mouseGrabInfo->windowWidth= xWindowAttributes.width;
        mouseGrabInfo->windowHeight = xWindowAttributes.height;
        mouseGrabInfo->draggedWindow = xButtonEvent->subwindow;

        XWarpPointer(display, None, mouseGrabInfo->draggedWindow, 0, 0, 0, 0, xWindowAttributes.width - 1, xWindowAttributes.height);
    }
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
