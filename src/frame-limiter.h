#pragma once

// A frame limiter is used to cap the framerate at a particular fps. This is
// just to save on CPU cycles (what kind of status bar needs more than 60 fps).

typedef struct FrameLimiter FrameLimiter;


FrameLimiter* newFrameLimiter(int targetFps);
void freeFrameLimiter(FrameLimiter* limiter);

// Call this from the main loop once per frame. It will sleep for however long
// is necessary to reach the target fps.
void applyFrameLimiter(FrameLimiter* limiter);
