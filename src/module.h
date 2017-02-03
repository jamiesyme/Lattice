#pragma once

typedef struct Surface Surface;

typedef void (*ModuleUpdateFunc)(Module*, Surface*);
typedef void (*ModuleFreeFunc)(Module*);

typedef struct Module {
  unsigned int width;
  unsigned int height;
  ModuleUpdateFunc updateFunc;
  ModuleFreeFunc freeFunc;
} Module;
