#pragma once

typedef struct Surface Surface;


void setDrawColor(Surface* surface, float r, float g, float b, float a);

void drawFullRect(Surface* surface);

void drawText(Surface* surface,
              unsigned int x,
              unsigned int y,
              unsigned int size,
              const char* fontName,
              const char* text);
