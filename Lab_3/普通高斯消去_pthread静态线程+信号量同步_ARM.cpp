#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
using namespace std;
const int N = 512;//用例大小
const int TestTime=5;//测试时间
float m[N][N];//测试用例

#define NUM_THREADS 7 //线程数量

typedef struct{
    int t_id;//线程id
}threadParam_t;

sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];

void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    int t_id = p->t_id;//线程编号

    for(int k=0;k<N;++k){
        sem_wait(&sem_workerstart[t_id]);
        for(int i=k+1+t_id;i<N;i+=NUM_THREADS){
            for(int j=k+1;j<N;j++){
                m[i][j]-=m[i][k]*m[k][j];
            }
            m[i][k]=0;
        }

        sem_post(&sem_main);
        sem_wait(&sem_workerend[t_id]);
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
    sem_init(&sem_main,0,0);
    for(int i=0;i<NUM_THREADS;++i){
        sem_init(&sem_workerstart[i],0,0);
        sem_init(&sem_workerend[i],0,0);
    }
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
    }

    for(int k=0;k<N;k++){
        //主线程做除法操作
        for(int j=k+1;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];
        }                           //除法部分
        m[k][k]=1.0;

        //开始唤醒工作线程
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_post(&sem_workerstart[t_id]);
        }

        //主线程睡眠(等待所有的工作线程完成次轮消去任务)
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_wait(&sem_main);
        }

        //主线程再次进入唤醒工作线进行下一轮次的消去任务
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_post(&sem_workerend[t_id]);
        }
    }


    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        pthread_join(handles[t_id],NULL);
    }

    sem_destroy(&sem_main);
    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        sem_destroy(&sem_workerstart[t_id]);
        sem_destroy(&sem_workerend[t_id]);
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