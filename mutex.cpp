#define CPPDB_SOURCE
#include "mutex.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)

#	include <windows.h>
	
#	ifdef impl_
#		undef impl_
#	endif
	
#	define impl_ ((CRITICAL_SECTION *)mutex_impl_)

	namespace cppdb {
		mutex::mutex() : mutex_impl_(0)
		{
			mutex_impl_ = new CRITICAL_SECTION();
			InitializeCriticalSection(impl_);
		}
		mutex::~mutex()
		{
			DeleteCriticalSection(impl_);
			delete impl_;
		}
		void mutex::lock()
		{
			EnterCriticalSection(impl_);
		}
		void mutex::unlock()
		{
			LeaveCriticalSection(impl_);
		}

	}


#else // POSIX

#	include <pthread.h>

#	ifdef impl_
#		undef impl_
#	endif

#	define impl_ ((pthread_mutex_t *)mutex_impl_)

	namespace cppdb {
		mutex::mutex() : mutex_impl_(0)
		{
			mutex_impl_ = new pthread_mutex_t();
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

#endif
