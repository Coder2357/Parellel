#include<iostream>
using namespace std;
const int N=10240;
int array[10240];
static int sum=0;
int add(int n,int* a){
    if(n>2){
        if(n%2==0){
            int count=0;
            for(int i=0;i<n;i=i+2){
                int sum1=0,sum2=0;
                sum1=sum1+a[i];
                sum2=sum2+a[i+1];
                a[count]=0;
                a[count]=sum1+sum2;
                count++;
            }
            return add(n/2,a);
        }
        else{
            int count=0;
            for(int i=0;i<n-2;i=i+2){
                int sum1=0,sum2=0;
                sum1=sum1+a[i];
                sum2=sum2+a[i+1];
                a[count]=0;
                a[count]=sum1+sum2;
                count++;
            }
            a[count]=a[n-1];
            return add(n/2+1,a);
        }
    }
    if(n==2){
        sum=a[0]+a[1];
        return sum;
    }
}
void Init(int n){
    for(int i=0;i<n;i++){
        array[i]=i;
    }
}
int main(){
    for(int i=0;i<N;i++){
        sum=0;
        Init(N);
        add(N,array);
    }
    cout<<sum<<endl;
    return 0;
}

