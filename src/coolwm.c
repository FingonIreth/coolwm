#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "debug.h"

int main()
{
    Display *display = XOpenDisplay(NULL);

    if(!display)
    {
        DLOG("failed to open display.");

        return EXIT_FAILURE;
    }
    else
    {
        DLOG("%d", XScreenCount(display));
    }

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
