#include <iostream>
#include <windows.h>
#include <pthread.h>
#include <malloc.h>
using namespace std;
const int N = 2048;//用例大小
const int TestTime=5;//测试时间
float m[N][N];//测试用例

typedef struct{
    int k;//消去轮次
    int t_id;//线程id
}threadParam_t;

void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;

    int k = p->k;//消去轮次
    int t_id = p->t_id;//线程编号
    int i = k + t_id + 1;//计算任务

    for(int j=k+1;j<N;j++){
        m[i][j]-=m[i][k]*m[k][j];
    }
    m[i][k]=0;
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
    for(int k=0;k<N;k++){
        for(int j=k+1;j<N;j++){
            m[k][j]=m[k][j]/m[k][k];
        }                           //除法部分
        m[k][k]=1.0;


        int worker_count=(N-1-k>=7)?7:(N-1-k);//工作线程数量,设置7保证效率
        pthread_t* handles=(pthread_t*)malloc(worker_count*sizeof(pthread_t));
        threadParam_t* param=(threadParam_t*)malloc(worker_count*sizeof(threadParam_t));
        for(int t_id=0;t_id<worker_count;t_id++){
            param[t_id].k=k;
            param[t_id].t_id=t_id;
        }

        for(int t_id=0;t_id<worker_count;t_id++){
            pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
        }

        for(int t_id=0;t_id<worker_count;t_id++){
            pthread_join(handles[t_id],NULL);
        }

        for (int i=k+1+worker_count;i<N;i++){
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
