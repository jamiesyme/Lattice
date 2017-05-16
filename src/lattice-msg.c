#include <stdio.h>
#include <string.h>

#include "module-type.h"
#include "radio.h"


static void displayUsage()
{
  printf("Usage:\n");
  printf("  lattice-msg hide\n");
  printf("  lattice-msg show\n");
  printf("  lattice-msg show-update (audio|date|time|workspace)\n");
  printf("  lattice-msg stop\n");
  printf("  lattice-msg toggle\n");
}

static void displayShowUpdateUsage()
{
  printf("Usage:\n");
  printf("  lattice-msg show-update (audio|date|time|workspace)\n");
}

static int sendHide()
{
  RadioMsg msg;
  msg.type = RMSG_HIDE_ALL;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

static int sendShow()
{
  RadioMsg msg;
  msg.type = RMSG_SHOW_ALL;
  msg.data = NULL;
  msg.length = 0;
  sendRadioMsg(msg);
  return 0;
}

static int sendShowUpdate(const char* moduleStr)
{
  if (moduleStr == NULL) {
    displayShowUpdateUsage();
    return 1;
  }

  ModuleType mt = strToModuleType(moduleStr);
  if (mt == MT_UNKNOWN) {
    printf("There is no module called \"%s\"\n\n", moduleStr);
    displayShowUpdateUsage();
    return 1;
  }

  RadioMsg msg;
  msg.type = RMSG_SHOW_UPDATE;
  msg.data = &mt;
  msg.length = sizeof mt;
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

static int sendToggle()
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

  if (strcmp(argv[1], "hide") == 0) {
    return sendHide();
  }

  if (strcmp(argv[1], "show") == 0) {
    return sendShow();
  }

  if (strcmp(argv[1], "show-update") == 0) {
    return sendShowUpdate(argc >= 3 ? argv[2] : NULL);
  }

  if (strcmp(argv[1], "stop") == 0) {
    return sendStop();
  }

  if (strcmp(argv[1], "toggle") == 0) {
    return sendToggle();
  }

  printf("There is no command called \"%s\"\n\n", argv[1]);
  displayUsage();
  return 1;
}
