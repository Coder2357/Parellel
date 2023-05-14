#include <iostream>
#include <windows.h>
#include <pthread.h>
using namespace std;
const int N = 2048;//������С
const int TestTime=5;//����ʱ��
float m[N][N];//��������

#define NUM_THREADS 7 //�߳�����

typedef struct{
    int t_id;//�߳�id
}threadParam_t;

//barrier����
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;



void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    int t_id = p->t_id;//�̱߳��
    for(int k=0;k<N;++k){
        //t_idΪ0���߳����������������������߳��ȵȴ�
        if(t_id==0){
            for(int j=k+1;j<N;j++){
                m[k][j]=m[k][j]/m[k][k];
            }
            m[k][k]=1.0;
        }

        //��һ��ͬ����
        pthread_barrier_wait(&barrier_Division);

        //ѭ����������
        for(int i=k+1+t_id;i<N;i+=NUM_THREADS){
            //��ȥ
            for(int j=k+1;j<N;++j){
                m[i][j]-=m[i][k]*m[k][j];
            }
            m[i][k]=0.0;
        }

        //�ڶ���ͬ����
        pthread_barrier_wait(&barrier_Elimination);
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
    //��ʼ��barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);

    //�����߳�
    pthread_t handles[NUM_THREADS];//������Ӧ��handle
    threadParam_t param[NUM_THREADS];//������Ӧ���߳����ݽṹ

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
