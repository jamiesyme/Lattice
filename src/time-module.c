#include <stdlib.h>

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
  setDrawColor(surface, 1.0, 1.0, 1.0, 0.8);
  drawRect(surface, 0, 0, module->width, module->height);

  setDrawColor(surface, 0, 0, 0, 0.8);
  drawText(surface, 10, 10, 32, "monaco", "Good");
}
