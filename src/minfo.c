#include <stdio.h>

#include "hub.h"
#include "radio.h"


typedef struct App {
  Hub* hub;
  RadioReceiver* radio;
  int shouldQuit;
} App;

static void processMsg(App* app, RadioMsg msg);


int main()
{
  App app;
  RadioMsg msg;

  app.hub = newHub();
  app.radio = newRadioReceiver();
  app.shouldQuit = 0;

  while (!app.shouldQuit) {
    if (shouldUpdateHub(app.hub)) {
      updateHub(app.hub);
      if (pollForRadioMsg(app.radio, &msg)) {
        processMsg(&app, msg);
      }
    } else {
      waitForRadioMsg(app.radio, &msg);
      processMsg(&app, msg);
    }
  }

  freeRadioReceiver(app.radio);
  freeHub(app.hub);
  return 0;
}

static void processMsg(App* app, RadioMsg msg)
{
  switch (msg.type) {
  case RMSG_SHOW_ALL:
    printf("showing all\n");
    showHubModules(app->hub);
    break;

  case RMSG_HIDE_ALL:
    printf("hiding all\n");
    hideHubModules(app->hub);
    break;

  case RMSG_STOP:
    printf("stopping\n");
    app->shouldQuit = 1;
    break;

  default:
    printf("unknown msg type - %i - ignoring\n", msg.type);
  }
}
