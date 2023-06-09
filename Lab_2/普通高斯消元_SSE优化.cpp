#include <iostream>
#include <windows.h>
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
//#include <smmintrin.h> //SSE4.1
//#include <nmmintrin.h> //SSE4.2
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
    __m128 t1,t2,t3;
    int j;
    for(int k=0;k<N;k++){
        float data1[4]={m[k][k],m[k][k],m[k][k],m[k][k]};
        t1=_mm_loadu_ps(data1);
        for(j=k+1;j<N-4;j+=4){//4 elements
            t2=_mm_loadu_ps(m[k]+j);
            t3=_mm_div_ps(t2,t1);
            _mm_storeu_ps(m[k]+j, t3);
        }
        for(;j<N;j++){m[k][j]=m[k][j]/m[k][k];}//除法部分
        m[k][k]=1.0;
        for (int i=k+1;i<N;i++){
            float data2[4]={m[i][k],m[i][k],m[i][k],m[i][k]};
            t1=_mm_loadu_ps(data2);
            for (j=k+1;j<N-4;j+=4){
                t2=_mm_loadu_ps(m[k]+j);
                t3=_mm_loadu_ps(m[i]+j);
                t3=_mm_sub_ps(t3,_mm_mul_ps(t1,t2));
                _mm_storeu_ps(m[i]+j, t3);
            }
            for(;j<N;j++){m[i][j]-=m[i][k]*m[k][j];}//消去部分
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
