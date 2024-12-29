#include <thread>
#include <iostream>
#include "futex.h"
#include <vector>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#define UNLOCKED 0
#define LOCKED 1

template<typename T> 
class FutexMutex {
	public:
		FutexMutex(T data) {
			guarded_data = data;
			locked = UNLOCKED;
		}


		int futex_wait() {
			return syscall(SYS_futex, &locked, FUTEX_WAIT, LOCKED, NULL, NULL, 0);
		}

		int futex_wake() {
			return syscall(SYS_futex, &locked, FUTEX_WAKE, 1, NULL, NULL, 0);
		}

		T* lock() {
			int zero = 0;
			while(!__atomic_compare_exchange_n(&locked, &zero, LOCKED, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)){
				zero = UNLOCKED;
				futex_wait();
			}
			return &guarded_data;
		}

		void unlock() {
			// add some guard rails to stop this from automatically just
			// unlocking a locked, prob resutnr some object in lock that
			// better handles this
			int one = LOCKED;
			__atomic_compare_exchange_n(&locked, &one, UNLOCKED, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
			futex_wake();
		}

	private: 
		T guarded_data;

		// 0 = unlocked, 1 = locked
		int locked;

		bool try_lock() {
			int zero = 0;
			return __atomic_compare_exchange_n(&locked, &zero, LOCKED, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
		}


};

#pragma optimize("", off)
void test_futex_mutex() {
	FutexMutex<int> x(0);

	std::vector<std::thread> threads;

	for(int i = 0; i < 3; i++){
		threads.emplace_back([&] {
			for(int i = 0; i < 30000; i++){
				int* data = x.lock();
				*data = *data + 1;
				x.unlock();
			}
		});
	}
	for (auto& t: threads){
		t.join();
	}

	for(int i = 0; i < 30000*30000; i++){
	}

	std::cout << "FutexMutex Test Synchronized: " << *x.lock() << std::endl;

	std::vector<std::thread> threads2;
	int x2 = 0;
	for(int i = 0; i < 3; i++){
		threads2.emplace_back([&] {
			for(int i = 0; i < 30000; i++){
				++x2;
			}
		});
	}

	for (auto& t: threads2){
		t.join();
	}

	for(int i = 0; i < 30000*30000; i++){
	}
	std::cout << "FutexMutex Test Unsynchronized: " << x2 << std::endl;
}
