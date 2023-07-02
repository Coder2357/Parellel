#include <sycl/sycl.hpp>
#include <dpct/dpct.hpp>
#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip>
#include <windows.h>
#include <chrono>

using namespace std;
const int N = 2048;
const int BLOCK_SIZE = 32;
float A[N][N];

void reset()
{
    A[0][0] = 0;
    for (int i = 0; i < N; i++)
    {
        A[i][i] = 1.0;
        for (int j = i + 1; j < N; j++)
        {
            A[i][j] = rand();
        }
    }

    for (int k = 0; k < N; k++)
    {

        for (int i = k + 1; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                A[i][j] += A[k][j];

            }
        }

    }
}


void division_kernel(float* data, int k, int N,
                     const sycl::nd_item<3> &item_ct1)
{
    int tid = item_ct1.get_local_range(2) * item_ct1.get_group(2) +
              item_ct1.get_local_id(2); // 计算线程索引
    int element = data[k * N + k];
    int temp = data[k * N + tid];
    data[k * N + tid] = (float)temp / element;

    return;
}
void eliminate_kernel(float* data, int k, int N,
                      const sycl::nd_item<3> &item_ct1) {
    int tx = item_ct1.get_local_range(2) * item_ct1.get_group(2) +
             item_ct1.get_local_id(2);
    if (tx == 0)  data[k * N + k] = 1.0;//对角线元素设为 1

    int row = k + 1 + item_ct1.get_group(2); // 每个块负责一行
    while (row < N) {
        int tid = item_ct1.get_local_id(2);
        while (k + 1 + tid < N) {
            int col = k + 1 + tid;
            float temp_1 = data[(row * N) + col];
            float temp_2 = data[(row * N) + k];
            float temp_3 = data[k * N + col];
            data[(row * N) + col] = temp_1 - temp_2 * temp_3;
            tid = tid + item_ct1.get_local_range(2);
        }
        /*
        DPCT1065:0: Consider replacing sycl::nd_item::barrier() with
        sycl::nd_item::barrier(sycl::access::fence_space::local_space) for
        better performance if there is no access to global memory.
        */
        item_ct1.barrier(); // 块内同步
        if (item_ct1.get_local_id(2) == 0) {
            data[row * N + k] = 0;
        }
        row += item_ct1.get_group_range(2);
    }
    return;
}
int main() try {
  dpct::device_ext &dev_ct1 = dpct::get_current_device();
  sycl::queue &q_ct1 = dev_ct1.default_queue();
    float* gpudata;
    reset();

    sycl::range<3> dimBlock(
        1, 1,
        BLOCK_SIZE); // dimBlock的三个参数分别表示线程块在 x、y、z 方向上的大小
    sycl::range<3> dimGrid(1, 1, N / BLOCK_SIZE); // dimGrid的三个参数分别表示线程网格在
                                                  // x、y、z 方向上的大小。

    dpct::event_ptr start, stop;
    std::chrono::time_point<std::chrono::steady_clock> start_ct1;
    std::chrono::time_point<std::chrono::steady_clock> stop_ct1;
    float elapsedTime = 0.0;
    start = new sycl::event();
    stop = new sycl::event();
    dpct::err0 ret;
    /*
    DPCT1003:5: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    ret = (gpudata = sycl::malloc_device<float>(N * N, q_ct1), 0);
    /*
    DPCT1003:6: Migrated API does not return error code. (*, 0) is inserted. You
    may need to rewrite this code.
    */
    ret = (q_ct1.memcpy(gpudata, A, N * N).wait(), 0);
    /*
    DPCT1012:3: Detected kernel execution time measurement pattern and generated
    an initial code for time measurements in SYCL. You can change the way time
    is measured depending on your goals.
    */
    start_ct1 = std::chrono::steady_clock::now();
    *start = q_ct1.ext_oneapi_submit_barrier();

    long long head, tail, freq; // timers
    // similar to CLOCKS_PER_SEC
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    // start time
    QueryPerformanceCounter((LARGE_INTEGER*)&head);

    for (int k = 0; k < N; k++) {

        /*
        DPCT1049:1: The work-group size passed to the SYCL kernel may exceed the
        limit. To get the device limit, query info::device::max_work_group_size.
        Adjust the work-group size if needed.
        */
        q_ct1.submit([&](sycl::handler &cgh) {
            auto N_ct2 = N;

            cgh.parallel_for(sycl::nd_range<3>(dimGrid * dimBlock, dimBlock),
                             [=](sycl::nd_item<3> item_ct1) {
                                 division_kernel(gpudata, k, N_ct2, item_ct1);
                             });
        }); // 负责除法任务的核函数

        dev_ct1.queues_wait_and_throw(); // CPU 与 GPU 之间的同步函数
        /*
        DPCT1010:7: SYCL uses exceptions to report errors and does not use the
        error codes. The call was replaced with 0. You need to rewrite this
        code.
        */
        ret = 0;

        /*
        DPCT1049:2: The work-group size passed to the SYCL kernel may exceed the
        limit. To get the device limit, query info::device::max_work_group_size.
        Adjust the work-group size if needed.
        */
        q_ct1.submit([&](sycl::handler &cgh) {
            auto N_ct2 = N;

            cgh.parallel_for(sycl::nd_range<3>(dimGrid * dimBlock, dimBlock),
                             [=](sycl::nd_item<3> item_ct1) {
                                 eliminate_kernel(gpudata, k, N_ct2, item_ct1);
                             });
        }); // 负责消去任务的核函数

        dev_ct1.queues_wait_and_throw();
        /*
        DPCT1010:9: SYCL uses exceptions to report errors and does not use the
        error codes. The call was replaced with 0. You need to rewrite this
        code.
        */
        ret = 0;
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "GPU_Time:" << ((tail - head) * 1000.0) / freq << "ms" << endl;

    /*
    DPCT1012:4: Detected kernel execution time measurement pattern and generated
    an initial code for time measurements in SYCL. You can change the way time
    is measured depending on your goals.
    */
    dpct::get_current_device().queues_wait_and_throw();
    stop_ct1 = std::chrono::steady_clock::now();
    *stop = q_ct1.ext_oneapi_submit_barrier();
    /*
    DPCT1003:11: Migrated API does not return error code. (*, 0) is inserted.
    You may need to rewrite this code.
    */
    ret = (q_ct1.memcpy(A, gpudata, N * N).wait(), 0);
    elapsedTime =
        std::chrono::duration<float, std::milli>(stop_ct1 - start_ct1).count();

    dpct::destroy_event(start);
    dpct::destroy_event(stop);

    return 0;
}
catch (sycl::exception const &exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__
            << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}