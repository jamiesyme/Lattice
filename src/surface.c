#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "surface.h"


typedef struct Surface {
  Display* xDisplay;
  Window xWindow;
  cairo_surface_t* cSurface;
  cairo_t* cContext;
  unsigned int width;
  unsigned int height;
} Surface;


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

// Sets the window type to "NOTIFICATION"
static void setWindowType(Display* xDisplay, Window xWindow)
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

// Sets the window to be sticky and appear across all virtual desktops
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

Surface* newSurface(int x, int y, unsigned int width, unsigned int height)
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

  // If the caller supplied negative position coords, convert them
  normalizePositionCoords(xDisplay, x, y, &x, &y);

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
  // of trail and error; I'm not completely sure how all of the values interact
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
                          x, y,
                          width, height,
                          0,
                          xVisualInfo.depth,
                          InputOutput,
                          xVisualInfo.visual,
                          xAttributeMask,
                          &xAttributes);

  // The functionality is the same even without these functions, but it's better
  // to be explicit about what we want.
  setWindowType(xDisplay, xWindow);
  setWindowSticky(xDisplay, xWindow);

  // Window configuration is done, so map the window to the screen
  XMapWindow(xDisplay, xWindow);

  // Initialize cairo
  cairo_surface_t* cs = cairo_xlib_surface_create(xDisplay,
                                                  xWindow,
                                                  xVisualInfo.visual,
                                                  width, height);
  cairo_t* cr = cairo_create(cs);

  // Put together the surface
  Surface* surface = malloc(sizeof(Surface));
  surface->xDisplay = xDisplay;
  surface->xWindow = xWindow;
  surface->cSurface = cs;
  surface->cContext = cr;
  surface->width = width;
  surface->height = height;
  return surface;
}

void freeSurface(Surface* surface)
{
  cairo_destroy(surface->cContext);
  cairo_surface_destroy(surface->cSurface);
  XUnmapWindow(surface->xDisplay, surface->xWindow);
  XDestroyWindow(surface->xDisplay, surface->xWindow);
  XCloseDisplay(surface->xDisplay);
  free(surface);
}

void flushSurface(Surface* surface)
{
  cairo_surface_flush(surface->cSurface);
  XFlush(surface->xDisplay);
}

cairo_t* getCairoContext(Surface* surface)
{
  return surface->cContext;
}

unsigned int getSurfaceWidth(Surface* surface)
{
  return surface->width;
}

unsigned int getSurfaceHeight(Surface* surface)
{
  return surface->height;
}
