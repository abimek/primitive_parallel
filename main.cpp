#include "mutex.h"
#include "futex.h"
#include "rwlock.h"
#include <cwchar>

int main() {
	test_rw_lock();
}
