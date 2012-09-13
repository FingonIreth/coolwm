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
    return *((Window *) a) == *((Window *) b) ? 0 : 1;
}


void changeTag(int *currentTag, int targetTag, GSList *windows, Display *display)
{
    *currentTag = targetTag;
    GSList *window_iterator = windows;
    while(window_iterator)
    {
        Client *client = (Client *) window_iterator->data;
        if(client->tag == targetTag)
        {
            DLOG("show window");
            XMapWindow(display, client->window);
        }
        else
        {
            DLOG("hide window");
            XUnmapWindow(display, client->window);
        }

        window_iterator = window_iterator->next;
    }
}
