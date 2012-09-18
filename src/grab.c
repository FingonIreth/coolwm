#include "grab.h"
#include "utils.h"

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
			if(modifierKeymap->modifiermap[i * modifierKeymap->max_keypermod
                                           + j]
			   == XKeysymToKeycode(display, XK_Num_Lock))
            {
				numlockMask = (1 << i);
            }
        }
    }
	XFreeModifiermap(modifierKeymap);

    return numlockMask;
}

Bool GrabButtons(Display *display)
{
    Window root = XDefaultRootWindow(display);

    XUngrabButton(display, AnyButton, AnyModifier, root);

    unsigned int numlockMask = NumlockMask(display);
    unsigned int modifiers[] = {0, LockMask, numlockMask,
                                numlockMask | LockMask};
    unsigned int buttons[] = {Button1, Button3};

    for(int b = 0; b < LENGTH(buttons); ++b)
    {
        for(int i = 0; i < LENGTH(modifiers); ++i)
        {
            XGrabButton(display, buttons[b], Mod4Mask | modifiers[i], root,
                        False,
                        ButtonPressMask | ButtonReleaseMask, GrabModeAsync,
                        GrabModeAsync,
                        None, None);
        }
    }

    return 0;
}

Bool GrabKeys(Display *display)
{
    Window root = XDefaultRootWindow(display);

    XUngrabKey(display, AnyKey, AnyModifier, root);

    KeyCode keycode[] = {XKeysymToKeycode(display, XK_Return),
                         XKeysymToKeycode(display, XK_r),
                         XKeysymToKeycode(display, XK_c),
                         XKeysymToKeycode(display, XK_q),
                         XKeysymToKeycode(display, XK_1),
                         XKeysymToKeycode(display, XK_2),
                         XKeysymToKeycode(display, XK_3),
                         XKeysymToKeycode(display, XK_4),
                         XKeysymToKeycode(display, XK_5)
                         };

    for(int k = 0; k < LENGTH(keycode); k++)
    {
        if(!keycode[k])
        {
            return 1;
        }

        unsigned int numlockMask = NumlockMask(display);
        unsigned int modifiers[] = {0, LockMask, numlockMask,
                                    numlockMask | LockMask};
        for(int i = 0; i < LENGTH(modifiers); ++i)
        {
            XGrabKey(display, keycode[k], Mod4Mask | modifiers[i], root,
                     True, GrabModeAsync, GrabModeAsync);
        }
    }

    XFlush(display);

    return 0;
}
