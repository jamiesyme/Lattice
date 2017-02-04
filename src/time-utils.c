#include <errno.h>
#include <time.h>

static long secondsToNano(time_t seconds)
{
  return (long)seconds * 1000000000;
}

static long nanoToMilli(long nanoSeconds)
{
  return nanoSeconds / 1000000;
}

static long timespecToMilli(struct timespec t)
{
  return nanoToMilli(secondsToNano(t.tv_sec) + t.tv_nsec);
}

long getTimeInMilliseconds()
{
  struct timespec timeSpec;
  clock_gettime(CLOCK_MONOTONIC, &timeSpec);
  return timespecToMilli(timeSpec);
}

void sleepForMilliseconds(long ms)
{
  if (ms <= 0) {
    return;
  }
  struct timespec req, rem;
  req.tv_sec = ms / 1000;
  req.tv_nsec = ms % 1000 * 1000000;
  while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
    req = rem;
  }
}
