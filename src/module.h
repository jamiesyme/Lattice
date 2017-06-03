#pragma once

// A module is a rendered component that's controlled and managed by the hub.
// You can think of a module as a widget. We will have a module to display the
// time, another module to display the date, etc.

#include "geometry-utils.h"

// ModuleType is stored in a separate file because some other files (namely
// lattice.c and lattice-msg.c) only require the module type information.
#include "module-type.h"

// See linux-window.h for warning about this forward declaration.
typedef struct _cairo cairo_t;

typedef struct AppConfig AppConfig;

typedef struct Module Module;


// TODO: description.
typedef void (*ModuleRenderFunc)(Module*, cairo_t*);

// TODO: description.
typedef void (*ModuleFreeFunc)(Module*);

struct Module {
  AppConfig* appConfig;

  // Read-only after init.
  // Used to identify module when running `lattice-msg alert ...`
  // Default: MT_UNKNOWN
  ModuleType type;

  // TODO: description.
  Rect rect;

  // TODO: description.
  float depth;

  // Called once per frame to render the module.
  // Default: NULL
  ModuleRenderFunc renderFunc;

  // Called once during teardown to free the extraData member.
  // Default: NULL
  ModuleFreeFunc freeFunc;

  // Optional module-specific data.
  // Default: NULL
  void* extraData;
};


// Sets the default values specified in struct definition.
void initModule(Module* module, AppConfig* config);

// TODO: description.
void setModuleDrawSize(Module* module, Dimensions drawSize);
