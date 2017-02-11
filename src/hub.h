#pragma once

typedef struct Hub Hub;
typedef enum ModuleType ModuleType;


Hub* newHub();

void freeHub(Hub* hub);


int shouldRenderHub(Hub* hub);

void renderHub(Hub* hub);


void showHubModules(Hub* hub);

void hideHubModules(Hub* hub);

void showModuleUpdate(Hub* hub, ModuleType mt);
