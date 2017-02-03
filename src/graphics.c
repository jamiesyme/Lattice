#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include "graphics.h"
#include "surface.h"


void setDrawColor(Surface* surface, float r, float g, float b, float a)
{
  cairo_set_source_rgba(getCairoContext(surface), r, g, b, a);
}

void drawFullRect(Surface* surface)
{
  cairo_t* cr = getCairoContext(surface);
  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_restore(cr);
}

void drawRect(Surface* surface,
              int x,
              int y,
              unsigned int width,
              unsigned int height)
{
  cairo_t* cr = getCairoContext(surface);
  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle(cr, x, y, width, height);
  cairo_paint(cr);
  cairo_restore(cr);
}

void drawText(Surface* surface,
              int x,
              int y,
              unsigned int size,
              const char* fontName,
              const char* text)
{
  cairo_t* cr = getCairoContext(surface);

  PangoFontDescription* fontInfo = pango_font_description_new();
  pango_font_description_set_family(fontInfo, fontName);
  pango_font_description_set_weight(fontInfo, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_absolute_size(fontInfo, size * PANGO_SCALE);

  PangoLayout* fontLayout = pango_cairo_create_layout(cr);
  pango_layout_set_font_description(fontLayout, fontInfo);
  pango_layout_set_text(fontLayout, text, -1);

  cairo_move_to(cr, x, y);
  pango_cairo_show_layout(cr, fontLayout);

  g_object_unref(fontLayout);
  pango_font_description_free(fontInfo);
}
