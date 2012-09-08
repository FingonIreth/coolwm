#include "grab.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

unsigned int NumlockMask(Display *display)
{
	XModifierKeymap *modifierKeymap;
	modifierKeymap = XGetModifierMapping(display);

	unsigned int numlockMask = 0;
	for(int i = 0; i < 8; i++)
    {
		for(int j = 0; j < modifierKeymap->max_keypermod; j++)
        {
			if(modifierKeymap->modifiermap[i * modifierKeymap->max_keypermod + j]
			   == XKeysymToKeycode(display, XK_Num_Lock))
            {
				numlockMask = (1 << i);
            }
        }
    }
	XFreeModifiermap(modifierKeymap);

    return numlockMask;
}

Bool GrabKeys(Display *display)
{
    Window root = XDefaultRootWindow(display);

    XUngrabKey(display, AnyKey, AnyModifier, root);

    KeyCode keycode[] = {XKeysymToKeycode(display, XK_r), XKeysymToKeycode(display, XK_c)};

    for(int k = 0; k < 2; k++)
    {
        if(!keycode[k])
        {
            return 1;
        }

        unsigned int numlockMask = NumlockMask(display);
        unsigned int modifiers[] = {0, LockMask, numlockMask, numlockMask | LockMask};
        int modifiersCount = 4;
        for(int i = 0; i < modifiersCount; ++i)
        {
            XGrabKey(display, keycode[k], Mod4Mask | modifiers[i], root,
                     True, GrabModeAsync, GrabModeAsync);
        }
    }

    XFlush(display);

    return 0;
}
