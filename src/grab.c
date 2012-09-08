#include "grab.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

unsigned int NumlockMask(Display *display)
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

Bool GrabKeys(Display *display)
{
    Window root = XDefaultRootWindow(display);

    XUngrabKey(display, AnyKey, AnyModifier, root);

    KeyCode keycode = XKeysymToKeycode(display, XK_r);
    if(!keycode)
    {
        return 1;
    }

    unsigned int numlockmask = NumlockMask(display);
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
