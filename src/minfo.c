#include <stdio.h>

#include "frame-limiter.h"
#include "hub.h"
#include "module-type.h"
#include "radio.h"


typedef struct App {
  // Used to cap the framerate at 60 fps
  FrameLimiter* frameLimiter;

  // Manages and renders all of the modules
  Hub* hub;

  // Listens for messages from minfo-msg
  RadioReceiver* radio;

  // This flag is set after receiving a RMSG_STOP message
  int shouldQuit;
} App;

static void processMsg(App* app, RadioMsg msg);


int main()
{
  App app;
  RadioMsg msg;

  app.frameLimiter = newFrameLimiter(60);
  app.hub = newHub();
  app.radio = newRadioReceiver();
  app.shouldQuit = 0;

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
      processMsg(&app, msg);
      freeRadioMsg(&msg);
    }

    // This will limit our framerate to a maximum of 60 fps
    applyFrameLimiter(app.frameLimiter);
  }

  freeRadioReceiver(app.radio);
  freeHub(app.hub);
  freeFrameLimiter(app.frameLimiter);
  return 0;
}

static void processMsg(App* app, RadioMsg msg)
{
  switch (msg.type) {
  case RMSG_HIDE_ALL:
    hideHub(app->hub);
    break;

  case RMSG_SHOW_ALL:
    showHub(app->hub);
    break;

  case RMSG_SHOW_UPDATE:
    showModuleUpdate(app->hub, *(ModuleType*)msg.data);
    break;

  case RMSG_STOP:
    app->shouldQuit = 1;
    break;

  case RMSG_TOGGLE_ALL:
    toggleHub(app->hub);
    break;

  default:
    printf("unknown msg type - %i - ignoring\n", msg.type);
  }
}
