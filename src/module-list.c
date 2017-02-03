#include <stdlib.h>
#include "module-list.h"

typedef struct ModuleNode ModuleNode;

struct ModuleNode {
  ModuleNode* next;
  Module* module;
};

struct ModuleList {
  ModuleNode* root;
  size_t count;
};


ModuleList* newModuleList()
{
  ModuleList* list = malloc(sizeof(ModuleList));
  list->root = 0;
  list->count = 0;
  return list;
}

void freeModuleList(ModuleList* list)
{
  free(list);
}

size_t addModuleToList(ModuleList* list, Module* module)
{
  if (list->count == 0) {
    list->root = malloc(sizeof(ModuleNode));
    list->root->next = 0;
    list->root->module = module;
    ++list->count;
    return 0;
  } else {
    size_t i = 1;
    ModuleNode* node = list->root;
    while (node->next != 0) {
      node = node->next;
      ++i;
    }
    node->next = malloc(sizeof(ModuleNode));
    node->next->next = 0;
    node->next->module = module;
    ++list->count;
    return i;
  }
}

int removeModuleFromList(ModuleList* list, size_t i)
{
  if (i >= list->count) {
    return 1;
  }
  ModuleNode* prevNode = 0;
  ModuleNode* currNode = list->root;
  for (size_t j = 0; j < i; ++j) {
    prevNode = currNode;
    currNode = currNode->next;
    if (currNode == 0) {
      return 1;
    }
  }
  prevNode->next = currNode->next;
  free(currNode);
  --list->count;
  return 0;
}

Module* getModuleFromList(ModuleList* list, size_t i)
{
  if (i >= list->count) {
    return 0;
  }
  ModuleNode* node = list->root;
  for (size_t j = 0; j < i; ++j) {
    node = node->next;
    if (node == 0) {
      return 0;
    }
  }
  return node->module;
}

size_t getModuleCountOfList(ModuleList* list)
{
  return list->count;
}
