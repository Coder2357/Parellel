#include <iostream>
#include <windows.h>
using namespace std;
const int N = 512;//用例大小
const int TestTime=5;//测试时间
float m[N][N];//测试用例
void m_reset(){//生成测试用例
    for(int i=0;i<N;i++){
        for(int j=0;j<i;j++)
            m[i][j]=0;
        m[i][i]=1.0;
        for(int j=i+1;j<N;j++)
            m[i][j]=rand();
    }
    for(int k=0;k<N;k++)
        for(int i=k+1;i<N;i++)
                for(int j=0;j<N;j++)
                    m[i][j]+=m[k][j];
}

void Gaussian(){
    for(int k=0;k<N;k++){
        for(int j=k+1;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];
        }                           //除法部分
        m[k][k]=1.0;
        for (int i=k+1;i<N;i++){
            for (int j=k+1;j<N;j++){
                m[i][j]-=m[i][k]*m[k][j];
            }                       //消去部分
            m[i][k]=0;
        }
    }
}

int main(){
    long long head, tail, freq;// timers
    for(int i=0;i<TestTime;i++){
        m_reset();
        // similar to CLOCKS_PER_SEC
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        // start time
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
        Gaussian();
        // end time
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        cout<<(tail - head)*1000.0/freq<< "ms" << endl;
    }
    return 0;
}
