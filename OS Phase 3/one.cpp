#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <queue>
#include <string>
using namespace std;

/*File Handlers*/
ifstream fin("input.txt");
ofstream fout("output.txt");
ofstream logFile("log.txt");

int chTimer[4] = {0,0,0,0};                         //Timers for each channel i = 1,2,3
bool chFlags[4] = {false, true, true, true};     //Flags for each channel
int chTimeCount[4] = {0,5,5,2};                     //Time taken by each channel
int chInterrupt[4] = {0,1,2,4};                     //Interrupt values for each channel

class cpu;
class mainMemory;
class supervisoryStorage;
class auxiliaryStorage;
class clock;
int getPTR();
void setTI(int);
void setIOI(int);

int allocate(){
    srand(time(0));
    return (rand() % 30);
}

/*Process Control Block*/
class PCB{
private:    
    int job_id;                 //Job id
    
    int TTL;                    //Total Time Limit
    int TLL;                    //Total Line Limit
    int TS;                     //Time Slice
    
    int TTC;                    //Total Time Counter
    int LLC;                    //Line Limit Counter
    int TSC;                    //Time Slice Counter
    
    char F;                     //Flag 'F' to hold 'P' or 'D'

    int startAddrP;             //Starting Address of Program card on Drum
    int pCount;                 //Total block count for program card
    int startAddrD;             //Starting Address of Data card on Drum
    int dCount;                 //Total block count for data card

public:
    void setPCB(int id, int ttl, int tll){              //PCB
        job_id = id;
        TTL = ttl;
        TLL = tll;
        TTC = 0;
        LLC = 0;
        TSC = 0;
        TS = -1;
        startAddrP = -1;
        startAddrD = -1;
        pCount = 0;
        dCount = 0;
    }

    void statPCB(){
        logFile << endl << "F = " << F << ", P = " << startAddrP << ", = " << pCount << ", D = " << startAddrD << ", = " << dCount << endl;
    }

    int getTTL() {return TTL;}
    int getTLL() {return TLL;}
    int getTTC() {return TTC;}
    int getLLC() {return LLC;}
    void incTTC() {TTC++;}
    void incLLC() {LLC++;}
    void incTSC() {TSC++;}
    bool ttcEqualsTTL() {return TTC == TTL;}
    bool tscEqualsTS() {return TSC == TS;}
    void setF(char temp) {F = temp;}
    char getF() {return F;}
    void setStrAddrP(int addr) {startAddrP = addr;}
    int getStrAddrP() {return startAddrP;}
    void incPCount() {pCount++;}
    int getPCount() {return pCount;}
    void setStrAddrD(int addr) {startAddrD = addr;}
    int getStrAddrD() {return startAddrD;}
    void incDCount() {dCount++;}
    int getDCount() {return dCount;}
}pcb;

/*Memory*/
//char mainMemory[300][4];                //Memory block of 300 locations of 4 bytes
class mainMemory {
private:
    char mem[300][4];

public:
    mainMemory(){
        memset(mem, '\0', 1200);
    }

    void init(){
        memset(mem, '\0', 1200);
    }

    void setPageTable(int PTR){
        memset(mem[PTR], '*', 40);
    }

    void getPT(){
        int start = getPTR();
        cout << "PTR = " << start << endl;
        for(int i=start; i<start+10; i++){
            cout<<"M["<<i<<"] :";
            for(int j=0 ; j<4; j++){
                cout<<mem[i][j];
            }
            cout<<endl;
        }
        cout<<endl;
    }

    void readMem(int loc = -1){
        for(int i=0; i < (loc == -1 ? 300 : loc); i++){
            cout<<"M["<<i<<"] :";
            for(int j=0 ; j<4; j++){
                cout<<mem[i][j];
            }
            cout<<endl;
        }
        cout<<endl;
    }
}MM;

//char supervisoryStorage[100][4];        //Supervisory storage of 10 buffers of 40 bytes
class supervisoryStorage {
private:
    queue<string> ifb;                          //Inputful Buffer Queue
    queue<string> ofb;                          //Outputful Buffer Queue
    int efbc;                                   //Empty Buffer Counter

public:
    supervisoryStorage(){
        efbc = 10;
    }

    void init(){
        efbc = 10;
        while(!ifb.empty()) ifb.pop();
        while(!ofb.empty()) ofb.pop();
    }

    int getEfbCount(){
        return efbc;
    }

    bool ifbEmpty(){
        if(ifb.empty()) return true;
        return false;
    }

    bool ofbEmpty(){
        if(ofb.empty()) return true;
        return false;
    }

    void pushIFB(){
        if(efbc > 0){
            string buf;
            getline(fin, buf);
            ifb.push(buf);
            efbc--;
        }
        else{
            cout << "Error: No empty buffer." << endl;
        }
    }

