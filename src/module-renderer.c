#include <cairo/cairo.h>
#include <stdlib.h>

#include "app-config.h"
#include "geometry-utils.h"
#include "module.h"
#include "module-renderer.h"
#include "surface.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))


typedef struct ModuleRenderer {
  // TODO: descriptions.

  AppConfig* appConfig;

  Module** modules;
  Module** sortedModules;
  size_t moduleCount;

} ModuleRenderer;

static void updateSortedModules(ModuleRenderer* renderer);


ModuleRenderer* newModuleRenderer(AppConfig* appConfig)
{
  ModuleRenderer* renderer = malloc(sizeof(ModuleRenderer));
  renderer->appConfig = appConfig;

  renderer->modules = NULL;
  renderer->sortedModules = NULL;
  renderer->moduleCount = 0;

  return renderer;
}

void freeModuleRenderer(ModuleRenderer* renderer)
{
  free(renderer->modules);
  free(renderer->sortedModules);
  free(renderer);
}

void addModuleToRenderer(ModuleRenderer* renderer, Module* module)
{
  size_t newSize = sizeof(Module*) * ++renderer->moduleCount;
  renderer->modules = realloc(renderer->modules, newSize);
  renderer->modules[renderer->moduleCount - 1] = module;
  renderer->sortedModules = realloc(renderer->sortedModules, newSize);
  renderer->sortedModules[renderer->moduleCount - 1] = NULL;
}

void renderModules(ModuleRenderer* renderer, Surface* surface)
{
  // When rendering, we need to sort the modules by depth, so modules with a
  // higher depth are renderered later.
  updateSortedModules(renderer);

  // Render all the modules
  cairo_t* cr = getCairoContext(surface);
  for (size_t i = 0; i < renderer->moduleCount; ++i) {
    Module* module = renderer->sortedModules[i];

    // Check for a render function
    if (module->renderFunc == 0) {
      continue;
    }

    // Check if the module is on-screen
    Rect moduleRect = module->rect;
    if (moduleRect.x >= 0 || moduleRect.y >= 0) {
      continue;
    }

    // Position the draw origin with the modules position
    cairo_save(cr);
    cairo_translate(cr, moduleRect.x, moduleRect.y);

    // Draw the module border
    int borderSize = (int)renderer->appConfig->moduleBorderSize;
    Color borderColor = renderer->appConfig->moduleBorderColor;
    setDrawColor(surface, borderColor);
    drawRect4(surface, 0, 0, moduleRect.width, moduleRect.height);
    cairo_translate(cr, borderSize, borderSize);

    // Draw the module background
    int paddingWidth = (int)renderer->appConfig->modulePaddingSize.width;
    int paddingHeight = (int)renderer->appConfig->modulePaddingSize.height;
    int backgroundWidth = (int)moduleRect.width - borderSize * 2;
    int backgroundHeight = (int)moduleRect.height - borderSize * 2;
    Color backgroundColor = renderer->appConfig->moduleBackgroundColor;
    setDrawColor(surface, backgroundColor);
    drawRect4(surface, 0, 0, backgroundWidth, backgroundHeight);
    cairo_translate(cr, paddingWidth, paddingHeight);

    // Draw the module
    module->renderFunc(module, surface);
    cairo_restore(cr);
  }
}

int isModuleRendererBusy(ModuleRenderer* renderer)
{
  // The renderer is busy if there are on-screen modules to render; a module is
  // on-screen if the position is negative, as the position is a negative offset
  // from the edge of the screen.
  Rect rect = getModuleRendererRect(renderer);
  return rect.x < 0 && rect.y < 0;
}

Rect getModuleRendererRect(ModuleRenderer* renderer)
{
  if (renderer->moduleCount == 0) {
    return (Rect){0, 0, 0, 0};
  }

  float minX = renderer->modules[0]->rect.x;
  float maxX = minX + renderer->modules[0]->rect.width;
  float minY = renderer->modules[0]->rect.y;
  float maxY = minY + renderer->modules[0]->rect.height;

  for (size_t i = 1; i < renderer->moduleCount; ++i) {
    Module* module = renderer->modules[i];
    minX = MIN(minX, module->rect.x);
    maxX = MAX(maxX, module->rect.x + module->rect.width);
    minY = MIN(minY, module->rect.y);
    maxY = MAX(maxY, module->rect.y + module->rect.height);
  }

  return (Rect){minX, minY, maxX - minX, maxY - minY};
}

void updateSortedModules(ModuleRenderer* renderer)
{
  for (size_t i = 0; i < renderer->moduleCount; ++i) {
    Module* unsortedModule = renderer->modules[i];

    // Try to insert this module somewhere in the sorted list
    for (size_t j = 0; j < i; ++j) {

      // Check if the depth is lower than the current module
      if (unsortedModule->depth <
          renderer->sortedModules[j]->depth) {

        // We need to insert this module at `j`, so we need to shuffle all of
        // the modules after `j` one to the right
        for (size_t k = i; k > j; --k) {
          renderer->sortedModules[k] = renderer->sortedModules[k - 1];
        }

        // Insert the module
        renderer->sortedModules[j] = unsortedModule;
        unsortedModule = NULL;
        break;
      }
    }

    // If the module wasn't inserted, push it at the end
    if (unsortedModule != NULL) {
      renderer->sortedModules[i] = unsortedModule;
    }
  }
}
