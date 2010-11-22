//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPDB_SOURCE
#include "atomic_counter.h"
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN__)

#   include <windows.h>
#   define cppdb_atomic_set(p,v) ((p)->l=v)
    long static cppdb_atomic_add_and_fetch_impl(volatile long *v,long d)
    {
	long old,prev;
	do{
		old = *v;
		prev = InterlockedCompareExchange(v,old+d,old);
	}
	while(prev != old);
	return old+d;
    }
#   define cppdb_atomic_add_and_fetch(p,d) cppdb_atomic_add_and_fetch_impl(&(p)->l,d)


#elif defined(CPPDB_HAVE_FREEBSD_ATOMIC)

#   include <sys/types.h>
#   include <machine/atomic.h>
#   define cppdb_atomic_set(p,v) ((p)->ui=v)
#   define cppdb_atomic_add_and_fetch(p,d) (atomic_fetchadd_int(&(p)->ui,d) + d)


#elif defined(CPPDB_HAVE_SOLARIS_ATOMIC)

#   include <atomic.h>
#   define cppdb_atomic_set(p,v) ((p)->ui=v)
#   define cppdb_atomic_add_and_fetch(p,d) (atomic_add_int_nv(&(p)->ui,d))


#elif defined(CPPDB_HAVE_MAC_OS_X_ATOMIC)

#   include <libkern/OSAtomic.h>
#   define cppdb_atomic_set(p,v) ((p)->i=v)
#   define cppdb_atomic_add_and_fetch(p,d) (OSAtomicAdd32(d,&(p)->i))

#elif defined CPPDB_HAS_GCC_SYNC

#   define cppdb_atomic_set(p,v) ((p)->l=v)
#   define cppdb_atomic_add_and_fetch(p,d) ( __sync_add_and_fetch(&(p)->i,d) )


#   elif defined(CPPDB_HAVE_GCC_BITS_EXCHANGE_AND_ADD)

#   include <bits/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define cppdb_atomic_set(p,v) ((p)->i=v)
#   define cppdb_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#elif defined(CPPDB_HAVE_GCC_EXT_EXCHANGE_AND_ADD)

#   include <ext/atomicity.h>
    using __gnu_cxx::__exchange_and_add;
#   define cppdb_atomic_set(p,v) ((p)->i=v)
#   define cppdb_atomic_add_and_fetch(p,d) ( __exchange_and_add(&(p)->i,d)+d )


#else  // Failing back to pthreads
# include <pthread.h>
# define CPPDB_PTHREAD_ATOMIC 
#endif


namespace cppdb {

#if !defined(CPPDB_PTHREAD_ATOMIC)
	atomic_counter::atomic_counter(long value)  
	{
		memset(&value_,0,sizeof(value_));
		mutex_ = 0;
		cppdb_atomic_set(&value_,value);
	}

	atomic_counter::~atomic_counter()
	{
	}
	
	long atomic_counter::inc()
	{
		return cppdb_atomic_add_and_fetch( &value_, 1 );
	}

	long atomic_counter::dec()
	{
		return cppdb_atomic_add_and_fetch( &value_, -1 );
	}

	long atomic_counter::get() const
	{
		return cppdb_atomic_add_and_fetch( &value_, 0 );
	}

#else

	#define MUTEX (reinterpret_cast<pthread_mutex_t *>(mutex_))
	atomic_counter::atomic_counter(long value) 
	{
		mutex_ = new pthread_mutex_t; 
		pthread_mutex_init(MUTEX,0);
		value_.l=value;
	}

	atomic_counter::~atomic_counter()
	{
		pthread_mutex_destroy(MUTEX);
		delete MUTEX;
		mutex_ = 0;
	}

	long atomic_counter::inc()
	{
		pthread_mutex_lock(MUTEX);
		long result= ++value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}

	long atomic_counter::dec()
	{
		pthread_mutex_lock(MUTEX);
		long result= --value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}

	long atomic_counter::get() const
	{
		pthread_mutex_lock(MUTEX);
		long result= value_.l;
		pthread_mutex_unlock(MUTEX);
		return result;
	}


#endif


} // cppdb



