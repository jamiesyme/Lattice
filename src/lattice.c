#include <math.h>
#include <stdio.h>

#include "app-config.h"
#include "geometry-utils.h"
#include "fps-limiter.h"
#include "hub.h"
#include "linux-window.h"
#include "module-type.h"
#include "radio.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))


typedef struct App {
  AppConfig config;

	// Manages the Xlib connection and cairo context
	LinuxWindow* window;
	
  // Used to cap the frame rate at 60 fps
  FpsLimiter* fpsLimiter;

  // Manages and renders all of the modules
  Hub* hub;

  // Listens for messages from lattice-msg
  RadioReceiver* radio;

  // This flag is set after receiving a RMSG_STOP message
  int shouldQuit;
} App;


static void processMsg(App* app, RadioMsg msg);

int main()
{
  App app;
  RadioMsg msg;
  Rect hubScreenRect;

  // TODO: Load config from disk
  app.config.moduleAlertDuration = 1500;
  app.config.moduleCloseDelay = 65;
  app.config.moduleCloseMoveDuration = 500;
  app.config.moduleCloseMoveMethod = IM_QUINTIC_EASE_OUT;
  app.config.moduleBackgroundColor = (Color){1.0f, 1.0f, 1.0f, 1.0f};
  app.config.moduleBorderColor = (Color){0.0f, 0.0f, 0.0f, 1.0f};
  app.config.moduleBorderSize = 2.0f;
  app.config.moduleLowerMoveDuration = 200;
  app.config.moduleLowerMoveMethod = IM_LINEAR;
  app.config.moduleMarginSize = 25;
  app.config.moduleMoveDuration = 200;
  app.config.moduleMoveMethod = IM_LINEAR;
  app.config.moduleOpenDelay = 65;
  app.config.moduleOpenMoveDuration = 500;
  app.config.moduleOpenMoveMethod = IM_QUINTIC_EASE_OUT;
  app.config.modulePaddingSize = (Dimensions){75.0f, 25.0f};
  app.config.moduleRaiseMoveDuration = 200;
  app.config.moduleRaiseMoveMethod = IM_LINEAR;
  app.config.windowOffset = (Point){0.0f, 25.0f};

  // Initialize the app
  app.window = newLinuxWindow(1, 1);
  app.fpsLimiter = newFpsLimiter(60);
  app.hub = newHub(&app.config);
  app.radio = newRadioReceiver();
  app.shouldQuit = 0;

  // Check for errors
  if (app.radio == NULL) {
    printf("Failed to create radio.\n");
    return 1;
  }

  // We'll keep running until a RMSG_STOP message is received
  while (!app.shouldQuit) {

	  // Draw the hub
	  if (shouldRenderHub(app.hub)) {

		  // Resize the window
		  hubScreenRect = getHubScreenRect(app.hub);
		  setLinuxWindowPosition(app.window,
                             (int)floor(hubScreenRect.x),
                             (int)floor(hubScreenRect.y));
		  setLinuxWindowSize(app.window,
                         MAX(1, (unsigned int)hubScreenRect.width),
                         MAX(1, (unsigned int)hubScreenRect.height));

		  // Render to the window
		  prepareLinuxWindowRender(app.window);
		  renderHub(app.hub, getCairoContextFromLinuxWindow(app.window));
		  finishLinuxWindowRender(app.window);
	  }

	  // Check for radio messages
	  if (shouldRenderHub(app.hub)) {

		  // The hub needs to keep rendering, so check for messages in a
		  // non-blocking way.
      if (pollForRadioMsg(app.radio, &msg)) {
        processMsg(&app, msg);
        freeRadioMsg(&msg);
      }
	  } else {

		  // The hub is finished rendering, so there is nothing to do until we get
		  // another radio message.
		  updateHubBeforeSleep(app.hub);
		  hideLinuxWindow(app.window);

      waitForRadioMsg(app.radio, &msg);

      showLinuxWindow(app.window);
      updateHubAfterSleep(app.hub);

      processMsg(&app, msg);
      freeRadioMsg(&msg);
	  }

    // This will limit our frame rate to a maximum of 60 fps
    limitFps(app.fpsLimiter);
  }

  freeRadioReceiver(app.radio);
  freeHub(app.hub);
  freeFpsLimiter(app.fpsLimiter);
  freeLinuxWindow(app.window);
  return 0;
}

static void processMsg(App* app, RadioMsg msg)
{
  switch (msg.type) {
  case RMSG_ALERT:
    alertHubModule(app->hub, *(ModuleType*)msg.data);
    break;

  case RMSG_HIDE_ALL:
    hideAllHubModules(app->hub);
    break;

  case RMSG_SHOW_ALL:
    showAllHubModules(app->hub);
    break;

  case RMSG_STOP:
    app->shouldQuit = 1;
    break;

  case RMSG_TOGGLE_ALL:
    toggleAllHubModules(app->hub);
    break;

  default:
    printf("unknown msg type - %i - ignoring\n", msg.type);
  }
}
