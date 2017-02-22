#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio-module.h"
#include "command-utils.h"
#include "draw-utils.h"
#include "module.h"
#include "surface.h"

#define BUF_SIZE 16


typedef enum SinkType {
  ST_UNKNOWN,
  ST_HEADPHONES,
  ST_SPEAKERS
} SinkType;

typedef struct Sink {
  SinkType type;
  int volume;
  int muted;
} Sink;


static SinkType strToSinkType(const char* str)
{
  if (strcmp(str, "headphones") == 0) {
    return ST_HEADPHONES;
  } else if (strcmp(str, "speakers") == 0) {
    return ST_SPEAKERS;
  } else {
    return ST_UNKNOWN;
  }
}

void renderAudioModule(Module* module, Surface* surface);

void initAudioModule(Module* module)
{
  module->type = MT_AUDIO;
  module->width = 250;
  module->height = 100;
  module->renderFunc = renderAudioModule;
}

void renderAudioModule(Module* module, Surface* surface)
{
  // Get the current sink settings from maudio
  int statusSink, statusVolume, statusMute;
  char bufSink[BUF_SIZE], bufVolume[BUF_SIZE], bufMute[BUF_SIZE];
  statusSink   = runCommand("maudio show sink", bufSink, BUF_SIZE);
  statusVolume = runCommand("maudio show volume", bufVolume, BUF_SIZE);
  statusMute   = runCommand("maudio show mute", bufMute, BUF_SIZE);

  if (statusSink != 0 || statusVolume != 0 || statusMute != 0) {
    printf("Could not run maudio command. Cannot update audio module.\n");
    return;
  }

  // Remove the newline characters that get returned by fgets
  strcpy(bufSink, strtok(bufSink, "\n"));
  strcpy(bufVolume, strtok(bufVolume, "\n"));
  strcpy(bufMute, strtok(bufMute, "\n"));

  // Convert string settings to a Sink
  Sink sink;
  sink.type = strToSinkType(bufSink);
  sink.volume = strtol(bufVolume, NULL, 10);
  sink.muted = strtol(bufMute, NULL, 10);

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

  // Draw the sink type
  char* text;
  if (sink.type == ST_HEADPHONES) {
    text = "Headphones";
  } else if (sink.type == ST_SPEAKERS) {
    text = "Speakers";
  } else {
    text = "Unknown";
  }
  TextSurface* textSurface = renderText(surface, 32, "monaco", text);
  setDrawColor(surface, 0, 0, 0, opacity);
  drawText(surface, textSurface, module->width / 2, module->height / 2 - 10, 1);
  freeTextSurface(textSurface);

  // Draw the volume indicator
  int fullLength = 160;
  int curLength = fullLength * sink.volume / 100;
  int thickness = 4;
  int x = module->width / 2 - fullLength / 2;
  int y = module->height / 3 * 2 + 5;
  if (sink.muted) {
    setDrawColor(surface, 0.8, 0.2, 0.2, opacity);
  } else {
    setDrawColor(surface, 0.1, 0.1, 0.1, opacity);
  }
  drawRect(surface, x, y, fullLength, thickness / 2);
  drawRect(surface, x, y, curLength, thickness);
}
