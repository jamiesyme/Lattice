#include <stdlib.h>

#include "frame-limiter.h"
#include "time-utils.h"


typedef struct FrameLimiter {
  int targetFps;
  Milliseconds lastTimeMs;
} FrameLimiter;


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
  Milliseconds startTime = limiter->lastTimeMs;
  Milliseconds endTime = getTimeInMilliseconds();
  Milliseconds elapsed = endTime - startTime;
  Milliseconds maxElapsed = 1000 / limiter->targetFps;
  sleepForMilliseconds(maxElapsed - elapsed);
  limiter->lastTimeMs = getTimeInMilliseconds();
}
