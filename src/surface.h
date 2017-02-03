#pragma once


typedef struct Surface Surface;

// TODO: Should we remove this and just return void*? Not sure how stable this
// typedef is between cairo versions. Here is the documentation:
// https://www.cairographics.org/manual/cairo-cairo-t.html#cairo-t
typedef struct _cairo cairo_t;


// Negative position coords will be subtracted from the max coords
Surface* newSurface(int x, int y, unsigned int width, unsigned int height);

void freeSurface(Surface* surface);

cairo_t* getCairoContext(Surface* surface);

void flushSurface(Surface* surface);

unsigned int getSurfaceWidth(Surface* surface);

unsigned int getSurfaceHeight(Surface* surface);
