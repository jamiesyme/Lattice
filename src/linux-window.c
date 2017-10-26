#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "linux-window.h"


typedef struct LinuxWindow {
  Display* xDisplay;
  Window xWindow;
  cairo_surface_t* cairoSurface;
  cairo_t* cairoContext;
} LinuxWindow;


// Input coords x and y can be positive or negative. Positive coords are already
// normalized, and negative coords will be subtracted from the maximum coords.
// Example: if screen size is 800x600, x = -10, and y = -10, then xOut = 789 and
// yOut = 589. The extra -1 is because the maximum coords are indexed from 0.
static void normalizePositionCoords(Display* xDisplay,
                                    int x,
                                    int y,
                                    int* xOut,
                                    int* yOut)
{
  int infoCount;
  XineramaScreenInfo* info;
  info = XineramaQueryScreens(xDisplay, &infoCount);

  if (x < 0) {
    *xOut = info[0].width - 1 + x;
  } else {
    *xOut = x;
  }

  if (y < 0) {
    *yOut = info[0].height - 1 + y;
  } else {
    *xOut = y;
  }
}

// Sets the window type to "_NET_WM_WINDOW_TYPE_NOTIFICATION"
// TODO: Why? Is this what makes the window float? Come on, Jamie, learn to
// document stuff properly...
static void setWindowTypeToNotification(Display* xDisplay, Window xWindow)
{
  Atom property = XInternAtom(xDisplay, "_NET_WM_WINDOW_TYPE", 0);
  Atom value = XInternAtom(xDisplay, "_NET_WM_WINDOW_TYPE_NOTIFICATION", 0);
  XChangeProperty(xDisplay,
                  xWindow,
                  property,
                  XA_ATOM,
                  32,
                  PropModeReplace,
                  (unsigned char*)&value,
                  1);
}

// Sets the window to be sticky, making it appear across all virtual desktops
static void setWindowSticky(Display* xDisplay, Window xWindow)
{
  Atom property = XInternAtom(xDisplay, "_NET_WM_STATE", 0);
  Atom value[4] = {
    XInternAtom(xDisplay, "_NET_WM_STATE_ADD", 0),
    XInternAtom(xDisplay, "_NET_WM_STATE_STICKY", 0),
    (Atom)0,
    (Atom)0
  };
  XChangeProperty(xDisplay,
                  xWindow,
                  property,
                  XA_ATOM,
                  32,
                  PropModeReplace,
                  (unsigned char*)value,
                  4);
}

// Sets the input flag in WM_HINTS to false
static void disableWindowInput(Display* xDisplay, Window xWindow)
{
  XWMHints xHints;
  xHints.flags = InputHint;
  xHints.input = 0;
  XSetWMHints(xDisplay, xWindow, &xHints);
}

