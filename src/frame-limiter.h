#pragma once

typedef struct FrameLimiter FrameLimiter;


FrameLimiter* newFrameLimiter(int targetFps);

void freeFrameLimiter(FrameLimiter* limiter);

void applyFrameLimiter(FrameLimiter* limiter);
