#include <math.h>

#include "interpolation-utils.h"


#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define PI 3.141592654f


void initInterpolatedValue(InterpolatedValue* interpolatedValue,
                           float start,
                           float goal,
                           Milliseconds duration,
                           InterpolationMethod method)
{
  interpolatedValue->start = start;
  interpolatedValue->goal = goal;
  interpolatedValue->current = start;
  interpolatedValue->method = method;
  interpolatedValue->timeDuration = duration;
  interpolatedValue->timeAccumulated = 0;
}

int isInterpolatedValueAtGoal(InterpolatedValue* interpolatedValue)
{
  return (interpolatedValue->timeAccumulated >=
          interpolatedValue->timeDuration);
}

void updateInterpolatedValue(InterpolatedValue* interpolatedValue,
                             Milliseconds deltaTime)
{
  // Update the time accumulator
  interpolatedValue->timeAccumulated += deltaTime;
  interpolatedValue->timeAccumulated = MIN(interpolatedValue->timeAccumulated,
                                           interpolatedValue->timeDuration);

  // Update the current value based on the interpolation method
  float timeMult = 1.0f;
  if (interpolatedValue->timeDuration != 0) {
    timeMult = ((float)interpolatedValue->timeAccumulated /
                (float)interpolatedValue->timeDuration);
  }
  float start = interpolatedValue->start;
  float diff = interpolatedValue->goal - start;

  switch (interpolatedValue->method) {
  case IM_LINEAR:
    interpolatedValue->current = start + diff * timeMult;
    break;

  case IM_SINE: {
    float sinMult = sin((timeMult - 0.5f) * PI) * 0.5f + 0.5f;
    interpolatedValue->current = start + diff * sinMult;
  } break;

  case IM_SINE_EASE_OUT: {
    float sinMult = sin(timeMult * PI / 2.0f);
    interpolatedValue->current = start + diff * sinMult;
  } break;
  }
}
