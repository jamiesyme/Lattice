#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command-utils.h"
#include "draw-utils.h"
#include "json.h"
#include "module.h"
#include "workspace-module.h"

#define DRAW_WIDTH 160
#define DRAW_HEIGHT 58
#define FONT_NAME "monaco"
#define FONT_SIZE 16
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

void renderWorkspaceModule(Module* module, cairo_t* cairoContext);

void initWorkspaceModule(Module* module)
{
  module->type = MT_WORKSPACE;
  module->renderFunc = renderWorkspaceModule;

  setModuleDrawSize(module, (Dimensions){DRAW_WIDTH, DRAW_HEIGHT});
}

void renderWorkspaceModule(Module* module, cairo_t* cairoContext)
{
  // Get the workspace info
  Workspace workspaces[WORKSPACE_COUNT];
  if (getWorkspaceInfo(workspaces) != 0) {
    printf("Could not get workspace info. Cannot update workspace module.\n");
    return;
  }

  // Draw the workspaces, split between two rows
  int outerWidth = 24;
  int outerHeight = 24;
  int borderSize = 3;
  int innerWidth = outerWidth - borderSize * 2;
  int innerHeight = outerHeight - borderSize * 2;
  int columnCount = WORKSPACE_COUNT / 2;
  for (size_t i = 0; i < WORKSPACE_COUNT; ++i) {

    // Compute the workspace position
    int outerX, outerY;
    //outerX = module->width / 2;
    //outerX -= DRAW_WIDTH / 2;
    outerX = (DRAW_WIDTH - outerWidth) * (i % columnCount) / (columnCount - 1);
    if (i < columnCount) {
      outerY = 0;
    } else {
      outerY = DRAW_HEIGHT - outerHeight;
    }
    int innerX, innerY;
    innerX = outerX + borderSize;
    innerY = outerY + borderSize;

    // Render the workspace index text
    char workspaceIndexStr[2];
    snprintf(workspaceIndexStr, 2, "%i", (int)(i + 1) % 10);
    TextSurface* ts = renderText(cairoContext,
                                 FONT_SIZE,
                                 FONT_NAME,
                                 workspaceIndexStr);
    int tsX = innerX + innerWidth / 2;
    int tsY = innerY + innerHeight / 2;

    // Get the draw color determined by the workspace state
    Color workspaceColor;
    if (workspaces[i].urgent) {
      workspaceColor = (Color){0.8f, 0.2f, 0.2f, 1.0f};

    } else if (workspaces[i].focused) {
      workspaceColor = (Color){0.1f, 0.1f, 0.1f, 1.0f};

    } else if (workspaces[i].empty) {
      workspaceColor = (Color){0.7f, 0.7f, 0.7f, 1.0f};

    } else {
      workspaceColor = (Color){0.1f, 0.1f, 0.1f, 1.0f};
    }

    // Draw the workspace
    setDrawColor(cairoContext, workspaceColor);
    drawRect4(cairoContext, outerX, outerY, outerWidth, outerHeight);
    if (workspaces[i].focused) {
      setDrawColor4(cairoContext, 1, 1, 1, 1);
      drawText(cairoContext, ts, tsX, tsY, 1);
    } else {
      setDrawColor4(cairoContext, 1, 1, 1, 1);
      drawRect4(cairoContext, innerX, innerY, innerWidth, innerHeight);
      setDrawColor(cairoContext, workspaceColor);
      drawText(cairoContext, ts, tsX, tsY, 1);
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

  // Prepare for parsing
  JsonParser jParser;
  JsonData jData;
  initJsonParser(&jParser, output, strlen(output));

  // Start with the array
  if (acceptJson(&jParser, &jData, JT_ARRAY_START) == 0) {
    return 1;
  }

  // Loop through workspaces
  while (acceptJson(&jParser, &jData, JT_ARRAY_END) == 0) {

    Workspace workspace;
    size_t workspaceIndex;

    workspace.empty = 0;
    workspace.visible = 0;
    workspace.focused = 0;
    workspace.urgent = 0;
    workspaceIndex = 0;

    // Open the workspace object
    if (acceptJson(&jParser, &jData, JT_OBJECT_START) == 0) {
      return 1;
    }

    // Loop through the workspace members
    while (acceptJson(&jParser, &jData, JT_OBJECT_END) == 0) {

      // Get the field name
      if (acceptJson(&jParser, &jData, JT_STRING) == 0) {
        return 1;
      }
      char* fieldName = jsonDataToString(&jParser, &jData);

      // Get the field value
      if (parseJson(&jParser, &jData) == 0) {
        return 1;
      }

      // If this is a nested object (like "rect"), we need to skip it
      if (jData.type == JT_OBJECT_START) {
        while (jData.type != JT_OBJECT_END) {
          if (parseJson(&jParser, &jData) == 0) {
            return 1;
          }
        }
        continue;
      }

      // Interpret the field
      if (strcmp("num", fieldName) == 0) {
        workspaceIndex = (int)jsonDataToNumber(&jParser, &jData) - 1;

      } else if (strcmp("visible", fieldName) == 0) {
        workspace.visible = jParser.str[jData.start] == 't';

      } else if (strcmp("focused", fieldName) == 0) {
        workspace.focused = jParser.str[jData.start] == 't';

      } else if (strcmp("urgent", fieldName) == 0) {
        workspace.urgent = jParser.str[jData.start] == 't';
      }

      // Don't forget to free the field name
      free(fieldName);
    }

    // Save the workspace data
    workspaces[workspaceIndex] = workspace;
  }

  freeCommandOutput(output);
  return 0;
}
