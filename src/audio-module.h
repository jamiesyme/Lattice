#pragma once

#include "module.h"

typedef struct Surface Surface;

Module* newAudioModule();

void freeAudioModule(Module* module);

void updateAudioModule(Module* module, Surface* surface);
