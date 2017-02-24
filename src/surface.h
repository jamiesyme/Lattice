#pragma once

// A surface is a wrapper around everything Xlib; it's basically a window
// wrapper. A cairo context is also created for each surface.

typedef struct Surface Surface;

// TODO: Should we remove this and just return void*? Not sure how stable this
// typedef is between cairo versions. Here is the documentation:
// https://www.cairographics.org/manual/cairo-cairo-t.html#cairo-t
typedef struct _cairo cairo_t;


// Creates a new surface with the specified dimensions and position. Negative
// position coords can be used to represent distance from right or bottom of
// monitor. Surface is created:
//   + w/o decorations - no border, no title bar, etc.
//   + floating        - relevant for tiling window managers
//   + sticky          - shows on all virtual desktops
Surface* newSurface(int x, int y, unsigned int width, unsigned int height);
void freeSurface(Surface* surface);


// This should be called once a frame to flush any queued drawing commands.
void flushSurface(Surface* surface);


// To prevent the window from stealing input events when fulling transparent,
// unmap the window.
void mapSurface(Surface* surface);
void unmapSurface(Surface* surface);


void setSurfacePosition(Surface* surface, int x, int y);
void setSurfaceSize(Surface* surface, unsigned int width, unsigned int height);

unsigned int getSurfaceWidth(Surface* surface);
unsigned int getSurfaceHeight(Surface* surface);


cairo_t* getCairoContext(Surface* surface);
