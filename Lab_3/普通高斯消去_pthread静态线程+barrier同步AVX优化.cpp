#include <iostream>
#include <windows.h>
#include <pthread.h>
#include <immintrin.h> //AVX、AVX2、AVX-512

using namespace std;
const int N = 2048;//用例大小
const int TestTime=5;//测试时间
float m[N][N];//测试用例

#define NUM_THREADS 7 //线程数量

typedef struct{
    int t_id;//线程id
}threadParam_t;

//barrier定义
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;



void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    __m256 t1,t2,t3;
    int j;

    int t_id = p->t_id;//线程编号
    for(int k=0;k<N;++k){
        float data1[8]={m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k],m[k][k]};
        t1=_mm256_loadu_ps(data1);

        //t_id为0的线程做除法操作，其他工作线程先等待
        if(t_id==0){
            for (j=k+1;j<=N-8;j+=8) {
                t2=_mm256_loadu_ps(m[k]+j);
                t3=_mm256_div_ps(t2,t1);
                _mm256_storeu_ps(m[k]+j,t3);//除法部分
            }
            for(;j<N;j++){
                m[k][j]=m[k][j]/m[k][k];
            }
            m[k][k]=1.0;
        }

        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        //循环划分任务
        for(int i=k+1+t_id;i<N;i+=NUM_THREADS){
            float data2[8]={m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k],m[i][k]};
            t1=_mm256_loadu_ps(data2);

            //消去
            for(j=k+1;j<=N-8;j+=8){
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

        //第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);
}

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
    //初始化barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);

    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
    }

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        pthread_join(handles[t_id],NULL);
    }

    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);
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
