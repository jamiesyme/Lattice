#pragma once

typedef struct Hub Hub;
typedef enum ModuleType ModuleType;


Hub* newHub();

void freeHub(Hub* hub);


int shouldUpdateHub(Hub* hub);

void updateHub(Hub* hub);


void showHubModules(Hub* hub);

void hideHubModules(Hub* hub);

void showModuleUpdate(Hub* hub, ModuleType mt);
