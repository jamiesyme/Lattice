#pragma once

// Lattice has a server part (lattice) and a client part (lattice-msg). They
// communicate by radio, which uses a TCP socket internally.

// Messages are sent and received in the RadioMsg format. Below are all the
// possible commands that can be sent by radio.
typedef enum {
  RMSG_HIDE_ALL,    // No data
  RMSG_SHOW_ALL,    // No data
  RMSG_SHOW_UPDATE, // data = single ModuleType
  RMSG_STOP,        // No data
  RMSG_TOGGLE_ALL   // No data
} RadioMsgType;

// When sending a radio message with any type that specifies no data, be sure to
// set data to NULL and length to 0.
typedef struct RadioMsg {
  RadioMsgType type;
  void* data;
  size_t length;
} RadioMsg;

// For any radio message received with waitForRadioMsg() or pollForRadioMsg(),
// call this to free the data field.
void freeRadioMsg(RadioMsg* msg);


// A radio receiver must be created to receive any messages. There is currently
// no way to specify which port to open the socket on, so creating two radio
// receivers will have undefined behaviour.
typedef struct RadioReceiver RadioReceiver;

RadioReceiver* newRadioReceiver();
void freeRadioReceiver(RadioReceiver* receiver);

// Blocks until a valid radio msg is received. Radio message is saved to msg.
// Remember to free msg with freeRadioMsg() when you're done with it.
void waitForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);

// If a valid radio msg is available, it will be saved to msg and non-zero will
// be returned. Does not block. Remember to free msg with freeRadioMsg() when
// you're done with it.
int pollForRadioMsg(RadioReceiver* receiver, RadioMsg* msg);


// lattice-msg will virtually always send one message per run, so no radio
// transmitter object is used to save the connection between calls. Instead,
// sendRadioMsg() will open a connection, send the message, and then close.
// Non-zero will be returned upon success.
// TODO: Consider using a UDP socket instead of TCP. I've never used them
// before, but this sounds like a solid use case for them.
int sendRadioMsg(RadioMsg msg);
