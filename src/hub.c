#include <cairo/cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw-utils.h"
#include "hub.h"
#include "module.h"
#include "surface.h"
#include "time-utils.h"

#include "audio-module.h"
#include "date-module.h"
#include "time-module.h"
#include "workspace-module.h"

#define MAX_MODULES 32


typedef struct Hub {
  // This used to be stored as a linked list, but this is much easier.
  Module modules[MAX_MODULES];
  size_t moduleCount;

  // Module padding is the vertical space between modules.
  unsigned int modulePadding;

  // Screen padding is the horizontal and vertical space between the modules and
  // the bottom right corner of the monitor. This helps position the surface.
  unsigned int screenPadding;

  // The hub dimensions match the surface (window) dimensions. The hub width is
  // the max width of any module. The hub height is the sum of each module's
  // height, plus padding between each module.
  unsigned int width, height;

  // A surface is a wrapper around a window. It's what we draw to.
  Surface* surface;

  // This flag tracks the state of all the modules. If they are all MS_OFF, this
  // flag is cleared; otherwise it's set. It's updated every time renderHub() is
  // run. Explicitly setting this flag is an effective way to clear the screen,
  // even if all the modules are off.
  int shouldRender;

  // In order to update the module opacities we need a delta time. This keeps
  // track of the time when the last render took place.
  Milliseconds lastRenderTime;
} Hub;


static void updateHubSize(Hub* hub)
{
  hub->width = 0;
  hub->height = 0;
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    if (hub->width == 0 || hub->width < hub->modules[i].width) {
      hub->width = hub->modules[i].width;
    }
    if (hub->height > 0) {
      hub->height += hub->modulePadding;
    }
    hub->height += hub->modules[i].height;
  }
}

Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  hub->moduleCount = 0;
  hub->modulePadding = 25;
  hub->screenPadding = 50;
  hub->shouldRender = 1;

  for (size_t i = 0; i < MAX_MODULES; ++i) {
    initModule(&hub->modules[i]);
  }
  initTimeModule(&hub->modules[hub->moduleCount++]);
  initDateModule(&hub->modules[hub->moduleCount++]);
  initAudioModule(&hub->modules[hub->moduleCount++]);
  initWorkspaceModule(&hub->modules[hub->moduleCount++]);

  updateHubSize(hub);
  hub->surface = newSurface(-(int)(hub->width + hub->screenPadding),
                            -(int)(hub->height + hub->screenPadding),
                            hub->width,
                            hub->height);

  hub->lastRenderTime = getTimeInMilliseconds();

	return hub;
}

void freeHub(Hub* hub)
{
  freeSurface(hub->surface);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    if (hub->modules[i].freeFunc != NULL) {
      hub->modules[i].freeFunc(&hub->modules[i]);
    }
  }
  free(hub);
}

int shouldRenderHub(Hub* hub)
{
  return hub->shouldRender;
}

static void updateRenderFlag(Hub* hub, Milliseconds delta)
{
  hub->shouldRender = 0;
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    updateModuleOpacity(&hub->modules[i], delta);
    if (hub->modules[i].state != MS_OFF) {
      hub->shouldRender = 1;
    }
  }
}

static void renderModules(Hub* hub)
{
  cairo_t* cr = getCairoContext(hub->surface);
  cairo_save(cr);
  cairo_translate(cr, 0, hub->height - 1);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    Module* module = &hub->modules[i];
    if (module->renderFunc == 0 || module->state == MS_OFF) {
      continue;
    }
    cairo_save(cr);
    cairo_translate(cr, hub->width - module->width, -module->height);
    module->renderFunc(module, hub->surface);
    cairo_restore(cr);
    cairo_translate(cr, 0, -(module->height + hub->modulePadding));
  }
  cairo_restore(cr);
}

void renderHub(Hub* hub)
{
  Milliseconds frameDuration = getTimeInMilliseconds() - hub->lastRenderTime;
  hub->lastRenderTime = getTimeInMilliseconds();

  updateRenderFlag(hub, frameDuration);

  // If we are rendering, we need to:
  //  1) Update the hub size - we want our surface to be as small as possible,
  //     so were not stealing more input focus than necessary.
  //  2) Map the surface - we unmap it when we're not rendering (see note
  //     below).
  //  3) Clear the screen
  //  4) Render the modules
  //
  // If we are not rendering, we will unmap the surface. This is to prevent
  // stealing input events from windows beneath when we are completely hidden.
  // It would be preferably to disable window events entirely for our surface,
  // since they're unused, but I don't know how to do that.
  if (hub->shouldRender) {
    mapSurface(hub->surface);

    setDrawColor(hub->surface, 0, 0, 0, 0);
    drawFullRect(hub->surface);

    renderModules(hub);
    flushSurface(hub->surface);

  } else {
    unmapSurface(hub->surface);
  }
}

static void refreshHub(Hub* hub)
{
  if (!hub->shouldRender) {
    hub->shouldRender = 1;
    hub->lastRenderTime = getTimeInMilliseconds();
  }
}

void showHub(Hub* hub)
{
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    setModuleState(&hub->modules[i], MS_ON_CONSTANT);
  }
  refreshHub(hub);
}

void hideHub(Hub* hub)
{
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    setModuleState(&hub->modules[i], MS_OFF);
  }
  refreshHub(hub);
}

void toggleHub(Hub* hub)
{
  int isOff = 0;
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    if (hub->modules[i].state == MS_OFF) {
      isOff = 1;
      break;
    }
  }
  if (isOff) {
    showHub(hub);
  } else {
    hideHub(hub);
  }
  refreshHub(hub);
}

void showModuleUpdate(Hub* hub, ModuleType mt)
{
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    if (hub->modules[i].type == mt) {
      if (hub->modules[i].state != MS_ON_CONSTANT) {
        setModuleState(&hub->modules[i], MS_ON_DYNAMIC);
        refreshHub(hub);
      }
      return;
    }
  }
}
