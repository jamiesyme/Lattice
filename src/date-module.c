#include <time.h>

#include "date-module.h"
#include "draw-utils.h"
#include "module.h"

#define DAYS_WIDTH 150
#define DRAW_WIDTH 150
#define DRAW_HEIGHT 60
#define PRIMARY_FONT_NAME "monaco"
#define PRIMARY_FONT_SIZE 32
#define SECONDARY_FONT_NAME "monaco"
#define SECONDARY_FONT_SIZE 16
#define UNUSED(x) (void)(x)


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


void renderDateModule(Module* module, cairo_t* cairoContext);

void initDateModule(Module* module)
{
  module->type = MT_DATE;
  module->renderFunc = renderDateModule;

  setModuleDrawSize(module, (Dimensions){DRAW_WIDTH, DRAW_HEIGHT});
}

void renderDateModule(Module* module, cairo_t* cairoContext)
{
  UNUSED(module);

  // Get the current time
  time_t t;
  struct tm* tm;
  t = time(NULL);
  tm = localtime(&t);

  char strTime[16];
  strftime(strTime, sizeof strTime, "%b %-d", tm);

  // Draw the date
  setDrawColor4(cairoContext, 0, 0, 0, 1);
  TextSurface* textSurface = renderText(cairoContext,
                                        PRIMARY_FONT_SIZE,
                                        PRIMARY_FONT_NAME,
                                        strTime);
  drawText(cairoContext, textSurface, DRAW_WIDTH / 2, DRAW_HEIGHT / 3 - 5, 1);
  freeTextSurface(textSurface);

  // Draw the days of the week
  int dayDiff = DAYS_WIDTH / 7;
  for (Weekday d = SUNDAY; d <= SATURDAY; ++d) {
    int x = DRAW_WIDTH / 2 - DAYS_WIDTH / 2 + dayDiff * d + dayDiff / 2;
    int y = DRAW_HEIGHT / 4 * 3 + 5;
    if ((int)d == tm->tm_wday) {
      setDrawColor4(cairoContext, 0, 0, 0, 1);
      drawRect4(cairoContext, x - 7, y + 10, 14, 1);
    } else {
      setDrawColor4(cairoContext, 0.6f, 0.6f, 0.6f, 1.0f);
    }
    textSurface = renderText(cairoContext,
                             SECONDARY_FONT_SIZE,
                             SECONDARY_FONT_NAME,
                             weekdayStrings[d]);
    drawText(cairoContext, textSurface, x, y, 1);
    freeTextSurface(textSurface);
  }
}
