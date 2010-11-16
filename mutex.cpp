#define CPPDB_SOURCE
#include "mutex.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define USE_WINAPI
#endif

#ifdef USE_WINAPI
#error implement me
#else
#include <pthread.h>

#ifdef impl_
#undef impl_
#endif

#define impl_ (*(pthread_mutex_t **)&mutex_impl_)

#endif

namespace cppdb {
	mutex::mutex() : mutex_impl_(0)
	{
		impl_ = new pthread_mutex_t();
		pthread_mutex_init(impl_,0);
	}
	mutex::~mutex()
	{
		pthread_mutex_destroy(impl_);
		delete impl_;
	}
	void mutex::lock()
	{
		pthread_mutex_lock(impl_);
	}
	void mutex::unlock()
	{
		pthread_mutex_unlock(impl_);
	}

}
