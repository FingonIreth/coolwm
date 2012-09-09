#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include "handlers.h"
#include "grab.h"
#include "utils.h"

void MappingNotifyHandler(Display *display, XEvent *event)
{
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

void MouseReleaseHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo,
                         Cursor *cursors)
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

void MousePressHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo,
                       Cursor *cursors)
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
                        GrabModeAsync, GrabModeAsync, None, cursors[MoveCursor],
                        CurrentTime) != GrabSuccess)
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
                        GrabModeAsync, GrabModeAsync, None, cursors[ResizeCursor],
                        CurrentTime) != GrabSuccess)
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
