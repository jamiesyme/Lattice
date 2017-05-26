#include <cairo/cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw-utils.h"
#include "geometry-utils.h"
#include "hub.h"
#include "module.h"
#include "module-director.h"
#include "module-renderer.h"
#include "surface.h"

#include "audio-module.h"
#include "date-module.h"
#include "time-module.h"
#include "workspace-module.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))


typedef struct Hub {
  AppConfig* appConfig;

  Module* modules;
  size_t moduleCount;
  ModuleDirector* moduleDirector;
  ModuleRenderer* moduleRenderer;
  int allModulesAreOpen;

  // A surface is a wrapper around a window; it's what we draw to.
  Surface* surface;

  // Explicitly setting this flag is an effective way to clear the screen,
  // regardless of what the module director and renderer are up to.
  int forceRender;

} Hub;

// TODO: description.
static Module* addModuleToHub(Hub* hub);

// TODO: description.
static void updateSurfaceRect(Hub* hub, Rect rect);


Hub* newHub(AppConfig* appConfig)
{
  Hub* hub = malloc(sizeof(Hub));
  hub->appConfig = appConfig;

  // Init surface.
  // The surface will be resized/repositioned each frame to maintain the optimal
  // size. The "optimal" size is one that obstructs other windows the least,
  // otherwise we steal the mouse cursor from them. For now, we will just create
  // the window as small as possible, which is 1x1.
  // See renderHub() for more info.
  hub->surface = newSurface(0, 0, 1, 1);
  hub->forceRender = 1;

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
  freeSurface(hub->surface);
  free(hub);
}

int shouldRenderHub(Hub* hub)
{
  // If a forced render was requested, then we render
  if (hub->forceRender) {
    return 1;
  }

  // The module renderer will be busy if any module is on-screen.
  // The module director will be busy if anything is moving.
  // In both cases, either something needs to be rendered or moved (which also
  // happens during renderHub()).
  return isModuleRendererBusy(hub->moduleRenderer) ||
         isModuleDirectorBusy(hub->moduleDirector);
}

void renderHub(Hub* hub)
{
  // This render satisfies any forced render request
  hub->forceRender = 0;

  // Allow the director to move the modules
  updateModuleDirector(hub->moduleDirector);

  // If we are rendering, we need to:
  //  1) Update the surface size - we want our surface to be as small as
  //     possible, so were not stealing more input focus than necessary.
  //  2) Map the surface - we unmap it when we're not rendering (see note
  //     below).
  //  3) Clear the screen
  //  4) Render the modules
  //  5) Flush the surface to display what we just rendered
  //
  // If we are not rendering, we will unmap the surface. This is to prevent
  // stealing input events from windows beneath when we are completely hidden.
  // It would be preferably to disable window events entirely for our surface,
  // since they're unused, but I don't know how to do that.
  if (isModuleRendererBusy(hub->moduleRenderer)) {

    // Get the the extents of all the modules. Note that the position is the a
    // negative offset from the edge of the screen, and also note that the
    // returned rect may be positioned partially or entirely off-screen.
    Rect rendererRect = getModuleRendererRect(hub->moduleRenderer);

    // Convert the renderer rect to a surface rect.
    // If rendererRect is off-screen, this should clamp the rect to
    // {0, 0, 0, 0}.
    // Otherwise, this should clip the rect to the on-screen portion.
    Rect surfaceRect;
    surfaceRect.x = MIN(0, rendererRect.x);
    surfaceRect.y = MIN(0, rendererRect.y);
    surfaceRect.width = rendererRect.width;
    surfaceRect.height = rendererRect.height;
    // These assignments will clamp the window size to the edge of the window,
    // but they also cause a lot of lag
    //surfaceRect.width = MIN(ABS(surfaceRect.x), rendererRect.width);
    //surfaceRect.height = MIN(ABS(surfaceRect.y), rendererRect.height);

    updateSurfaceRect(hub, surfaceRect);
    mapSurface(hub->surface);

    Color clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    setDrawColor(hub->surface, clearColor);
    drawFullRect(hub->surface);

    cairo_t* cr = getCairoContext(hub->surface);
    cairo_save(cr);
    cairo_translate(cr, -surfaceRect.x, -surfaceRect.y);
    renderModules(hub->moduleRenderer, hub->surface);
    cairo_restore(cr);

    flushSurface(hub->surface);

  } else {
    unmapSurface(hub->surface);
  }
}

void showHub(Hub* hub)
{
  openModulesWithDirector(hub->moduleDirector);
  hub->allModulesAreOpen = 1;
}

void hideHub(Hub* hub)
{
  closeModulesWithDirector(hub->moduleDirector);
  hub->allModulesAreOpen = 0;
}

void toggleHub(Hub* hub)
{
  if (hub->allModulesAreOpen) {
    hideHub(hub);
  } else {
    showHub(hub);
  }
}

void showModuleUpdate(Hub* hub, ModuleType moduleType)
{
  for (size_t i = 0; i < hub->moduleCount; ++i) {
    Module* module = &hub->modules[i];
    if (module->type == moduleType) {
      alertModuleWithDirector(hub->moduleDirector, module);
    }
  }
}

Module* addModuleToHub(Hub* hub)
{
  // Allocate
  hub->modules = realloc(hub->modules, sizeof(Module) * ++hub->moduleCount);

  // Init
  initModule(&hub->modules[hub->moduleCount - 1], hub->appConfig);

  // Return
  return &hub->modules[hub->moduleCount - 1];
}

void updateSurfaceRect(Hub* hub, Rect rect)
{
  setSurfaceSize(hub->surface,
                 MAX(1, (unsigned int)rect.width),
                 MAX(1, (unsigned int)rect.height));
  setSurfacePosition(hub->surface, (int)floor(rect.x), (int)floor(rect.y));
}
