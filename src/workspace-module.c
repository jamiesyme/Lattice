#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command-utils.h"
#include "draw-utils.h"
#include "module.h"
#include "surface.h"
#include "workspace-module.h"

#define WORKSPACE_COUNT 10


typedef struct Workspace {
  int empty;
  int visible;
  int focused;
  int urgent;
} Workspace;


// Uses i3-msg to get current workspace info. Expects workspaces to contain at
// least WORKSPACE_COUNT workspaces. Returns zero on success.
static int getWorkspaceInfo(Workspace* workspaces);

void renderWorkspaceModule(Module* module, Surface* surface);

void initWorkspaceModule(Module* module)
{
  module->type = MT_WORKSPACE;
  module->width = 250;
  module->height = 100;
  module->renderFunc = renderWorkspaceModule;
}

void renderWorkspaceModule(Module* module, Surface* surface)
{
  // Get the workspace info
  Workspace workspaces[WORKSPACE_COUNT];
  if (getWorkspaceInfo(workspaces) != 0) {
    printf("Could not get workspace info. Cannot update workspace module.\n");
    return;
  }

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

  // Draw the workspaces, split between two rows
  int areaWidth = 160;
  int areaHeight = 58;
  int outerWidth = 24;
  int outerHeight = 24;
  int borderSize = 3;
  int innerWidth = outerWidth - borderSize * 2;
  int innerHeight = outerHeight - borderSize * 2;
  int columnCount = WORKSPACE_COUNT / 2;
  for (size_t i = 0; i < WORKSPACE_COUNT; ++i) {

    // Compute the workspace position
    int outerX, outerY;
    outerX = module->width / 2;
    outerX -= areaWidth / 2;
    outerX += (areaWidth - outerWidth) * (i % columnCount) / (columnCount - 1);
    outerY = module->height / 2;
    if (i < columnCount) {
      outerY -= areaHeight / 2;
    } else {
      outerY += areaHeight / 2 - outerHeight;
    }
    int innerX, innerY;
    innerX = outerX + borderSize;
    innerY = outerY + borderSize;

    // Render the workspace index text
    char workspaceIndexStr[2];
    snprintf(workspaceIndexStr, 2, "%i", (int)(i + 1) % 10);
    TextSurface* ts = renderText(surface, 16, "monaco", workspaceIndexStr);
    int tsX = innerX + innerWidth / 2;
    int tsY = innerY + innerHeight / 2;

    // Get the draw color determined by the workspace state
    float r, g, b;
    if (workspaces[i].urgent) {
      r = 0.8;
      g = 0.2;
      b = 0.2;

    } else if (workspaces[i].focused) {
      r = 0.1;
      g = 0.1;
      b = 0.1;

    } else if (workspaces[i].empty) {
      r = 0.7;
      g = 0.7;
      b = 0.7;

    } else {
      r = 0.1;
      g = 0.1;
      b = 0.1;
    }

    // Draw the workspace
    setDrawColor(surface, r, g, b, opacity);
    drawRect(surface, outerX, outerY, outerWidth, outerHeight);
    if (workspaces[i].focused) {
      setDrawColor(surface, 1, 1, 1, opacity);
      drawText(surface, ts, tsX, tsY, 1);
    } else {
      setDrawColor(surface, 1, 1, 1, opacity);
      drawRect(surface, innerX, innerY, innerWidth, innerHeight);
      setDrawColor(surface, r, g, b, opacity);
      drawText(surface, ts, tsX, tsY, 1);
    }

    // Free the workspace index text
    freeTextSurface(ts);
  }
}

static int getWorkspaceInfo(Workspace* workspaces)
{
  char* output;
  if (runCommandAlloc("i3-msg -t get_workspaces", &output) != 0) {
    return 1;
  }

  // Expected format of output command (I added the newlines and spacing):
  // [
  //   {
  //     "num": 5,
  //     "name": "5",
  //     "visible": true,
  //     "focused": false,
  //     "rect": {
  //       "x": 2560,
  //       "y": 0,
  //       "width": 1920,
  //       "height": 1052
  //     },
  //     "output": "DVI-D-0",
  //     "urgent": false
  //   },
  //   ...
  // ]

  // Reset all workspaces
  for (size_t i = 0; i < WORKSPACE_COUNT; ++i) {
    workspaces[i].empty = 1;
    workspaces[i].visible = 0;
    workspaces[i].focused = 0;
    workspaces[i].urgent = 0;
  }

  // HACK: below we manually parse the json output. This should really be
  // replaced with a real json library.

  // Initialize output index and skip the leading '['
  size_t oi = 0;
  oi += 1;

  // Loop through workspaces until we hit the end of our output, denoted by ']'
  while (output[oi] != ']') {

    Workspace workspace;
    size_t workspaceIndex;

    workspace.empty = 0;
    workspace.visible = 0;
    workspace.focused = 0;
    workspace.urgent = 0;
    workspaceIndex = 0;

    // Skip leading '{'
    oi += 1;

    // Loop through the workspace members
    while (output[oi] != '}') {

      // Skip leading '"'
      oi += 1;

      // Read field name
      char* fieldName = &output[oi];
      size_t fieldNameSize = strcspn(fieldName, "\"");
      oi += fieldNameSize + 1;

      // Skip ':'
      oi += 1;

      // If this is a nested object (like "rect"), we need to skip it
      if (output[oi] == '{') {
        oi += strcspn(&output[oi], "}") + 1;
        if (output[oi] == ',') {
          oi += 1;
        }
        continue;
      }

      // Read field value
      char* fieldValue = &output[oi];
      size_t fieldValueSize = strcspn(fieldValue, ",}");
      oi += fieldValueSize;
      if (output[oi] == ',') {
        oi += 1;
      }

      // Interpret the field
      if (strncmp("num", fieldName, fieldNameSize) == 0) {
        workspaceIndex = strtoul(fieldValue, NULL, 10) - 1;

      } else if (strncmp("visible", fieldName, fieldNameSize) == 0) {
        workspace.visible = strncmp("false", fieldValue, fieldValueSize);

      } else if (strncmp("focused", fieldName, fieldNameSize) == 0) {
        workspace.focused = strncmp("false", fieldValue, fieldValueSize);

      } else if (strncmp("urgent", fieldName, fieldNameSize) == 0) {
        workspace.urgent = strncmp("false", fieldValue, fieldValueSize);
      }
    }

    // Skip trailing '}'
    oi += 1;

    // Skip trailing ','
    if (output[oi] == ',') {
      oi += 1;
    }

    // Save the workspace data
    workspaces[workspaceIndex] = workspace;
  }

  freeCommandOutput(output);
  return 0;
}
