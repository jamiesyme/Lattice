#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "frame-limiter.h"


typedef struct FrameLimiter {
  int targetFps;
  long lastTimeMs;
} FrameLimiter;


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

static long getTimeInMilliseconds()
{
  struct timespec timeSpec;
  clock_gettime(CLOCK_MONOTONIC, &timeSpec);
  return timespecToMilli(timeSpec);
}

static void sleepForMilliseconds(long ms)
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

FrameLimiter* newFrameLimiter(int targetFps)
{
  FrameLimiter* limiter = malloc(sizeof(FrameLimiter));
  limiter->targetFps = targetFps;
  limiter->lastTimeMs = getTimeInMilliseconds();
  return limiter;
}

void freeFrameLimiter(FrameLimiter* limiter)
{
  free(limiter);
}

void applyFrameLimiter(FrameLimiter* limiter)
{
  long startTime = limiter->lastTimeMs;
  long endTime = getTimeInMilliseconds();
  long elapsed = startTime - endTime;
  long maxElapsed = 1000 / limiter->targetFps;
  sleepForMilliseconds(maxElapsed - elapsed);
  limiter->lastTimeMs = getTimeInMilliseconds();
}
