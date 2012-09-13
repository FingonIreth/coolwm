#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <stdlib.h>

#include "handlers.h"
#include "grab.h"
#include "utils.h"
#include "debug.h"

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

int KeyPressHandler(Display *display, XEvent *xEvent, int *currentTag, GSList *windows)
{
    XKeyEvent *xKeyEvent = (XKeyEvent *)xEvent;

    if(xKeyEvent->state & Mod4Mask)
    {
        KeySym keySym = XkbKeycodeToKeysym(display, xKeyEvent->keycode, 0, 0);
        if(keySym == XK_r)
        {
            char* command[] = {"dmenu_run", NULL};
            Spawn(command);
        }
        else if (keySym == XK_c)
        {
            if(xKeyEvent->subwindow)
            {
                if(SendEvent(display, xKeyEvent->subwindow,
                             XInternAtom(display, "WM_DELETE_WINDOW", True)))
                {
                    XKillClient(display, xKeyEvent->subwindow);
                }
            }
        }
        else if (keySym == XK_q)
        {
            return 1;
        }
        else if (keySym == XK_1)
        {
            changeTag(currentTag, 1, windows, display);
        }
        else if (keySym == XK_2)
        {
            changeTag(currentTag, 2, windows, display);
        }
        else if (keySym == XK_3)
        {
            changeTag(currentTag, 3, windows, display);
        }
        else if (keySym == XK_4)
        {
            changeTag(currentTag, 4, windows, display);
        }
        else if (keySym == XK_5)
        {
            changeTag(currentTag, 5, windows, display);
        }
    }

    return 0;
}

void ConfigureRequestHandler(Display *display, XEvent *xEvent)
{
    XConfigureRequestEvent *xConfigureReuqestEvent = (XConfigureRequestEvent *)xEvent;

    XWindowChanges xWindowChanges;
    xWindowChanges.x = xConfigureReuqestEvent->x;
    xWindowChanges.y = xConfigureReuqestEvent->y;
    xWindowChanges.width = xConfigureReuqestEvent->width;
    xWindowChanges.height= xConfigureReuqestEvent->height;
    xWindowChanges.border_width = xConfigureReuqestEvent->border_width;
    xWindowChanges.sibling = xConfigureReuqestEvent->above;
    xWindowChanges.stack_mode = xConfigureReuqestEvent->detail;

    XConfigureWindow(display, xConfigureReuqestEvent->window,
                     xConfigureReuqestEvent->value_mask, &xWindowChanges);
}

void MapRequestHandler(Display *display, XEvent *xEvent, GSList **windows, int *currentTag)
{
    XMapRequestEvent *xMapRequestEvent = (XMapRequestEvent *)xEvent;
    XWindowAttributes xWindowAttributes;

    if(!XGetWindowAttributes(display, xMapRequestEvent->window, &xWindowAttributes))
    {
        return;
    }

    if(xWindowAttributes.override_redirect)
    {
        return;
    }

    Client *client= (Client *)malloc(sizeof(xMapRequestEvent->window));
    client->window= xMapRequestEvent->window;
    client->tag = *currentTag;
    Window root = XDefaultRootWindow(display);
    if(xMapRequestEvent->parent == root && !g_slist_find_custom(*windows, client, compareWindows))
    {
        *windows = g_slist_prepend(*windows, (gpointer)client);
    }

    XMapWindow(display, xMapRequestEvent->window);
}

void DestroyNotifyHandler(Display *display, XEvent *xEvent, GSList **windows, int *currentTag)
{
    XDestroyWindowEvent *xDestroyWindowEvent = (XDestroyWindowEvent *)xEvent;
    GSList *client_node = g_slist_find_custom(*windows, &xDestroyWindowEvent->window, compareWindows);
    //XXX special treatment when unmap is caused by iconify/changing tag?
    //Change state to withdrawnstate or something
    if(!client_node)
    {
        return;
    }

    Client *client = client_node->data;
    *windows = g_slist_remove(*windows, client_node->data);
    free(client);
}
