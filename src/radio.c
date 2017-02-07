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


struct RadioReceiver {
  int sockFd;
  int isBlocking;
};

static int readBytes(int sockFd, unsigned int min, void* buffer);

static void setBlocking(RadioReceiver* receiver, int blocking);


RadioReceiver* newRadioReceiver()
{
  int sockFd;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(6006);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  bind(sockFd, (struct sockaddr*)&addr, sizeof addr);
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
  int status;
  struct sockaddr_in addr;
  socklen_t addrSize;
  RadioMsg msg;

  setBlocking(receiver, 1);

  addrSize = sizeof addr;
  while (1) {
    sockFd = accept(receiver->sockFd, (struct sockaddr*)&addr, &addrSize);
    status = readBytes(sockFd, sizeof msg.type, (void*)&msg.type);
    close(sockFd);
    if (status == 0) {
      *msgOut = msg;
      return;
    }
  }
}

int pollForRadioMsg(RadioReceiver* receiver, RadioMsg* msgOut)
{
  int sockFd;
  int status;
  struct sockaddr_in addr;
  socklen_t addrSize;
  RadioMsg msg;

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
    status = readBytes(sockFd, sizeof msg.type, (void*)&msg.type);
    close(sockFd);
    if (status == 0) {
      *msgOut = msg;
      return 1;
    }
  }
}

void sendRadioMsg(RadioMsg msg)
{
  int sockFd;
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(6006);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  sockFd = socket(PF_INET, SOCK_STREAM, 0);
  connect(sockFd, (struct sockaddr*)&addr, sizeof addr);
  write(sockFd, &msg.type, sizeof msg.type);
  close(sockFd);
}

static int readBytes(int sockFd, unsigned int min, void* buffer)
{
  int bytesRead = 0;
  int result;
  while (bytesRead < min) {
    result = read(sockFd, buffer + bytesRead, min - bytesRead);
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
