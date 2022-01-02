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

/*Error Messages*/
string error[7] = {"No Error", "Out of Data", "Line Limit Exceeded", "Time Limit Exceeded",
    "Operation Code Error", "Operand Error", "Invalid Page Fault"};

int chTimer[4] = {0,0,0,0};                         //Timers for each channel i = 1,2,3
bool chFlags[4] = {false, true, true, true};        //Flags for each channel
int chTimeCount[4] = {0,5,5,2};                     //Time taken by each channel
int chInterrupt[4] = {0,1,2,4};                     //Interrupt values for each channel

class cpu;
class mainMemory;
class supervisoryStorage;
class auxiliaryStorage;
class clock;
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
    int TLC;                    //Total Line Counter
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
        TLC = 0;
        TSC = 0;
        TS = -1;
        startAddrP = -1;
        startAddrD = -1;
        pCount = 0;
        dCount = 0;
    }

    void clear(){
        setPCB(0,0,0);
    }

    void statPCB(){
        logFile << endl << "F = " << F << ", P = " << startAddrP << ", = " << pCount << ", D = " << startAddrD << ", = " << dCount << endl;
    }

    int getJobId() {return job_id;}
    int getTTL() {return TTL;}
    int getTLL() {return TLL;}
    int getTTC() {return TTC;}
    int getTLC() {return TLC;}
    void incTTC() {TTC++;}
    void incTLC() {TLC++;}
    void decTLC() {TLC--;}
    void incTSC() {TSC++;}
    bool ttcEqualsTTL() {return TTC == TTL;}
    bool tlcGreaterTLL() {return TLC > TLL;}
    bool tscEqualsTS() {return TSC == TS;}
    void setF(char temp) {F = temp;}
    char getF() {return F;}
    void setStrAddrP(int addr) {startAddrP = addr;}
    int getStrAddrP() {return startAddrP;}
    void incPCount() {pCount++;}
    void decPCount() {pCount--;}
    int getPCount() {return pCount;}
    void setStrAddrD(int addr) {startAddrD = addr;}
    int getStrAddrD() {return startAddrD;}
    void incDCount() {dCount++;}
    void decDCount() {dCount--;}  
    int getDCount() {return dCount;}
}pcb;

/*Memory*/
//char mainMemory[300][4];                //Memory block of 300 locations of 4 bytes
class mainMemory {
private:
    char mem[300][4];

public:
    mainMemory(){
        // memset(mem, '\0', 1200);
    }

    void init(){
        memset(mem, '\0', 1200);
    }

    void initPageTable(int PTR){
        memset(mem[PTR], '*', 40);
    }

    void storePageTable(int PTR, int m){
        int currPTR = PTR;
        while(mem[currPTR][0]!='*')
            currPTR++;

        itoa(m, mem[currPTR], 10);
    }

