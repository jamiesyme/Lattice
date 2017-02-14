#pragma once

// The audio module displays the enabled output sink (headphones or speakers),
// as well as the volume. It also has an indicator for when the audio is muted;
// currently, it colors the volume bar red when muted.

typedef struct Module Module;

void initAudioModule(Module* module);
