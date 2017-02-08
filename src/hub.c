#include <cairo/cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "graphics.h"
#include "hub.h"
#include "module.h"
#include "surface.h"

#include "audio-module.h"
#include "date-module.h"
#include "time-module.h"

#define MAX_MODULES 32


typedef struct Hub {
  int isShown;
  int shouldClear;
  unsigned int modulePadding;
  unsigned int screenPadding;
  Module modules[MAX_MODULES];
  size_t moduleCount;
  Surface* surface;
} Hub;


Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  hub->shouldClear = 1;
  hub->isShown = 1; //0; Disabled for development
  hub->modulePadding = 25;
  hub->screenPadding = 50;
  hub->moduleCount = 0;
  newAudioModule(&hub->modules[hub->moduleCount++]);
  newDateModule(&hub->modules[hub->moduleCount++]);
  newTimeModule(&hub->modules[hub->moduleCount++]);
  {
    unsigned int width = 0, height = 0;
    for (size_t i = 0; i < hub->moduleCount; ++i) {
      Module* module = &hub->modules[i];
      if (width == 0 || width < module->width) {
        width = module->width;
      }
      height += module->height + (height == 0 ? 0 : hub->modulePadding);
    }

    int x, y;
    x = -(width + hub->screenPadding);
    y = -(height + hub->screenPadding);

    hub->surface = newSurface(x, y, width, height);
  }
	return hub;
}

void freeHub(Hub* hub)
{
  freeSurface(hub->surface);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    Module* module = &hub->modules[i];
    if (module->freeFunc != NULL) {
      module->freeFunc(module);
    }
  }
  free(hub);
}

int shouldUpdateHub(Hub* hub)
{
  return hub->isShown || hub->shouldClear;
}

void updateHub(Hub* hub)
{
  if (hub->isShown) {
    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);

    cairo_t* cr = getCairoContext(hub->surface);
    cairo_save(cr);
    for (size_t i = 0; i < hub->moduleCount; ++i) {
      Module* module = &hub->modules[i];
      if (module->updateFunc == 0) {
        continue;
      }
      cairo_save(cr);
      cairo_translate(cr, getSurfaceWidth(hub->surface) - module->width, 0);
      module->updateFunc(module, hub->surface);
      cairo_restore(cr);
      cairo_translate(cr, 0, module->height + hub->modulePadding);
    }
    cairo_restore(cr);

  } else {
    setDrawColor(hub->surface, 0, 0, 0, 0.5);
    drawFullRect(hub->surface);
    hub->shouldClear = 0;
  }
  flushSurface(hub->surface);
}

void showHubModules(Hub* hub)
{
  hub->isShown = 1;
}

void hideHubModules(Hub* hub)
{
  hub->isShown = 0;
  hub->shouldClear = 1;
}
