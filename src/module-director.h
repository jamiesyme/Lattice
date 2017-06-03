#pragma once

// TODO: description.

typedef struct AppConfig AppConfig;
typedef struct Module Module;
typedef struct Rect Rect;

typedef struct ModuleDirector ModuleDirector;


ModuleDirector* newModuleDirector(AppConfig* config);
void freeModuleDirector(ModuleDirector* director);

void updateModuleDirector(ModuleDirector* director);
void updateModuleDirectorAfterSleep(ModuleDirector* director);

void addModuleToDirector(ModuleDirector* director, Module* module);

void openModulesWithDirector(ModuleDirector* director);
void closeModulesWithDirector(ModuleDirector* director);
void alertModuleWithDirector(ModuleDirector* director, Module* module);

int isModuleDirectorBusy(ModuleDirector* director);

Rect getModuleDirectorOpenRect(ModuleDirector* director);
