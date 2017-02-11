#pragma once

typedef enum {
  RMSG_HIDE_ALL,
  RMSG_SHOW_ALL,
  RMSG_SHOW_UPDATE,
  RMSG_STOP,
  RMSG_TOGGLE_ALL
} RadioMsgType;

typedef struct RadioMsg {
  RadioMsgType type;
  void* data;
  size_t length;
} RadioMsg;

void freeRadioMsg(RadioMsg* msg);


typedef struct RadioReceiver RadioReceiver;

RadioReceiver* newRadioReceiver();

void freeRadioReceiver(RadioReceiver* receiver);

void waitForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);

int pollForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);


void sendRadioMsg(RadioMsg msg);
