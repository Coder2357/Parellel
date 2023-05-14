#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <omp.h>
using namespace std;
const int N = 2048;//������С
const int TestTime=5;//����ʱ��
float m[N][N];//��������

#define NUM_THREADS 8

void m_reset(){//���ɲ�������
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
    int i,j,k;
    float tmp;
    #pragma omp parallel num_threads(NUM_THREADS), private(i, j, k, tmp)
    for(k=0;k<N;k++){
        #pragma omp single
        {
            tmp=m[k][k];
            for(j=k+1;j<N;j++){
                m[k][j]=m[k][j]/tmp;
            }                           //��������
            m[k][k]=1.0;
        }
    }

        #pragma omp for
        for (i=k+1;i<N;++i){
            tmp=m[i][k];
            for (j=k+1;j<N;++j){
                m[i][j]-=tmp*m[k][j];
            }                       //��ȥ����
            m[i][k]=0;
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
