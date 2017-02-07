#pragma once

typedef enum {
  RMSG_HIDE_ALL,
  RMSG_SHOW_ALL,
  RMSG_STOP
} RadioMsgType;

typedef struct RadioMsg {
  RadioMsgType type;
} RadioMsg;


typedef struct RadioReceiver RadioReceiver;

RadioReceiver* newRadioReceiver();

void freeRadioReceiver(RadioReceiver* receiver);

void waitForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);

int pollForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);


void sendRadioMsg(RadioMsg msg);