    string popIFB(){
        if(!ifb.empty()){
            string temp = ifb.front();
            ifb.pop();
            efbc++;
            return temp;
        }
        cout << "Error: No inputful buffer." << endl;
        return "";
    }

    string getIFB(){
        if(!ifb.empty()){
            string temp = ifb.front();
            return temp;
        }
        cout << "Error: No inputful buffer." << endl;
        return "";
    }

    void pushOFB(string buf){
        if(efbc > 0){
            ofb.push(buf);
            efbc--;
        }
        else{
            cout << "Error: No empty buffer." << endl;
        }
    }

    string popOFB(){
        if(!ofb.empty()){
            string temp = ofb.front();
            ofb.pop();
            efbc++;
            return temp;
        }
        cout << "Error: No outputful buffer." << endl;
        return "";
    }
    
}ss;

//char auxiliaryStorage[500][4];          //Auxiliary storage of 500 locations
class auxiliaryStorage {
private:
    char as[500][4];

public:
    auxiliaryStorage(){
        memset(as, '\0', 2000);
    }

    void init(){
        memset(as, '\0', 2000);
    }

    void storeAS(){
        int m;

        if(pcb.getF() == 'P'){
            cout << "P" << endl;
            if(pcb.getStrAddrP() != -1){
                m = pcb.getStrAddrP() + 10 * pcb.getPCount();
            }
            else{
                m = 0;
                while(as[m][0] != '\0')
                    m += 10;
                pcb.setStrAddrP(m);
            }            
            pcb.incPCount();
        }
        else{
            cout << "D" << endl;
            if(pcb.getStrAddrD() != -1){
                m = pcb.getStrAddrD() + 10 * pcb.getDCount();
            }
            else{
                m = 0;
                while(as[m][0] != '\0')
                    m += 10;
                pcb.setStrAddrD(m);
            }
            pcb.incDCount();
        }
        
        strcpy(as[m], ss.popIFB().c_str());
    }

    void readAS(int loc = -1){
        for(int i=0; i < (loc == -1 ? 500 : loc); i++){
            cout<<"as["<<i<<"] :";
            for(int j=0 ; j<4; j++){
                cout << as[i][j];
            }
            cout << endl;
        }
        cout << endl;
    }

}as;

class clock {
private:
    int clk;                                   //Universal timer (clock)

public:
    clock(){
        clk = 0;
    }

    void getTime(){
        cout << "Universal Clock = " << clk << ", Channel 1 timer = " << chTimer[1] << ", Channel 2 timer = " << chTimer[2] <<
            ", Channel 3 timer = " << chTimer[3] << endl;
    }

    void simulate(int chID){
        logFile << "chID = " << chID << ", Clock = " << clk << ", TTC = " << pcb.getTTC();
        // pcb.incTTC();
        // if(pcb.ttcEqualsTTL()){
        //     setTI(2);
        // }

        // pcb.incTSC();
        // if(pcb.tscEqualsTS()){
        //     setTI(1);
        // }

        for(int i=0; i<chTimeCount[chID]; i++){
            pcb.incTTC();
            if(pcb.ttcEqualsTTL()){
                setTI(2);
                return;
            }

            pcb.incTSC();
            if(pcb.tscEqualsTS()){
                setTI(1);
                return;
            }

            clk++;
            for(int j=1; j<4; j++){
                chTimer[j]++;
                if(chFlags[j] && chTimer[j] == chTimeCount[j]){
                    setIOI(chInterrupt[j]);
                    chFlags[j] = false;
                }
            }
        }
        logFile << ", Clock = " << clk << ", TTC = " << pcb.getTTC();
    }

}uClk;

class cpu {
private:
    char IR[4];                             //Instruction register 4 bytes
    char R[5];                              //General Purpose Regsiter 4 bytes
    int PTR;                                //Page Table Register

    int IC;                                 //Instruction Counter
    int C;                                  //Toggle
    
    int SI;                                 //Service Interrupt
    int PI;                                 //Program Interrupt
    int TI;                                 //Time Interrupt
    int IOI;                                //Input-Output Interrupt

    queue<PCB> LQ;                                 //Load Queue
    queue<PCB> RQ;                                 //Ready Queue
    queue<PCB> IOQ;                                //Input-Output Queue
    queue<PCB> TQ;                                 //Terminate Queue
    
    string caseTask;
    bool breakFlag;                         //To terminate current execution

public:
    cpu(){
        memset(IR, '\0', 4);
        memset(R, '\0', 5);
        C = 0;
        SI = 0;
        PI = 0;
        TI = 0;
        IOI = 1;
        breakFlag = false;
    }

