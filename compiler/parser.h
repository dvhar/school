#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

typedef struct node_t
{
  struct node_t *node1, *node2, *node3, *node4;
  token_t tk1, tk2;
  string label;
} node_t;

node_t* parser();

#endif
