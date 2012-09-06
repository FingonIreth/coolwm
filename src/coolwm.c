#include <stdio.h>
#include <X11/Xlib.h>

int main()
{
    Display *display = XOpenDisplay(NULL);


    printf("Hello world\n");


    XCloseDisplay(display);

    return 0;
}
