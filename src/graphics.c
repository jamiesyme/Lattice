#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdlib.h>

#include "graphics.h"
#include "surface.h"


struct TextSurface {
  PangoFontDescription* fontInfo;
  PangoLayout* fontLayout;
  unsigned int fontSize;
};


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

TextSurface* renderText(Surface* surface,
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

  TextSurface* textSurface = malloc(sizeof(TextSurface));
  textSurface->fontInfo = fontInfo;
  textSurface->fontLayout = fontLayout;
  textSurface->fontSize = size;
  return textSurface;
}

void drawText(Surface* surface,
              TextSurface* textSurface,
              int x,
              int y,
              int centered)
{
  cairo_t* cr = getCairoContext(surface);

  // Transform the render position
  // This involves applying the text's offset
  // If the text is centered, we'll have to offset by half the size as well
  PangoRectangle inkRect, logicalRect;
  pango_layout_get_extents(textSurface->fontLayout, &inkRect, &logicalRect);

  if (centered) {
    x += logicalRect.x / PANGO_SCALE / 2;
    y += logicalRect.y / PANGO_SCALE / 2;
    x -= logicalRect.width / PANGO_SCALE / 2;
    y -= logicalRect.height / PANGO_SCALE / 2;
  } else {
    x += logicalRect.x / PANGO_SCALE;
    y += logicalRect.y / PANGO_SCALE;
  }

  // Render the text
  cairo_save(cr);
  cairo_move_to(cr, x, y);
  pango_cairo_show_layout(cr, textSurface->fontLayout);
  cairo_restore(cr);
}

void freeTextSurface(TextSurface* textSurface)
{
  g_object_unref(textSurface->fontLayout);
  pango_font_description_free(textSurface->fontInfo);
  free(textSurface);
}
