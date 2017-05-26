#pragma once

// TODO: description.

typedef struct AppConfig AppConfig;
typedef struct Module Module;


typedef struct ModuleDirector ModuleDirector;

ModuleDirector* newModuleDirector(AppConfig* config);
void freeModuleDirector(ModuleDirector* director);

void updateModuleDirector(ModuleDirector* director);

void addModuleToDirector(ModuleDirector* director, Module* module);

void openModulesWithDirector(ModuleDirector* director);
void closeModulesWithDirector(ModuleDirector* director);
void alertModuleWithDirector(ModuleDirector* director, Module* module);

int isModuleDirectorBusy(ModuleDirector* director);
