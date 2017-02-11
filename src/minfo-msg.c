#include <stdio.h>
#include <string.h>

#include "module-type.h"
#include "radio.h"


int main(int argc, char** argv)
{
  RadioMsg msg;
  msg.data = NULL;
  msg.length = 0;

  if (argc < 2 ||
      strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0) {
    printf("TODO: print help\n");
    return 0;
  }

  if (strcmp(argv[1], "hide") == 0) {
    msg.type = RMSG_HIDE_ALL;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "show") == 0) {
    msg.type = RMSG_SHOW_ALL;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "show-update") == 0) {
    if (argc < 3) {
      printf("TODO: print show-update usage\n");
      return 0;
    }
    ModuleType mt = strToModuleType(argv[2]);
    if (mt == MT_UNKNOWN) {
      printf("unknown module: %s\n", argv[2]);
      return 1;
    }
    msg.type = RMSG_SHOW_UPDATE;
    msg.data = &mt;
    msg.length = sizeof mt;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "stop") == 0) {
    msg.type = RMSG_STOP;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "toggle") == 0) {
    msg.type = RMSG_TOGGLE_ALL;
    sendRadioMsg(msg);
    return 0;
  }

  printf("unknown command: %s\n", argv[1]);
  return 1;
}
