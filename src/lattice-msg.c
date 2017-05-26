#include <stdio.h>
#include <string.h>

#include "module-type.h"
#include "radio.h"


static void displayUsage()
{
  printf("Usage:\n");
  printf("  lattice-msg alert (audio|date|time|workspace)\n");
  printf("  lattice-msg hide-all\n");
  printf("  lattice-msg show-all\n");
  printf("  lattice-msg stop\n");
  printf("  lattice-msg toggle-all\n");
}

static void displayAlertUsage()
{
  printf("Usage:\n");
  printf("  lattice-msg alert (audio|date|time|workspace)\n");
}

static int sendAlert(const char* moduleStr)
{
  if (moduleStr == NULL) {
    displayAlertUsage();
    return 1;
  }

  ModuleType mt = strToModuleType(moduleStr);
  if (mt == MT_UNKNOWN) {
    printf("There is no module called \"%s\"\n\n", moduleStr);
    displayAlertUsage();
    return 1;
  }

  RadioMsg msg;
  msg.type = RMSG_ALERT;
  msg.data = &mt;
  msg.length = sizeof mt;
  sendRadioMsg(msg);
  return 0;
}

static int sendHideAll()
{
  RadioMsg msg;
  msg.type = RMSG_HIDE_ALL;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

static int sendShowAll()
{
  RadioMsg msg;
  msg.type = RMSG_SHOW_ALL;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

static int sendStop()
{
  RadioMsg msg;
  msg.type = RMSG_STOP;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

static int sendToggleAll()
{
  RadioMsg msg;
  msg.type = RMSG_TOGGLE_ALL;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

int main(int argc, char** argv)
{
  if (argc < 2 ||
      strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0) {
    displayUsage();
    return 0;
  }

  if (strcmp(argv[1], "hide-all") == 0) {
    return sendHideAll();
  }

  if (strcmp(argv[1], "show-all") == 0) {
    return sendShowAll();
  }

  if (strcmp(argv[1], "alert") == 0) {
    return sendAlert(argc >= 3 ? argv[2] : NULL);
  }

  if (strcmp(argv[1], "stop") == 0) {
    return sendStop();
  }

  if (strcmp(argv[1], "toggle-all") == 0) {
    return sendToggleAll();
  }

  printf("There is no command called \"%s\"\n\n", argv[1]);
  displayUsage();
  return 1;
}
