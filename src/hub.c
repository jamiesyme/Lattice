#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "graphics.h"
#include "hub.h"
#include "surface.h"


typedef struct HubCtrlState {
  int shouldShowAll;
  int shouldQuit;
} HubCtrlState;

typedef struct Hub {
  pthread_mutex_t lock;
  pthread_cond_t cond;
  HubCtrlState ctrlState;
} Hub;


Hub* newHub()
{
  Hub* hub = malloc(sizeof(Hub));
  pthread_mutex_init(&hub->lock, 0);
  pthread_cond_init(&hub->cond, 0);
  hub->ctrlState.shouldShowAll = 0;
  hub->ctrlState.shouldQuit = 0;
	return hub;
}

void freeHub(Hub* hub)
{
  pthread_cond_destroy(&hub->cond);
  pthread_mutex_destroy(&hub->lock);
	free(hub);
}

int runHub(Hub* hub)
{
  // Create the window
  Surface* surface = newSurface(100, 100);

  while (1) {

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
      printf("hiding\n");

      setDrawColor(surface, 0, 1.0, 0, 0.5);
      drawFullRect(surface);

      // Draw the screen if showing all
    } else {
      printf("showing\n");

      setDrawColor(surface, 1.0, 0.0, 0.0, 0.5);
      drawFullRect(surface);

      setDrawColor(surface, 0, 0, 1, 1);
      drawText(surface, 10, 10, 32, "monaco", "Good");
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
    }
  }

  return 0;
}

void stopHub(Hub* hub)
{
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldQuit == 0) {
    hub->ctrlState.shouldQuit = 1;
    pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}

void showHubModules(Hub* hub)
{
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldShowAll == 0) {
    hub->ctrlState.shouldShowAll = 1;
    pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}

void hideHubModules(Hub* hub)
{
  pthread_mutex_lock(&hub->lock);
  if (hub->ctrlState.shouldShowAll != 0) {
    hub->ctrlState.shouldShowAll = 0;
    // Do NOT signal condition. The hub will never be waiting for a message if
    // it is currently showing a module.
    //pthread_cond_signal(&hub->cond);
  }
  pthread_mutex_unlock(&hub->lock);
}
