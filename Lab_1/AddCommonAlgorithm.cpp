#include<iostream>
using namespace std;
const int N=10240;
int array[10240];
void Init(int n){
    for(int i=0;i<n;i++){
        array[i]=i;
    }
}
int main(){
    int sum=0;
    for(int j=0;j<N;j++){
        Init(N);
        sum=0;
        for(int i=0;i<N;i++){
            sum=sum+array[i];
        }

    }
    cout<<sum<<endl;
    return 0;
}

