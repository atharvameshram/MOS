#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <queue>
#include <vector>
using namespace std;

/*File Handlers*/
ifstream fin("input.txt");
ofstream fout("output.txt");

/*Memory*/
char mainMemory[300][4];                //Memory block of 300 locations of 4 bytes
char supervisoryStorage[100][4];        //Supervisory storage of 10 buffers of 40 bytes
char auxiliaryStorage[500][4];          //Auxiliary storage of 500 locations

int uniTimer = 0;                                   //Universal timer (clock)
int chTimer[4] = {0,0,0,0};                         //Timers for each channel i = 1,2,3
bool chFlags[4] = {false, false, false, false};     //Flags for each channel
int chTimeCount[4] = {0,5,5,2};                     //Time taken by each channel
int chInterrupt[4] = {0,1,2,4};                     //Interrupt values for each channel

//char buffer[40];                        //Buffer memory for input file of 40 bytes 
char IR[4];                             //Instruction register 4 bytes
char R[5];                              //General Purpose Regsiter 4 bytes
int IC;                                 //Instruction Counter
int C;                                  //Toggle

queue<int> efb;                         //Empty Buffer Queue
queue<int> ifb;                         //Inputful Buffer Queue
queue<int> ofb;                         //Outputful Buffer Queue

queue<int> LQ;                                 //Load Queue
queue<int> RQ;                                 //Ready Queue
queue<int> IOQ;                                //Input-Output Queue
queue<int> TQ;                                 //Terminate Queue

int SI;                                 //Service Interrupt
int PI;                                 //Program Interrupt
int TI;                                 //Time Interrupt
int IOI;                                //Input-Output Interrupt

int PTR;                                //Page Table Register
string caseTask;                        //Indicates case task for channel 3
bool breakFlag;                         //To terminate current execution

/*Process Control Block*/
struct PCB{
    int job_id;                 //Job id
    
    int TTL;                    //Total Time Limit
    int TLL;                    //Total Line Limit
    int TS;                     //Time Slice
    
    int TTC;                    //Total Time Counter
    int LLC;                    //Line Limit Counter
    int TSC;                    //Time Slice Counter
    
    char F;                     //'F' to hold 'P' or 'D'

    int startAddrP;             //Starting Address of Program card on Drum
    int pCount;                 //Total block count for program card
    int startAddrD;             //Starting Address of Data card on Drum
    int dCount;                 //Total block count for data card

    void setPCB(int id, int ttl, int tll){              //PCB
        job_id = id;
        TTL = ttl;
        TLL = tll;
        TTC = 0;
        LLC = 0;
        TSC = 0;
        startAddrP = -1;
        startAddrD = -1;
        pCount = 0;
        dCount = 0;
    }
};

PCB pcb;                        //Global Process Control Block

/*Error Messages*/
string error[7] = {"No Error", "Out of Data", "Line Limit Exceeded", "Time Limit Exceeded",
    "Operation Code Error", "Operand Error", "Invalid Page Fault"};

/*Functions*/
void init();
void mos();

int addressMap(int VA);

//void execute_user_program();
void start_execution();

int allocate();

void startCH(int i);
void simulation(int i);

void IR1();
//void IR2();
void IR3();

void load();