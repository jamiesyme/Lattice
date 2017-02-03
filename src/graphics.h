#pragma once

typedef struct Surface Surface;


void setDrawColor(Surface* surface, float r, float g, float b, float a);

void drawFullRect(Surface* surface);

void drawRect(Surface* surface,
              int x,
              int y,
              unsigned int width,
              unsigned int height);

void drawText(Surface* surface,
              int x,
              int y,
              unsigned int size,
              const char* fontName,
              const char* text);
