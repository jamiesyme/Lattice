#include <time.h>

#include "draw-utils.h"
#include "module.h"
#include "surface.h"
#include "time-module.h"

#define DRAW_WIDTH 95
#define DRAW_HEIGHT 40
#define FONT_NAME "monaco"
#define FONT_SIZE 32


void renderTimeModule(Module* module, Surface* surface);

void initTimeModule(Module* module)
{
  module->type = MT_TIME;
  module->renderFunc = renderTimeModule;

  setModuleDrawSize(module, (Dimensions){DRAW_WIDTH, DRAW_HEIGHT});
}

void renderTimeModule(Module* module, Surface* surface)
{
  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  // Convert the current time into a string
  char strTime[16];
  strftime(strTime, sizeof strTime, "%-I:%M", tm);

  // Render the time string
  setDrawColor(surface, (Color){0.0f, 0.0f, 0.0f, 1.0f});
  TextSurface* textSurface = renderText(surface, FONT_SIZE, FONT_NAME, strTime);
  Dimensions textSize = getTextDimensions(textSurface);
  drawText(surface, textSurface, textSize.width / 2, textSize.height / 2, 1);
  freeTextSurface(textSurface);
}
