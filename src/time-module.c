#include <time.h>

#include "draw-utils.h"
#include "module.h"
#include "surface.h"
#include "time-module.h"


void renderTimeModule(Module* module, Surface* surface);

void initTimeModule(Module* module)
{
  module->type = MT_TIME;
  module->width = 250;
  module->height = 100;
  module->renderFunc = renderTimeModule;
}

void renderTimeModule(Module* module, Surface* surface)
{
  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  char strTime[16];
  strftime(strTime, sizeof strTime, "%-I:%M", tm);

  // Draw the border and background rects
  float opacity = getModuleOpacity(module);
  int border = 4;
  setDrawColor(surface, 0, 0, 0, opacity);
  drawRect(surface, 0, 0, module->width, module->height);
  setDrawColor(surface, 1.0, 1.0, 1.0, opacity);
  drawRect(surface,
           border, border,
           module->width - border * 2,
           module->height - border * 2);

  // Draw the time
  setDrawColor(surface, 0, 0, 0, opacity);
  TextSurface* textSurface = renderText(surface, 32, "monaco", strTime);
  drawText(surface, textSurface, module->width / 2, module->height / 2, 1);
  freeTextSurface(textSurface);
}