    void getPageTable(int start){
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

    string getMem(int loc, int lim = 4){
        string temp;
        
        if(lim == -1){
            for(int i=loc; i<loc+10; i++)
                for(int j=0; j<4; j++)
                    temp += mem[i][j];
        }
        else{
            for(int j=0; j<lim; j++)
                temp += mem[loc][j];
        }
        return temp;
    }

    int storeMem(int PTR, string buffer, int RA = -1){
        if(RA != -1){
            strcpy(mem[RA], buffer.c_str());
            return 0;
        }
        else
        {
            int m;
            do{
                m = allocate();
            }while(mem[m*10][0]!='\0');

            storePageTable(PTR, m);

            if(!buffer.empty())
                strcpy(mem[m*10], buffer.c_str());
            return m;
        }
    }

    void setMem(int RA, char R[]){
        memcpy(mem[RA], R, 4);
    }

    void clearMemBlock(int loc){
        memset(mem[loc], '\0', 40);
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
        // efbc = 10;
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
        // memset(as, '\0', 2000);
    }

    void init(){
        memset(as, '\0', 2000);
    }

    string getTrack(int loc){
        string temp;
        
        for(int i=loc; i<loc+10; i++)
            for(int j=0; j<4; j++)
                temp += as[i][j];
    
        return temp;
    }

    string getOsTrack(int& m){
        string temp = "";
        
        while(m != 500 && as[m][0] == '\0')
            m += 10;

        if(m != 500){
            for(int i=m; i<m+10; i++)
                for(int j=0; j<4; j++)
                    temp += as[i][j];
        }

        return temp;
    }

    void storeAS(int x = 0, int RA = 0){
        if(x == 1){
            int m = 0;
            while(as[m][0] != '\0')
                m += 10;
            strcpy(as[m], MM.getMem(RA, -1).c_str());
        }
        else{
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

    void clearTrack(int loc){
        memset(as[loc], '\0', 40);
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
        if(chID == -1){
            logFile << "Clock = " << clk << ", TTC = " << pcb.getTTC();
            clk++;
            
            pcb.incTTC();
            if(pcb.ttcEqualsTTL()){
                setTI(2);
                return;
            }
            logFile << ", Clock = " << clk << ", TTC = " << pcb.getTTC() << endl;
        }
        else{
            logFile << "chID = " << chID << ", Clock = " << clk << ", TTC = " << pcb.getTTC();
            
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
    
    string caseTask;                        //Channel 3 tasks
    bool breakFlag;                         //To terminate current execution
    bool eof;                               //End of file
    bool eos;                               //End of Output Spooling
    int ra;                                 //Real Address of memory for GD, PD
    int EM;                                 //Error Message Code = 0
    int EM2;                                //Error Messgae Code = -1

public:
    cpu(){
        // memset(IR, '\0', 4);
        // memset(R, '\0', 5);
        // C = 0;
        // SI = 0;
        // PI = 0;
        // TI = 0;
        // IOI = 1;
        // breakFlag = false;
        // eof = false;
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

    void initCpu(){
        memset(IR, '\0', 4);
        memset(R, '\0', 5);
        C = 0;
        SI = 0;
        PI = 0;
        TI = 0;
        IOI = 1;
        PTR = -1;
        breakFlag = false;
        eof = false;
        eos = false;
        EM = 0;
        EM2 = -1;
    }

    void init(){
        logFile << "Initialising..." << endl;
        
        initCpu();
        MM.init();
        ss.init();
        as.init();
    }

    int mos(){
        logFile << "In Master mode, SI = " << SI << ", PI = " << PI << ", TI = " << TI << ", IOI = " << IOI << endl;
        if(TI == 0 || TI == 1){
            if(SI != 0){
                switch(SI){
                    case 1:
                        //read(RA);
                        RQ.pop();
                        IOQ.push(pcb);
                        IR3();
                        break;
                    case 2:
                        //write(RA);
                        RQ.pop();
                        IOQ.push(pcb);
                        IR3();
                        break;
                    case 3:
                        //Terminate(0);
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    default:
                        cout<<"Error with SI."<<endl;
                }
                return 0;
            }
            else if(PI != 0){
                switch(PI){
                    case 1:
                        //Terminate(4);
                        EM = 4;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    case 2:
                        //Terminate(5);
                        EM = 5;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    case 3:
                        PI = 0;
                        //Page Fault checking
                        char temp[3];
                        memset(temp,'\0',3);
                        memcpy(temp, IR, 2);

                        if(!strcmp(temp,"GD") || !strcmp(temp,"SR")){
                            int m = MM.storeMem(PTR, "");
                            
                            cout << "Valid Page Fault, page frame = " << m << endl;
                            MM.readMem();

                            if(pcb.ttcEqualsTTL()){
                                TI = 2;
                                PI = 3;
                                mos();
                                break;
                            }
                            uClk.simulate(-1);
                            return 1;
                        }
                        else if(!strcmp(temp,"PD") || !strcmp(temp,"LR") || !strcmp(temp,"H") || !strcmp(temp,"CR") || !strcmp(temp,"BT")){
                            //Terminate(6);
                            EM = 6;
                            RQ.pop();
                            TQ.push(pcb);
                            breakFlag = true;

                            if(pcb.ttcEqualsTTL()){
                                TI = 2;
                                PI = 3;
                                mos();
                                break;
                            }
                            uClk.simulate(-1);
                        }
                        else{
                            PI = 1;
                            mos();
                        }
                        return 0;
                    default:
                        cout<<"Error with PI."<<endl;
                }
                PI = 0;
                return 0;
            }
        }
        else{
            if(SI != 0){
                switch(SI){
                    case 1:
                        //Terminate(3);
                        EM = 3;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    case 2:
                        //write(RA);
                        //if(!breakFlag) Terminate(3);
                        RQ.pop();
                        IOQ.push(pcb);
                        IR3();
                        if(!breakFlag){
                            EM = 3;
                            TQ.push(pcb);
                            breakFlag = true;
                        }
                        break;
                    case 3:
                        //Terminate(0);
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    default:
                        cout<<"Error with SI."<<endl;
                }
                SI = 0;
            }
            else if(PI != 0){
                switch(PI){
                    case 1:
                        //Terminate(3,4);
                        EM = 3;
                        EM2 = 4;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    case 2:
                        //Terminate(3,5);
                        EM = 3;
                        EM2 = 5;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    case 3:
                        //Terminate(3);
                        EM = 3;
                        RQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                        break;
                    default:
                        cout<<"Error with PI."<<endl;
                }
                PI = 0;
            }
            return 0;
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

        if(!(eof && eos) && (SI != 0 || PI != 0 || TI != 0 || IOI != 0)) mos();
        return 0;
    }

    void startCH(int chID){
        IOI -= chInterrupt[chID];
        chTimer[chID] = 0;
        chFlags[chID] = true;
    }

    void IR1(){
        char temp[5];                           //Temporary Variable to check for $AMJ, $DTA, $END

        if(!fin.eof() && ss.getEfbCount() > 0){
            startCH(1);
            logFile << "IOI = " << IOI << ", IR1 -- ";    
            ss.pushIFB();                                     

            memset(temp, '\0', 5);
            memcpy(temp, ss.getIFB().c_str(), 4);
            
            if(!strcmp(temp,"$AMJ")){
                
                int jobId, TTL, TLL;
                memcpy(temp, ss.getIFB().c_str()+4, 4);
                jobId = atoi(temp);
                cout << jobId << endl;
                memcpy(temp, ss.getIFB().c_str()+8, 4);
                TTL = atoi(temp);
                memcpy(temp, ss.getIFB().c_str()+12, 4);
                TLL = atoi(temp);
                pcb.setPCB(jobId, TTL, TLL);
                
                pcb.setF('P');
                
                if(PTR == -1){
                    PTR = allocate()*10;
                    MM.initPageTable(PTR);
                }

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
            }

            logFile << "IR1 simulate -- ";
            uClk.simulate(1);
            logFile << ", IOI = " << IOI << endl;
        }
        else if(fin.eof()){
            eof = true;
        }
    }

    std::string rtrim(const std::string &s)
    {
        size_t end = s.find_last_not_of('\0');
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    void IR2(){
        if(!ss.ofbEmpty()){
            startCH(2);
            logFile << "IOI = " << IOI << ", IR2 -- ";

            string temp = ss.popOFB();
            rtrim(temp);
            fout << temp;
            
            logFile << "IR2 simulate -- ";
            uClk.simulate(2);
            logFile << ", IOI = " << IOI << endl;
        }
        else{
            eos = true;
        }
    }
    
    void IR3(){
        if(!ss.ifbEmpty()){
            caseTask = "IS";
            startCH(3);
        }
        else
        {
            if(!TQ.empty()){
                if(ss.getEfbCount() > 0){
                    caseTask = "OS";
                    startCH(3);
                }
            }

            if(!LQ.empty()){
                caseTask = "LD";
                startCH(3);
            }

            if(!IOQ.empty()){
                if(SI == 1){
                    if(pcb.getDCount() == 0){
                        EM = 3; 
                        IOQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                    }
                    else{
                        caseTask = "RD";
                        startCH(3);
                    }
                }
                else if(SI == 2){
                    if(pcb.tlcGreaterTLL()){
                        EM = 2;
                        IOQ.pop();
                        TQ.push(pcb);
                        breakFlag = true;
                    }
                    else{
                        caseTask = "WT";
                        startCH(3);
                    }
                }
                SI = 0;
            }
        }

        logFile << "IOI = " << IOI << ", IR3 -- ";
        
        if(caseTask == "IS"){
            caseTask = "";

            as.storeAS();
            as.readAS(20);
            
            logFile << "IR3 simulate IS -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;
        }
        else if(caseTask == "LD"){
            caseTask = "";

            int start = pcb.getStrAddrP();
            MM.storeMem(PTR, as.getTrack(start));
            as.clearTrack(start);
            MM.readMem();

            pcb.setStrAddrP(start+10);
            pcb.decPCount();
            
            logFile << "IR3 simulate LD -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;

            if(pcb.getPCount() == 0){
                LQ.pop();
                RQ.push(pcb);
                start_execution();
            }
        }
        else if(caseTask == "RD"){
            caseTask = "";

            int start = pcb.getStrAddrD();
            MM.storeMem(PTR, as.getTrack(start), ra);
            as.clearTrack(start);
            MM.readMem();

            pcb.setStrAddrD(start+10);
            pcb.decDCount();
            
            IOQ.pop();
            RQ.push(pcb);

            logFile << "IR3 simulate RD -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;
        }
        else if(caseTask == "WT"){
            caseTask = "";

            as.storeAS(1, ra);
            as.readAS(20);
            pcb.incTLC();

            if(TI == 2){
                TQ.push(pcb);
                breakFlag = true;
            }
            else    RQ.push(pcb);

            IOQ.pop();

            logFile << "IR3 simulate WT -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;
        }
        else if(caseTask == "OS"){
            caseTask = "";

            int m;
            string temp = as.getOsTrack(m);

            if(m == 500){
                ss.pushOFB("\n");
                ss.pushOFB("\n");

                int jobId = pcb.getJobId();
                pcb.clear();
                TQ.pop();
                as.init();
                MM.init();

                if(EM == 0){ 
                    ss.pushOFB("Job id #" + to_string(jobId) + " terminated normally. " + error[EM]);
                }
                else{
                    ss.pushOFB("Job id #" + to_string(jobId) + " terminated abnormally due to " + error[EM] + (EM2 != -1 ? (". " + error[EM2]) : ""));
                    ss.pushOFB("IC="+to_string(IC)+", IR="+IR+", C="+to_string(C)+", R="+R+", TTL="+to_string(pcb.getTTL())+", TTC="+to_string(pcb.getTTC())+", TLL="+to_string(pcb.getTLL())+", TLC="+to_string(pcb.getTLC()));
                }
            }
            else{
                eos = false;
                rtrim(temp);
                ss.pushOFB(temp+"\n");
                as.clearTrack(m);
                pcb.decTLC();

                as.getOsTrack(m);
                if(m == 500){
                    ss.pushOFB("\n");
                    ss.pushOFB("\n");

                    pcb.clear();
                    TQ.pop();
                    as.init();
                    MM.init();

                    if(EM == 0){ 
                        ss.pushOFB("Job id #" + to_string(pcb.getJobId()) + " terminated normally. " + error[EM] + "\n");
                    }
                    else{
                        ss.pushOFB("Job id #" + to_string(pcb.getJobId()) + " terminated abnormally due to " + error[EM] + (EM2 != -1 ? (". " + error[EM2]) : "") + "\n");
                        ss.pushOFB("IC="+to_string(IC)+", IR="+IR+", C="+to_string(C)+", R="+R+", TTL="+to_string(pcb.getTTL())+", TTC="+to_string(pcb.getTTC())+", TLL="+to_string(pcb.getTLL())+", TLC="+to_string(pcb.getTLC()) + "\n");
                    }
                }
            }
            logFile << "IR3 simulate OS -- ";
            uClk.simulate(3);
            logFile << ", IOI = " << IOI << endl;
            
        }
        else{
            logFile << endl;
        }
    }
    
    void start_execution(){
        IC = 0;
        execute_user_program();
    }

    int addressMap(int VA){
        if(0 <= VA && VA < 100){
            int pte = PTR + VA/10;
            if(MM.getMem(pte, 1) == "*"){
                PI = 3;
                return 0;
            }
            cout << "In addressMap(), VA = " << VA << ", pte = " << pte << ", M[pte] = " << MM.getMem(pte) << endl;
            return atoi(MM.getMem(pte).c_str())*10 + VA%10;
        }
        PI = 2;
        return 0;
    }


    void execute_user_program(){
        char temp[3], loca[2];
        int locIR, RA;
        
        while(true){
            if(breakFlag) break;
            
            RA = addressMap(IC);
            if(PI != 0){
                if(mos()){
                    continue;
                }
                break;
            }
            cout << "IC = " << IC << ", RA = " << RA << endl;
            memcpy(IR, MM.getMem(RA).c_str(), 4);           //Memory to IR, instruction fetched
            IC += 1;

            memset(temp,'\0',3);
            memcpy(temp,IR,2);
            for(int i=0; i<2; i++){
                if(!((47 < IR[i+2] && IR[i+2] < 58) || IR[i+2] == 0)){
                    PI = 2;
                    break;
                }
                loca[i] = IR[i+2];
            }

            if(PI != 0){
                mos();
                break;
            }
            
            //loca[0] = IR[2];
            //loca[1] = IR[3];
            locIR = atoi(loca);
            
            RA = addressMap(locIR);
            if(PI != 0){
                if(mos()){
                    IC--;
                    continue;
                }
                break;
            }

            cout << "IC = " << IC << ", RA = " << RA << ", IR = " << IR << endl;
            if(pcb.ttcEqualsTTL()){
                TI = 2;
                PI = 3;
                mos();
                break;
            }

            if(!strcmp(temp,"LR")){
                memcpy(R, MM.getMem(RA).c_str(), 4);
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"SR")){
                // memcpy(M[RA],R,4);
                MM.setMem(RA, R);
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"CR")){
                if(!strcmp(R,MM.getMem(RA).c_str()))
                    C = 1;
                else
                    C = 0;
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"BT")){
                if(C == 1)
                    IC = RA;
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"GD")){
                SI = 1;
                ra = RA;
                mos();
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"PD")){
                SI = 2;
                ra = RA;
                mos();
                uClk.simulate(-1);
            }
            else if(!strcmp(temp,"H")){
                SI = 3;
                mos();
                uClk.simulate(-1);
                SI = 0;
                break;
            }
            else{
                PI = 1;
                mos();
                break;
            }
            memset(IR, '\0', 4);
        }
    }

}cpu;

void setTI(int value){ cpu.setTI(value); }
void setIOI(int value){ cpu.setIOI(value); }

int main()
{
    cpu.start();
    fin.close();
    fout.close();
    return 0;
}