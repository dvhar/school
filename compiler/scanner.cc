//David Hardy  CS4280  3/6/19
#include<iostream>
#include<string>
#include<map>
#include "scanner.h"
#define NUM_STATES 4
using namespace std;

//table, maps, and list of special characters
static int table[NUM_STATES][256];
static map<string, int> keywords = {
    {"Begin",   KW_BEGIN},
    {"End",     KW_END},
    {"Loop",    KW_LOOP},
    {"Void",    KW_VOID},
    {"INT",     KW_INT},
    {"Return",  KW_RETURN},
    {"Read",    KW_READ},
    {"Output",  KW_OUTPUT},
    {"Program", KW_PROGRAM},
    {"IFF",     KW_IFF},
    {"Then",    KW_THEN},
    {"Let",     KW_LET}};
static map<string, int> delimt = {
    {"=", D_EQ},
    {"<", D_LESS},
    {">", D_GREATER},
    {":", D_COLON},
    {"+", D_PLUS},
    {"-", D_MINUS},
    {"*", D_STAR},
    {"/", D_SLASH},
    {"%", D_PERCENT},
    {".", D_DOT},
    {"(", D_LPAREN},
    {")", D_RPAREN},
    {",", D_COMMA},
    {"{", D_LCURL},
    {"}", D_RCURL},
    {";", D_SEMICOL},
    {"[", D_LBRKT},
    {"]", D_RBRKT}};
static int delims[] = { '=','<','>',':','+','-','*','/','%','.','(',')',',','{','}',';','[',']' };
static map<int, string> enumString ={
    {ERROR,       "Error"},
    {ERR_NOLOWER, "Nothing begins with lowercase"},
    {ERR_BADCHAR, "Illegal character"}};

//exit with detailed error message
static void error(int code, int col, int line) { 
    cerr << "\nScanner Error: line "+to_string(line)+" col "+to_string(col)+". "+enumString[code]+"\n";
    exit(1);
}

//identifies white space
static int isWhiteSpace(int c){ return c == ' ' || c == '\n' || c == '\t'; } 

//create a token with tokenID, string value, and line number
static token_t token(int id, string instance, int lineNo){
    token_t tok;
    tok.id = id;
    tok.instance = instance;
    tok.lineNo = lineNo;
    return tok;
}

//initialize the FSA table
static void initializeTable(){

    //initialize table if not already initialized
    static int tableInitialized = 0;
    if (tableInitialized) return;
    //initialize table to illegal character error
    for (int i=0; i<NUM_STATES; i++)
        for (int j=0; j<256; j++)
            table[i][j] = ERR_BADCHAR;
    //first index 0 = initial state, 1 = id state, 2 = int state, 3 = delim state
    //next state when it finds a delimiter
    for (int i=0; i<18; i++){
        table[0][delims[i]] = STATE_DELIM;
        table[1][delims[i]] = ID;
        table[2][delims[i]] = INT;
        table[3][delims[i]] = DELIM;}
    //next state when it finds an uppercase letter
    for (int c='A'; c<='Z'; c++){
        table[0][c] = table[1][c] = STATE_ID;
        table[2][c] = INT;
        table[3][c] = DELIM;}
    //next state when it finds a lowercase letter
    for (int c='a'; c<='z'; c++){
        table[0][c] = ERR_NOLOWER;
        table[1][c] = STATE_ID;
        table[2][c] = ERR_NOLOWER;
        table[3][c] = ERR_NOLOWER;}
    //next state when it finds a digit
    for (int c='0'; c<='9'; c++){
        table[0][c] = table[2][c] = STATE_INT;
        table[1][c] = STATE_ID;
        table[3][c] = DELIM;}
    //next state when it finds whitespace
    table[0][' '] = table[0]['\n'] = table[0]['\t'] = table[0][EOF] = STATE_INITIAL;
    table[1][' '] = table[1]['\n'] = table[1]['\t'] = table[1][EOF] = ID;
    table[2][' '] = table[2]['\n'] = table[2]['\t'] = table[2][EOF] = INT;
    table[3][' '] = table[3]['\n'] = table[3]['\t'] = table[3][EOF] = DELIM;
    //set static variable so table initializer only runs once
    tableInitialized = 1;
}

//filter out comments from input stream
static int filteredGetc(){
    int next;
    while ((next = cin.get()) == '\\')
        while ((next = cin.peek()) != '\n') cin.get();
    return next;
}
static int filteredPeek(){
    int next = filteredGetc();
    cin.unget();
    return next;
}

//get one token at a time from comment-filtered cin
token_t scanner(){

    //initialize table and varables
    initializeTable();
    int state = STATE_INITIAL, nextState, nextChar;
    static int lineNo = 1, colNo = 1;
    string S("");

    //loop until get a token or an error
    while ( (state & FINAL) == 0 ){

        //peek at the next character to find the next state
        nextState = table[state][filteredPeek()];
        if (nextState & ERROR) { 
            error(nextState,colNo,lineNo); 
            return token(nextState, to_string(colNo), lineNo); 
        }

        //return a token if final
        if (nextState & FINAL) {
            //check for keywords and identifiers
            if (state == STATE_ID){
                if (keywords[S])
                    return token(keywords[S], S, lineNo);
                return token(ID, S, lineNo);
            //return other types of tokens. check for delimiters/operator types
            } else {
                if (state == STATE_DELIM && delimt[S]) nextState = delimt[S];
                return token(nextState, S, lineNo);
            }

        //update state and add next character to token string if not whitespace
        } else {
            state = nextState;
            nextChar = filteredGetc();
            colNo++;
            if (!isWhiteSpace(nextChar)) S += nextChar;
            if (nextChar == '\n') { lineNo++; colNo = 1; }
            if (nextChar == EOF) return token(EOF, "", lineNo);
        }
    }

    //should never get to here, but need a return value
    return token(ERROR, to_string(colNo), lineNo);
}
