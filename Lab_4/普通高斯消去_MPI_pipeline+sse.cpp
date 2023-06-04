#include "mpi.h"
#include <iostream>
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
//#include <smmintrin.h> //SSE4.1
//#include <nmmintrin.h> //SSE4.2

using namespace std;
const int N = 512;//用例大小
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

     __m128 t1,t2,t3;

    if(rank==0){//0号进程初始化矩阵
        m_reset();
    }

    double start_time = MPI_Wtime();
    int j=0;
    for(int k=0;k<N;k++){
        if(k>=r1&&k<=r2){
            float data1[4]={m[k][k],m[k][k],m[k][k],m[k][k]};
            t1=_mm_loadu_ps(data1);
            for(j=k+1;j<N-4;j+=4){//4 elements
                t2=_mm_loadu_ps(m[k]+j);
                t3=_mm_div_ps(t2,t1);
                _mm_storeu_ps(m[k]+j, t3);
            }
            for(;j<N;j++){m[k][j]=m[k][j]/m[k][k];}//除法部分
            m[k][k]=1.0;

            for(j=1;j<size;j++){
                MPI_Send(&m[k][0],N,MPI_FLOAT,j,0,MPI_COMM_WORLD);
            }
        }
        else{
            MPI_Recv(&m[k][0],N,MPI_FLOAT,0,0,MPI_COMM_WORLD,MPI_STATUSES_IGNORE);
        }

        for (int i=max(r1,k+1);i<r2;i++){
            float data2[4]={m[i][k],m[i][k],m[i][k],m[i][k]};
            t1=_mm_loadu_ps(data2);
            for (j=k+1;j<N-4;j+=4){
                t2=_mm_loadu_ps(m[k]+j);
                t3=_mm_loadu_ps(m[i]+j);
                t3=_mm_sub_ps(t3,_mm_mul_ps(t1,t2));
                _mm_storeu_ps(m[i]+j, t3);
            }
            for(;j<N;j++){m[i][j]-=m[i][k]*m[k][j];}//消去部分
            m[i][k]=0;
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
