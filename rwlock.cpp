#include "mutex.h"
#include <vector>
#include <thread>
#include <iostream>

#define UNLOCKED 0
#define LOCKED_WRITE 1

template<typename T>
class RWLock {

	T data;
	bool reading = true;

	// value stored in mutex is the number of readers
	Mutex<int> mutex;


public:


	RWLock(T content) : mutex(0) {
		data = content;
	}
	// Compare swap if the value is UNLOCKED, set the value too 2 if it is.
	// If locked, compare and swap if the value is LOCKED_WRITE, if it is,
	// rerun the atomic operations.
	// Difficult: Atomic comapre and increment. Difficulty = we're splitting
	// up these operatoins.
	// Should only be read.
	T* lock_read() {
		// some kind of infite loop is required, this is likely the best
		// implementation.
		while(true){
			int* count = mutex.lock();
			if(reading == true || *count == 0){
				reading = true;
				(*count)++;
				if(*count > 1){
					std::cout << "Read Lock Count: " << *count << std::endl;
				}
				mutex.unlock();
				return &data;
			}
			mutex.unlock();
		}
	}

	// Compare and swap if the value is UNLOCKED, set value to LOCKED_WRITE
	// if it is.
	// Otherwise rerun.
	T* lock_write() {
		while(true){
			int* count = mutex.lock();
			if(*count == 0){
				reading = false;
				(*count)++;
				std::cout << "Write Lock Count: " << *count << std::endl;
				mutex.unlock();
				return &data;
			}
			mutex.unlock();
		}
	}

	void unlock() {
		int* count = mutex.lock();
		(*count)--;
		mutex.unlock();
	}
};

void test_rw_lock() {

	RWLock<int> lock(5);

	std::vector<std::thread> threads;
	for(int i = 0; i < 20; i++){
		threads.emplace_back([&] {
			for(int g = 0; g < 1000; g++){
				int* data = lock.lock_read();
				lock.unlock();
			}
		});
		threads.emplace_back([&] {
			for(int g = 0; g < 100; g++){
				int* data = lock.lock_write();
				(*data)++;
				lock.unlock();
			}
		});
	}

	for (auto& t: threads){
		t.join();
	}

	for(int i = 0; i < 30000*30000; i++){
	}

	std::cout << "RWLock Test Synchronized: " << *lock.lock_read() << std::endl;;
}
