//David Hardy  CS4280  4/6/19
#include<iostream>
#include "scanner.h"
#include "parser.h"
using namespace std;

/*
<program>  ->      <vars> <block>
<block>    ->      Begin <vars> <stats> End
<vars>     ->      empty | INT Identifier Integer <vars> 
<expr>     ->      <A> + <expr> | <A> - <expr> | <A>
<A>        ->      <N> * <A> | <N>
<N>        ->      <M> / <N> | <M>
<M>        ->      -<M> | <R>
<R>        ->      [ <expr> ] | Identifier | Integer
<stats>    ->      <stat> : <mStat>
<mStat>    ->      empty | <stat> : <mStat>
<stat>     ->      <in> | <out> | <block> | <if> | <loop> | <assign>
<in>       ->      Read [ Identifier ]  
<out>      ->      Output [ <expr> ]
<if>       ->      IFF [ <expr> <RO> <expr> ] <stat>
<loop>     ->      Loop [ <expr> <RO> <expr> ] <stat>
<assign>   ->      Identifier = <expr>  
<RO>       ->      < | =< | > | => | == | =
*/

static token_t tk;

//error function takes string and adds token details
static void error(string err){
    cerr << "\nError line " << tk.lineNo << ": " << err << ". Found " << tk.instance << endl;
    exit(1);
}

//create node with label
static node_t *getNode(string label){
    node_t *node = new node_t;
    node->label = label;
    node->node1 = node->node2 = node->node3 = node->node4 = NULL;
    node->tk1.id = 0;
    node->tk2.id = 0;
    return node;
}

//headers for recursive descent functions
static node_t* parseBlock();
static node_t* parseVars();
static node_t* parseExpr();
static node_t* parseA();
static node_t* parseN();
static node_t* parseM();
static node_t* parseR();
static node_t* parseStats();
static node_t* parseMStat();
static node_t* parseStat();
static node_t* parseIn();
static node_t* parseOut();
static node_t* parseIf();
static node_t* parseLoop();
static node_t* parseAssign();
static node_t* parseRO();

//node1 is vars
//node2 is block
node_t* parser(){
    node_t *n = getNode("Program");
    tk = scanner();
    n->node1 = parseVars();
    n->node2 = parseBlock();
    if (tk.id != EOF)
        error("Expected end of input");
    return n;
}

//tk1 is ID
//tk2 is integer value
//node1 is vars
static node_t* parseVars(){
    if (tk.id == KW_INT){
        node_t *n = getNode("Vars");
        tk = scanner();
        if (tk.id != ID)
            error("Expected an identifier after INT");
        n->tk1 = tk;
        tk = scanner();
        if (tk.id != INT)
            error("Expected an integer after identifier");
        n->tk2 = tk;
        tk = scanner();
        n->node1 = parseVars();
        return n;
    }
    //else empty
    return NULL;
}

//node1 is vars
//node2 is stats
static node_t* parseBlock(){
    node_t *n = getNode("Block");
    if (tk.id != KW_BEGIN)
        error("Expected block to start with Begin");
    tk = scanner();
    n->node1 = parseVars();
    n->node2 = parseStats();
    if (tk.id != KW_END)
        error("Expected block to end with 'End'");
    tk = scanner();
    return n;
}

//node1 is stat
//node2 is mStat
static node_t* parseStats(){
    node_t *n = getNode("Stats");
    n->node1 = parseStat();
    if (tk.id != D_COLON)
        error("Expected colon after statement");
    tk = scanner();
    n->node2 = parseMStat();
    return n;
}

//node1 is stat production
static node_t* parseStat(){
    node_t *n = getNode("Stat");
    switch(tk.id){
        case KW_READ:
            n->node1 = parseIn();
            return n;
        case KW_OUTPUT:
            n->node1 = parseOut();
            return n;
        case KW_BEGIN:
            n->node1 = parseBlock();
            return n;
        case KW_IFF:
            n->node1 = parseIf();
            return n;
        case KW_LOOP:
            n->node1 = parseLoop();
            return n;
        case ID:
            n->node1 = parseAssign();
            return n;
    }
    error("Expected a statement (In, Out, IFF, Loop, ID)");
    return NULL;
}

//node1 is stat
//node2 is mStat
static node_t* parseMStat(){
    node_t *n = getNode("MStat");
    //see if next token is first of a stat
    switch(tk.id){
        case KW_READ:
        case KW_OUTPUT:
        case KW_BEGIN:
        case KW_IFF:
        case KW_LOOP:
        case ID:
            n->node1 = parseStat();
            if (tk.id != D_COLON)
                error("Expected colon after statement");
            tk = scanner();
            n->node2 = parseMStat();
            return n;
    }
    //otherwise empty
    return n;
}

