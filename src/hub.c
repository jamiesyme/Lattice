#include <cairo/cairo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "graphics.h"
#include "hub.h"
#include "module.h"
#include "module-list.h"
#include "surface.h"
#include "time-module.h"


long secondsToNano(time_t seconds)
{
  return (long)seconds * 1000000000;
}

int nanoToMilli(long nanoSeconds)
{
  return (int)(nanoSeconds / 1000000);
}

int timespecToMilli(struct timespec t)
{
  return nanoToMilli(secondsToNano(t.tv_sec) + t.tv_nsec);
}


typedef struct HubCtrlState {
  int shouldShowAll;
  int shouldQuit;
} HubCtrlState;

typedef struct Hub {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  HubCtrlState ctrlState;
  ModuleList* modules;
  unsigned int modulePadding;
  unsigned int screenPadding;
} Hub;


Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  pthread_mutex_init(&hub->lock, 0);
  pthread_cond_init(&hub->cond, 0);
  hub->ctrlState.shouldShowAll = 0;
  hub->ctrlState.shouldQuit = 0;
  hub->modules = newModuleList();
  hub->modulePadding = 25;
  hub->screenPadding = 50;
  addModuleToList(hub->modules, newTimeModule());
	return hub;
}

void freeHub(Hub* hub)
{
  for (size_t i = 0; i < getModuleCountOfList(hub->modules); ++i) {
    Module* module = getModuleFromList(hub->modules, i);
    module->freeFunc(module);
  }
  freeModuleList(hub->modules);
  pthread_cond_destroy(&hub->cond);
  pthread_mutex_destroy(&hub->lock);
	free(hub);
}

int runHub(Hub* hub)
{
  // Create the surface
  Surface* surface;
  {
    // Calculate the width and height
    unsigned int width = 0, height = 0;
    for (size_t i = 0; i < getModuleCountOfList(hub->modules); ++i) {
      Module* module = getModuleFromList(hub->modules, i);
      if (width == 0 || width < module->width) {
        width = module->width;
      }
      if (height == 0 || height < module->height) {
        height = module->height;
      }
    }
    if (width == 0 || height == 0) {
      printf("No modules to display. Terminating.\n");
      return 1;
    }

    // Calculate the position
    // Negative coords are subtracted from the max coord
    int x, y;
    x = -(width + hub->screenPadding);
    y = -(height + hub->screenPadding);

    // Create the surface
    surface = newSurface(x, y, width, height);
  }

  while (1) {

    // Mark the start time so we can sleep later
    struct timespec startTime, endTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    // Save state
    pthread_mutex_lock(&hub->lock);
    HubCtrlState ctrlState = hub->ctrlState;
    pthread_mutex_unlock(&hub->lock);

    // Quit if requested
    if (ctrlState.shouldQuit != 0) {
      break;
    }

    // Clear the screen if hiding all
    if (ctrlState.shouldShowAll == 0) {
      setDrawColor(surface, 0, 1.0, 0, 0.5);
      drawFullRect(surface);

      // Draw the screen if showing all
    } else {
      cairo_t* cr = getCairoContext(surface);
      cairo_save(cr);
      for (size_t i = 0; i < getModuleCountOfList(hub->modules); ++i) {
        Module* module = getModuleFromList(hub->modules, i);
        if (module->updateFunc == 0) {
          continue;
        }
        cairo_save(cr);
        cairo_translate(cr, getSurfaceWidth(surface) - module->width, 0);
        module->updateFunc(module, surface);
        cairo_restore(cr);
        cairo_translate(cr, 0, module->height + hub->modulePadding);
      }
      cairo_restore(cr);
    }
    flushSurface(surface);

    // Wait for a message if we're hidden. This is purely to save resources
    // instead of busy waiting until we either have to quit or show the modules
    // again.
    if (ctrlState.shouldShowAll == 0) {
      pthread_mutex_lock(&hub->lock);
      pthread_cond_wait(&hub->cond, &hub->lock);
      pthread_mutex_unlock(&hub->lock);
      // We don't need to worry about spurious wake-ups. The worst that will
      // happen is we will run through the while loop again and then go back to
      // waiting.

      // If we're not hiding, then we want to cap our framerate at 60 fps
    } else {
      clock_gettime(CLOCK_MONOTONIC, &endTime);
      int elapsed = timespecToMilli(endTime) - timespecToMilli(startTime);
      int maxElapsed = 1000 / 60;
      if (elapsed < maxElapsed) {
        struct timespec sleepFor;
        sleepFor.tv_sec = (maxElapsed - elapsed) / 1000;
        sleepFor.tv_nsec = (maxElapsed - elapsed) % 1000 * 1000000;
        nanosleep(&sleepFor, NULL);
      }
    }
  }

  return 0;
}

void stopHub(Hub* hub)
{
  // We must set the "quit" flag in a thread-safe manner
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldQuit == 0) {
    hub->ctrlState.shouldQuit = 1;
    pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}

void showHubModules(Hub* hub)
{
  // We must set the "show_all" flag in a thread-safe manner
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldShowAll == 0) {
    hub->ctrlState.shouldShowAll = 1;
    pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}

void hideHubModules(Hub* hub)
{
  // We must unset the "show_all" flag in a thread-safe manner
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldShowAll != 0) {
    hub->ctrlState.shouldShowAll = 0;
    // Do NOT signal condition. The hub will never be waiting for a message if
    // it is currently showing a module.
    //pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}
