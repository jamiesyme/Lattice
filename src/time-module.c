#include <stdlib.h>
#include <time.h>

#include "graphics.h"
#include "surface.h"
#include "time-module.h"


typedef struct TimeModule {
  Module base;
} TimeModule;


Module* newTimeModule()
{
  TimeModule* module = malloc(sizeof(TimeModule));
  module->base.width = 150;
  module->base.height = 100;
  module->base.updateFunc = updateTimeModule;
  module->base.freeFunc = freeTimeModule;
  return (Module*)module;
}

void freeTimeModule(Module* module)
{
  free(module);
}

void updateTimeModule(Module* module, Surface* surface)
{
  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  char strTime[6];
  strftime(strTime, sizeof strTime, "%H:%M", tm);

  // Draw the rect
  setDrawColor(surface, 1.0, 1.0, 1.0, 0.8);
  drawRect(surface, 0, 0, module->width, module->height);

  // Draw the time
  setDrawColor(surface, 0, 0, 0, 0.8);
  TextSurface* textSurface = renderText(surface, 32, "monaco", strTime);
  drawText(surface, textSurface, 10, 10, 0);
  freeTextSurface(textSurface);
}
