#pragma once

// LinuxWindow wraps Xlib and manages the cairo context.

// TODO: Should we remove this and just return void*? Not sure how stable this
// typedef is between cairo versions. Here is the documentation:
// https://www.cairographics.org/manual/cairo-cairo-t.html#cairo-t
typedef struct _cairo cairo_t;

typedef struct LinuxWindow LinuxWindow;
	

// Creates a new window with the specified dimensions, and the following
// attributes:
//   + w/o decorations - no border, no title bar, etc.
//   + floating        - relevant for tiling window managers
//   + sticky          - shows on all virtual desktops
// width and height cannot be zero.
// The window will be positioned at (0, 0).
// The window will be showing by default.
LinuxWindow* newLinuxWindow(unsigned int width, unsigned int height);
void freeLinuxWindow(LinuxWindow* window);


// These should be called each frame, pre and post render. They deal with
// grouping the draw calls appropriately for Xlib.
void prepareLinuxWindowRender(LinuxWindow* window);
void finishLinuxWindowRender(LinuxWindow* window);


// Hiding the window when not in use will prevent stealing input intended for
// the window/app beneath.
void showLinuxWindow(LinuxWindow* window);
void hideLinuxWindow(LinuxWindow* window);


// Set the window position and size.
// x and y can be negative to represent offset from the right or bottom of the
// screen.
// width and height cannot be zero.
void setLinuxWindowPosition(LinuxWindow* window, int x, int y);
void setLinuxWindowSize(LinuxWindow* window,
                        unsigned int width,
                        unsigned int height);


cairo_t* getCairoContextFromLinuxWindow(LinuxWindow* window);
