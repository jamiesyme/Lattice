#include <stdlib.h>

#include "fps-limiter.h"
#include "time-utils.h"


typedef struct FpsLimiter {
  int targetFps;
  Milliseconds lastTimeMs;
} FpsLimiter;


FpsLimiter* newFpsLimiter(int targetFps)
{
  FpsLimiter* limiter = malloc(sizeof(FpsLimiter));
  limiter->targetFps = targetFps;
  limiter->lastTimeMs = getTimeInMilliseconds();
  return limiter;
}

void freeFpsLimiter(FpsLimiter* limiter)
{
  free(limiter);
}

void limitFps(FpsLimiter* limiter)
{
  Milliseconds startTime = limiter->lastTimeMs;
  Milliseconds endTime = getTimeInMilliseconds();
  Milliseconds elapsed = endTime - startTime;
  Milliseconds maxElapsed = 1000 / limiter->targetFps;
  sleepForMilliseconds(maxElapsed - elapsed);
  limiter->lastTimeMs = getTimeInMilliseconds();
}
