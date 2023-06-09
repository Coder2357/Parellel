#include <iostream>
#include <windows.h>
#include <immintrin.h> //AVX、AVX2、AVX-512
using namespace std;
const int N = 2048;//用例大小
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
    __m256 t1,t2,t3;
    int j;
    for(int k=0;k<N;k++){
        float data1[8]={m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k]};
        t1=_mm256_loadu_ps(data1);
        for(j=k+1;j<k+8-k%8;j++){
            m[k][j]=m[k][j]/m[k][k];//对齐
        }
        for (;j<=N-8;j+=8) {
            t2=_mm256_loadu_ps(m[k]+j);
            t3=_mm256_div_ps(t2,t1);
            _mm256_storeu_ps(m[k]+j,t3);//除法部分
        }
        for(;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];
        }
        for(int i=k+1;i<N;i++){
            float data2[8]={m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k]};
            t1=_mm256_loadu_ps(data2);
            for(j=k+1;j<k+8-k%8;j++){
                m[i][j]=m[i][j]-m[i][k]*m[k][j];//对齐
            }
            for(;j<=N-8;j+=8){
                t2=_mm256_loadu_ps(m[i]+j);
                t3=_mm256_loadu_ps(m[k]+j);
                t3=_mm256_sub_ps(t2,_mm256_mul_ps(t1,t3));
                _mm256_storeu_ps(m[i]+j,t3);
            }
            for(;j<N;j++){
                m[i][j]=m[i][j]-m[i][k]*m[k][j];//消去部分
            }
            m[i][k]=0;
        }
    }
}

int main(){
    long long head,tail,freq;// timers
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
