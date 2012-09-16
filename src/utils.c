#include "utils.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <glib.h>

#include "debug.h"

void Spawn(char* command[])
{
    if(fork() == 0)
    {
        setsid();
        execvp(command[0], command);
        exit(EXIT_FAILURE);
    }
}

void CatchExitStatus(int signum)
{
    while(0 < waitpid(-1, NULL, WNOHANG));
}

int SendEvent(Display *display, Window window, Atom protocol)
{
    int protocolsCount;
    Atom *protocols;
    Bool foundProtocol = False;
    XEvent xEvent;

    if(XGetWMProtocols(display, window, &protocols, &protocolsCount))
    {
        int index = protocolsCount - 1;
        while(!foundProtocol && index >= 0)
        {
            foundProtocol = (protocols[index] == protocol);
            index -= 1;
        }
        XFree(protocols);
    }
    if(foundProtocol)
    {
        xEvent.type = ClientMessage;
        xEvent.xclient.window = window;
        xEvent.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = protocol;
        xEvent.xclient.data.l[1] = CurrentTime;
        XSendEvent(display, window, False, NoEventMask, &xEvent);
    }

    return !foundProtocol;
}

gint compareWindows(gconstpointer a, gconstpointer b)
{
    return ((Client*) a)->window == ((Client*) b)->window ? 0 : 1;
}


void changeTag(ScreenInfo *screen, int targetTag, GSList *windows, Display *display)
{
    screen->currentTag = targetTag;
    GSList *window_iterator = windows;
    while(window_iterator)
    {
        Client *client = (Client *) window_iterator->data;
        if(client->tag == targetTag && client->screenNumber == screen->screenNumber)
        {
            DLOG("show window");
            XMapWindow(display, client->window);
        }
        else if(client->screenNumber == screen->screenNumber)
        {
            DLOG("hide window");
            XUnmapWindow(display, client->window);
        }

        window_iterator = window_iterator->next;
    }
}

//XXX wrong and not robust
int PointToScreenNumber(ScreenInfo *screenInfo, int *screenCount, int x, int y)
{
    for(int i = 0; i < *screenCount; ++i)
    {
        if(screenInfo[i].x <= x && x <= screenInfo[i].x + screenInfo[i].width
           && screenInfo[i].y <= y && y <= screenInfo[i].y + screenInfo[i].height)
        {
            return screenInfo[i].screenNumber;
        }
    }

    return -1;
}

ScreenInfo *ScreenNumberToScreen(ScreenInfo *screenInfo, int screenCount, int screenNumber)
{
    for(int i = 0; i < screenCount; ++i)
    {
        if(screenInfo[i].screenNumber == screenNumber)
        {
            return screenInfo + i;
        }
    }

    return NULL;
}

//XXX pay attention to window's border
void TileScreen(Display *display, GSList *windows, ScreenInfo *screen, int tag)
{
    int windowCount = 0;
    GSList *windowIterator = windows;
    while(windowIterator)
    {
        Client *client = (Client *) windowIterator->data;
        if(client->screenNumber == screen->screenNumber && client->tag == tag)
        {
            windowCount += 1;
        }

        windowIterator = windowIterator->next;
    }

    windowIterator = windows;
    int windowX = 0;
    while(windowIterator)
    {
        Client *client = (Client *) windowIterator->data;
        if(client->screenNumber == screen->screenNumber && client->tag == tag)
        {
            XMoveResizeWindow(display, client->window, windowX + screen->x, screen->y + 0,
                              screen->width/windowCount, screen->height);
            windowX += screen->width/windowCount;
        }

        windowIterator = windowIterator->next;
    }

}
