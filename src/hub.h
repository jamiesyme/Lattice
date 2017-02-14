#pragma once

// A hub is a collection of modules. It controls the initialization, positioning,
// rendering, and teardown of these modules. It also manages a surface onto which
// the modules are rendered.

typedef struct Hub Hub;
typedef enum ModuleType ModuleType;


// Allocates and initializes a hub containing the following modules:
//  + TimeModule
//  + DateModule
//  + AudioModule
//  + WorkspaceModule
Hub* newHub();
void freeHub(Hub* hub);

// Returns non-zero if renderHub() should be called. The result may only change
// after the following function calls:
//  + renderHub()
//  + showHub()
//  + hideHub()
//  + toggleHub()
//  + showModuleUpdate()
// This means that after zero is returned, it is safe to sleep until further hub
// interactions.
int shouldRenderHub(Hub* hub);

// Renders the modules to the surface.
void renderHub(Hub* hub);

// Turns all of the modules on or off.
void showHub(Hub* hub);
void hideHub(Hub* hub);
void toggleHub(Hub* hub);

// If a module is off, this will temporarily turn it on by setting the state to
// MS_ON_DYNAMIC. The module will fade in and fade out.
void showModuleUpdate(Hub* hub, ModuleType mt);
