#pragma once

// draw-utils is a collection of commonly used draw functions. Everything done
// here can also be done manually by getting the cairo context from the surface.

#include "geometry-utils.h"

typedef struct Surface Surface;


// You can imagine that a text surface is an image. You must first render the
// text to an image using renderText(), and then you can draw it to the surface
// using drawText(). Don't forget to free the text surface with
// freeTextSurface().
typedef struct TextSurface TextSurface;

typedef struct Color {
  float r;
  float g;
  float b;
  float a;
} Color;


// Equivalent to cairo_set_source_rgba().
void setDrawColor(Surface* surface, Color color);
void setDrawColor4(Surface* surface, float r, float g, float b, float a);


// Draws a rectangle covering the entire surface.
void drawFullRect(Surface* surface);

void drawRect4(Surface* surface,
               int x,
               int y,
               unsigned int width,
               unsigned int height);

// Loads the fontName at the specified size and renders the text to a text
// surface. You can then draw this text to the screen using a subsequent call to
// drawText(). You can keep this text surface for however long you wish, but be
// sure to free it with freeTextSurface() when you're done.
// NOTE: this function does not do any actual drawing to the supplied surface.
TextSurface* renderText(Surface* surface,
                        unsigned int size,
                        const char* fontName,
                        const char* text);

void drawText(Surface* surface,
              TextSurface* textSurface,
              int x,
              int y,
              int centered);

Dimensions getTextDimensions(TextSurface* textSurface);

void freeTextSurface(TextSurface* textSurface);
