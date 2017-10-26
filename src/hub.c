#include <cairo/cairo.h>
#include <stdlib.h>
#include <stdio.h>

#include "geometry-utils.h"
#include "hub.h"
#include "module.h"
#include "module-director.h"
#include "module-renderer.h"

#include "audio-module.h"
#include "date-module.h"
#include "time-module.h"
#include "workspace-module.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define UNUSED(x) (void)(x)


typedef struct Hub {
  AppConfig* appConfig;

  Module* modules;
  size_t moduleCount;
  int allModulesAreOpen;

  ModuleDirector* moduleDirector;
  ModuleRenderer* moduleRenderer;
} Hub;


static Module* addModuleToHub(Hub* hub)
{
	hub->moduleCount++;
  hub->modules = realloc(hub->modules, sizeof(Module) * hub->moduleCount);
  initModule(&hub->modules[hub->moduleCount - 1], hub->appConfig);
  return &hub->modules[hub->moduleCount - 1];
}

Hub* newHub(AppConfig* appConfig)
{
  Hub* hub = malloc(sizeof(Hub));
  hub->appConfig = appConfig;

  // Init modules
  hub->allModulesAreOpen = 0;
  hub->modules = NULL;
  hub->moduleCount = 0;
  initTimeModule(addModuleToHub(hub));
  initDateModule(addModuleToHub(hub));
  initAudioModule(addModuleToHub(hub));
  initWorkspaceModule(addModuleToHub(hub));

  // Init module director
  hub->moduleDirector = newModuleDirector(hub->appConfig);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    addModuleToDirector(hub->moduleDirector, &hub->modules[i]);
  }

  // Init module renderer
  hub->moduleRenderer = newModuleRenderer(hub->appConfig);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    addModuleToRenderer(hub->moduleRenderer, &hub->modules[i]);
  }

	return hub;
}

void freeHub(Hub* hub)
{
  freeModuleRenderer(hub->moduleRenderer);
  freeModuleDirector(hub->moduleDirector);
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    Module* module = &hub->modules[i];
    if (module->freeFunc != NULL) {
      module->freeFunc(module);
    }
  }
  free(hub->modules);
  free(hub);
}

int shouldRenderHub(Hub* hub)
{
  // The module renderer will be busy if any module is on-screen.
  // The module director will be busy if anything is moving.
  // In both cases, either something needs to be rendered or moved (which also
  // happens during renderHub()).
  return (isModuleRendererBusy(hub->moduleRenderer) ||
          isModuleDirectorBusy(hub->moduleDirector));
}

Rect getHubScreenRect(Hub* hub)
{
  // Get the rect of the modules if they were all opened. This is the largest
  // screen that we need to support.
  Rect directorRect = getModuleDirectorOpenRect(hub->moduleDirector);

  // The open rect is potentially partially off-screen. The screen rect should be
  // clamped to the screen.
  Rect screenRect = directorRect;
  if (screenRect.x > 0.0f) {
    screenRect.x = 0.0f;
  }
  if (screenRect.y > 0.0f) {
    screenRect.y = 0.0f;
  }
  if (screenRect.x + screenRect.width > 0.0f) {
    screenRect.width = -screenRect.x;
  }
  if (screenRect.y + screenRect.height > 0.0f) {
    screenRect.height = -screenRect.y;
  }

  return screenRect;
}

void renderHub(Hub* hub, cairo_t* cairoContext)
{
	Rect screenRect;

  // Let the module renderer do it's thing
  if (isModuleRendererBusy(hub->moduleRenderer)) {

	  // Since the window resizes and moves as modules move, we need to correct
	  // that with a translation.
	  screenRect = getHubScreenRect(hub);
    cairo_save(cairoContext);
    cairo_translate(cairoContext, -screenRect.x, -screenRect.y);
    renderModules(hub->moduleRenderer, cairoContext);
    cairo_restore(cairoContext);
  }

  // Allow the director to move the modules. This happens after the rendering so
  // that the window can be resized before the next frame.
  updateModuleDirector(hub->moduleDirector);
}

void updateHubBeforeSleep(Hub* hub)
{
  UNUSED(hub);
	// TODO: Will we ever actually need a before hook?
}

void updateHubAfterSleep(Hub* hub)
{
  updateModuleDirectorAfterSleep(hub->moduleDirector);
}

void showAllHubModules(Hub* hub)
{
  openModulesWithDirector(hub->moduleDirector);
  hub->allModulesAreOpen = 1;
}

void hideAllHubModules(Hub* hub)
{
  closeModulesWithDirector(hub->moduleDirector);
  hub->allModulesAreOpen = 0;
}

void toggleAllHubModules(Hub* hub)
{
  if (hub->allModulesAreOpen) {
    hideAllHubModules(hub);
  } else {
    showAllHubModules(hub);
  }
}

void alertHubModule(Hub* hub, ModuleType moduleType)
{
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    Module* module = &hub->modules[i];
    if (module->type == moduleType) {
      alertModuleWithDirector(hub->moduleDirector, module);
    }
  }
}
