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
//  + showAllHubModules()
//  + hideAllHubModules()
//  + toggleAllHubModules()
//  + alertHubModule()
// This means that after zero is returned, it is safe to sleep until further hub
// interactions.
int shouldRenderHub(Hub* hub);

// Renders the modules to the surface.
void renderHub(Hub* hub);

// Turns all of the modules on or off.
void showAllHubModules(Hub* hub);
void hideAllHubModules(Hub* hub);
void toggleAllHubModules(Hub* hub);

// If a module is closed, this will open it temporarily.
void alertHubModule(Hub* hub, ModuleType mt);

// Since the application sleeps when there is nothing to do, the delta times can
// get huge. To compensate for this, this function is called after sleeping for
// any significant amount of time.
void updateHubAfterSleep(Hub* hub);
