#pragma once

#include "time-utils.h"

typedef enum InterpolationMethod {
  // Linear interpolation provides constant speed
  IM_LINEAR,
  // Sine interpolation eases in and eases out
  IM_SINE,
  // Sine interpolation that only eases out
  IM_SINE_EASE_OUT
} InterpolationMethod;

typedef struct InterpolatedValue {
  float start;
  float goal;
  float current;
  InterpolationMethod method;
  Milliseconds timeDuration;
  Milliseconds timeAccumulated;
} InterpolatedValue;


void initInterpolatedValue(InterpolatedValue* interpolatedValue,
                           float start,
                           float goal,
                           Milliseconds duration,
                           InterpolationMethod method);

int isInterpolatedValueAtGoal(InterpolatedValue* interpolatedValue);

void updateInterpolatedValue(InterpolatedValue* interpolatedValue,
                             Milliseconds deltaTime);
