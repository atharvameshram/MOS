#include "os3.h"

int main()
{
    load();
    fin.close();
    fout.close();
    return 0;
}

void load(){
    init();
    mos();
}

void init(){
    memset(mainMemory, '\0', 1200);
    memset(supervisoryStorage, '\0', 400);
    memset(auxiliaryStorage, '\0', 2000);
    memset(IR, '\0', 4);
    memset(R, '\0', 5);
    C = 0;
    SI = 0;
    PI = 0;
    TI = 0;
    IOI = 1;
    breakFlag = false;
    for(int i=0; i<10; i++) efb.push(i);
}

void mos(){
    switch (IOI)
    {
        case 0:
            break;
        case 1:
            IR1();
            break;
        case 2:
            //IR2();
            break;
        case 3:
            //IR2();
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
            //IR2();
            break;
        case 7:
            //IR2();
            IR1();
            //IR3();
            break;
        default:
            cout << "Invalid IOI value!" << endl;
            break;
    }

}

void IR1(){
    //int m;                                  //Variable to hold memory location
    //int currPTR;                            //Points to the last empty location in Page Table Register
    char temp[5];                           //Temporary Variable to check for $AMJ, $DTA, $END
    //memset(buffer, '\0', 40);

    if(!fin.eof() && !efb.empty()){
        fin.getline(supervisoryStorage[efb.front()],41);                
        ifb.push(efb.front());                                      
        efb.pop();                                                  
        
        startCH(1);

        memset(temp, '\0', 5);
        memcpy(temp,supervisoryStorage[ifb.front()],4);
        
        if(!strcmp(temp,"$AMJ")){
            
            int jobId, TTL, TLL;
            memcpy(temp, supervisoryStorage[ifb.front()]+4, 4);
            jobId = atoi(temp);
            memcpy(temp, supervisoryStorage[ifb.front()]+8, 4);
            TTL = atoi(temp);
            memcpy(temp, supervisoryStorage[ifb.front()]+12, 4);
            TLL = atoi(temp);
            pcb.setPCB(jobId, TTL, TLL);
            
            pcb.F = 'P';
            
            PTR = allocate()*10;
            memset(mainMemory[PTR], '*', 40);
            
            memset(supervisoryStorage[ifb.front()], '\0', 40);
            efb.push(ifb.front());
            ifb.pop();

            cout << "AMJ" << endl;
            
            simulation(1);
        }
        else if(!strcmp(temp,"$DTA")){
            cout << "DTA" << endl;
            pcb.F = 'D';
            memset(supervisoryStorage[ifb.front()], '\0', 40);
            efb.push(ifb.front());
            ifb.pop();

            simulation(1);
        }
        else if(!strcmp(temp,"$END")){
            //PLACE PCB ON LQ
            cout << "END" << endl;
            LQ.push(pcb);

            memset(supervisoryStorage[ifb.front()], '\0', 40);
            efb.push(ifb.front());
            ifb.pop();

            /*if(!LQ.empty()){
                IR3("LD");
            }*/

            simulation(1);
        }
        else{

            if(!ifb.empty()){
                caseTask = "IS";
                IR3();
                simulation(1);
            }
            //simulation(1);

        }
    }
}


void IR3(){
    startCH(3);

    if(caseTask == "IS"){
        int m;

        if(pcb.F == 'P'){
            cout << "P" << endl;
            if(pcb.startAddrP != -1){
                m = pcb.startAddrP + 10 * pcb.pCount;
            }
            else{
                m = 0;
                while(auxiliaryStorage[m][0] != '\0')
                    m += 10;
                pcb.startAddrP = m;
            }            
            pcb.pCount++;
        }
        else{
            cout << "D" << endl;
            if(pcb.startAddrD != -1){
                m = pcb.startAddrD + 10 * pcb.dCount;
            }
            else{
                m = 0;
                while(auxiliaryStorage[m][0] != '\0')
                    m += 10;
                pcb.startAddrD = m;
            }
            pcb.dCount++;
        }
        
        strcpy(auxiliaryStorage[m], supervisoryStorage[ifb.front()]);

        cout << "PTR = " << PTR << endl;
        for(int i=0; i<50; i++){
            cout<<"DRUM["<<i<<"] :";
            for(int j=0 ; j<4; j++){
                cout<<auxiliaryStorage[i][j];
            }
            cout<<endl;
        }
        cout<<endl;

        memset(supervisoryStorage[ifb.front()], '\0', 40);
        efb.push(ifb.front());
        ifb.pop();
    }
    else if(caseTask == "LD"){
        while(pcb.F == 'P' ? pcb.pCount : pcb.dCount){
            int m;
          do
            {
                m = allocate() * 10;
            } while (auxiliaryStorage[m][0] != '\0');

            int currPTR = PTR;
            while(mainMemory[currPTR][0]!='*')
                currPTR++;

            itoa(m, mainMemory[currPTR], 10);
            currPTR++;

            strcpy(mainMemory[m*10], auxiliaryStorage[ifb.front()]);

            pcb.F == 'P' ? pcb.pCount-- : pcb.dCount--;
        }

        RQ.push(LQ.front());
        LQ.pop();
    }
    simulation(3);
    //.....
}

void startCH(int i){
    IOI -= chInterrupt[i];
    chTimer[i] = 0;
    chFlags[i] = true;
}

void simulation(int i){
    for(int count=1; count<=chTimeCount[i]; count++){
        if(pcb.TTC == pcb.TTL) TI = 2;
        pcb.TTC++;

        if(pcb.TSC == pcb.TS) TI = 1;
        pcb.TSC++;

        uniTimer++;

        for(int j=1; j<4; j++){
            if(chFlags[j]){
                chTimer[j]++;
                if(chTimer[j] == chTimeCount[j]){
                    IOI += chInterrupt[j];
                    chFlags[j] = false;
                }
            }
        }
    }

    cout << "TTC = " << pcb.TTC << ", Clock = " << uniTimer << ", IOI = " << IOI << endl;

    if(SI != 0 || PI != 0 || TI != 0 || IOI != 0) mos();
}

void start_execution(){
    IC = 0;
    //execute_user_program();
}

int addressMap(int VA){
    if(0 <= VA && VA < 100){
        int pte = PTR + VA/10;
        if(mainMemory[pte][0] == '*'){
            PI = 3;
            return 0;
        }
        cout << "In addressMap(), VA = " << VA << ", pte = " << pte << ", M[pte] = " << mainMemory[pte] << endl;
        return atoi(mainMemory[pte])*10 + VA%10;
    }
    PI = 2;
    return 0;
}

int allocate(){
    srand(time(0));
    return (rand() % 30);
}
