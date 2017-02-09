#include <errno.h>
#include <time.h>

#include "time-utils.h"

#define S_TO_MS 1000
#define S_TO_NS 1000000000
#define MS_TO_S 1 / 1000
#define MS_TO_NS 1000000
#define NS_TO_S 1 / 1000000000
#define NS_TO_MS 1 / 1000000


static Milliseconds timespecToMilli(struct timespec t)
{
  return (t.tv_sec * S_TO_NS + t.tv_nsec) * NS_TO_MS;
}

Milliseconds getTimeInMilliseconds()
{
  struct timespec timeSpec;
  clock_gettime(CLOCK_MONOTONIC, &timeSpec);
  return timespecToMilli(timeSpec);
}

void sleepForMilliseconds(Milliseconds ms)
{
  if (ms <= 0) {
    return;
  }
  struct timespec req, rem;
  req.tv_sec = ms * MS_TO_S;
  req.tv_nsec = (ms % S_TO_MS) * MS_TO_NS;
  while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
    req = rem;
  }
}
