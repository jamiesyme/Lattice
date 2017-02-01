#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
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
} Surface;


struct MwmHints {
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long input_mode;
  unsigned long status;
};

enum {
  MWM_HINTS_FUNCTIONS = (1L << 0),
  MWM_HINTS_DECORATIONS =  (1L << 1),

  MWM_FUNC_ALL = (1L << 0),
  MWM_FUNC_RESIZE = (1L << 1),
  MWM_FUNC_MOVE = (1L << 2),
  MWM_FUNC_MINIMIZE = (1L << 3),
  MWM_FUNC_MAXIMIZE = (1L << 4),
  MWM_FUNC_CLOSE = (1L << 5)
};


Surface* newSurface(unsigned int width, unsigned int height)
{
  int status;
  Display* xDisplay = XOpenDisplay(0);
  if (xDisplay == 0) {
    printf("Failed to connect to X.\n");
    return 0;
  }
  int xScreen = DefaultScreen(xDisplay);
  Window xRoot = RootWindow(xDisplay, xScreen);
  XVisualInfo xVisualInfo;
  status = XMatchVisualInfo(xDisplay,
                            xScreen,
                            32,
                            TrueColor,
                            &xVisualInfo);
  if (status == 0) {
    printf("No 32-bit visual available from X.\n");
    return 0;
  }
  XSetWindowAttributes xAttributes;
  xAttributes.background_pixmap = None;
  xAttributes.border_pixel = 0;
  xAttributes.colormap = XCreateColormap(xDisplay,
                                         xRoot,
                                         xVisualInfo.visual,
                                         AllocNone);
  xAttributes.event_mask = ExposureMask;
  int xAttributeMask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;
  Window xWindow = XCreateWindow(xDisplay,
                                 xRoot,
                                 0, 0,
                                 width, height,
                                 0,
                                 xVisualInfo.depth,
                                 InputOutput,
                                 xVisualInfo.visual,
                                 xAttributeMask,
                                 &xAttributes);

  // Make the window floating
  // I'd prefer "_NET_WM_WINDOW_TYPE_NOTIFICATION", but i3 still tiles that
  Atom windowType = XInternAtom(xDisplay, "_NET_WM_WINDOW_TYPE", 0);
  Atom windowTypeDesktop = XInternAtom(xDisplay, "_NET_WM_WINDOW_TYPE_UTILITY", 0);
  XChangeProperty(xDisplay,
                  xWindow,
                  windowType,
                  XA_ATOM,
                  32,
                  PropModeReplace,
                  (unsigned char*)&windowTypeDesktop,
                  1);

  // Remove the window decorations
  Atom motifHintsType = XInternAtom(xDisplay, "_MOTIF_WM_HINTS", 0);
  struct MwmHints motifHints;
  motifHints.flags = MWM_HINTS_DECORATIONS;
  motifHints.decorations = 0;
  XChangeProperty(xDisplay,
                  xWindow,
                  motifHintsType,
                  XA_ATOM,
                  32,
                  PropModeReplace,
                  (unsigned char*)&motifHints,
                  5);

  XMapWindow(xDisplay, xWindow);
  XSelectInput(xDisplay, xWindow, ExposureMask | KeyPressMask);

  // Create cairo surface
  cairo_surface_t* cs = cairo_xlib_surface_create(xDisplay,
                                                  xWindow,
                                                  xVisualInfo.visual,
                                                  width, height);
  cairo_xlib_surface_set_size(cs, width, height); // Is this really needed?

  // Create cairo context
  cairo_t* cr = cairo_create(cs);

  // Put together the surface
  Surface* surface = malloc(sizeof(Surface));
  surface->xDisplay = xDisplay;
  surface->xWindow = xWindow;
  surface->cSurface = cs;
  surface->cContext = cr;
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

cairo_t* getCairoContext(Surface* surface)
{
  return surface->cContext;
}
