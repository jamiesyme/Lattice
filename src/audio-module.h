#pragma once

// The audio module displays the enabled output sink (headphones or speakers),
// as well as the volume. It also has an indicator for when the audio is muted;
// currently, it colors the volume bar red when muted.
// NOTE: the audio module depends on an external program called maudio to get
// the current audio settings. The program can be found here:
// https://github.com/jamiesyme/dotfiles/tree/master/maudio

typedef struct Module Module;

void initAudioModule(Module* module);