LinuxWindow* newLinuxWindow(unsigned int width, unsigned int height)
{
  int status;
  Display* xDisplay;
  int xScreen;
  Window xRoot, xWindow;
  XVisualInfo xVisualInfo;
  XSetWindowAttributes xAttributes;
  int xAttributeMask;

  // Connect to X
  xDisplay = XOpenDisplay(0);
  if (xDisplay == 0) {
    printf("Failed to connect to X.\n");
    return 0;
  }

  xScreen = DefaultScreen(xDisplay);
  xRoot = RootWindow(xDisplay, xScreen);

  // Find a visual info profile for our window that supports transparency
  status = XMatchVisualInfo(xDisplay,
                            xScreen,
                            32,
                            TrueColor,
                            &xVisualInfo);
  if (status == 0) {
    printf("No 32-bit visual available from X.\n");
    return 0;
  }

  // The window attributes have been developed based on the xlib docs and a lot
  // of trial and error; I'm not completely sure how all of the values interact
  // with each other, otherwise I'd document this a bit more.
  xAttributes.background_pixmap = None;
  xAttributes.border_pixel = 0;
  xAttributes.colormap = XCreateColormap(xDisplay,
                                         xRoot,
                                         xVisualInfo.visual,
                                         AllocNone);
  xAttributes.event_mask = ExposureMask;
  xAttributes.override_redirect = 1;
  xAttributeMask = (CWBackPixmap |
                    CWBorderPixel |
                    CWColormap |
                    CWEventMask |
                    CWOverrideRedirect);

  // Create the window
  xWindow = XCreateWindow(xDisplay,
                          xRoot,
                          0, 0,
                          width, height,
                          0,
                          xVisualInfo.depth,
                          InputOutput,
                          xVisualInfo.visual,
                          xAttributeMask,
                          &xAttributes);

  // Set some extra window properties to get the behaviour we need out of X
  setWindowTypeToNotification(xDisplay, xWindow);
  setWindowSticky(xDisplay, xWindow);
  disableWindowInput(xDisplay, xWindow);

  // Initialize cairo
  cairo_surface_t* cs = cairo_xlib_surface_create(xDisplay,
                                                  xWindow,
                                                  xVisualInfo.visual,
                                                  width, height);
  cairo_t* cr = cairo_create(cs);

  // Put together the surface
  LinuxWindow* window = malloc(sizeof(LinuxWindow));
  window->xDisplay = xDisplay;
  window->xWindow = xWindow;
  window->cairoSurface = cs;
  window->cairoContext = cr;
  return window;
}

void freeLinuxWindow(LinuxWindow* window)
{
  cairo_destroy(window->cairoContext);
  cairo_surface_destroy(window->cairoSurface);
  XDestroyWindow(window->xDisplay, window->xWindow);
  XCloseDisplay(window->xDisplay);
  free(window);
}

void prepareLinuxWindowRender(LinuxWindow* window)
{
  cairo_push_group(window->cairoContext);

  // Clear the screen
  cairo_save(window->cairoContext);
  cairo_set_source_rgba(window->cairoContext, 0.f, 0.f, 0.f, 0.f);
  cairo_set_operator(window->cairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint(window->cairoContext);
  cairo_restore(window->cairoContext);
}

void finishLinuxWindowRender(LinuxWindow* window)
{
  XEvent e;

  cairo_pop_group_to_source(window->cairoContext);
  cairo_save(window->cairoContext);
  cairo_set_operator(window->cairoContext, CAIRO_OPERATOR_SOURCE);
  cairo_paint(window->cairoContext);
  cairo_restore(window->cairoContext);
  cairo_surface_flush(window->cairoSurface);

  // Throw away the x events, we don't need them.
  // Not sure if this is necessary, but I don't what happens when the queue gets
  // full.
  while (XPending(window->xDisplay)) {
    XNextEvent(window->xDisplay, &e);
  }
}

void showLinuxWindow(LinuxWindow* window)
{
  XMapWindow(window->xDisplay, window->xWindow);
  XFlush(window->xDisplay);
}

void hideLinuxWindow(LinuxWindow* window)
{
  XUnmapWindow(window->xDisplay, window->xWindow);
  XFlush(window->xDisplay);
}

void setLinuxWindowPosition(LinuxWindow* window, int x, int y)
{
  XWindowChanges changes;
  normalizePositionCoords(window->xDisplay, x, y, &changes.x, &changes.y);
  XConfigureWindow(window->xDisplay,
                   window->xWindow,
                   CWX | CWY,
                   &changes);
}

void setLinuxWindowSize(LinuxWindow* window,
                        unsigned int width,
                        unsigned int height)
{
  XWindowChanges changes;
  changes.width = width;
  changes.height = height;
  XConfigureWindow(window->xDisplay,
                   window->xWindow,
                   CWWidth | CWHeight,
                   &changes);

  cairo_xlib_surface_set_size(window->cairoSurface, width, height);
}

cairo_t* getCairoContextFromLinuxWindow(LinuxWindow* window)
{
  return window->cairoContext;
}