    char* getIR() {return IR;}
    void setIR(char* temp) {memcpy(IR, temp, 4);}
    char* getR() {return R;}
    void setR(char* temp) {memcpy(R, temp, 4);}
    int getPTR() {return PTR;}
    void setPTR(int val) {PTR = val;}
    int getIC() {return IC;}
    void setIC(int val) {IC = val;}
    int getC() {return C;}
    void setC(int val) {C = val;}
    int getSI() {return SI;}
    void setSI(int val) {SI = val;}
    int getPI() {return PI;}
    void setPI(int val) {PI = val;}
    int getTI() {return TI;}
    void setTI(int val) {TI = val;}
    int getIOI() {return IOI;}
    void setIOI(int val) {IOI += val;}
    bool getBreakFlag() {return breakFlag;}
    void setBreakFlag(bool flag) {breakFlag = flag;}

    void start(){
        init();
        mos();
    }

    void init(){
        logFile << "Initialising..." << endl;
        memset(IR, '\0', 4);
        memset(R, '\0', 5);
        C = 0;
        SI = 0;
        PI = 0;
        TI = 0;
        IOI = 1;
        breakFlag = false;

        MM.init();
        ss.init();
        as.init();
    }

    void mos(){
        logFile << "In Master mode, SI = " << SI << ", PI = " << PI << ", TI = " << TI << ", IOI = " << IOI << endl;
        if(TI == 2){
            breakFlag = true;
            return;
        }

        switch (IOI)
        {
            case 0:
                break;
            case 1:
                IR1();
                break;
            case 2:
                IR2();
                break;
            case 3:
                IR2();
                IR1();
                break;
            case 4:
                IR3();
                break;
            case 5:
                IR1();
                IR3();
                break;
            case 6:
                IR3();
                IR2();
                break;
            case 7:
                IR2();
                IR1();
                IR3();
                break;
            default:
                cout << "Invalid IOI value!" << endl;
                break;
        }

        if(SI != 0 || PI != 0 || TI != 0 || IOI != 0) mos();
    }

    void startCH(int chID){
        IOI -= chInterrupt[chID];
        chTimer[chID] = 0;
        chFlags[chID] = true;
    }

    void IR1(){
        char temp[5];                           //Temporary Variable to check for $AMJ, $DTA, $END
        startCH(1);
        logFile << "IOI = " << IOI << ", IR1 -- ";
        

        if(!fin.eof() && ss.getEfbCount() > 0){
            ss.pushIFB();                                     

            memset(temp, '\0', 5);
            memcpy(temp, ss.getIFB().c_str(), 4);
            
            if(!strcmp(temp,"$AMJ")){
                
                int jobId, TTL, TLL;
                memcpy(temp, ss.getIFB().c_str()+4, 4);
                jobId = atoi(temp);
                memcpy(temp, ss.getIFB().c_str()+8, 4);
                TTL = atoi(temp);
                memcpy(temp, ss.getIFB().c_str()+12, 4);
                TLL = atoi(temp);
                pcb.setPCB(jobId, TTL, TLL);
                
                pcb.setF('P');
                
                PTR = allocate()*10;
                MM.setPageTable(PTR);
                
                ss.popIFB();

                cout << "AMJ" << endl;    
            }
            else if(!strcmp(temp,"$DTA")){
                cout << "DTA" << endl;
                pcb.setF('D');

                ss.popIFB();
            }
            else if(!strcmp(temp,"$END")){
                //PLACE PCB ON LQ
                cout << "END" << endl;
                LQ.push(pcb);

                ss.popIFB();

                /*if(!LQ.empty()){
                    IR3("LD");
                }*/
                //......
            }
            else{

                if(!ss.ifbEmpty()){
                   caseTask = "IS";
                }
            }
            logFile << "IR1 simulate -- ";
            uClk.simulate(1);
            logFile << ", IOI = " << IOI << endl;
        }
    }

    void IR2(){ startCH(2); logFile << "IOI = " << IOI << ", IR2 -- " << endl;}
    
    void IR3(){
        startCH(3);
        logFile << "IOI = " << IOI << ", IR3 -- ";
        
        if(caseTask == "IS"){
            caseTask = "";
            as.storeAS();
            as.readAS(20);
            
            logFile << "IR3 simulate -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;
        }
    }
}cpu;

//PCB pcb;                        //Global Process Control Block

/*Error Messages*/
string error[7] = {"No Error", "Out of Data", "Line Limit Exceeded", "Time Limit Exceeded",
    "Operation Code Error", "Operand Error", "Invalid Page Fault"};

// /*Functions*/
// void init();
// void mos();

// int addressMap(int VA);

// //void execute_user_program();
// void start_execution();

int getPTR(){ return cpu.getPTR(); }
void setTI(int value){ cpu.setTI(value); }
void setIOI(int value){ cpu.setIOI(value); }

int main()
{
    cpu.start();
    return 0;
}