#include <time.h>

#include "date-module.h"
#include "draw-utils.h"
#include "module.h"
#include "surface.h"


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


void renderDateModule(Module* module, Surface* surface);

void initDateModule(Module* module)
{
  module->type = MT_DATE;
  module->width = 250;
  module->height = 120;
  module->renderFunc = renderDateModule;
}

void renderDateModule(Module* module, Surface* surface)
{
  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  char strTime[16];
  strftime(strTime, sizeof strTime, "%b %-d", tm);

  // Draw the border and background rects
  float opacity = getModuleOpacity(module);
  int border = 4;
  setDrawColor(surface, 0, 0, 0, opacity);
  drawRect(surface, 0, 0, module->width, module->height);
  setDrawColor(surface, 1.0, 1.0, 1.0, opacity);
  drawRect(surface,
           border, border,
           module->width - border * 2,
           module->height - border * 2);

  // Draw the date
  setDrawColor(surface, 0, 0, 0, opacity);
  TextSurface* textSurface = renderText(surface, 32, "monaco", strTime);
  drawText(surface, textSurface, module->width / 2, module->height / 3, 1);
  freeTextSurface(textSurface);

  // Draw the days of the week
  int daysWidth = 150;
  int dayDiff = daysWidth / 7;
  for (Weekday d = SUNDAY; d <= SATURDAY; ++d) {
    int x = module->width / 2 - daysWidth / 2 + dayDiff * d + dayDiff / 2;
    int y = module->height / 4 * 3 - 5;
    if (d == tm->tm_wday) {
      setDrawColor(surface, 0, 0, 0, opacity);
      drawRect(surface, x - 7, y + 10, 14, 1);
    } else {
      setDrawColor(surface, 0.6, 0.6, 0.6, opacity);
    }
    textSurface = renderText(surface, 16, "monaco", weekdayStrings[d]);
    drawText(surface, textSurface, x, y, 1);
    freeTextSurface(textSurface);
  }
}
