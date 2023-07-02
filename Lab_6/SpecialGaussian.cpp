#include<iostream>
#include<cmath>
#include<chrono>
#include<fstream>
#include<sstream>
#include<cstring>
using namespace std;
const int TestTime = 5;
const int cols = 85401;//列数
const int E_rows=5724;//消元行行数
const int R_rows=756;//被消元行行数


void Init(int** E,int** R,int E_rows,int R_rows,int cols,int* line){
    /*数据读取初始化,构造E，R*/
    for (int i = 0; i < E_rows; i++){
        for (int j = 0; j < cols; j++){
            E[i][j] = 0;
        }
    }
    for (int i = 0; i < R_rows; i++){
        for (int j = 0; j < cols; j++){
            R[i][j] = 0;
        }
    }
    ifstream file("E.txt");
    char search[10000] = { 0 };
    int readNum,index = 0;
    while (file.getline(search, sizeof(search)) ){
        int Flags = 0;  
        stringstream readline(search);
        while (readline >> readNum) {
            if (Flags == 0) {
                line[index] = readNum;
                Flags = 1;
            }
            int offset = readNum % 32;
            int post = readNum / 32;
            int temp = 1 << offset;
            E[index][cols-post-1] += temp;
        }
        index++;
    }
    file.close();
    file.open("R.txt");
    index = 0;
    while (file.getline(search, sizeof(search))) {
        stringstream readline(search);
        int Flags = 0;      
        while (readline >> readNum) {
            if (Flags == 0) {
                index = readNum;   
                Flags = 1;
            }
            int offset = readNum % 32;  
            int post = readNum / 32;
            int temp = 1 << offset;
            R[index][cols-post-1] += temp;
        }
    }
    file.close();
}

bool IsNull(int** M, int index){
    for (int i = 0; i < cols; i++)
    {
        if (M[index][i] != 0)
            return false;
    }
    return true;
}

void Gaussian(int rowNum, int* line, int** R, int** E){
    for (int i = 0; i < rowNum; i++){
        // while (line[i] != -1){  
        //     if (IsNull(R, line[i])){
        //         for (int j = 0; j < cols; j++){
        //             R[line[i]][j] = E[i][j];
        //         }
        //         line[i] = -1; 
        //         break;
        //     }
        //     else{
        //         for (int j = 0; j < cols; i++){
        //             E[i][j] = E[i][j] ^ R[line[i]][j];//异或
        //         }   
        //         /*int k = 0;
        //         while (E[i][k] == 0 && i < cols){
        //             k++;
        //         }
        //         if (k == cols){
        //             line[i] = -1;
        //             return;
        //         }
        //         int temp=E[i][k];
        //         int j=0;
        //         while(temp!=0){
        //             temp=temp>>1;
        //             j++;
        //         }       
        //         line[i]=(cols-k+1)*32+j-1;*/  
        //     }
        // }
    }
}

int main()
{
    ifstream file("E.txt");
    char search[10000] = { 0 };
    file.close();
    int* line = new int[cols];    
    int** E = new int* [cols];
    int** R = new int* [cols];
    for (int i = 0; i < E_rows; ++i){
        E[i] = new int[cols];
    }
    for (int i = 0; i < R_rows; ++i){
        R[i] = new int[cols];
    }
    for(int i=0;i<TestTime;i++){
        Init(E,R,E_rows,R_rows,cols,line);
        std::chrono::duration<double, std::milli> time{};
        auto start = std::chrono::high_resolution_clock::now();
        Gaussian(E_rows, line, R, E);
        auto end = std::chrono::high_resolution_clock::now();
        time += end - start;
        std::cout << time.count() << std::endl;
    }

    return 0;
}