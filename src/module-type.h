#pragma once

typedef enum ModuleType {
  MT_UNKNOWN,
  MT_AUDIO,
  MT_DATE,
  MT_TIME
} ModuleType;

ModuleType strToModuleType(const char* str);
