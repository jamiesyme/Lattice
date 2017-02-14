#pragma once

// The workspace module displays a list of 10 workspaces, and indicates which
// ones are active, which ones are visible, and which ones are urgent.
// NOTE: the workspace module depends on i3-msg, which is part of the i3 window
// manager.

typedef struct Module Module;

void initWorkspaceModule(Module* module);
