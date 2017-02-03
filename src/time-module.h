#pragma once

#include "module.h"

typedef struct Surface Surface;


typedef struct TimeModule {
  Module base;
} TimeModule;

TimeModule* newTimeModule();

void freeTimeModule(TimeModule* module);

void updateTimeModule(TimeModule* module, Surface* surface);
