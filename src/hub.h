#pragma once

// A hub is a collection of modules. It controls the initialization, positioning,
// rendering, and teardown of these modules. It also manages a surface onto which
// the modules are rendered.

// See linux-window.h for warning about this forward declaration.
typedef struct _cairo cairo_t;

typedef enum ModuleType ModuleType;
typedef struct Rect Rect;

typedef struct Hub Hub;


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

// The app's window will be scaled and positioned based on the modules. This
// function returns the bounding box of all of the modules, excluding the area
// where x >= 0 or y >= 0. In other words, this is the rect of the hub that is
// on-screen.
Rect getHubScreenRect(Hub* hub);

// Renders the modules using cairo.
void renderHub(Hub* hub, cairo_t* cairoContext);

// Since the application sleeps when there is nothing to do, the delta times can
// get huge. To compensate for this, these functions are called before and after
// sleeping.
void updateHubBeforeSleep(Hub* hub);
void updateHubAfterSleep(Hub* hub);

// Opens and closes all of the modules.
void showAllHubModules(Hub* hub);
void hideAllHubModules(Hub* hub);
void toggleAllHubModules(Hub* hub);

// If a module is closed, this will open it temporarily.
void alertHubModule(Hub* hub, ModuleType mt);

