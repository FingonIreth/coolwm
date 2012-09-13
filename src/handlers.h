#ifndef HANDLERS_H
#define HANDLERS_H

#include <X11/Xlib.h>
#include <glib.h>

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
void MouseReleaseHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo,
                         Cursor *cursors);
void MousePressHandler(Display *display, XEvent *xEvent, MouseGrabInfo *mouseGrabInfo,
                       Cursor *cursors);

int KeyPressHandler(Display *display, XEvent *xEvent, int *currentTag, GSList *windows);

void ConfigureRequestHandler(Display *display, XEvent *xEvent);

void MapRequestHandler(Display *display, XEvent *xEvent, GSList **windows, int *currentTag);

void DestroyNotifyHandler(Display *display, XEvent *xEvent, GSList **windows, int *currentTag);



#endif /* HANDLERS_H */
