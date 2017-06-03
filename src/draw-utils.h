#pragma once

// draw-utils is a collection of commonly used draw functions.

#include "geometry-utils.h"

// See linux-window.h for warning about this forward declaration.
typedef struct _cairo cairo_t;

// You can imagine that a text surface is an image. You must first render the
// text to an image using renderText(), and then you can draw it using
// drawText(). Don't forget to free the text surface with freeTextSurface().
typedef struct TextSurface TextSurface;

typedef struct Color {
  float r;
  float g;
  float b;
  float a;
} Color;


// Equivalent to cairo_set_source_rgba().
void setDrawColor(cairo_t* cairoContext, Color color);
void setDrawColor4(cairo_t* cairoContext, float r, float g, float b, float a);

void drawRect4(cairo_t* cairoContext,
               int x,
               int y,
               unsigned int width,
               unsigned int height);

// Loads the fontName at the specified size and renders the text to a text
// surface. You can then draw this text to the screen using a subsequent call to
// drawText(). You can keep this text surface for however long you wish, but be
// sure to free it with freeTextSurface() when you're done.
// NOTE: this function does not do any actual drawing.
TextSurface* renderText(cairo_t* cairoContext,
                        unsigned int size,
                        const char* fontName,
                        const char* text);

void drawText(cairo_t* cairoContext,
              TextSurface* textSurface,
              int x,
              int y,
              int centered);

Dimensions getTextDimensions(TextSurface* textSurface);

void freeTextSurface(TextSurface* textSurface);
