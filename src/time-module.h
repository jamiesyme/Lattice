#pragma once

#include "module.h"

typedef struct Surface Surface;

Module* newTimeModule();

void freeTimeModule(Module* module);

void updateTimeModule(Module* module, Surface* surface);
