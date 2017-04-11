#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "radio.h"

#define PORT 6006


typedef struct RadioReceiver {
  int sockFd;
  int isBlocking;
} RadioReceiver;


static void setBlocking(RadioReceiver* receiver, int blocking);

static int readRadioMsg(int sockFd, RadioMsg* msgOut);
static int readBytes(int sockFd, void* buffer, size_t count);
static int writeBytes(int sockFd, void* buffer, size_t count);

void freeRadioMsg(RadioMsg* msg)
{
  free(msg->data);
}

RadioReceiver* newRadioReceiver()
{
  int sockFd, reuse;
  struct sockaddr_in addr;

  reuse = 1;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Create the socket
  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockFd == -1) {
    printf("Could not create socket. errno=%i\n", errno);
    return NULL;
  }

  // Bind the socket
  if (bind(sockFd, (struct sockaddr*)&addr, sizeof addr) == -1) {
    printf("Could not bind socket. errno=%i\n", errno);
    close(sockFd);
    return NULL;
  }

  // Set the socket options so the port can be re-used.
  // This is useful when minfo is being started and stopped in quick succession
  // during development, so we don't have to wait for the TCP timeout.
  if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse) == -1 ||
      setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof reuse) == -1) {
    printf("Could not set socket options. errno=%i\n", errno);
    close(sockFd);
    return NULL;
  }

  // Listen on the socket
  if (listen(sockFd, 20) == -1) {
    printf("Could not listen on socket. errno=%i\n", errno);
    close(sockFd);
    return NULL;
  }

  // The socket was set up successfully, now we can allocate the receiver
  RadioReceiver* receiver = malloc(sizeof(RadioReceiver));
  receiver->sockFd = sockFd;
  receiver->isBlocking = 1;
  return receiver;
}

void freeRadioReceiver(RadioReceiver* receiver)
{
  close(receiver->sockFd);
  free(receiver);
}

void waitForRadioMsg(RadioReceiver* receiver, RadioMsg* msgOut)
{
  int sockFd;
  struct sockaddr_in addr;
  socklen_t addrSize;

  // waitForRadioMsg() should block until a new message is received, so we will
  // set the receiver socket to blocking.
  setBlocking(receiver, 1);

  addrSize = sizeof addr;
  while (1) {

    // Wait for a new client to connect
    sockFd = accept(receiver->sockFd, (struct sockaddr*)&addr, &addrSize);
    if (sockFd == -1) {
      printf("Could not accept on socket. errno=%i\n", errno);
      // TODO: Limit the max retries before returning an error
      continue;
    }

    // Get the message from the client.
    // If the client disconnects or sends an invalid message, readRadioMsg()
    // will return zero.
    if (readRadioMsg(sockFd, msgOut)) {
      close(sockFd);
      return;
    }

    // We didn't get a valid message, so disconnect from the client and try
    // again.
    close(sockFd);
  }
}

int pollForRadioMsg(RadioReceiver* receiver, RadioMsg* msgOut)
{
  int sockFd;
  struct sockaddr_in addr;
  socklen_t addrSize;

  addrSize = sizeof addr;
  while (1) {

    // pollForRadioMsg() only reads a message if one is available, so set the
    // receiving socket to non-blocking.
    setBlocking(receiver, 0);

    // Check if a new client is connecting
    sockFd = accept(receiver->sockFd, (struct sockaddr*)&addr, &addrSize);
    if (sockFd == -1) {

      // If no client was found, return false
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
      }

      // Otherwise, this is an error
      printf("Could not accept on socket. errno=%i\n", errno);
      return 0;
    }

    // Now that we've accepted a new client, we will set the receiving socket to
    // blocking so that we can wait for a new message from that client.
    setBlocking(receiver, 1);

    // Wait for the client's message.
    // If the client disconnects or sends an invalid message, readRadioMsg()
    // will return zero.
    if (readRadioMsg(sockFd, msgOut)) {
      close(sockFd);
      return 1;
    }

    // We didn't get a valid message, so disconnect from the client and try
    // again.
    close(sockFd);
  }
}

int sendRadioMsg(RadioMsg msg)
{
  int sockFd;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Create the socket
  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockFd == -1) {
    printf("Could not create socket. errno=%i\n", errno);
    return 0;
  }

  // Connect the socket
  if (connect(sockFd, (struct sockaddr*)&addr, sizeof addr) == -1) {
    printf("Could not connect with socket. errno=%i\n", errno);
    return 0;
  }

  // Send the message on the socket
  if (!writeBytes(sockFd, &msg.type, sizeof msg.type) ||
      !writeBytes(sockFd, &msg.length, sizeof msg.length) ||
      !writeBytes(sockFd, msg.data, msg.length)) {
    printf("Could not send message on socket. errno=%i\n", errno);
    return 0;
  }

  // We're done
  close(sockFd);
  return 1;
}

static void setBlocking(RadioReceiver* receiver, int blocking)
{
  if (receiver->isBlocking != blocking) {
    int flags = fcntl(receiver->sockFd, F_GETFL, 0);
    if (blocking) {
      flags = flags & ~O_NONBLOCK;
    } else {
      flags = flags | O_NONBLOCK;
    }
    fcntl(receiver->sockFd, F_SETFL, flags);
    receiver->isBlocking = blocking;
  }
}

static int readRadioMsg(int sockFd, RadioMsg* msgOut)
{
  // Get the radio message type and length (these parts of the message fixed in
  // size).
  if (!readBytes(sockFd, (void*)&msgOut->type, sizeof msgOut->type) ||
      !readBytes(sockFd, (void*)&msgOut->length, sizeof msgOut->length)) {
    return 0;
  }

  // If there is extra data with the message, read that too
  msgOut->data = NULL;
  if (msgOut->length > 0) {

    // First we have to allocate the data
    msgOut->data = malloc(msgOut->length);

    // Now pull the data from the socket
    if (!readBytes(sockFd, msgOut->data, msgOut->length)) {
      free(msgOut->data);
      return 0;
    }
  }
  return 1;
}

static int readBytes(int sockFd, void* buffer, size_t count)
{
  size_t bytesRead = 0;
  ssize_t result;
  while (bytesRead < count) {
    result = read(sockFd, buffer + bytesRead, count - bytesRead);
    if (result < 1) {
      if (result == 0) {
        printf("Could not read from socket. Zero returned.\n");
        return 0;
      } else {
        printf("Could not read from socket. errno=%i\n", errno);
        return 0;
      }
    }
    bytesRead += result;
  }
  return 1;
}

static int writeBytes(int sockFd, void* buffer, size_t count)
{
  size_t bytesWritten = 0;
  ssize_t result;
  while (bytesWritten < count) {
    result = write(sockFd, buffer + bytesWritten, count - bytesWritten);
    if (result < 1) {
      if (result == 0) {
        printf("Could not write on socket. Zero returned.\n");
        return 0;
      } else {
        printf("Could not write on socket. errno=%i\n", errno);
        return 0;
      }
    }
    bytesWritten += result;
  }
  return 1;
}
