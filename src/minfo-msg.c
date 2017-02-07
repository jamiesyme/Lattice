#include <stdio.h>
#include <string.h>
#include "radio.h"

int main(int argc, char** argv)
{
  RadioMsg msg;

  if (argc < 2 ||
      strcmp(argv[1], "-h") == 0 ||
      strcmp(argv[1], "--help") == 0) {
    printf("TODO: print help\n");
    return 0;
  }

  if (strcmp(argv[1], "toggle") == 0) {
    printf("TODO: toggle\n");
    return 0;
  }

  if (strcmp(argv[1], "show") == 0) {
    msg.type = RMSG_SHOW_ALL;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "hide") == 0) {
    msg.type = RMSG_HIDE_ALL;
    sendRadioMsg(msg);
    return 0;
  }

  if (strcmp(argv[1], "stop") == 0) {
    msg.type = RMSG_STOP;
    sendRadioMsg(msg);
    return 0;
  }

  printf("unknown command: %s\n", argv[1]);
  return 1;
}
