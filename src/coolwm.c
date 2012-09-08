#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debug.h"

Bool grabKeys(Display *display);

void mappingNotifyHandler(Display *display, XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if(ev->request == MappingKeyboard)
        grabKeys(display);
}

unsigned int numlockMask(Display *display)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	unsigned int numlockmask = 0;
	modmap = XGetModifierMapping(display);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++)
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
			   == XKeysymToKeycode(display, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);

    return numlockmask;
}

Bool grabKeys(Display *display)
{
    Window root = XDefaultRootWindow(display);

    XUngrabKey(display, AnyKey, AnyModifier, root);

    KeyCode keycode = XKeysymToKeycode(display, XK_r);
    if(!keycode)
    {
        return 1;
    }

    unsigned int numlockmask = numlockMask(display);
    unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
    int modifiers_count = 4;
    for(int i = 0; i < modifiers_count; ++i)
    {
        XGrabKey(display, keycode, Mod4Mask | modifiers[i], root,
                 True, GrabModeAsync, GrabModeAsync);
    }

    XFlush(display);

    return 0;
}

void spawn(char* cmd[]);

void keypressHandler(XEvent *xevent)
{
    char* cmd[] = {"dmenu_run", NULL};
    spawn(cmd);
}

void run(Display *display)
{
    XEvent xevent;

    XSync(display, False);
    while(!XNextEvent(display, &xevent))
    {
        switch(xevent.type)
        {
            case MappingNotify:
                DLOG("mappingnotify event");
                mappingNotifyHandler(display, &xevent);
                break;
            case KeyPress:
                DLOG("keypress event");
                keypressHandler(&xevent);
                break;
            case KeyRelease:
                DLOG("keyrelease event");
                break;
            default:
                DLOG("event type %d not handled", xevent.type);
        }
    }
}

void catch_exit_status(int sig)
{
    while(0 < waitpid(-1, NULL, WNOHANG));
}

void spawn(char* cmd[])
{
    signal(SIGCHLD, catch_exit_status); //this should be run only once
    if(fork() == 0)
    {
        setsid();
        execvp(cmd[0], cmd);
        exit(EXIT_FAILURE);
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

    if(grabKeys(display))
    {
        DLOG("failed to grabKeys.");
    }

    run(display);

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