//tk1 is identifier
static node_t* parseIn(){
    //eat Read token
    tk = scanner();
    node_t *n = getNode("In");
    if (tk.id != D_LBRKT)
        error("Expected [ after Read");
    tk = scanner();
    if (tk.id != ID)
        error("Expected identifier after [");
    n->tk1 = tk;
    tk = scanner();
    if (tk.id != D_RBRKT)
        error("Expected ] after identifier");
    tk = scanner();
    return n;
}

//node1 is expression
static node_t* parseOut(){
    //eat Output token
    tk = scanner();
    node_t *n = getNode("Out");
    if (tk.id != D_LBRKT)
        error("Expected [ after Output");
    tk = scanner();
    n->node1 = parseExpr();
    if (tk.id != D_RBRKT)
        error("Expected ] after expression");
    tk = scanner();
    return n;
}

//tk1 is - or +
//node1 is A
//node2 is expression
static node_t* parseExpr(){
    node_t *n = getNode("Expr");
    n->node1 = parseA();
    switch(tk.id){
        case D_MINUS:
        case D_PLUS:
            n->tk1 = tk;
            tk = scanner();
            n->node2 = parseExpr();
            return n;
    }
    //else just A
    return n;
}

//tk1 is *
//node1 is N
//node2 is A
static node_t* parseA(){
    node_t *n = getNode("A");
    n->node1 = parseN();
    if (tk.id == D_STAR){
        n->tk1 = tk;
        tk = scanner();
        n->node2 = parseA();
        return n;
    }
    //else just N
    return n;
}

//tk1 is /
//node1 is M
//node2 is N
static node_t* parseN(){
    node_t *n = getNode("N");
    n->node1 = parseM();
    if (tk.id == D_SLASH){
        n->tk1 = tk;
        tk = scanner();
        n->node2 = parseN();
        return n;
    }
    //else just M
    return n;
}

//tk1 is -
//node1 is M or R
static node_t* parseM(){
    node_t *n = getNode("M");
    switch(tk.id){
        case D_MINUS:
            n->tk1 = tk;
            tk = scanner();
            n->node1 = parseM();
            return n;
        case D_LBRKT:
        case ID:
        case INT:
            n->node1 = parseR();
            return n;
    }
    error("Expected -, [ expression ], integer, or identifier");
    return n;
}

//tk1 is identifier or integer
//node1 is expression
static node_t* parseR(){
    node_t *n = getNode("R");
    switch(tk.id){
        case D_LBRKT:
            tk = scanner();
            n->node1 = parseExpr();
            if (tk.id != D_RBRKT)
                error("Expected ] after expression");
            tk = scanner();
            return n;
        case ID:
        case INT:
            n->tk1 = tk;
            tk = scanner();
            return n;
    }
    error("Expected [, integer, or identifier");
    return n;
}

//node1 is expression
//node2 is relational operator
//node3 is expression
//node4 is stat
static node_t* parseIf(){
    //eat IFF token
    tk = scanner();
    node_t *n = getNode("Iff");
    if (tk.id != D_LBRKT)
        error("Expected [ after IFF");
    tk = scanner();
    n->node1 = parseExpr();
    n->node2 = parseRO();
    n->node3 = parseExpr();
    if (tk.id != D_RBRKT)
        error("Expected ] after expression in IFF condition");
    tk = scanner();
    n->node4 = parseStat();
    return n;
}

//node1 is expression
//node2 is relational operator
//node3 is expression
//node4 is stat
static node_t* parseLoop(){
    //eat Loop token
    tk = scanner();
    node_t *n = getNode("Loop");
    if (tk.id != D_LBRKT)
        error("Expected [ after Loop");
    tk = scanner();
    n->node1 = parseExpr();
    n->node2 = parseRO();
    n->node3 = parseExpr();
    if (tk.id != D_RBRKT)
        error("Expected ] after expression in Loop condition");
    tk = scanner();
    n->node4 = parseStat();
    return n;
}

//tk1 is identifier
//node1 is expression
static node_t* parseAssign(){
    node_t *n = getNode("Assign");
    if (tk.id != ID)
        error("Expected assignment to start with identifier");
    n->tk1 = tk;
    tk = scanner();
    if (tk.id != D_EQ)
        error("Expected = after identifier '"+n->tk1.instance+"' in assignment");
    tk = scanner();
    n->node1 = parseExpr();
    return n;
}

//tk1 is relational operator
//tk2 is relational operator
static node_t* parseRO(){
    node_t *n = getNode("RO");
    switch(tk.id){
        case D_EQ:
            n->tk1 = tk;
            tk = scanner();
            switch(tk.id){
                case D_EQ:
                case D_LESS:
                case D_GREATER:
                    n->tk2 = tk;
                    tk = scanner();
                    return n;
            }
            return n;
        case D_LESS:
        case D_GREATER:
            n->tk1 = tk;
            tk = scanner();
            return n;
    }
    error("Expected relational operator");
    return n;
}
