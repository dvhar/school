//David Hardy  CS4280  4/24/19
#include "parser.h"
#include "scanner.h"
#include "codeGen.h"
#include <iostream>
#include <vector>
#include <set>
enum names {VAR, LABEL, POP, PRINT};
using namespace std;

//traversable stack for identifiers and scope delimiters
static vector<string> vars;

//save asm code to string stream before printing to file
stringstream target;

//find identifier in whole stack or just in top scope. Don't count delimiters
int find(string id, bool limitScope){
    string delimiter = limitScope ? "delim" : "nodelim";
    int distance=0;
    for (vector<string>::reverse_iterator it = vars.rbegin(); it != vars.rend() && *it != delimiter; ++it){
        if (id == *it)
            return distance;
        if (*it != "delim")
            ++distance;
    }
    return -1;
}

//static semantics checker and code generator
void codeGen(node_t* node){
    if (node == NULL) return;

    //push scope delimiter if found a new block
    if (node->label == "Block")
        vars.push_back("delim");

    //check duplicate declarations in the same scope and push identifier
    if (node->label == "Vars"){
        if (find(node->tk1.instance, true)>=0){
            cerr << "Error line " << node->tk1.lineNo << ": " << node->tk1.instance << " was redeclared in the same scope.\n";
            exit(1);}
        vars.push_back(node->tk1.instance);
        target << "    PUSH" << endl
               << "    LOAD " << node->tk2.instance << endl
               << "    STACKW 0" << endl;
    }
    //make sure identifiers are declared before used. tk2 is never Identifier, so only need to check tk1
    else if (node->tk1.id == ID && find(node->tk1.instance, false)<0){
            cerr << "Error line " << node->tk1.lineNo << ": " << node->tk1.instance << " is not defined.\n";
            exit(1);}

    //traverse tree and call gen functions for code-generating nodes
    if (node->label == "Program") genProgram(node);
    else if (node->label == "Expr") genExpr(node);
    else if (node->label == "A") genA(node);
    else if (node->label == "N") genN(node);
    else if (node->label == "M") genM(node);
    else if (node->label == "R") genR(node);
    else if (node->label == "In") genRead(node);
    else if (node->label == "Out") genOut(node);
    else if (node->label == "Iff") genIff(node);
    else if (node->label == "Loop") genLoop(node);
    else if (node->label == "Assign") genAssign(node);
    else {
        codeGen(node->node1);
        codeGen(node->node2);
    }

    //pop scope when done with block
    if (node->label == "Block"){
        while (vars.back() != "delim" && vars.size() != 0){
            vars.pop_back();
            target << "    POP" << endl;
        }
        //pop the scope delimiter after identifiers in scope are popped
        vars.pop_back();
    }
}



//create labels and temporary vars
static string makeNames(int type){
    static int lcount=1, tcount=1;
    static set<string> tempVars;
    string name;
    switch (type){
        case VAR:
            name =  "T"+to_string(tcount++);
            tempVars.insert(name + " 0");
            break;
        case LABEL:
            name =  "L"+to_string(lcount++);
            break;
        case POP:
            --tcount;
            break;
        case PRINT:
            for (set<string>::iterator it = tempVars.begin(); it != tempVars.end(); ++it)
                target << *it << endl;
            break;
    }
    return name;
}

//prints stop and vars at the end
void genProgram(node_t * node){
    codeGen(node->node1);
    codeGen(node->node2);
    target << "STOP" << endl;
    makeNames(PRINT);
}

//negation
void genM(node_t * node){
    codeGen(node->node1);
    if (node->tk1.id)
        target << "    MULT -1" << endl;
}

//var or val terminal, or expression
void genR(node_t * node){
    if (node->tk1.id == INT)
        target << "    LOAD " << node->tk1.instance << endl;
    else if (node->tk1.id == ID)
        target << "    STACKR " << find(node->tk1.instance, false) << endl;
    else 
        codeGen(node->node1);
}

//multiplication
void genA(node_t * node){
    codeGen(node->node1);
    if (node->tk1.id == D_STAR){
        string tempv = makeNames(VAR);
        target << "    STORE " << tempv << endl;
        codeGen(node->node2);
        target << "    MULT " << tempv << endl;
        makeNames(POP);
    }
}

//addition and subtraction
void genExpr(node_t * node){
    string tempv;
    if (node->tk1.id == D_MINUS || node->tk1.id == D_PLUS){
        codeGen(node->node2);
        tempv = makeNames(VAR);
        target << "    STORE " << tempv << endl;
    }
    codeGen(node->node1);
    if (node->tk1.id == D_PLUS){
        target << "    ADD " << tempv << endl;
        makeNames(POP);
    }
    if (node->tk1.id == D_MINUS){
        target << "    SUB " << tempv << endl;
        makeNames(POP);
    }
}

//write
void genOut(node_t * node){
    codeGen(node->node1);
    string tempv = makeNames(VAR);
    target << "    STORE " << tempv << endl
           << "    WRITE " << tempv << endl;
    makeNames(POP);
}

//division
void genN(node_t * node){
    if (node->tk1.id == D_SLASH){
        codeGen(node->node2);
        string tempv = makeNames(VAR);
        target << "    STORE " << tempv << endl;
        codeGen(node->node1);
        target << "    DIV " << tempv << endl;
        makeNames(POP);
    }
    else
        codeGen(node->node1);
}

//read
void genRead(node_t * node){
    string tempv = makeNames(VAR);
    target << "    READ " << tempv << endl
           << "    LOAD " << tempv << endl
           << "    STACKW " << find(node->tk1.instance, false) << endl;
    makeNames(POP);
}

//assignment
void genAssign(node_t * node){
    codeGen(node->node1);
    target << "    STACKW " << find(node->tk1.instance, false) << endl;
}

//generate code to jump to 'false' label
void genRO(node_t * node, string fLabel){
    //bit shift second token to remove overlap
    switch ( node->tk1.id | (node->tk2.id<<5) ){
        case D_EQ:
            target << "    BRPOS " << fLabel << endl
                   << "    BRNEG " << fLabel << endl;
            break;
        case D_EQ|(D_EQ<<5):
            target << "    BRZERO " << fLabel << endl;
            break;
        case D_LESS:
            target << "    BRZPOS " << fLabel << endl;
            break;
        case D_GREATER:
            target << "    BRZNEG " << fLabel << endl;
            break;
        case D_EQ|(D_LESS<<5):
            target << "    BRPOS " << fLabel << endl;
            break;
        case D_EQ|(D_GREATER<<5):
            target << "    BRNEG " << fLabel << endl;
            break;
    }
}

//Loop
void genLoop(node_t * node){
    string tlabel = makeNames(LABEL);
    string flabel = makeNames(LABEL);
    target << tlabel << ": NOOP\n";
    codeGen(node->node3);
    string tempv = makeNames(VAR);
    target << "    STORE " << tempv << endl;
    codeGen(node->node1);
    target << "    SUB " << tempv << endl;
    makeNames(POP);
    genRO(node->node2, flabel);
    codeGen(node->node4);
    target << "    BR " << tlabel << endl
           << flabel << ": NOOP\n";
}

//IFF
void genIff(node_t * node){
    string flabel = makeNames(LABEL);
    codeGen(node->node3);
    string tempv = makeNames(VAR);
    target << "    STORE " << tempv << endl;
    codeGen(node->node1);
    target << "    SUB " << tempv << endl;
    makeNames(POP);
    genRO(node->node2, flabel);
    codeGen(node->node4);
    target << flabel << ": NOOP\n";
}
