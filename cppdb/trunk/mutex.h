#ifndef CPPDB_MUTEX_H
#define CPPDB_MUTEX_H

#include "defs.h"

namespace cppdb {

	class CPPDB_API mutex {
		mutex(mutex const &);
		void operator=(mutex const  &);
	public:
		class guard;
		mutex();
		~mutex();
		void lock();
		void unlock();
	private:
		void *mutex_impl_;
	};

	class mutex::guard {
		guard(guard const &);
		void operator=(guard const &);
	public:
		guard(mutex &m) : m_(&m)
		{
			m_->lock();
		}
		~guard()
		{
			m_->unlock();
		}
	private:
		mutex *m_;
	};
}
#endif
