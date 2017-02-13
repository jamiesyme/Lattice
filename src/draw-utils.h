#pragma once

typedef struct Surface Surface;
typedef struct TextSurface TextSurface;


void setDrawColor(Surface* surface, float r, float g, float b, float a);

void drawFullRect(Surface* surface);

void drawRect(Surface* surface,
              int x,
              int y,
              unsigned int width,
              unsigned int height);

TextSurface* renderText(Surface* surface,
                        unsigned int size,
                        const char* fontName,
                        const char* text);

void drawText(Surface* surface,
              TextSurface* textSurface,
              int x,
              int y,
              int centered);

void freeTextSurface(TextSurface* textSurface);
