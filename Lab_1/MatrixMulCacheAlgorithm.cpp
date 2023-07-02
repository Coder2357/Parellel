#include <iostream>
using namespace std;
const int N=10240;
int array[N][N];
int mul[N];
void Init(){
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            array[i][j]=i+j;
        }
        mul[i]=i;
    }
}
int main()
{
    Init();
    int* result=new int[N];
    for(int i=0;i<N;i++){//逐行访问
        result[i]=0;
        for(int j=0;j<N;j++){
            result[i]+=array[i][j]*mul[j];
        }
    }
    /*for(int i=0;i<rows;i++){
        cout<<result[i]<<endl;
    }*/
    return 0;
}
