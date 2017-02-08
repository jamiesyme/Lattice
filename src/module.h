#pragma once

typedef struct Surface Surface;
typedef struct Module Module;

typedef void (*ModuleUpdateFunc)(Module*, Surface*);
typedef void (*ModuleFreeFunc)(Module*);

struct Module {
  unsigned int width;
  unsigned int height;
  ModuleUpdateFunc updateFunc;
  ModuleFreeFunc freeFunc;
  void* extraData;
};
