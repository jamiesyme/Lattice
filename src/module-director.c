#include <stdlib.h>

#include "app-config.h"
#include "geometry-utils.h"
#include "interpolation-utils.h"
#include "module.h"
#include "module-director.h"
#include "time-utils.h"


typedef struct DirectedModule {
  Module* module;
  InterpolatedValue interpolatedX;
  InterpolatedValue interpolatedY;
  InterpolatedValue interpolatedDepth;
  Milliseconds waitTimeLeft;
  Milliseconds alertTimeLeft;
  int open;
} DirectedModule;

typedef struct ModuleDirector {
  AppConfig* appConfig;
  DirectedModule* modules;
  size_t moduleCount;
  Milliseconds lastUpdateTime;
} ModuleDirector;


static int isModuleOnScreen(DirectedModule* directedModule)
{
  return (directedModule->module->rect.x < 0 &&
          directedModule->module->rect.y < 0);
}

static int isModuleMoving(DirectedModule* directedModule)
{
  return (!isInterpolatedValueAtGoal(&directedModule->interpolatedX) ||
          !isInterpolatedValueAtGoal(&directedModule->interpolatedY) ||
          !isInterpolatedValueAtGoal(&directedModule->interpolatedDepth));
}

ModuleDirector* newModuleDirector(AppConfig* config)
{
  ModuleDirector* director = malloc(sizeof(ModuleDirector));
  director->appConfig = config;
  director->modules = NULL;
  director->moduleCount = 0;
  director->lastUpdateTime = getTimeInMilliseconds();
  return director;
}

void freeModuleDirector(ModuleDirector* director)
{
  free(director);
}

void updateModuleDirector(ModuleDirector* director)
{
  // Calculate the delta time
  Milliseconds thisUpdateTime = getTimeInMilliseconds();
  Milliseconds deltaTime = thisUpdateTime - director->lastUpdateTime;
  director->lastUpdateTime = thisUpdateTime;

  // Update all of the modules
  Milliseconds timeLeft;
  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];

    // Use up wait time
    timeLeft = deltaTime;
    if (timeLeft <= directedModule->waitTimeLeft) {
      directedModule->waitTimeLeft -= timeLeft;
      continue;
    } else {
      timeLeft -= directedModule->waitTimeLeft;
      directedModule->waitTimeLeft = 0;
    }

    // Use up alert time
    if (directedModule->alertTimeLeft > 0 && !isModuleMoving(directedModule)) {
      if (timeLeft <= directedModule->alertTimeLeft) {
        directedModule->alertTimeLeft -= timeLeft;
        continue;
      } else {
        timeLeft -= directedModule->alertTimeLeft;
        directedModule->alertTimeLeft = 0;

        // If the alert time was just used up, and the module is not marked as
        // open, we have to close the module
        if (!directedModule->open) {
          float xGoal = 0.0f;
          initInterpolatedValue(&directedModule->interpolatedX,
                                directedModule->interpolatedX.current,
                                xGoal,
                                director->appConfig->moduleCloseMoveDuration,
                                director->appConfig->moduleCloseMoveMethod);
        }
      }
    }

    // If the module isn't moving, and the module is lowered, we should raise it
    // because it has made it to its destination position.
    if (!isModuleMoving(directedModule) &&
        directedModule->module->depth < 0.0f) {
      float depthGoal = 0.0f;
      initInterpolatedValue(&directedModule->interpolatedDepth,
                            directedModule->interpolatedDepth.current,
                            depthGoal,
                            director->appConfig->moduleRaiseMoveDuration,
                            director->appConfig->moduleRaiseMoveMethod);
    }

    // Move the module
    updateInterpolatedValue(&directedModule->interpolatedX, timeLeft);
    updateInterpolatedValue(&directedModule->interpolatedY, timeLeft);
    updateInterpolatedValue(&directedModule->interpolatedDepth, timeLeft);
    directedModule->module->rect.x = directedModule->interpolatedX.current;
    directedModule->module->rect.y = directedModule->interpolatedY.current;
    directedModule->module->depth = directedModule->interpolatedDepth.current;
  }
}

void updateModuleDirectorAfterSleep(ModuleDirector* director)
{
  // Updating our last update time will reduce the delta time to near zero for
  // the first update after sleeping. After this first update, the delta times
  // should return to normal.
  director->lastUpdateTime = getTimeInMilliseconds();
}

void addModuleToDirector(ModuleDirector* director, Module* module)
{
  director->moduleCount++;
  director->modules = realloc(director->modules,
                              sizeof(DirectedModule) * director->moduleCount);
  DirectedModule* directedModule = &director->modules[director->moduleCount - 1];
  directedModule->module = module;
  directedModule->waitTimeLeft = 0;
  directedModule->alertTimeLeft = 0;
  directedModule->open = 0;
  initInterpolatedValue(&directedModule->interpolatedX,
                        module->rect.x,
                        module->rect.x,
                        0,
                        IM_LINEAR);
  initInterpolatedValue(&directedModule->interpolatedY,
                        module->rect.y,
                        module->rect.y,
                        0,
                        IM_LINEAR);
  initInterpolatedValue(&directedModule->interpolatedDepth,
                        module->depth,
                        module->depth,
                        0,
                        IM_LINEAR);
}

