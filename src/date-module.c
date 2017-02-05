#include <stdlib.h>
#include <time.h>

#include "graphics.h"
#include "surface.h"
#include "date-module.h"


typedef struct DateModule {
  Module base;
} DateModule;

// Compatible with tm.tm_wday
typedef enum Weekday {
  SUNDAY = 0,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
} Weekday;

// Indexed by Weekday enum
char* weekdayStrings[] = {"S", "M", "T", "W", "T", "F", "S"};


Module* newDateModule()
{
  DateModule* module = malloc(sizeof(DateModule));
  module->base.width = 250;
  module->base.height = 120;
  module->base.updateFunc = updateDateModule;
  module->base.freeFunc = freeDateModule;
  return (Module*)module;
}

void freeDateModule(Module* module)
{
  free(module);
}

void updateDateModule(Module* module, Surface* surface)
{
  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  char strTime[16];
  strftime(strTime, sizeof strTime, "%b %-d", tm);

  // Draw the rect
  setDrawColor(surface, 1, 1, 1, 0.8);
  drawRect(surface, 0, 0, module->width, module->height);

  // Draw the date
  setDrawColor(surface, 0, 0, 0, 0.8);
  TextSurface* textSurface = renderText(surface, 32, "monaco", strTime);
  drawText(surface, textSurface, module->width / 2, module->height / 3, 1);
  freeTextSurface(textSurface);

  // Draw the days
  int daysWidth = 150;
  int dayDiff = daysWidth / 7;
  for (Weekday d = SUNDAY; d <= SATURDAY; ++d) {
    int x = module->width / 2 - daysWidth / 2 + dayDiff * d + dayDiff / 2;
    int y = module->height / 4 * 3 - 5;
    if (d == tm->tm_wday) {
      setDrawColor(surface, 0, 0, 0, 0.8);
      drawRect(surface, x - 7, y + 10, 14, 1);
    } else {
      setDrawColor(surface, 0.6, 0.6, 0.6, 0.8);
    }
    textSurface = renderText(surface, 16, "monaco", weekdayStrings[d]);
    drawText(surface, textSurface, x, y, 1);
    freeTextSurface(textSurface);
  }
}
