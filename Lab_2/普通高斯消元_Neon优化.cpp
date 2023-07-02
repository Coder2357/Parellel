#include <stdio.h>
#include <iostream>
#include <arm_neon.h>
#include <sys/time.h>
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
    float32x4_t t1,t2,t3;
    int j;
    for(int k=0;k<N;k++){
        t1=vmovq_n_f32(m[k][k]);
        for(j=k+1;j<=N-4;j+=4){//4 elements
            t2=vld1q_f32(&(m[k][j]));
            t2=vdivq_f32(t2,t1);
            vst1q_f32(&(m[k][j]),t1);
        }
        for(;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];//除法部分
        }
        m[k][k]=1.0;
        for(int i=k+1;i<N;i++){
            t1=vmovq_n_f32(m[i][k]);
            for(j=k+1;j<=N-4;j+=4){
                t2=vld1q_f32(&(m[i][j]));
                t3=vld1q_f32(&(m[k][j]));
                t1=vmulq_f32(t1,t3);
                t2=vsubq_f32(t2,t1);
            }
            for(;j<N;j++){
                m[i][j]-=m[i][k]*m[k][j];
            }
            m[i][k] = 0;//消去部分
        }
    }
}

int main(){
    for(int i=0;i<TestTime;i++){
        m_reset();
        struct timeval start,end;//timers
        float timeuse;
        gettimeofday(&start,NULL);
        Gaussian();
        gettimeofday(&end,NULL);
        timeuse=1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
        printf ("%f s\n",timeuse/1000000);
    }
    return 0;
}
