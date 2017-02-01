#pragma once


typedef struct Surface Surface;

// TODO: Should we remove this and just return void*? Not sure how stable this
// typedef is between cairo versions. Here is the documentation:
// https://www.cairographics.org/manual/cairo-cairo-t.html#cairo-t
typedef struct _cairo cairo_t;


Surface* newSurface(unsigned int width, unsigned int height);

void freeSurface(Surface* surface);

cairo_t* getCairoContext(Surface* surface);
