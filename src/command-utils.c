#include <stdio.h>
#include <stdlib.h>

#include "command-utils.h"

#define BUF_SIZE 4096


int runCommand(const char* command, char* output, size_t outputSize)
{
  FILE* file;
  int status;
  size_t currentSize;

  // Start the process
  file = popen(command, "r");
  if (file == NULL) {
    printf("error opening pipe");
    return -2;
  }

  // Get the process output
  if (output != NULL && outputSize > 0) {
    currentSize = 0;
    while (!feof(file) && currentSize < outputSize) {
      currentSize += fread(output + currentSize,
                           sizeof(char),
                           (outputSize - currentSize),
                           file);
    }

    // Add null terminator
    if (currentSize == outputSize) {
      output[currentSize - 1] = '\0';
    } else {
      output[currentSize] = '\0';
    }
  }

  // Get the process exit status
  status = pclose(file);
  if (status < 0) {
    printf("command not found");
    return -1;
  }

  return status;
}

int runCommandAlloc(const char* command, char** output)
{
  FILE* file;
  int status;
  size_t maxOutputSize, currentSize;

  // Start the process
  file = popen(command, "r");
  if (file == NULL) {
    printf("error opening pipe");
    return -2;
  }

  // Get the process output
  if (output != NULL) {
    maxOutputSize = 0;
    currentSize = 0;
    *output = NULL;
    while (!feof(file)) {
      while (currentSize >= maxOutputSize) {
        maxOutputSize += BUF_SIZE;
        *output = realloc(*output, sizeof(char) * maxOutputSize);
      }
      currentSize += fread(*output + currentSize,
                           sizeof(char),
                           (maxOutputSize - currentSize),
                           file);
    }

    // Add null terminator
    if (currentSize >= maxOutputSize) {
      *output = realloc(*output, sizeof(char) * currentSize + 1);
    }
    (*output)[currentSize] = '\0';
  }

  // Get the process exit status
  status = pclose(file);
  if (status < 0) {
    printf("command not found");
    return -1;
  }

  return status;
}

void freeCommandOutput(char* output)
{
  free(output);
}
