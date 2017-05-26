#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio-module.h"
#include "command-utils.h"
#include "draw-utils.h"
#include "module.h"
#include "surface.h"

#define BUF_SIZE 16
#define BAR_LENGTH 160.0f
#define BAR_THICKNESS 4.0f
#define DRAW_WIDTH 175.0f
#define DRAW_HEIGHT 50.0f
#define FONT_NAME "monaco"
#define FONT_SIZE 32


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
  module->renderFunc = renderAudioModule;

  setModuleDrawSize(module, (Dimensions){DRAW_WIDTH, DRAW_HEIGHT});
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

  // Draw the sink type
  char* text;
  if (sink.type == ST_HEADPHONES) {
    text = "Headphones";
  } else if (sink.type == ST_SPEAKERS) {
    text = "Speakers";
  } else {
    text = "Unknown";
  }
  TextSurface* textSurface = renderText(surface, FONT_SIZE, FONT_NAME, text);
  setDrawColor4(surface, 0, 0, 0, 1);
  drawText(surface, textSurface, DRAW_WIDTH / 2, DRAW_HEIGHT / 2 - 10, 1);
  freeTextSurface(textSurface);

  // Draw the volume indicator
  int curLength = BAR_LENGTH * sink.volume / 100;
  int x = DRAW_WIDTH / 2 - BAR_LENGTH / 2;
  int y = DRAW_HEIGHT / 3 * 2 + 10;
  if (sink.muted) {
    setDrawColor4(surface, 0.8f, 0.2f, 0.2f, 1.0f);
  } else {
    setDrawColor4(surface, 0.1f, 0.1f, 0.1f, 1.0f);
  }
  drawRect4(surface, x, y, BAR_LENGTH, BAR_THICKNESS / 2);
  drawRect4(surface, x, y, curLength, BAR_THICKNESS);
}
