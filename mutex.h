#ifndef MUTEX_H
#define MUTEX_H
template<typename T> 
class Mutex {
	public:
		Mutex(T data);

		T* lock();

		void unlock();

	private: 
		T guarded_data;

		// 0 = unlocked, 1 = locked
		int locked;

		bool try_lock() ;


};

void test_mutex();

#endif
