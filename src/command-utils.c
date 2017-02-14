#include <stdio.h>
#include <stdlib.h>

#include "command-utils.h"

#define BUF_SIZE 4096


int runCommand(const char* command, char** output)
{
  FILE* file;
  int status;
  size_t maxOutputSize;

  // Start the process
  file = popen(command, "r");
  if (file == NULL) {
    printf("error opening pipe");
    return -2;
  }

  // Get the process output
  if (output != NULL) {
    maxOutputSize = 0;
    *output = NULL;
    while (!feof(file)) {
      maxOutputSize += BUF_SIZE;
      *output = realloc(*output, sizeof(char) * maxOutputSize);
      fread(*output + maxOutputSize - BUF_SIZE, sizeof(char), BUF_SIZE, file);
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

void freeCommandOutput(char* output)
{
  free(output);
}
