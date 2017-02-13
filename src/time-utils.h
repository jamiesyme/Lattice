#pragma once

// A 32-bit integer for milliseconds would wrap around 25 days, so a 64-bit
// integer is used instead, which will last more than 290 million years.
typedef long long Milliseconds;

// Returns the time since epoch in milliseconds.
Milliseconds getTimeInMilliseconds();

void sleepForMilliseconds(Milliseconds ms);
