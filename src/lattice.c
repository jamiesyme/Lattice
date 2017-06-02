#include <stdio.h>

#include "app-config.h"
#include "fps-limiter.h"
#include "hub.h"
#include "module-type.h"
#include "radio.h"


typedef struct App {
  AppConfig config;

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

  // TODO: Load config from disk
  app.config.moduleAlertDuration = 1500;
  app.config.moduleCloseDelay = 100;
  app.config.moduleCloseMoveDuration = 200;
  app.config.moduleCloseMoveMethod = IM_LINEAR;
  app.config.moduleBackgroundColor = (Color){1.0f, 1.0f, 1.0f, 1.0f};
  app.config.moduleBorderColor = (Color){0.0f, 0.0f, 0.0f, 1.0f};
  app.config.moduleBorderSize = 2.0f;
  app.config.moduleLowerMoveDuration = 200;
  app.config.moduleLowerMoveMethod = IM_LINEAR;
  app.config.moduleMarginSize = 25;
  app.config.moduleMoveDuration = 200;
  app.config.moduleMoveMethod = IM_LINEAR;
  app.config.moduleOpenDelay = 100;
  app.config.moduleOpenMoveDuration = 200;
  app.config.moduleOpenMoveMethod = IM_LINEAR;
  app.config.modulePaddingSize = (Dimensions){75.0f, 25.0f};
  app.config.moduleRaiseMoveDuration = 200;
  app.config.moduleRaiseMoveMethod = IM_LINEAR;
  app.config.windowOffset = (Point){0.0f, 25.0f};

  // Initialize the app
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

    // We ask the hub if it needs to do any rendering. If it does, we'll render
    // it, and then check for any new messages from the radio.
    if (shouldRenderHub(app.hub)) {
      renderHub(app.hub);

      // Check for messages in a non-blocking way
      if (pollForRadioMsg(app.radio, &msg)) {
        processMsg(&app, msg);
        freeRadioMsg(&msg);
      }
    } else {
      // Since the hub doesn't need to do any rendering (all the modules are
      // MS_OFF), we can check for messages in a blocking way. This will allow
      // our application to sleep until the user sends another request.
      waitForRadioMsg(app.radio, &msg);

      // After sleeping, we have to let the hub know
      updateHubAfterSleep(app.hub);

      // Then we can process the message as normal
      processMsg(&app, msg);
      freeRadioMsg(&msg);
    }

    // This will limit our frame rate to a maximum of 60 fps
    limitFps(app.fpsLimiter);
  }

  freeRadioReceiver(app.radio);
  freeHub(app.hub);
  freeFpsLimiter(app.fpsLimiter);
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
