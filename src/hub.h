#pragma once

typedef struct Hub Hub;


Hub* newHub();

void freeHub(Hub* hub);


int shouldUpdateHub(Hub* hub);

void updateHub(Hub* hub);


void showHubModules(Hub* hub);

void hideHubModules(Hub* hub);
