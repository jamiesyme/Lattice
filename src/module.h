#pragma once

#include "module-type.h"

typedef struct Surface Surface;
typedef struct Module Module;

typedef void (*ModuleUpdateFunc)(Module*, Surface*, float);
typedef void (*ModuleFreeFunc)(Module*);

struct Module {
  ModuleType type;
  unsigned int width;
  unsigned int height;
  ModuleUpdateFunc updateFunc;
  ModuleFreeFunc freeFunc;
  void* extraData;
};
