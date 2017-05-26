#pragma once

// TODO: description.

typedef struct AppConfig AppConfig;
typedef struct Module Module;
typedef struct Rect Rect;
typedef struct Surface Surface;


typedef struct ModuleRenderer ModuleRenderer;

ModuleRenderer* newModuleRenderer(AppConfig* appConfig);
void freeModuleRenderer(ModuleRenderer* renderer);

void addModuleToRenderer(ModuleRenderer* renderer, Module* module);

void renderModules(ModuleRenderer* renderer, Surface* surface);

int isModuleRendererBusy(ModuleRenderer* renderer);

Rect getModuleRendererRect(ModuleRenderer* renderer);
