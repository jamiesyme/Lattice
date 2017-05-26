#pragma once

#include "draw-utils.h"
#include "interpolation-utils.h"
#include "time-utils.h"

// TODO: description

typedef struct AppConfig {
  Milliseconds        moduleAlertDuration;     // module-director.c
  Milliseconds        moduleCloseMoveDuration; // module-director.c
  InterpolationMethod moduleCloseMoveMethod;   // module-director.c
  Color               moduleBackgroundColor;   // module-renderer.c
  Color               moduleBorderColor;       // module-renderer.c
  float               moduleBorderSize;        // module-renderer.c
  Milliseconds        moduleLowerMoveDuration; // module-director.c
  InterpolationMethod moduleLowerMoveMethod;   // module-director.c
  float               moduleMarginSize;        // module-director.c
  Milliseconds        moduleMoveDuration;      // module-director.c
  InterpolationMethod moduleMoveMethod;        // module-director.c
  Milliseconds        moduleOpenMoveDuration;  // module-director.c
  InterpolationMethod moduleOpenMoveMethod;    // module-director.c
  Dimensions          modulePaddingSize;       // module-renderer.c
  Milliseconds        moduleRaiseMoveDuration; // module-director.c
  InterpolationMethod moduleRaiseMoveMethod;   // module-director.c
  Point               windowOffset;            // module-director.c
} AppConfig;
