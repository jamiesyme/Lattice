#pragma once

// TODO: description.

// See linux-window.h for warning about this forward declaration.
typedef struct _cairo cairo_t;

typedef struct AppConfig AppConfig;
typedef struct Module Module;
typedef struct Rect Rect;

typedef struct ModuleRenderer ModuleRenderer;


ModuleRenderer* newModuleRenderer(AppConfig* appConfig);
void freeModuleRenderer(ModuleRenderer* renderer);

void addModuleToRenderer(ModuleRenderer* renderer, Module* module);

void renderModules(ModuleRenderer* renderer, cairo_t* cairoContext);

int isModuleRendererBusy(ModuleRenderer* renderer);

Rect getModuleRendererRect(ModuleRenderer* renderer);
