//David Hardy  CS4280  4/30/19
#ifndef CODEGEN_H
#define CODEGEN_H
#include <sstream>
#include "scanner.h"
#include "parser.h"

extern stringstream target;

void codeGen(node_t*);
void genProgram(node_t *);
void genM(node_t *);
void genR(node_t *);
void genA(node_t *);
void genExpr(node_t *);
void genOut(node_t *);
void genN(node_t *);
void genRead(node_t *);
void genAssign(node_t *);
void genLoop(node_t *);
void genIff(node_t *);
int find(string, bool);

#endif
