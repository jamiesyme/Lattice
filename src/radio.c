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

  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  bind(sockFd, (struct sockaddr*)&addr, sizeof addr);
  setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
  setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof reuse);
  listen(sockFd, 20);

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

  setBlocking(receiver, 1);

  addrSize = sizeof addr;
  while (1) {
    sockFd = accept(receiver->sockFd, (struct sockaddr*)&addr, &addrSize);
    if (readRadioMsg(sockFd, msgOut) == 0) {
      close(sockFd);
      return;
    }
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
    setBlocking(receiver, 0);
    sockFd = accept(receiver->sockFd, (struct sockaddr*)&addr, &addrSize);
    if (sockFd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
      }
      printf("failed to accept socket - %i\n", errno);
      return 0;
    }
    setBlocking(receiver, 1);
    if (readRadioMsg(sockFd, msgOut) == 0) {
      close(sockFd);
      return 1;
    }
    close(sockFd);
  }
}

void sendRadioMsg(RadioMsg msg)
{
  int sockFd;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  connect(sockFd, (struct sockaddr*)&addr, sizeof addr);
  writeBytes(sockFd, &msg.type, sizeof msg.type);
  writeBytes(sockFd, &msg.length, sizeof msg.length);
  writeBytes(sockFd, msg.data, msg.length);
  close(sockFd);
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
  if (readBytes(sockFd, (void*)&msgOut->type, sizeof msgOut->type) != 0) {
    return 1;
  }
  if (readBytes(sockFd, (void*)&msgOut->length, sizeof msgOut->length) != 0) {
    return 1;
  }
  if (msgOut->length != 0) {
    msgOut->data = malloc(msgOut->length);
    if (readBytes(sockFd, msgOut->data, msgOut->length) != 0) {
      free(msgOut->data);
      return 1;
    }
  } else {
    msgOut->data = NULL;
  }
  return 0;
}

static int readBytes(int sockFd, void* buffer, size_t count)
{
  size_t bytesRead = 0;
  ssize_t result;
  while (bytesRead < count) {
    result = read(sockFd, buffer + bytesRead, count - bytesRead);
    if (result < 1) {
      if (result == 0) {
        printf("failed to read from socket - socket closed\n");
        return 1;
      } else {
        printf("failed to read from socket - %i\n", errno);
        return 2;
      }
    }
    bytesRead += result;
  }
  return 0;
}

static int writeBytes(int sockFd, void* buffer, size_t count)
{
  size_t bytesWritten = 0;
  ssize_t result;
  while (bytesWritten < count) {
    result = write(sockFd, buffer + bytesWritten, count - bytesWritten);
    if (result < 1) {
      if (result == 0) {
        printf("failed to write to socket - write() returned 0\n");
        return 1;
      } else {
        printf("failed to write to socket - %i\n", errno);
        return 2;
      }
    }
    bytesWritten += result;
  }
  return 0;
}
