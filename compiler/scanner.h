//David Hardy  CS4280  3/6/19
#ifndef SCANNER_H
#define SCANNER_H
#include<map>
#include<string>
using namespace std;

//token type
typedef struct token_t
{
  int id;
  string instance;
  int lineNo;
} token_t;


//states are array indexes, others use bits for type of token
enum enums { 
    //error codes
    ERROR       = 1<<20, 
    ERR_NOLOWER = ERROR|1, 
    ERR_BADCHAR = ERROR|2, 
    //final states. subtype bits are shifted to make room for delimiter/operator types
    FINAL       = 1<<21, 
    ID          = FINAL|1,
    INT         = FINAL|2,
    //final state for delims and bit for all delimiter/operators
    DELIMBIT    = 1<<22,
    DELIM       = FINAL|DELIMBIT,
    D_PLUS      = DELIM|1,
    D_EQ        = DELIM|2,
    D_LESS      = DELIM|3,
    D_GREATER   = DELIM|4,
    D_COLON     = DELIM|5,
    D_MINUS     = DELIM|6,
    D_STAR      = DELIM|7,
    D_SLASH     = DELIM|8,
    D_PERCENT   = DELIM|9,
    D_DOT       = DELIM|10,
    D_LPAREN    = DELIM|11,
    D_RPAREN    = DELIM|12,
    D_COMMA     = DELIM|13,
    D_LCURL     = DELIM|14,
    D_RCURL     = DELIM|15,
    D_SEMICOL   = DELIM|16,
    D_LBRKT     = DELIM|17,
    D_RBRKT     = DELIM|18,
    //non-final states,
    STATE_INITIAL = 0, 
    STATE_ID      = 1, 
    STATE_INT     = 2, 
    STATE_DELIM   = 3,
    //keywords
    KEYWORD    = 1<<23,
    KW_BEGIN   = KEYWORD|1,
    KW_END     = KEYWORD|2,
    KW_LOOP    = KEYWORD|3,
    KW_VOID    = KEYWORD|4,
    KW_INT     = KEYWORD|5,
    KW_RETURN  = KEYWORD|6,
    KW_READ    = KEYWORD|7,
    KW_OUTPUT  = KEYWORD|8,
    KW_PROGRAM = KEYWORD|9,
    KW_IFF     = KEYWORD|10,
    KW_THEN    = KEYWORD|11,
    KW_LET     = KEYWORD|12
};

token_t scanner();

#endif
