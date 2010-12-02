///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPDB_SOURCE
#include <cppdb/mutex.h>

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
