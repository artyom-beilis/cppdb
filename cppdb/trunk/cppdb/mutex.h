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
#ifndef CPPDB_MUTEX_H
#define CPPDB_MUTEX_H

#include <cppdb/defs.h>

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
