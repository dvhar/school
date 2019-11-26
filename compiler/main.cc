//David Hardy  CS4280  4/6/19
#include<iostream>
#include<fstream>
#include<string>
#include<cerrno>
#include<cstring>
#include "parser.h"
#include "codeGen.h"
using namespace std;


int main(int argc, char** argv){
  
    if (argc > 2) { cerr << "One argument maximum.\n"; exit(1); }
    ifstream infile;
    ofstream outfile;
    string outname = "out.asm";

    //set input and output targets if given a file name
    if (argc == 2){
        string inputname(argv[1]);
        infile.open(inputname + ".input1");
        outname = inputname + ".asm";
        if (! infile.is_open()) {
            cerr << "Error: Could not open file "+inputname+".input1 : "+strerror(errno) << endl;
            exit(1);
        }
        cin.rdbuf(infile.rdbuf());
    } 

    //create parse tree and close input when done
    node_t* parseTree = parser();
    infile.close();
  
    //generate asm code and check static semantics. display file name if successful
    codeGen(parseTree);
    cout << outname << endl;
  
    //write asm to target file if no errors
    outfile.open(outname);
    outfile << target.str();
    outfile.close();
    return(0);
}
