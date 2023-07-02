#include <iostream>
#include <chrono>
#include <CL/sycl.hpp>
using namespace cl::sycl;
using namespace std;
const int N = 512;//用例大小
const int TestTime=5;//测试时间
buffer<float, 2> buf(range(N, N));//测试用例

void m_reset(){//生成测试用例
    host_accessor m{ buf ,read_write };
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
    queue q;
    for(int k=0;k<N;k++){
        
        // for(int j=k+1;j<N;j++){
        //     m[k][j]=m[k][j]/m[k][k];
        // }                           //除法部分
		q.submit([&](handler& h) {
			accessor m{ buf, h, read_write };
			h.parallel_for(range(N - k), [=](auto idx) {
				int j = k + idx;
				m[k][j] = m[k][j] / m[k][k];
			});
		});
        
        //m[k][k] = 1.0;
        host_accessor r{ buf ,read_write };
		r[k][k] = 1;
        
        // for (int i=k+1;i<N;i++){
        //     for (int j=k+1;j<N;j++){
        //         m[i][j]-=m[i][k]*m[k][j];
        //     }                       //消去部分
        //     m[i][k]=0;
        // }     
		q.submit([&](handler& h) {
			accessor m{ buf, h, read_write };
			h.parallel_for(range(N - (k + 1), N - (k + 1)), [=](auto idx) {
				int i = k + 1 + idx.get_id(0);
				int j = k + 1 + idx.get_id(1);
				m[i][j] = m[i][j] - m[i][k] * m[k][j];
			});
		});
		q.submit([&](handler& h) {
			accessor m{ buf, h, read_write };
			h.parallel_for(range(N - (k + 1)), [=](auto idx) {
				int i = k + 1 + idx;
				m[i][k] = 0;
			});
		});       
    }
    q.wait();
}

int main(){
    m_reset();
    Gaussian();
    for(int i=0;i<TestTime;i++){
        m_reset();
        std::chrono::duration<double, std::milli> time{};
        auto start = std::chrono::high_resolution_clock::now();
        Gaussian();
        auto end = std::chrono::high_resolution_clock::now();
        time += end - start;
        std::cout << time.count() << std::endl;
    }
    return 0;
}