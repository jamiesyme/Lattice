#pragma once

// Runs a command using popen(). Returns the exit status of the command. If
// output is not NULL, then the output of the command is captured, and must be
// freed. To free the output you can either use free() or freeCommandOutput().
int runCommand(const char* command, char** output);

// Equivalent to free(); provided for convenience.
void freeCommandOutput(char* output);
