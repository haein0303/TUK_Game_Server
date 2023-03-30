#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
const auto MAX_THREADS = 64;
using namespace std;
using namespace std::chrono;

struct NUM {
	alignas(64) volatile int sum;
};
NUM num[MAX_THREADS];

volatile int sum[MAX_THREADS];
mutex g_m;
void thread_func(int num_threads,const int thread_i) {
	
	for (auto i = 0; i < 50000000 / num_threads; ++i) {

		num[thread_i].sum += 2;

	}
}
int main() {
	vector<thread> threads;
	for (auto i = 1; i <= MAX_THREADS; i *= 2) {
		for (auto& j : num) {
			j.sum = 0;
		}
		threads.clear();
		auto start = high_resolution_clock::now();
		for (auto j = 0; j < i; ++j) threads.push_back(thread{ thread_func, i,j });
		for (auto& tmp : threads) tmp.join();

		int t_sum = 0;
		for (auto& j : num) {
			t_sum += j.sum;
		}

		auto duration = high_resolution_clock::now() - start;
		cout << i << " Threads, Sum = " << t_sum;
		cout << " Duration = " << duration_cast<milliseconds>(duration).count() << " milliseconds\n";
	}
}
