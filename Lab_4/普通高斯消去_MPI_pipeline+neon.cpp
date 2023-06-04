#include "mpi.h"
#include <iostream>
#include <arm_neon.h>
using namespace std;
const int N = 2048;//用例大小
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

    int rank,size;
    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int r1=rank*(N-N%size)/size;
    int r2=(rank==size-1)?N-1:(rank+1)*(N-N%size)/size-1;

    if(rank==0){//0号进程初始化矩阵
        m_reset();
    }

    float32x4_t t1,t2,t3;
    double start_time = MPI_Wtime();
    int j=0;
    for(int k=0;k<N;k++){
        if(k>=r1&&k<=r2){
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
            for(j=1;j<size;j++){
                MPI_Send(&m[k][0],N,MPI_FLOAT,j,0,MPI_COMM_WORLD);
            }
        }
        else{
            MPI_Recv(&m[k][0],N,MPI_FLOAT,0,0,MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
        }

        for (int i=max(r1,k+1);i<r2;i++){
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
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0)
    {
        double end_time = MPI_Wtime();
        cout <<"mpi:"<<(end_time - start_time) * 1000<<"ms"<<endl;
    }
    MPI_Finalize();
}

int main(){


    Gaussian();

    return 0;
}
