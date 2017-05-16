#pragma once

// The module type is stored in a separate file from module because some other
// files (namely lattice.c and lattice-msg.c) only require the module type
// information.

typedef enum ModuleType {
  MT_UNKNOWN,
  MT_AUDIO,
  MT_DATE,
  MT_TIME,
  MT_WORKSPACE
} ModuleType;

// Converts a string into a module type. Strings should be all lowercase. The
// mapping is as follows:
//   "audio": MT_AUDIO
//   "date": MT_DATE
//   "time": MT_TIME
//   "workspace": MT_WORKSPACE
// If the string doesn't match any of the above, MT_UNKNOWN is returned.
ModuleType strToModuleType(const char* str);
