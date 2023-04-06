#include<iostream>
#include<thread>
#include<mutex>

using namespace std;

volatile bool done = false;
volatile int* b;

int g_error = 0;

void worker() {
	for (int i = 0; i < 100000000; ++i) {
		*b = -(1 + *b);
	}
	done = true;
}
void watcher() {
	while (done == false) {
		int v = *b;
		if ((v != 0) && (v != -1)) {
			printf("%08X, ", v);
			g_error++;
		}
	}
}

int main() {
	//b = new volatile int{ 0 };

	int a[32];
	long long addr = reinterpret_cast<long long>(&a[31]);
	addr = (addr / 64) * 64;
	addr = addr - 1;
	b = reinterpret_cast<int*>(addr);
	*b = 0;


	thread t1{ worker };
	
	thread t2{ watcher};
	t1.join();
	t2.join();

	cout << "Error : " << g_error << endl;

}