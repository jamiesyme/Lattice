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


typedef struct FadeModule {
  Module* module;
  Milliseconds opaqueTimeLeft;
  Milliseconds fadeTimeLeft;
} FadeModule;

typedef struct Hub {
  int isShown;
  int shouldClear;

  unsigned int modulePadding;
  unsigned int screenPadding;
  Module modules[MAX_MODULES];
  size_t moduleCount;
  float opacity;

  FadeModule fadeModules[MAX_MODULES];
  size_t fadeModuleCount;
  Milliseconds opaqueDuration;
  Milliseconds fadeDuration;
  Milliseconds lastUpdateTime;

  Surface* surface;
} Hub;

static void removeFadeModule(Hub* hub, size_t i);


Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  hub->shouldClear = 1;
  hub->isShown = 0;

  hub->modulePadding = 25;
  hub->screenPadding = 50;
  hub->moduleCount = 0;
  hub->opacity = 0.8f;
  newTimeModule(&hub->modules[hub->moduleCount++]);
  newDateModule(&hub->modules[hub->moduleCount++]);
  newAudioModule(&hub->modules[hub->moduleCount++]);

  hub->fadeModuleCount = 0;
  hub->opaqueDuration = 1000;
  hub->fadeDuration = 500;
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

int shouldUpdateHub(Hub* hub)
{
  return hub->isShown || hub->shouldClear || hub->fadeModuleCount > 0;
}

void updateHub(Hub* hub)
{
  Milliseconds frameDuration = getTimeInMilliseconds() - hub->lastUpdateTime;
  hub->lastUpdateTime = getTimeInMilliseconds();

  if (hub->isShown) {
    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);

    cairo_t* cr = getCairoContext(hub->surface);
    cairo_save(cr);
    cairo_translate(cr, 0, getSurfaceHeight(hub->surface) - 1);
    for (size_t i = 0; i < hub->moduleCount; ++i) {
      Module* module = &hub->modules[i];
      if (module->updateFunc == 0) {
        continue;
      }
      cairo_save(cr);
      cairo_translate(cr,
                      getSurfaceWidth(hub->surface) - module->width,
                      -module->height);
      module->updateFunc(module, hub->surface, hub->opacity);
      cairo_restore(cr);
      cairo_translate(cr, 0, -(module->height + hub->modulePadding));
    }
    cairo_restore(cr);

  } else if (hub->fadeModuleCount > 0) {
    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);

    cairo_t* cr = getCairoContext(hub->surface);
    cairo_save(cr);
    cairo_translate(cr, 0, getSurfaceHeight(hub->surface) - 1);
    for (size_t i = 0; i < hub->fadeModuleCount; ++i) {
      FadeModule* fadeModule = &hub->fadeModules[i];
      Module* module = hub->fadeModules[i].module;
      if (module->updateFunc == 0) {
        continue;
      }
      float opacity = hub->opacity;
      if (fadeModule->opaqueTimeLeft > 0.f) {
        fadeModule->opaqueTimeLeft -= frameDuration;
      } else {
        opacity *= (float)fadeModule->fadeTimeLeft / hub->fadeDuration;
        fadeModule->fadeTimeLeft -= frameDuration;
        if (fadeModule->fadeTimeLeft < 0.f) {
          removeFadeModule(hub, i);
        }
      }
      cairo_save(cr);
      cairo_translate(cr,
                      getSurfaceWidth(hub->surface) - module->width,
                      -module->height);
      module->updateFunc(module, hub->surface, opacity);
      cairo_restore(cr);
      cairo_translate(cr, 0, -(module->height + hub->modulePadding));
    }
    cairo_restore(cr);

  } else {
    setDrawColor(hub->surface, 0, 0, 0, 0);
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

void showModuleUpdate(Hub* hub, ModuleType mt)
{
  // To prevent large time deltas after not updating the hub for awhile, we will
  // reset the update time if we haven't displayed in awhile. If we didn't do
  // this the opaqueTimeLeft of fade modules would be wiped out in a single
  // frame.
  if (!shouldUpdateHub(hub)) {
    hub->lastUpdateTime = getTimeInMilliseconds();
  }

  // Reset the fade module if present
  for (int i = 0; i < hub->fadeModuleCount; ++i) {
    if (hub->fadeModules[i].module->type == mt) {
      hub->fadeModules[i].opaqueTimeLeft = hub->opaqueDuration;
      hub->fadeModules[i].fadeTimeLeft = hub->fadeDuration;
      return;
    }
  }

  // Add the fade module
  for (int i = 0; i < hub->moduleCount; ++i) {
    if (hub->modules[i].type == mt) {
      FadeModule* fadeModule = &hub->fadeModules[hub->fadeModuleCount];
      fadeModule->module = &hub->modules[i];
      fadeModule->opaqueTimeLeft = hub->opaqueDuration;
      fadeModule->fadeTimeLeft = hub->fadeDuration;
      hub->fadeModuleCount += 1;
      return;
    }
  }
}

static void removeFadeModule(Hub* hub, size_t i)
{
  if (i >= hub->fadeModuleCount) {
    return;
  }
  hub->fadeModules[i] = hub->fadeModules[hub->fadeModuleCount - 1];
  hub->fadeModuleCount -= 1;
}
