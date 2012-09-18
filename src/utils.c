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
        xEvent.xclient.message_type =
                                     XInternAtom(display, "WM_PROTOCOLS", True);
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


void changeTag(ScreenInfo *screen, int targetTag, GSList *windows,
               Display *display)
{
    screen->currentTag = targetTag;
    GSList *window_iterator = windows;
    while(window_iterator)
    {
        Client *client = (Client *) window_iterator->data;
        if(client->tag == targetTag
           && client->screenNumber == screen->screenNumber)
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

int SquaredPointToRectangleDistance(int x, int y, int topLeftX, int topLeftY, int width, int height)
{
    int xDistance = 0;
    if(x < topLeftX)
    {
        xDistance = topLeftX - x;
    }
    else if(x > topLeftX + width - 1)
    {
        xDistance = x - topLeftX - width + 1;
    }

    int yDistance = 0;
    if(y < topLeftY)
    {
        yDistance = topLeftY - y;
    }
    else if(y > topLeftX + height - 1)
    {
        yDistance = y - topLeftY - height + 1;
    }

    return xDistance * xDistance + yDistance * yDistance;
}

int PointToScreenNumber(ScreenInfo *screenInfo, int *screenCount, int x, int y)
{
    int screenNumber = -1;
    int shortestDistance = -1;
    for(int i = 0; i < *screenCount; ++i)
    {
        int distance = SquaredPointToRectangleDistance(x, y, screenInfo[i].x,
                                                       screenInfo[i].y,
                                                       screenInfo[i].width,
                                                       screenInfo[i].height);

        if(shortestDistance ==  -1 || distance < shortestDistance)
        {
            screenNumber = screenInfo[i].screenNumber;
            shortestDistance = distance;
        }

    }

    return screenNumber;
}


ScreenInfo *ScreenNumberToScreen(ScreenInfo *screenInfo, int screenCount,
                                 int screenNumber)
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
            XMoveResizeWindow(display, client->window, windowX + screen->x,
                                screen->y + 0, screen->width/windowCount,
                                screen->height);
            windowX += screen->width/windowCount;
        }

        windowIterator = windowIterator->next;
    }

}
