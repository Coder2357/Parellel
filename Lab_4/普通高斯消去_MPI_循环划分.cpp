#include "mpi.h"
#include <iostream>
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
    if(rank==0){//0号进程初始化矩阵
        m_reset();
    }
    double start_time = MPI_Wtime();
    int j=0;
    for(int k=0;k<N;k++){
        if(k%size==rank){
            for(int j=k+1;j<N;j++){
                m[k][j]=m[k][j]/m[k][k];
            }
            m[k][k]=1.0;
            for(j=1;j<size;j++){
                if(j!=rank){
                    MPI_Send(&m[k][0],N,MPI_FLOAT,j,0,MPI_COMM_WORLD);
                }
            }
        }
        else{
            MPI_Recv(&m[k][0],N,MPI_FLOAT,0,0,MPI_COMM_WORLD,MPI_STATUSES_IGNORE);

        }

        for (int i=k+1;i<N;i++){
            if(i%size==rank){
                for (int j=k+1;j<N;j++){
                    m[i][j]-=m[i][k]*m[k][j];
                }                       //消去部分
                m[i][k]=0;
            }
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
