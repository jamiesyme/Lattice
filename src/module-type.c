#include <string.h>

#include "module-type.h"


ModuleType strToModuleType(const char* str)
{
  if (strcmp(str, "audio") == 0) {
    return MT_AUDIO;
  } else if (strcmp(str, "date") == 0) {
    return MT_DATE;
  } else if (strcmp(str, "time") == 0) {
    return MT_TIME;
  } else {
    return MT_UNKNOWN;
  }
}
