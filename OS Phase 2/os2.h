#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
using namespace std;

/*File Handlers*/
ifstream fin("input.txt");
ofstream fout("output.txt");

/*Memory*/
char M[300][4];                 //Memory block of 300 locations of 4 bytes
char buffer[40];                //Buffer memory for input file of 40 bytes 
char IR[4];                     //Instruction register 4 bytes
char R[5];                      //General Purpose Regsiter 4 bytes
int IC;                         //Instruction Counter
int C;                          //Toggle
int SI;                         //Service Interrupt
int PI;                         //Program Interrupt
int TI;                         //Time Interrupt
int PTR;                        //Page Table Register
bool breakFlag;                 //To terminate current execution

/*Process Control Block*/
struct PCB{
    int job_id;                 //Job id
    int TTL;                    //Total Time Limit
    int TLL;                    //Total Line Limit
    int TTC;                    //Total Time Counter
    int LLC;                    //Line Limit Counter

    void setPCB(int id, int ttl, int tll){              //PCB
        job_id = id;
        TTL = ttl;
        TLL = tll;
        TTC = 0;
        LLC = 0;
    }
};

PCB pcb;                        //Global Process Control Block

/*Error Messages*/
string error[7] = {"No Error", "Out of Data", "Line Limit Exceeded", "Time Limit Exceeded",
    "Operation Code Error", "Operand Error", "Invalid Page Fault"};

/*Functions*/
void init();
void read(int RA);
void write(int RA);
int addressMap(int VA);
void execute_user_program();
void start_execution();
int allocate();
void load();
