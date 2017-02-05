#pragma once

#include "module.h"

typedef struct Surface Surface;

Module* newDateModule();

void freeDateModule(Module* module);

void updateDateModule(Module* module, Surface* surface);
