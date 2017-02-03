#pragma once

#include <stddef.h>

typedef struct Module Module;
typedef struct ModuleList ModuleList;


ModuleList* newModuleList();

void freeModuleList(ModuleList* list);


// Adds module to end of list
// Returns index of new module
size_t addModuleToList(ModuleList* list, Module* module);

// Removes module at index `i`
// Returns zero on success
int removeModuleFromList(ModuleList* list, size_t i);

// Gets module at index `i`
// Returns null on failure
Module* getModuleFromList(ModuleList* list, size_t i);

// Gets size of module list
size_t getModuleCountOfList(ModuleList* list);
