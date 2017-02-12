#include <stddef.h>

#include "module.h"


void initModule(Module* module)
{
  module->type = MT_UNKNOWN;
  module->width = 0;
  module->height = 0;
  module->state = MS_OFF;
  module->opacityObj.full = 0.8;
  module->opacityObj.fadeInDuration = 100;
  module->opacityObj.holdDuration = 1500;
  module->opacityObj.fadeOutDuration = 500;
  module->opacityObj.timeAccum = 0;
  module->renderFunc = NULL;
  module->freeFunc = NULL;
  module->extraData = NULL;
}

void setModuleState(Module* module, ModuleState state)
{
  if (state == MS_ON_DYNAMIC) {
    switch (module->state) {
    case MS_OFF:
      module->state = state;
      module->opacityObj.timeAccum = 0;
      break;

    case MS_ON_CONSTANT:
      module->state = state;
      module->opacityObj.timeAccum = module->opacityObj.fadeInDuration;
      break;

    case MS_ON_DYNAMIC:
      if (module->opacityObj.timeAccum > module->opacityObj.fadeInDuration) {
        module->opacityObj.timeAccum = module->opacityObj.fadeInDuration;
      }
    }
  } else {
    module->state = state;
  }
}

float updateModuleOpacity(Module* module, Milliseconds delta)
{
  if (module->state == MS_ON_DYNAMIC) {
    Milliseconds maxAccum = (module->opacityObj.fadeInDuration +
                             module->opacityObj.holdDuration +
                             module->opacityObj.fadeOutDuration);
    module->opacityObj.timeAccum += delta;
    if (module->opacityObj.timeAccum >= maxAccum) {
      setModuleState(module, MS_OFF);
    }
  }
  return getModuleOpacity(module);
}

float getModuleOpacity(Module* module)
{
  float opacity;

  switch (module->state) {
  case MS_OFF:
    opacity = 0.0f;
    break;

  case MS_ON_CONSTANT:
    opacity = module->opacityObj.full;
    break;

  case MS_ON_DYNAMIC: {
    float f = module->opacityObj.full;
    Milliseconds t, t1, t2, t3;
    t = module->opacityObj.timeAccum;
    t1 = module->opacityObj.fadeInDuration;
    t2 = t1 + module->opacityObj.holdDuration;
    t3 = t2 + module->opacityObj.fadeOutDuration;
    if (t < t1) {
      opacity = (float)t / t1 * f;
    } else if (t < t2) {
      opacity = f;
    } else if (t < t3) {
      opacity = f - (float)(t - t2) / (t3 - t2) * f;
    } else {
      opacity = 0.0f;
    }
  } break;
  }

  return opacity;
}
