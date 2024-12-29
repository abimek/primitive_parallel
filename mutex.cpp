#include <thread>
#include <iostream>
#include "mutex.h"
#include <vector>

template<typename T>
Mutex<T>::Mutex(T data) {
	guarded_data = data;
	locked = 0;
}

template<typename T>
T* Mutex<T>::lock() {
	while (!try_lock()) {
	}
	return &guarded_data;
}

template<typename T>
void Mutex<T>::unlock() {
	// add some guard rails to stop this from automatically just
	// unlocking a locked, prob resutnr some object in lock that
	// better handles this
	int one = 1;
	__atomic_compare_exchange_n(&locked, &one, 0, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}

template<typename T>
bool Mutex<T>::try_lock() {
	int zero = 0;
	return __atomic_compare_exchange_n(&locked, &zero, 1, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}


#pragma optimize("", off)
void test_mutex() {
	Mutex<int> x(0);

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

	std::cout << "Mutex Test Synchronized: " << *x.lock() << std::endl;;


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
	std::cout << "Mutex Test Unsynchronized: " << x2 << std::endl;
}
