#include <iostream>
#include <windows.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;
const int N = 2048;//������С
const int TestTime=5;//����ʱ��
float m[N][N];//��������

#define NUM_THREADS 7 //�߳�����

typedef struct{
    int t_id;//�߳�id
}threadParam_t;

sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];

void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    int t_id = p->t_id;//�̱߳��

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
    sem_init(&sem_main,0,0);
    for(int i=0;i<NUM_THREADS;++i){
        sem_init(&sem_workerstart[i],0,0);
        sem_init(&sem_workerend[i],0,0);
    }
    //�����߳�
    pthread_t handles[NUM_THREADS];//������Ӧ��handle
    threadParam_t param[NUM_THREADS];//������Ӧ���߳����ݽṹ

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
    }

    for(int k=0;k<N;k++){
        //���߳�����������
        for(int j=k+1;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];
        }                           //��������
        m[k][k]=1.0;

        //��ʼ���ѹ����߳�
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_post(&sem_workerstart[t_id]);
        }

        //���߳�˯��(�ȴ����еĹ����߳���ɴ�����ȥ����)
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_wait(&sem_main);
        }

        //���߳��ٴν��뻽�ѹ����߽�����һ�ִε���ȥ����
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
