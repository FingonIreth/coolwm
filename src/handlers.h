#ifndef HANDLERS_H
#define HANDLERS_H

#include <X11/Xlib.h>

typedef struct {
    int isActive;
    int mouseX;
    int mouseY;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    unsigned int dragButton;
    Window draggedWindow;
} MouseGrabInfo;

void MappingNotifyHandler(Display *display, XEvent *event);
void MouseMotionHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo);
void MouseReleaseHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo);
void MousePressHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo);
int KeyPressHandler(Display *display, XEvent *xEvent);

#endif /* HANDLERS_H */
