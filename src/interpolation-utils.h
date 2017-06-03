#pragma once

#include "time-utils.h"

typedef enum InterpolationMethod {
  IM_LINEAR,
  IM_SINE,
  IM_SINE_EASE_OUT,
  IM_CUBIC_EASE_OUT,
  IM_QUARTIC_EASE_OUT,
  IM_QUINTIC_EASE_OUT
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
