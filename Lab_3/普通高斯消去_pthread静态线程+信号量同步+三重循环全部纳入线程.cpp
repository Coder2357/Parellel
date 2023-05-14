#include <iostream>
#include <windows.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;
const int N = 2048;//用例大小
const int TestTime=5;//测试时间
float m[N][N];//测试用例

#define NUM_THREADS 8 //线程数量

typedef struct{
    int t_id;//线程id
}threadParam_t;

sem_t sem_leader;
sem_t sem_Division[NUM_THREADS-1];
sem_t sem_Elimination[NUM_THREADS-1];

void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    int t_id = p->t_id;//线程编号
    for(int k=0;k<N;++k){
        //t_id为0的线程做除法操作，其他工作线程先等待
        if(t_id==0){
            for(int j=k+1;j<N;j++){
                m[k][j]=m[k][j]/m[k][k];
            }
            m[k][k]=1.0;
        }
        else{
            sem_wait(&sem_Division[t_id-1]);
        }
        //t_id为0的任务线程唤醒其他工作线程,进行消去操作
        if(t_id==0){
            for(int i=0;i<NUM_THREADS-1;++i){
                sem_post(&sem_Division[i]);
            }
        }

        //循环划分任务
        for(int i=k+1+t_id;i<N;i+=NUM_THREADS){
            //消去
            for(int j=k+1;j<N;++j){
                m[i][j]-=m[i][k]*m[k][j];
            }
            m[i][k]=0.0;
        }


        if(t_id==0){
            for(int i=0;i<NUM_THREADS-1;++i){
                sem_wait(&sem_leader);//等待其他worker完成消去
            }

            for(int i=0;i<NUM_THREADS-1;++i){
                sem_post(&sem_Elimination[i]);//通知其他worker进入下一轮
            }
        }
        else{
            sem_post(&sem_leader);//通知leader,已完成消去任务
            sem_wait(&sem_Elimination[t_id-1]);//等待通知，进入下一轮
        }
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
    sem_init(&sem_leader,0,0);
    for(int i=0;i<NUM_THREADS-1;++i){
        sem_init(&sem_Division[i],0,0);
        sem_init(&sem_Elimination[i],0,0);
    }
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

    sem_destroy(&sem_leader);
    for(int t_id=0;t_id<NUM_THREADS-1;t_id++){
        sem_destroy(&sem_Division[t_id]);
        sem_destroy(&sem_Elimination[t_id]);
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
