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
#include "time-utils.h"

#define MAX_MODULES 32


typedef struct Hub {
  int shouldRender;

  unsigned int modulePadding;
  unsigned int screenPadding;
  Module modules[MAX_MODULES];
  size_t moduleCount;

  Milliseconds lastUpdateTime;

  Surface* surface;
} Hub;


Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  hub->shouldRender = 1;

  hub->modulePadding = 25;
  hub->screenPadding = 50;
  hub->moduleCount = 0;
  for (int i = 0; i < MAX_MODULES; ++i) {
    initModule(&hub->modules[i]);
  }
  initTimeModule(&hub->modules[hub->moduleCount++]);
  initDateModule(&hub->modules[hub->moduleCount++]);
  initAudioModule(&hub->modules[hub->moduleCount++]);

  hub->lastUpdateTime = getTimeInMilliseconds();

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

int shouldRenderHub(Hub* hub)
{
  return hub->shouldRender;
}

void renderHub(Hub* hub)
{
  Milliseconds frameDuration = getTimeInMilliseconds() - hub->lastUpdateTime;
  hub->lastUpdateTime = getTimeInMilliseconds();

  hub->shouldRender = 0;
  for (int i = 0; i < hub->moduleCount; ++i) {
    updateModuleOpacity(&hub->modules[i], frameDuration);
    if (hub->modules[i].state != MS_OFF) {
      hub->shouldRender = 1;
    }
  }

  if (hub->shouldRender) {
    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);

    cairo_t* cr = getCairoContext(hub->surface);
    cairo_save(cr);
    cairo_translate(cr, 0, getSurfaceHeight(hub->surface) - 1);
    for (size_t i = 0; i < hub->moduleCount; ++i) {
      Module* module = &hub->modules[i];
      if (module->renderFunc == 0) {
        continue;
      }
      cairo_save(cr);
      cairo_translate(cr,
                      getSurfaceWidth(hub->surface) - module->width,
                      -module->height);
      module->renderFunc(module, hub->surface);
      cairo_restore(cr);
      cairo_translate(cr, 0, -(module->height + hub->modulePadding));
    }
    cairo_restore(cr);

  } else {
    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);
  }

  flushSurface(hub->surface);
}

void showHubModules(Hub* hub)
{
  for (int i = 0; i < hub->moduleCount; ++i) {
    setModuleState(&hub->modules[i], MS_ON_CONSTANT);
  }
  hub->shouldRender = 1;
}

void hideHubModules(Hub* hub)
{
  for (int i = 0; i < hub->moduleCount; ++i) {
    setModuleState(&hub->modules[i], MS_OFF);
  }
  hub->shouldRender = 1;
}

void showModuleUpdate(Hub* hub, ModuleType mt)
{
  for (int i = 0; i < hub->moduleCount; ++i) {
    if (hub->modules[i].type == mt) {
      if (hub->modules[i].state != MS_ON_CONSTANT) {
        setModuleState(&hub->modules[i], MS_ON_DYNAMIC);
      }
      return;
    }
  }
}
