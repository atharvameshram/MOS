#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

/*File Handlers*/
ifstream fin("input.txt");
ofstream fout("output.txt");

/*Memory*/
char M[100][4];
char buffer[40];
char IR[4];
char R[5];
int IC;
int C;

void init(){
    memset(M, '\0', 400);
    memset(buffer, '\0', 40);
    memset(IR, '\0', 4);
    memset(R, '\0', 5);
    C = 0;
}

void read(int loc){
    fin.getline(M[loc], 40);
}

void write(int loc){
    char str[40];
    int k = 0;
    for(int i=loc; i<(loc+10); i++){
        for(int j=0; j<4; j++)
            str[k++] = M[i][j];
    }
    fout << str << endl;
}

void Terminate(){
    fout << endl << endl;
}

void mos(int SI, int loc = 0){
    switch(SI){
        case 1:
            read(loc);
            break;
        case 2:
            write(loc);
            break;
        case 3:
            Terminate();
            break;
        default:
            cout<<"Error! Wrong SI."<<endl;
    }
}

void execute_user_program(){
    char temp[3], loca[2];
    int loc;
    while(true){
        memcpy(IR, M[IC], 4);
        IC += 1;
        memcpy(temp,IR,2);
        loca[0] = IR[2];
        loca[1] = IR[3];
        loc = atoi(loca);
        if(!strcmp(temp,"LR"))
            memcpy(R,M[loc],4);
        else if(!strcmp(temp,"SR"))
            memcpy(M[loc],R,4);
        else if(!strcmp(temp,"CR")){
            if(!strcmp(R,M[loc]))
                C = 1;
            else
                C = 0;
        }
        else if(!strcmp(temp, "AD")){
            char val[4];
            memcpy(val, M[loc], 4);
            memcpy(R, to_string(atoi(R) + atoi(val)).c_str(), 4);
        }
        else if(!strcmp(temp, "SB")){
            char val[4];
            memcpy(val, M[loc], 4);
            memcpy(R, to_string(atoi(R) - atoi(val)).c_str(), 4);
        }
        else if(!strcmp(temp,"BT")){
            if(C == 1)
                IC = loc;
        }
        else if(!strcmp(temp,"GD"))
            mos(1, loc);
        else if(!strcmp(temp,"PD"))
            mos(2, loc);
        else if(!strcmp(temp,"H")){
            mos(3);
            break;
        }
    }
}

void start_execution(){
    IC = 0;
    execute_user_program();
}

void load(){
    int m = 0;
    char temp[5];
    while(!fin.eof()){
        fin.getline(buffer,41);
        memset(temp, '\0', 5);
        memcpy(temp,buffer,4);
        if(!strcmp(temp,"$AMJ"))
            init();
        else if(!strcmp(temp,"$DTA"))
            start_execution();
        else if(!strcmp(temp,"$END"))
            m = 0;
        else{
            strcpy(M[m],buffer);
            for(int i=m; i<m+10; i++){
                cout<<"M["<<i<<"] :";
                for(int j=0 ; j<4; j++){
                    cout<<M[i][j];
                }
                cout<<endl;
            }
            cout<<endl;
            m += 10;
        }
    }
}

int main()
{
    load();
    fin.close();
    fout.close();
    return 0;
}