void openModulesWithDirector(ModuleDirector* director)
{
  float xOffset = director->appConfig->windowOffset.x;
  float yOffset = director->appConfig->windowOffset.y;

  float goal;
  Milliseconds duration;
  InterpolationMethod method;

  Milliseconds waitTime = 0;
  float yPos = -yOffset;
  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];

    // Set the x goal
    goal = -directedModule->module->rect.width - xOffset;
    duration = director->appConfig->moduleOpenMoveDuration;
    method = director->appConfig->moduleOpenMoveMethod;
    if (isModuleOnScreen(directedModule)) {
      duration = director->appConfig->moduleMoveDuration;
      method = director->appConfig->moduleMoveMethod;
    }
    initInterpolatedValue(&directedModule->interpolatedX,
                          directedModule->interpolatedX.current,
                          goal,
                          duration,
                          method);

    // Set the y goal
    yPos -= directedModule->module->rect.height;
    goal = yPos;
    duration = 0;
    method = director->appConfig->moduleMoveMethod;
    if (isModuleOnScreen(directedModule)) {
      duration = director->appConfig->moduleMoveDuration;
    }
    initInterpolatedValue(&directedModule->interpolatedY,
                          directedModule->interpolatedY.current,
                          goal,
                          duration,
                          method);

    // If the module is already on screen, and it's not at the correct y
    // position, we want to lower the module so it appears to move behind the
    // other modules that are just opening.
    if (isModuleOnScreen(directedModule) &&
        isInterpolatedValueAtGoal(&directedModule->interpolatedY)) {
      goal = -1.0f;
      duration = director->appConfig->moduleLowerMoveDuration;
      method = director->appConfig->moduleLowerMoveMethod;
      initInterpolatedValue(&directedModule->interpolatedDepth,
                            directedModule->interpolatedDepth.current,
                            goal,
                            duration,
                            method);
    }

    // Add wait time for the later modules
    directedModule->waitTimeLeft = waitTime;
    waitTime += director->appConfig->moduleOpenDelay;

    // Mark the module as opened
    directedModule->open = 1;

    // Put the margin between the modules
    yPos -= director->appConfig->moduleMarginSize;
  }
}

void closeModulesWithDirector(ModuleDirector* director)
{
  float goal;
  Milliseconds duration;
  InterpolationMethod method;

  Milliseconds waitTime = 0;
  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];

    // Set the x goal
    goal = 0.0f;
    duration = director->appConfig->moduleCloseMoveDuration;
    method = director->appConfig->moduleCloseMoveMethod;
    initInterpolatedValue(&directedModule->interpolatedX,
                          directedModule->interpolatedX.current,
                          goal,
                          duration,
                          method);

    // Add wait time for the later modules
    directedModule->waitTimeLeft = waitTime;
    waitTime += director->appConfig->moduleCloseDelay;

    // Mark the module as closed
    directedModule->open = 0;
  }
}

void alertModuleWithDirector(ModuleDirector* director, Module* module)
{
  float xOffset = director->appConfig->windowOffset.x;

  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];

    if (directedModule->module != module) {
      continue;
    }

    // If the module is open, then an alert shouldn't do anything
    if (directedModule->open) {
      break;
    }

    // Set the x goal
    float goal = -directedModule->module->rect.width - xOffset;
    Milliseconds duration;
    InterpolationMethod method;
    if (isModuleOnScreen(directedModule)) {
      duration = director->appConfig->moduleMoveDuration;
      method = director->appConfig->moduleMoveMethod;
    } else {
      duration = director->appConfig->moduleOpenMoveDuration;
      method = director->appConfig->moduleOpenMoveMethod;
    }
    initInterpolatedValue(&directedModule->interpolatedX,
                          directedModule->interpolatedX.current,
                          goal,
                          duration,
                          method);

    // Set the alert time
    directedModule->alertTimeLeft = director->appConfig->moduleAlertDuration;
  }
}

int isModuleDirectorBusy(ModuleDirector* director)
{
  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];
    if (isModuleOnScreen(directedModule) || isModuleMoving(directedModule)) {
      return 1;
    }
  }
  return 0;
}

Rect getModuleDirectorOpenRect(ModuleDirector* director)
{
  float xOffset = director->appConfig->windowOffset.x;
  float yOffset = director->appConfig->windowOffset.y;

  float maxX = 0.0f; // Doesn't change
  float minX = maxX;
  float maxY = -yOffset; // Doesn't change
  float minY = maxY;

  float goal;
  float yPos = -yOffset;
  for (size_t i = 0; i < director->moduleCount; ++i) {
    DirectedModule* directedModule = &director->modules[i];

    // Handle the x axis
    goal = -directedModule->module->rect.width - xOffset;
    if (goal < minX) {
      minX = goal;
    }

    // Handle the y axis
    yPos -= directedModule->module->rect.height;
    goal = yPos;
    if (goal < minY) {
      minY = goal;
    }

    // Put the margin between the modules
    yPos -= director->appConfig->moduleMarginSize;
  }

  Rect rect = {minX, minY, maxX - minX, maxY - minY};
  return rect;
}
